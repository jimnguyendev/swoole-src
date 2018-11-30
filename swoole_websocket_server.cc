/*
  +----------------------------------------------------------------------+
  | Swoole                                                               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

#include "php_swoole.h"
#include "swoole_http.h"
#include "swoole_coroutine.h"

extern "C"
{
#include <ext/standard/url.h>
#include <ext/standard/sha1.h>
#include <ext/standard/php_var.h>
#include <ext/standard/php_string.h>
#include <ext/date/php_date.h>
#include <main/php_variables.h>
}

#include "websocket.h"
#include "connection.h"
#include "base64.h"
#include "thirdparty/swoole_http_parser.h"

static zend_class_entry swoole_websocket_server_ce;
zend_class_entry *swoole_websocket_server_ce_ptr;
static zend_object_handlers swoole_websocket_server_handlers;

static zend_class_entry swoole_websocket_frame_ce;
zend_class_entry *swoole_websocket_frame_ce_ptr;
static zend_object_handlers swoole_websocket_frame_handlers;

static zend_class_entry swoole_websocket_closeframe_ce;
static zend_class_entry *swoole_websocket_closeframe_ce_ptr;
static zend_object_handlers swoole_websocket_closeframe_handlers;

static int websocket_handshake(swListenPort *, http_context *);

static PHP_METHOD(swoole_websocket_server, push);
static PHP_METHOD(swoole_websocket_server, exist);
static PHP_METHOD(swoole_websocket_server, isEstablished);
static PHP_METHOD(swoole_websocket_server, pack);
static PHP_METHOD(swoole_websocket_server, unpack);
static PHP_METHOD(swoole_websocket_server, disconnect);

static PHP_METHOD(swoole_websocket_frame, __toString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_push, 0, 0, 2)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, opcode)
    ZEND_ARG_INFO(0, finish)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_disconnect, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_INFO(0, code)
    ZEND_ARG_INFO(0, reason)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_pack, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, opcode)
    ZEND_ARG_INFO(0, finish)
    ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_unpack, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_exist, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_server_isEstablished, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_websocket_frame_void, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry swoole_websocket_server_methods[] =
{
    PHP_ME(swoole_websocket_server, push,           arginfo_swoole_websocket_server_push, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_websocket_server, disconnect,     arginfo_swoole_websocket_server_disconnect, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_websocket_server, exist,          arginfo_swoole_websocket_server_exist, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_websocket_server, isEstablished,  arginfo_swoole_websocket_server_isEstablished, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_websocket_server, pack,           arginfo_swoole_websocket_server_pack, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(swoole_websocket_server, unpack,         arginfo_swoole_websocket_server_unpack, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

const zend_function_entry swoole_websocket_frame_methods[] =
{
    PHP_ME(swoole_websocket_frame, __toString,      arginfo_swoole_websocket_frame_void, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_websocket_server, pack,           arginfo_swoole_websocket_server_pack, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(swoole_websocket_server, unpack,         arginfo_swoole_websocket_server_unpack, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

static void php_swoole_websocket_construct_frame(zval *zframe, zend_long opcode, char *payload, size_t payload_length, zend_bool finish)
{
    if (opcode == WEBSOCKET_OPCODE_CLOSE)
    {
        object_init_ex(zframe, swoole_websocket_closeframe_ce_ptr);
        if (payload_length >= SW_WEBSOCKET_CLOSE_CODE_LEN)
        {
            // WebSocket Close code
            zend_update_property_long(
                swoole_websocket_closeframe_ce_ptr, zframe, ZEND_STRL("code"),
                (payload[0] << 8) ^ (payload[1] & 0xFF)
            );
            if (payload_length > SW_WEBSOCKET_CLOSE_CODE_LEN)
            {
                // WebSocket Close reason message
                zend_update_property_stringl(
                    swoole_websocket_closeframe_ce_ptr, zframe, ZEND_STRL("reason"),
                    payload + SW_WEBSOCKET_CLOSE_CODE_LEN, payload_length - SW_WEBSOCKET_CLOSE_CODE_LEN
                );
            }
        }
    }
    else
    {
        object_init_ex(zframe, swoole_websocket_frame_ce_ptr);
        zend_update_property_stringl(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("data"), payload, payload_length);
    }
    zend_update_property_bool(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("finish"), finish);
    zend_update_property_long(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("opcode"), opcode);
}

void php_swoole_websocket_frame_unpack(swString *data, zval *zframe)
{
    swWebSocket_frame frame;

    if (data->length < sizeof(frame.header))
    {
        ZVAL_BOOL(zframe, 0);
        return;
    }

    swWebSocket_decode(&frame, data);
    php_swoole_websocket_construct_frame(zframe, frame.header.OPCODE, frame.payload, frame.payload_length, frame.header.FIN);
}

int php_swoole_websocket_frame_pack(swString *buffer, zval *zdata, zend_bool opcode, zend_bool fin, zend_bool mask)
{
    char *data = NULL;
    size_t length = 0;
    zend_long code = WEBSOCKET_CLOSE_NORMAL;
    if (Z_TYPE_P(zdata) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zdata), swoole_websocket_frame_ce_ptr))
    {
        zval *zframe = zdata;
        zval *ztmp = NULL;
        zdata = NULL;
        if ((ztmp = sw_zend_read_property(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("opcode"), 1)))
        {
            convert_to_long(ztmp);
            opcode = Z_LVAL_P(ztmp);
        }
        if (opcode == WEBSOCKET_OPCODE_CLOSE)
        {
            if ((ztmp = sw_zend_read_property_not_null(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("code"), 1)))
            {
                convert_to_long(ztmp);
                code = Z_LVAL_P(ztmp);
            }
            if ((ztmp = sw_zend_read_property_not_null(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("reason"), 1)))
            {
                zdata = ztmp;
            }
        }
        if (!zdata && (ztmp = sw_zend_read_property(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("data"), 1)))
        {
            zdata = ztmp;
        }
        if ((ztmp = sw_zend_read_property(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("finish"), 1)))
        {
            convert_to_boolean(ztmp);
            fin = Z_BVAL_P(ztmp);
        }
        if ((ztmp = sw_zend_read_property(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("mask"), 1)))
        {
            convert_to_boolean(ztmp);
            mask = Z_BVAL_P(ztmp);
        }
    }
    if (unlikely(opcode > SW_WEBSOCKET_OPCODE_MAX))
    {
        swoole_php_fatal_error(E_WARNING, "the maximum value of opcode is %d.", SW_WEBSOCKET_OPCODE_MAX);
        return SW_ERR;
    }
    if (zdata && !ZVAL_IS_NULL(zdata))
    {
        convert_to_string(zdata);
        data = Z_STRVAL_P(zdata);
        length = Z_STRLEN_P(zdata);
    }
    switch(opcode)
    {
    case WEBSOCKET_OPCODE_CLOSE:
        return swWebSocket_pack_close_frame(buffer, code, data, length, mask);
    default:
        swWebSocket_encode(buffer, data, length, opcode, fin, mask);
    }
    return SW_OK;
}


void swoole_websocket_onOpen(http_context *ctx)
{
    int fd = ctx->fd;

    swConnection *conn = swWorker_get_connection(SwooleG.serv, fd);
    if (!conn)
    {
        swoole_error_log(SW_LOG_NOTICE, SW_ERROR_SESSION_CLOSED, "session[%d] is closed.", fd);
        return;
    }

    zend_fcall_info_cache *fci_cache = php_swoole_server_get_fci_cache(SwooleG.serv, conn->from_fd, SW_SERVER_CB_onOpen);
    if (fci_cache)
    {
        swServer *serv = SwooleG.serv;
        zval *zserv = (zval *) serv->ptr2;
        zval *zrequest_object = ctx->request.zobject;
        zval _retval, *retval = &_retval;

        zval args[2];
        args[0] = *zserv;
        args[1] = *zrequest_object;

        if (SwooleG.enable_coroutine)
        {
            long ret = sw_coro_create(fci_cache, 2, args, retval);
            if (ret < 0)
            {
                SwooleG.serv->factory.end(&SwooleG.serv->factory, fd);
                return;
            }
        }
        else
        {
            if (sw_call_user_function_fast_ex(NULL, fci_cache, retval, 2, args) == FAILURE)
            {
                swoole_php_error(E_WARNING, "onOpen handler error");
            }
        }

        if (UNEXPECTED(EG(exception)))
        {
            zend_exception_error(EG(exception), E_ERROR);
        }
        if (retval)
        {
            zval_ptr_dtor(retval);
        }
    }
}

/**
 * default onRequest callback
 */
