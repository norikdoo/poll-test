# poll-test cmdline tool

> poll-test cmdline tool for testing poll() syscall on arbitrary file.

## Setup

```shell
make
```

## Usage

```shell
usage: poll-test [-f file]
        [-m POLLMASK]
        [-o FOPEN_FLAGS]
        [-t timeout]
        [-l] [-i] [-e] [-w]
        [-h]
```

## Example

Basic example:

```shell
$ poll-test -f /dev/beacon-pulse
 [WARN]: Using default open() file flags: 0x00000002
 [WARN]: Using default poll() mask value: 0x0005 -> POLLIN|POLLOUT
 [WARN]: Using default poll() timeout value: -1 (no timeout)
 [INFO]: Do single poll() on /dev/beacon-pulse (mask: 0x0005 -> POLLIN|POLLOUT, poll_timeout: -1 (no timeout))
 [INFO]: (0) poll() returned revents: 0x0001 -> POLLIN
```


Specify poll() timeout and do calls in a loop:
```shell
$ poll-test -f /dev/beacon-pulse -t 5000 -l
 [WARN]: Using default open() file flags: 0x00000002
 [WARN]: Using default poll() mask value: 0x0005 -> POLLIN|POLLOUT
 [INFO]: Do loop poll() on /dev/beacon-pulse (mask: 0x0005 -> POLLIN|POLLOUT, poll_timeout: 5000 ms)
 [INFO]: (0) poll() timeout of 5000 msec reached...
 [INFO]: (1) poll() timeout of 5000 msec reached...
 [INFO]: (2) poll() timeout of 5000 msec reached...
 [INFO]: (3) poll() returned revents: 0x0001 -> POLLIN
 [INFO]: (4) poll() timeout of 5000 msec reached...
...
```


Specify POLLMASK:

```shell
$ poll-test -f /dev/beacon-pulse -m "POLLIN|POLLRDNORM|POLLPRI"
 [WARN]: Using default open() file flags: 0x00000002
 [WARN]: Using default poll() timeout value: -1 (no timeout)
 [INFO]: Do single poll() on /dev/beacon-pulse (mask: 0x0043 -> POLLIN|POLLPRI|POLLRDNORM, poll_timeout: -1 (no timeout))
 [INFO]: (0) poll() returned revents: 0x0043 -> POLLIN|POLLPRI|POLLRDNORM

```


Using poll-test on sysfs attributes (triggered with sysfs_notify()):

```shell
$ poll-test -f /sys/devices/housekeeper/movement/beacon-pulse/value -m "POLLPRI" -i -e
 [WARN]: Using default open() file flags: 0x00000002
 [WARN]: Using default poll() timeout value: -1 (no timeout)
 [INFO]: Do single poll() on /sys/devices/housekeeper/movement/beacon-pulse/value (mask: 0x0002 -> POLLPRI, poll_time out: -1 (no timeout))
 [INFO]: Initial dummy read() returns 2 bytes
 [INFO]: (0) poll() returned revents: 0x000A -> POLLPRI|POLLERR
 [INFO]: (0) Dummy read() after poll() returns 2 bytes

```


## License

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
- Copyright 2020 © <a href="http://www.norik.com" target="_blank">Norik systems d.o.o.</a>.

