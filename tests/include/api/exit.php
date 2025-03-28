<?php
declare(strict_types=1);
if (PHP_VERSION_ID>= 80400) {
    $exit_status_list = [
        1,
        'exit',
        0,
    ];
} else {
    $exit_status_list = [
        'undef',
        null,
        true,
        false,
        1,
        1.1,
        'exit',
        ['exit' => 'ok'],
        (object)['exit' => 'ok'],
        STDIN,
        0
    ];
}

function route()
{
    controller();
}

function controller()
{
    your_code();
}

function your_code()
{
    global $exit_status_list;
    co::sleep(.001);
    $exit_status = array_shift($exit_status_list);
    if ($exit_status === 'undef') {
        exit;
    } else {
        exit($exit_status);
    }
}

$chan = new Swoole\Coroutine\Channel;

go(function () use ($chan, $exit_status_list) {
    foreach ($exit_status_list as $val) {
        $chan->push($val);
    }
});

for ($i = 0; $i < count($exit_status_list); $i++) {
    go(function () use ($exit_status_list, $chan) {
        try {
            // in coroutine
            route();
        } catch (\Swoole\ExitException $e) {
            Assert::assert($e->getFlags() & SWOOLE_EXIT_IN_COROUTINE);
            $exit_status = $chan->pop();
            if (PHP_VERSION_ID < 80400) {
                $exit_status = $exit_status === 'undef' ? null : $exit_status;
            }
            Assert::same($e->getStatus(), $exit_status);
            var_dump($e->getStatus());
            // exit coroutine
            return;
        }
        echo "never here\n";
    });
}

Swoole\Event::wait();
