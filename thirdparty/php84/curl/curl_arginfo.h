/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: ddfcdd8a0bf0ee6c338ec1689c6de5d7fd87303d */
#include "swoole_curl_interface.h"

#if defined(SW_USE_CURL) && PHP_VERSION_ID >= 80400
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_close, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_swoole_native_curl_copy_handle, 0, 1, CurlHandle, MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_errno, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_error, 0, 1, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_swoole_native_curl_escape, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_swoole_native_curl_unescape arginfo_swoole_native_curl_escape

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_setopt, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_swoole_native_curl_exec, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_getinfo, 0, 1, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, option, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_swoole_native_curl_init, 0, 0, CurlHandle, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, url, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#if LIBCURL_VERSION_NUM >= 0x073E00 /* Available since 7.62.0 */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_upkeep, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_add_handle, 0, 2, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_close, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_errno, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_exec, 0, 2, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
	ZEND_ARG_INFO(1, still_running)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_getcontent, 0, 1, IS_STRING, 1)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_swoole_native_curl_multi_info_read, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(1, queued_messages, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_swoole_native_curl_multi_init, 0, 0, CurlMultiHandle, 0)
ZEND_END_ARG_INFO()

#define arginfo_swoole_native_curl_multi_remove_handle arginfo_swoole_native_curl_multi_add_handle

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_select, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, multi_handle, CurlMultiHandle, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 0, "1.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_multi_strerror, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, error_code, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_pause, 0, 2, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_swoole_native_curl_reset arginfo_swoole_native_curl_close

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_setopt_array, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_swoole_native_curl_setopt, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, handle, CurlHandle, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_swoole_native_curl_strerror arginfo_swoole_native_curl_multi_strerror

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_swoole_native_curl_version, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

static const zend_function_entry swoole_native_curl_functions[] = {
	ZEND_FE(swoole_native_curl_close, arginfo_swoole_native_curl_close)
	ZEND_FE(swoole_native_curl_copy_handle, arginfo_swoole_native_curl_copy_handle)
	ZEND_FE(swoole_native_curl_errno, arginfo_swoole_native_curl_errno)
	ZEND_FE(swoole_native_curl_error, arginfo_swoole_native_curl_error)
	ZEND_FE(swoole_native_curl_escape, arginfo_swoole_native_curl_escape)
	ZEND_FE(swoole_native_curl_unescape, arginfo_swoole_native_curl_unescape)
	ZEND_FE(swoole_native_curl_multi_setopt, arginfo_swoole_native_curl_multi_setopt)
	ZEND_FE(swoole_native_curl_exec, arginfo_swoole_native_curl_exec)
	ZEND_FE(swoole_native_curl_getinfo, arginfo_swoole_native_curl_getinfo)
	ZEND_FE(swoole_native_curl_init, arginfo_swoole_native_curl_init)
#if LIBCURL_VERSION_NUM >= 0x073E00 /* Available since 7.62.0 */
	ZEND_FE(swoole_native_curl_upkeep, arginfo_swoole_native_curl_upkeep)
#endif
	ZEND_FE(swoole_native_curl_multi_add_handle, arginfo_swoole_native_curl_multi_add_handle)
	ZEND_FE(swoole_native_curl_multi_close, arginfo_swoole_native_curl_multi_close)
	ZEND_FE(swoole_native_curl_multi_errno, arginfo_swoole_native_curl_multi_errno)
	ZEND_FE(swoole_native_curl_multi_exec, arginfo_swoole_native_curl_multi_exec)
	ZEND_FE(swoole_native_curl_multi_getcontent, arginfo_swoole_native_curl_multi_getcontent)
	ZEND_FE(swoole_native_curl_multi_info_read, arginfo_swoole_native_curl_multi_info_read)
	ZEND_FE(swoole_native_curl_multi_init, arginfo_swoole_native_curl_multi_init)
	ZEND_FE(swoole_native_curl_multi_remove_handle, arginfo_swoole_native_curl_multi_remove_handle)
	ZEND_FE(swoole_native_curl_multi_select, arginfo_swoole_native_curl_multi_select)
	ZEND_FE(swoole_native_curl_multi_strerror, arginfo_swoole_native_curl_multi_strerror)
	ZEND_FE(swoole_native_curl_pause, arginfo_swoole_native_curl_pause)
	ZEND_FE(swoole_native_curl_reset, arginfo_swoole_native_curl_reset)
	ZEND_FE(swoole_native_curl_setopt_array, arginfo_swoole_native_curl_setopt_array)
	ZEND_FE(swoole_native_curl_setopt, arginfo_swoole_native_curl_setopt)
	ZEND_FE(swoole_native_curl_strerror, arginfo_swoole_native_curl_strerror)
	ZEND_FE(swoole_native_curl_version, arginfo_swoole_native_curl_version)
	ZEND_FE_END
};

static const zend_function_entry class_CurlHandle_methods[] = {
	ZEND_FE_END
};

static const zend_function_entry class_CurlMultiHandle_methods[] = {
	ZEND_FE_END
};

static const zend_function_entry class_CurlShareHandle_methods[] = {
	ZEND_FE_END
};
#endif
