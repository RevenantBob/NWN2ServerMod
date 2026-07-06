# NWN2ServerMod
Neverwinter Nights 2 Server Modding

## Requirements
- Visual Studio 2026
- Neverwinter Nights 2: Enhanced Edition (Legacy Not Supported)
- 64-Bit Only

## Languages / Libraries Used
- Windows API
- [C++23 Standard](https://cppreference.com/cpp/23)


## Configuration Path

NWN2ServerMod will load the configuration file `nwn2mod.config` in the executables directory (Not the working directory).

The path to the configuration may be optionally provided as a command-line argument.

```
NWN2ServerMod.exe <config-path>
```

## Configuration Format

The configuration is a JSON file which is described below.

**NOTE:** Pay attention to escape sequences in JSON strings. Only UTF-8 format is supported for JSON.

```
{
    "server_exe": "C:/SomePath/To/NWN2Server64.exe",
    "server_args": "-moduledir <Module>",
    "server_directory": "C:\\SomePath\\To\\Working\\Directory\\",
    "loader_dll": "C:/SomePath/To/NWN2ModLoader.dll",
    "loader_log": "C:/SomePath/To/LogFile.txt",
    "server_log": "C:/SomePath/To/ServerLog.txt",
    "debug_log": true,
    "std_log": false
}
```

### server_exe (Required)

The full path to the `NWN2Server64.exe` executable.

### server_args (Required)

The arguments passed to `NWN2Server64.exe` when executed.

### server_directory (optional)

The optional working directory of `NWN2Server64.exe`. This defaults to the parent path of `server_exe`.

### loader_dll (optional)

The optional full path to the `NWN2ModLoader.dll`. This defaults to `NWN2ModLoader.dll` in the same parent path as `NWN2ServerMod.exe`.

### loader_log (optional)

The optional full path to a log file written to by `NWN2ModLoader.dll`. This will contain errors and information about the server. If omitted there will be no log file created.

### server_log (optional)

The optional full path to a log file written to by `NWN2ServerMod.exe`. This will contain errors and information about the startup of the NWN2 server modding before hooking occurs and after. If omitted there will be no log file created.

### debug_log (optional)

If `true` then logs will additionally output with `OutputDebugString`. Defaults to `false`.

### std_log (optional)

If `true` then logs will additionally output to stdout and stderr. Defaults to `true`.'

# Contributing

See CONTRIBUTING.md for information.