void swoole_websocket_onRequest(http_context *ctx)
{
    const char *bad_request =
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Cache-Control: must-revalidate,no-cache,no-store\r\n"
            "Content-Length: 83\r\n"
            "Server: " SW_HTTP_SERVER_SOFTWARE "\r\n\r\n"
            "<html><body><h2>HTTP 400 Bad Request</h2><hr><i>Powered by Swoole</i></body></html>";

    swServer_tcp_send(SwooleG.serv, ctx->fd, (char *) bad_request, strlen(bad_request));
    ctx->end = 1;
    swServer_tcp_close(SwooleG.serv, ctx->fd, 0);
    swoole_http_context_free(ctx);
}

void php_swoole_sha1(const char *str, int _len, unsigned char *digest)
{
    PHP_SHA1_CTX context;
    PHP_SHA1Init(&context);
    PHP_SHA1Update(&context, (unsigned char *) str, _len);
    PHP_SHA1Final(digest, &context);
}

static int websocket_handshake(swListenPort *port, http_context *ctx)
{
    zval *header = ctx->request.zheader;
    HashTable *ht = Z_ARRVAL_P(header);
    zval *pData;

    if (!(pData = zend_hash_str_find(ht, ZEND_STRL("sec-websocket-key"))))
    {
        php_error_docref(NULL, E_WARNING, "header no sec-websocket-key");
        return SW_ERR;
    }
    convert_to_string(pData);

    swString_clear(swoole_http_buffer);
    swString_append_ptr(swoole_http_buffer, ZEND_STRL("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"));

    int n;
    char sec_websocket_accept[128];
    memcpy(sec_websocket_accept, Z_STRVAL_P(pData), Z_STRLEN_P(pData));
    memcpy(sec_websocket_accept + Z_STRLEN_P(pData), SW_WEBSOCKET_GUID, sizeof(SW_WEBSOCKET_GUID) - 1);

    char sha1_str[20];
    bzero(sha1_str, sizeof(sha1_str));
    php_swoole_sha1(sec_websocket_accept, Z_STRLEN_P(pData) + sizeof(SW_WEBSOCKET_GUID) - 1, (unsigned char *) sha1_str);

    char encoded_str[50];
    bzero(encoded_str, sizeof(encoded_str));
    n = swBase64_encode((unsigned char *) sha1_str, sizeof(sha1_str), encoded_str);

    char _buf[128];
    n = snprintf(_buf, sizeof(_buf), "Sec-WebSocket-Accept: %*s\r\n", n, encoded_str);

    swString_append_ptr(swoole_http_buffer, _buf, n);
    swString_append_ptr(swoole_http_buffer, ZEND_STRL("Sec-WebSocket-Version: " SW_WEBSOCKET_VERSION "\r\n"));
    if (port->websocket_subprotocol)
    {
        swString_append_ptr(swoole_http_buffer, ZEND_STRL("Sec-WebSocket-Protocol: "));
        swString_append_ptr(swoole_http_buffer, port->websocket_subprotocol, port->websocket_subprotocol_length);
        swString_append_ptr(swoole_http_buffer, ZEND_STRL("\r\n"));
    }
    swString_append_ptr(swoole_http_buffer, ZEND_STRL("Server: " SW_WEBSOCKET_SERVER_SOFTWARE "\r\n\r\n"));

    swTrace("websocket header len:%ld\n%s \n", swoole_http_buffer->length, swoole_http_buffer->str);

    swConnection *conn = swWorker_get_connection(SwooleG.serv, ctx->fd);
    if (!conn)
    {
        swoole_error_log(SW_LOG_NOTICE, SW_ERROR_SESSION_CLOSED, "session[%d] is closed.", ctx->fd);
        return SW_ERR;
    }
    conn->websocket_status = WEBSOCKET_STATUS_ACTIVE;
    return swServer_tcp_send(SwooleG.serv, ctx->fd, swoole_http_buffer->str, swoole_http_buffer->length);
}

