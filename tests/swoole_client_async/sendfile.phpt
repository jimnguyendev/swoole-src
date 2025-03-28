--TEST--
swoole_client_async: async sendfile
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';

$port = get_one_free_port();

$pm = new ProcessManager;
$pm->parentFunc = function ($pid) use ($port) {
    $client = new Swoole\Async\Client(SWOOLE_SOCK_TCP);
    $client->on("connect", function (Swoole\Async\Client $cli) {
        $cli->send(pack('N', filesize(TEST_IMAGE)));
        $ret = $cli->sendfile(TEST_IMAGE);
        Assert::assert($ret);
    });
    $client->on("receive", function (Swoole\Async\Client $cli, $data) {
        $cli->send(pack('N', 8) . 'shutdown');
        $cli->close();
        Assert::same($data, md5_file(TEST_IMAGE));
    });
    $client->on("error", function ($cli) {
        echo "Connect failed\n";
    });
    $client->on("close", function ($cli) {

    });
    $client->connect(TCP_SERVER_HOST, $port, 0.5);
    Swoole\Event::wait();
};

$pm->childFunc = function () use ($pm, $port) {
    $serv = new \Swoole\Server(TCP_SERVER_HOST, $port, SWOOLE_BASE, SWOOLE_SOCK_TCP);
    $serv->set([
        "worker_num" => 1,
        'log_file' => '/dev/null',
        'open_length_check' => true,
        'dispatch_mode' => 1,
        'package_length_type' => 'N',
        'package_length_offset' => 0,
        'package_body_offset' => 4,
        'package_max_length' => 2000000,
    ]);
    $serv->on("WorkerStart", function (\Swoole\Server $serv) use ($pm) {
        $pm->wakeup();
    });
    $serv->on("Receive", function (\Swoole\Server $serv, $fd, $rid, $data) {
        if (substr($data, 4, 8) == 'shutdown') {
            $serv->shutdown();
            return;
        }
        $serv->send($fd, md5(substr($data, 4)));
    });
    $serv->start();
};

$pm->childFirst();
$pm->run();
?>
--EXPECT--
