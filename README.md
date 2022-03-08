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
### *nix
* Depends `pkg-config` and `libmobiledevice` development files.

### Windows
* Simply cmake the project. You can install Visual Studio with CMake.


## License
GPL v3