int swoole_websocket_onMessage(swEventData *req)
{
    int fd = req->info.fd;
    zend_bool finish = 0;
    zend_long opcode = 0;

    zval *zdata;
    SW_MAKE_STD_ZVAL(zdata);

    char frame_header[2];
    php_swoole_get_recv_data(zdata, req, frame_header, SW_WEBSOCKET_HEADER_LEN);

    // frame info has already decoded in swWebSocket_dispatch_frame
    finish = frame_header[0] ? 1 : 0;
    opcode = frame_header[1];

    if (opcode == WEBSOCKET_OPCODE_CLOSE)
    {
        if (!SwooleG.serv->listen_list->open_websocket_close_frame)
        {
            zval_ptr_dtor(zdata);
            return SW_OK;
        }
    }

    zval *zframe;
    SW_MAKE_STD_ZVAL(zframe);
    php_swoole_websocket_construct_frame(zframe, opcode, Z_STRVAL_P(zdata), Z_STRLEN_P(zdata), finish);
    zend_update_property_long(swoole_websocket_frame_ce_ptr, zframe, ZEND_STRL("fd"), fd);

    swServer *serv = SwooleG.serv;
    zval *zserv = (zval *) serv->ptr2;
    zval _retval, *retval = &_retval;

    zend_fcall_info_cache *fci_cache = php_swoole_server_get_fci_cache(serv, req->info.from_fd, SW_SERVER_CB_onMessage);
    zval args[2];
    args[0] = *zserv;
    args[1] = *zframe;

    if (SwooleG.enable_coroutine)
    {
        long ret = sw_coro_create(fci_cache, 2, args, retval);
        if (ret < 0)
        {
            zval_ptr_dtor(zdata);
            zval_ptr_dtor(zframe);
            SwooleG.serv->factory.end(&SwooleG.serv->factory, fd);
            return SW_OK;
        }
    }
    else
    {
        if (sw_call_user_function_fast_ex(NULL, fci_cache, retval, 2, args) == FAILURE)
        {
            swoole_php_error(E_WARNING, "onMessage handler error");
        }
    }

    if (UNEXPECTED(EG(exception)))
    {
        zend_exception_error(EG(exception), E_ERROR);
    }
    if (retval)
    {
        zval_ptr_dtor(retval);
    }
    zval_ptr_dtor(zdata);
    zval_ptr_dtor(zframe);
    return SW_OK;
}

