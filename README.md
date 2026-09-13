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
    "std_log": false,
    "plugins": [
        "C:/SomePath/To/MyPlugin.dll"
    ]
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

If `true` then logs will additionally output to stdout and stderr. Defaults to `true`.

### plugins (optional)

A list of full paths to plugin DLLs to load. Each is loaded once all of NWN2ModLoader's own hooks are attached, in the order listed. If omitted, no plugins are loaded.

## Plugins

A plugin is a DLL that exports two `extern "C"` functions:

```cpp
extern "C" __declspec(dllexport) IPlugin* CreatePlugin(IPluginHost* host);
extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin);
```

`CreatePlugin` is called once at load time with an `IPluginHost*`, so the plugin can call its API (e.g. looking up other plugins) as needed. `DestroyPlugin` is called on shutdown so the plugin's own module frees the object it allocated.

`IPlugin` (`src/NWN2Plugin/Plugin.h`) is the interface a plugin implements. This header lives in its own folder, separate from NWN2ModLoader's internal implementation, since it's the one file a plugin author actually needs to consume:

- `GetPluginId()` — a short, stable string ID for the plugin. It's used to route calls to it, and must be unique among loaded plugins.
- `OnInitialize(host)` — called once per plugin, right after every plugin listed in the config has finished loading. Unlike `CreatePlugin` (where a plugin listed later in the config wouldn't have loaded yet), every other plugin is guaranteed to already exist by this point, so it's the right place to look up and interact with them.
- `OnNWNXSetString` / `OnNWNXSetInt` / `OnNWNXSetFloat` / `OnNWNXGetString` / `OnNWNXGetInt` / `OnNWNXGetFloat` — called when a script calls `NWNX_...` with this plugin's ID as the `plugin` argument.
- `OnSetBinaryData` / `OnGetBinaryData` — called when a script calls `StoreCampaignObject(sCampaignName, sVarName, oObject, oPlayer)` / `RetrieveCampaignObject(sCampaignName, sVarName, ...)` with `sCampaignName` matching this plugin's ID, e.g. `StoreCampaignObject("MyPlugin", "SomeVar", oItem, oPC)`. `sVarName` is passed through untouched as the plugin's own data key.

NWN2ServerMod **never** calls the engine's own `CCampaignDB::SetBinaryData`/`GetBinaryData` — not for plugin-claimed campaign names, not for anything else. A plugin whose ID matches the campaign name is fully responsible for storing and retrieving that data itself (its own database, a file, wherever); nothing is ever written to or read from NWN2Server's own SQLite-backed campaign DB for those calls, or for any others — unmatched campaign names are simply dropped.

`OnSetBinaryData` returns `bool`, and that return value **is** `StoreCampaignObject`'s return value as seen by the calling script — `ExecuteCommandStoreCampaignObject` pushes it directly onto the script VM's stack as the function's result, rather than just using it internally. Return `true` only if you actually stored the data. An unclaimed campaign name reports `false` to the script.

`OnGetBinaryData` is given an `IBinaryDataResult&`: call `result.Allocate(size)` once you know the final size of your data and fill the buffer it returns; never calling `Allocate` reports "no data" for that variable. This is surfaced to the engine as a null `shared_ptr<DataBlock>`, the same value the real engine's `ExecuteCommandRetrieveCampaignObject` treats as "not found" and already null-checks correctly (falling through to `OBJECT_INVALID`). If you call `Allocate` but later decide you have nothing to return after all, call `result.Clear()` to go back to reporting "no data" instead of having to track that yourself.

`OnNWNXGetString` is given an `IStringResult&` rather than a `char*`/capacity pair: call `result.Allocate(length)` and fill the buffer it returns (the host guarantees a null terminator right after it — you never write one yourself), or `result.Set(value)` if you already have a null-terminated string. Leave `result` untouched to report "no value", or call `result.Clear()` to undo an `Allocate`/`Set` you made speculatively. `OnNWNXGetInt`/`OnNWNXGetFloat` keep the simpler `bool` + `int&`/`float&` shape — there's no buffer/capacity/null-termination hazard with a single primitive, so there was nothing to fix there.

### Calling a Plugin from NWScript

A module's own scripts reach a plugin entirely through NWScript's existing `StoreCampaignObject`/`RetrieveCampaignObject` and `NWNX*` functions — NWN2ServerMod adds no new script functions. Which plugin gets the call is decided purely by matching one of the arguments against that plugin's `GetPluginId()`; everything else is forwarded straight through to the matching `IPlugin` callback described above.

**Campaign object storage**, routed to `OnSetBinaryData`/`OnGetBinaryData`:

```nwscript
StoreCampaignObject("Sample", "Sword", oSword, oPC);
object oSwordGotten = RetrieveCampaignObject("Sample", "Sword", GetLocation(oPC));
```

- `sCampaignName` (`"Sample"`) is the plugin selector — a script can only reach a plugin this way by passing that plugin's exact ID as the campaign name. An unmatched name gets `StoreCampaignObject` = `FALSE` / `RetrieveCampaignObject` = `OBJECT_INVALID`, exactly as if the variable had never been stored.
- `sVarName` (`"Sword"`) is forwarded unmodified as `varName` — it's the plugin's own storage key, meaning whatever the plugin wants it to.
- `oObject`/the returned object is serialized/deserialized by the engine itself; the plugin only ever sees or produces the raw bytes (`data`/`size` into `OnSetBinaryData`, the buffer filled via `IBinaryDataResult::Allocate` out of `OnGetBinaryData`).
- `oPlayer` never reaches the plugin as an object — the engine resolves it to the identity string described under `OnSetBinaryData` above before the plugin ever sees it, as `player`.
- `RetrieveCampaignObject`'s `lLocation` argument (where the reconstructed object gets placed) and its optional `oCreator` argument are handled entirely by the engine; neither is passed to the plugin.

**Ad hoc named values**, routed to `OnNWNXSetString`/`OnNWNXSetInt`/`OnNWNXSetFloat`/`OnNWNXGetString`/`OnNWNXGetInt`/`OnNWNXGetFloat`:

```nwscript
NWNXSetFloat("Sample", "Float", "", 0, 11.0);
NWNXSetString("Sample", "String", "", 0, "StringValue");
NWNXSetInt("Sample", "Int", "", 0, 33);
float fValue = NWNXGetFloat("Sample", "Float", "", 0);
string sValue = NWNXGetString("Sample", "String", "", 0);
int iValue = NWNXGetInt("Sample", "Int", "", 0);
```

- `sPlugin` (`"Sample"`) is the plugin selector, same as `sCampaignName` above.
- `sFunction`, `sParam1`, and `nParam2` are forwarded unmodified as the callback's `function`, `param1`, `param2` — the host never interprets them itself, they're just a routing/key scheme the plugin defines the meaning of (`SamplePlugin` combines all three into one string key; a plugin is free to use them however it likes, including ignoring `param1`/`param2` entirely as in this example).
- The value being set or read (`value` in, `outValue`/return in) is opaque data the plugin is responsible for storing and returning itself.
- A `NWNXGetInt`/`NWNXGetFloat` call for a key the plugin doesn't recognize returns `0`/`0.0`; `NWNXGetString` returns `""`. There's no separate "not found" signal visible to the script — a plugin that needs that distinction has to encode it into the value itself (e.g. a sentinel).

`IPluginHost` (also `src/NWN2Plugin/Plugin.h`) is the small vtable interface behind the `host` pointer passed to `CreatePlugin`/`OnInitialize` — the real host class (NWN2ModLoader's internal `NWN2Mod`) implements it, but a plugin never needs to see that class's real definition, link against `NWN2ModLoader.lib`, or know anything about its ABI beyond this interface. `host->GetPlugin(id)` looks up another loaded plugin by its ID, returning `nullptr` if none is loaded with that ID — this is how one plugin can check for or interact with another. `host->RunScript(script, objectId)` runs a compiled script (a `.ncs` resref) immediately against the given object ID, exactly as NWScript's own `ExecuteScript(sScript, oTarget)` would; the target script must be a bare `void main()` (no parameters), and it returns `false` if the engine's virtual machine isn't ready yet (before the server has finished starting up) or the script couldn't be run. `NWScriptObject::OBJECT_INVALID` (also in `Plugin.h`) matches NWScript's own `OBJECT_INVALID` constant, for a script that doesn't need a valid target. Like `IBinaryDataResult`/`IStringResult`, `IPluginHost` is a stable vtable contract: only ever append new methods at the end.

Every method besides `GetPluginId` uses only raw pointers, `size_t`, primitives, and small virtual interfaces (like `IBinaryDataResult`/`IStringResult`) — no `std::string`, `std::vector`, `std::function`, or C-style callback/context pairs cross the DLL boundary. STL types aren't ABI-stable across differing compiler/CRT/iterator-debug-level settings, so a plugin built with slightly mismatched settings could silently corrupt memory instead of failing to link; a virtual interface gives the same "host owns the allocation" behavior without that risk, or the rawness of a bare function pointer/fixed buffer. All methods except `GetPluginId` also have no-op default implementations, so a plugin only needs to override what it uses.

**On the `shared_ptr` return:** `OnGetBinaryData`'s populated (non-empty) case works by constructing a `std::shared_ptr<DataBlock>` inside NWN2ModLoader.dll (via `std::make_shared`) and handing it back to the engine through the hidden return-value pointer. `CCampaignDB::GetBinaryData` builds its own return value with `std::make_shared<DataBlock>` too, using the standard MSVC `_Ref_count_base` control-block layout (interlocked refcount, vtable-based `_Destroy`/`_Delete_this`) that our own toolset produces. `DataBlock` itself allocates with `malloc`/frees with `free`, which is what our own `DataBlock` matches. The empty-result case (a plugin that never calls `Allocate`) carries no risk either way, since a null `shared_ptr` never touches its control block at all.

**Build requirements:** a plugin must be built with the same toolset as `NWN2ModLoader.dll` (currently `v145`) so its `IPlugin` vtable layout and calling convention match — that's unavoidable for any C++ virtual-interface plugin ABI. Nothing beyond that is required now that no STL container crosses the boundary.

`IPlugin` is a vtable-based ABI: treat it like a stable contract and only ever append new methods at the end.

`src/SamplePlugin/` is a complete, minimal `IPlugin` implementation (ID `"Sample"`) that backs every callback with an in-memory map (keyed by variable name for `OnSetBinaryData`/`OnGetBinaryData`, and by function/param1/param2 for the `NWNX*` callbacks), reporting "not found" as `false`/an untouched result exactly as described above. It builds as part of the solution to `SamplePlugin.dll` alongside the other projects.

It also logs every callback and the parameters it was given, using `Logger`/`Data` (copied into `src/NWN2Plugin/` alongside `Plugin.h`, since a plugin can't link against `NWN2Shared.lib` without also pulling in the loader's own dependencies) — this is proof, from outside the debugger, that the host is actually routing calls to the plugin. The log file sits next to `SamplePlugin.dll` itself (same path, `.log` extension), found by resolving the plugin's own module handle rather than the host process's, so it works regardless of what process loaded the DLL or its working directory.

Its `OnInitialize` override also calls `host->GetPlugin(GetPluginId())` and logs whether it got itself back, as a working example of one plugin looking another (or itself) up through the host.

# Contributing

See CONTRIBUTING.md for information.