# afc_sync

Sync iDevice(iOS) files to computer.

```
.\afc_sync.exe --src "/DCIM" --dst "R:/iphone_dcim" --skip-exist --max-skips 10
```

If `--skip-exist` is specified, existing file on PC will not be overwritten.

If `--max-skip` specified, after skipping n files, program will terminate.

If `--network` specified, attemp to connect over network.

## GUI
A Flutter cross-platfrom GUI wrapper is available at: https://github.com/star-39/afc_sync_flutter

## Build
### UNIX
* Depends on `pkg-config` and `libmobiledevice` .
* Meson.

Example:
```
# RedHat
dnf in pkg-config meson libimobiledevice-devel
# Darwin
brew install meson libimobiledevice

meson setup build
ninja -vC build
```

### Windows
* Use MSYS2 to build.
* You need to put libimobiledevice's DLL files together with the afc_sync EXE.


## License
GPL v3
