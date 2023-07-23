# vol-sync - A simple tool for synchronizing disk volumes through SSH tunnels

## Summary

vol-sync is a simple rsync like tool but especially made for disk volumes.

Rsync was designed for transferring files and requires some special command-line options
to work well with disk volumes.

vol-sync aims to transfer full and/or incremental disk volumes using a SSH tunnel
from a local host to a remote host. To achieve this the client on the local host connects
to the remote host and runs an instance of vol-sync there. These two instances communicate
with each other and use MD5 hashes to determine chunks of data which differ between the local host
and target host. If the hashes do not match the chunk is updated on the target side and otherwise
skipped.

Just like rsync vol-sync needs to be installed on the local and remote host. Their versions are
checked during initialisation and the transfer is aborted if they mismatch.

vol-sync checks if the target volume is large enough for the source volume to fit in. If
the target volume is large one could use tools like "resize2fs" to make use of the available
space.

## Caution

vol-sync does not ask for confirmation when writing to a target volume!
Thus there is a risk for loosing data if the target volume was chosen incorrectly!

Do not use the "-s" switch manually, this switch is for internal use only!

## Usage

vol-sync shall be used as following:

```
USAGE:  vol-sync [OPTIONS] [SOURCE TARGET]

Mandatory arguments in client mode:
  SOURCE      Source volume (device) on the local host
  TARGET      Target volume (device) on the remote host,
              given as [USER@]HOST:VOLUME

Optional arguments:
  -e COMMAND  SSH command (default: ssh)
  -h          Show this usage information
  -i SECONDS  Print status every x seconds (default: 15; 0=off)
  -r PATH     Absolute path to the vol-sync executable on the remote machine
              (default: none to use the search path)
  -s TARGET   Run in server mode and use the given target volume (device)
              (internally used when connecting to a remote machine)
  -v          Verify chunks after write (default: off)

Version information:
  vol-sync 0.0.1
```

### Simple use case: Synchronize a hard disk partition to a hard disk partition on the remote host

```
$ vol-sync /dev/sdb1 user@remote-host:/dev/sdd4
```

vol-sync tries to connect to "remote-host" and authenticates as user "user",
then the block device /dev/sdb1 from the local host is synchronized to /dev/sdd4 on the
remote host.

### Extended use case: Synchronize a LVM volume to a LVM volume on the remote host using special "ssh" options and a vol-sync from a non-standard path

```
$ vol-sync -e 'ssh -p 2222' -r /opt/vol-sync/bin /dev/vg-a/vol1 user@remote-host:/dev/vg-b/vol2
```

vol-sync tries to connect to "remote-host" port 2222 and authenticates as user "user".
On the remote host vol-sync is launched from /opt/vol-sync/bin/vol-sync, then the block device
/dev/vg-a/vol1 from the local host is synchronized to /dev/vg-b/vol2 on the remote host.