int swoole_websocket_onHandshake(swListenPort *port, http_context *ctx)
{
    int fd = ctx->fd;
    int ret = websocket_handshake(port, ctx);
    if (ret == SW_ERR)
    {
        swServer_tcp_close(SwooleG.serv, fd, 1);
    }
    else
    {
        swoole_websocket_onOpen(ctx);
    }

    //free client data
    if (!ctx->end)
    {
        swoole_http_context_free(ctx);
    }

    return SW_OK;
}

void swoole_websocket_init(int module_number)
{
    SWOOLE_INIT_CLASS_ENTRY_EX(swoole_websocket_server, "Swoole\\WebSocket\\Server", "swoole_websocket_server", NULL, swoole_websocket_server_methods, swoole_http_server);
    SWOOLE_SET_CLASS_SERIALIZABLE(swoole_websocket_server, zend_class_serialize_deny, zend_class_unserialize_deny);
    SWOOLE_SET_CLASS_CLONEABLE(swoole_websocket_server, zend_class_clone_deny);
    SWOOLE_SET_CLASS_UNSET_PROPERTY_HANDLER(swoole_websocket_server, zend_class_unset_property_deny);

    SWOOLE_INIT_CLASS_ENTRY(swoole_websocket_frame, "Swoole\\WebSocket\\Frame", "swoole_websocket_frame", NULL, swoole_websocket_frame_methods);
    SWOOLE_INIT_CLASS_ENTRY_EX(swoole_websocket_closeframe, "Swoole\\WebSocket\\CloseFrame", "swoole_websocket_closeframe", NULL, NULL, swoole_websocket_frame);

    zend_declare_property_long(swoole_websocket_frame_ce_ptr, ZEND_STRL("fd"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(swoole_websocket_frame_ce_ptr, ZEND_STRL("data"), "", ZEND_ACC_PUBLIC);
    zend_declare_property_long(swoole_websocket_frame_ce_ptr, ZEND_STRL("opcode"), WEBSOCKET_OPCODE_TEXT, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(swoole_websocket_frame_ce_ptr, ZEND_STRL("finish"), 1, ZEND_ACC_PUBLIC);

    zend_declare_property_long(swoole_websocket_closeframe_ce_ptr, ZEND_STRL("opcode"), WEBSOCKET_OPCODE_CLOSE, ZEND_ACC_PUBLIC);
    zend_declare_property_long(swoole_websocket_closeframe_ce_ptr, ZEND_STRL("code"), WEBSOCKET_CLOSE_NORMAL, ZEND_ACC_PUBLIC);
    zend_declare_property_string(swoole_websocket_closeframe_ce_ptr, ZEND_STRL("reason"), "", ZEND_ACC_PUBLIC);

    // status
    SWOOLE_RAW_DEFINE(WEBSOCKET_STATUS_CONNECTION);
    SWOOLE_RAW_DEFINE(WEBSOCKET_STATUS_HANDSHAKE);
    REGISTER_LONG_CONSTANT("WEBSOCKET_STATUS_FRAME", WEBSOCKET_STATUS_ACTIVE, CONST_CS | CONST_PERSISTENT);
    SWOOLE_RAW_DEFINE(WEBSOCKET_STATUS_ACTIVE);
    SWOOLE_RAW_DEFINE(WEBSOCKET_STATUS_CLOSING);
    // all opcodes
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_CONTINUATION);
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_TEXT);
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_BINARY);
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_CLOSE);
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_PING);
    SWOOLE_RAW_DEFINE(WEBSOCKET_OPCODE_PONG);
    // close error
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_NORMAL);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_GOING_AWAY);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_PROTOCOL_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_DATA_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_STATUS_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_ABNORMAL);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_MESSAGE_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_POLICY_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_MESSAGE_TOO_BIG);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_EXTENSION_MISSING);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_SERVER_ERROR);
    SWOOLE_RAW_DEFINE(WEBSOCKET_CLOSE_TLS);
}

