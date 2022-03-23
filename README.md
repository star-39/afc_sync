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
### Linux
* Depends `pkg-config` and `libmobiledevice` development files.
* Both CMake and Meson are supported, if one fails, try another.

Example:
```
dnf in pkg-config cmake libimobiledevice-devel
cmake .
make
```

### Darwin
* It's recommended to use Meson build.

Example:
```
brew install meson libimobiledevice
meson setup build
ninja -vC build
```

### Windows
* Only CMake and a C compiler(MSVC) are required.


## License
GPL v3