static sw_inline int swoole_websocket_server_push(int fd, swString *buffer)
{
    if (unlikely(fd <= 0))
    {
        swoole_php_fatal_error(E_WARNING, "fd[%d] is invalid.", fd);
        return SW_ERR;
    }

    swConnection *conn = swWorker_get_connection(SwooleG.serv, fd);
    if (!conn || conn->websocket_status < WEBSOCKET_STATUS_HANDSHAKE)
    {
        SwooleG.error = SW_ERROR_WEBSOCKET_UNCONNECTED;
        swoole_php_fatal_error(E_WARNING, "the connected client of connection[%d] is not a websocket client or closed.", (int ) fd);
        return SW_ERR;
    }

    int ret = swServer_tcp_send(SwooleG.serv, fd, buffer->str, buffer->length);
    swServer *serv = SwooleG.serv;
    if (ret < 0 && SwooleG.error == SW_ERROR_OUTPUT_BUFFER_OVERFLOW && serv->send_yield)
    {
        zval _return_value;
        zval *return_value = &_return_value;
        zval _yield_data;
        ZVAL_STRINGL(&_yield_data, buffer->str, buffer->length);
        ZVAL_FALSE(return_value);
        php_swoole_server_send_yield(serv, fd, &_yield_data, return_value);
        ret = Z_BVAL_P(return_value) ? SW_OK : SW_ERR;
    }
    return ret;
}

static sw_inline int swoole_websocket_server_close(int fd, swString *buffer, uint8_t real_close)
{
    int ret = swoole_websocket_server_push(fd, buffer);
    if (ret < 0 || !real_close)
    {
        return ret;
    }
    swConnection *conn = swWorker_get_connection(SwooleG.serv, fd);
    if (conn)
    {
        // Change status immediately to avoid double close
        conn->websocket_status = WEBSOCKET_STATUS_CLOSING;
        // Server close connection immediately
        return SwooleG.serv->close(SwooleG.serv, fd, SW_FALSE);
    }
    else
    {
        return SW_ERR;
    }
}

static PHP_METHOD(swoole_websocket_server, disconnect)
{
    zend_long fd = 0;
    zend_long code = WEBSOCKET_CLOSE_NORMAL;
    char *data = NULL;
    size_t length = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|ls", &fd, &code, &data, &length) == FAILURE)
    {
        RETURN_FALSE;
    }
    swString_clear(swoole_http_buffer);
    if (swWebSocket_pack_close_frame(swoole_http_buffer, code, data, length, 0) < 0)
    {
        RETURN_FALSE;
    }
    SW_CHECK_RETURN(swoole_websocket_server_close(fd, swoole_http_buffer, 1));
}

static PHP_METHOD(swoole_websocket_server, push)
{
    zend_long fd = 0;
    zval *zdata = NULL;
    zend_long opcode = WEBSOCKET_OPCODE_TEXT;
    zend_bool fin = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz|lb", &fd, &zdata, &opcode, &fin) == FAILURE)
    {
        RETURN_FALSE;
    }

    swString_clear(swoole_http_buffer);
    if (php_swoole_websocket_frame_pack(swoole_http_buffer, zdata, opcode, fin, 0) < 0)
    {
        RETURN_FALSE;
    }
    switch (opcode)
    {
    case WEBSOCKET_OPCODE_CLOSE:
        SW_CHECK_RETURN(swoole_websocket_server_close(fd, swoole_http_buffer, fin));
        break;
    default:
        SW_CHECK_RETURN(swoole_websocket_server_push(fd, swoole_http_buffer));
    }
}

static PHP_METHOD(swoole_websocket_server, pack)
{
    swString *buffer = SwooleTG.buffer_stack;
    zval *zdata = NULL;
    zend_long opcode = WEBSOCKET_OPCODE_TEXT;
    zend_bool fin = 1;
    zend_bool mask = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|lbb", &zdata, &opcode, &fin, &mask) == FAILURE)
    {
        RETURN_FALSE;
    }
    swString_clear(buffer);
    if (php_swoole_websocket_frame_pack(buffer, zdata, opcode, fin, mask) < 0)
    {
        RETURN_EMPTY_STRING();
    }
    else
    {
        RETVAL_STRINGL(buffer->str, buffer->length);
    }
}

static PHP_METHOD(swoole_websocket_frame, __toString)
{
    swString *buffer = SwooleTG.buffer_stack;
    zval *zdata = getThis();
    swString_clear(buffer);
    if (php_swoole_websocket_frame_pack(buffer, zdata, WEBSOCKET_OPCODE_TEXT, 1, 0) < 0)
    {
        RETURN_EMPTY_STRING();
    }
    else
    {
        RETVAL_STRINGL(buffer->str, buffer->length);
    }
}

static PHP_METHOD(swoole_websocket_server, unpack)
{
    swString buffer;
    bzero(&buffer, sizeof(buffer));

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &buffer.str, &buffer.length) == FAILURE)
    {
        RETURN_FALSE;
    }

    php_swoole_websocket_frame_unpack(&buffer, return_value);
}

static PHP_METHOD(swoole_websocket_server, exist)
{
    zend_long fd;

    swServer *serv = (swServer *) swoole_get_object(getThis());
    if (serv->gs->start == 0)
    {
        php_error_docref(NULL, E_WARNING, "the server is not running.");
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &fd) == FAILURE)
    {
        RETURN_FALSE;
    }

    swConnection *conn = swWorker_get_connection(serv, fd);
    if (!conn)
    {
        RETURN_FALSE;
    }
    //connection is closed
    if (conn->active == 0 || conn->closed)
    {
        RETURN_FALSE;
    }
    swConnection *server_sock = swServer_connection_get(serv, conn->from_fd);
    if (server_sock)
    {
        swListenPort *port = (swListenPort *) server_sock->object;
        //not websocket port
        if (port && !port->open_websocket_protocol)
        {
            RETURN_TRUE;
        }
    }
    //have not handshake
    if (conn->websocket_status < WEBSOCKET_STATUS_ACTIVE)
    {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

static PHP_METHOD(swoole_websocket_server, isEstablished)
{
    zend_long fd;

    swServer *serv = (swServer *) swoole_get_object(getThis());
    if (serv->gs->start == 0)
    {
        php_error_docref(NULL, E_WARNING, "the server is not running.");
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &fd) == FAILURE)
    {
        RETURN_FALSE;
    }

    swConnection *conn = swWorker_get_connection(serv, fd);
    //not isEstablished
    if (!conn || conn->active == 0 || conn->closed || conn->websocket_status < WEBSOCKET_STATUS_ACTIVE)
    {
        RETURN_FALSE;
    }
    else
    {
        RETURN_TRUE;
    }
}
