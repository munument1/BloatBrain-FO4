# F4SE Plugin

This directory contains the Fallout 4 game-side integration for BloatBrain-FO4.

## Current state

The first C++ scaffold now provides:

- a CommonLibF4/F4SE plugin entry point
- a background `std::jthread` for bridge I/O
- a WinSock TCP client
- automatic reconnect attempts to `127.0.0.1:8765`
- blocking socket work kept off the Fallout 4 game thread

Actor sampling and action execution are intentionally not implemented yet. The next milestone is to select one test Bloatfly and feed its observations through the existing connection.

## Toolchain

The scaffold follows the current `libxse/commonlibf4-template` layout:

- XMake 3.0+
- C++23 compiler (Visual Studio/MSVC or Clang-CL)
- CommonLibF4 from `libxse/commonlibf4`
- F4SE runtime installed in Fallout 4 for actual in-game loading

## Prepare CommonLibF4

From PowerShell:

```powershell
cd f4se-plugin
./bootstrap-commonlib.ps1
```

This clones CommonLibF4 into `f4se-plugin/lib/commonlibf4/`. The directory is ignored by Git because it is a local build dependency.

## Build

```powershell
cd f4se-plugin
xmake build
```

Optional deployment variables supported by the CommonLibF4 build rules can be used to copy output to a Fallout 4 install or mod-manager directory.

## Run the external bridge first

From the repository root:

```powershell
python -m pip install -e ".[dev]"
python -m bridge.tcp_server
```

The bridge listens on `127.0.0.1:8765` by default.

When the F4SE plugin loads, its worker thread attempts to connect to that endpoint. If the bridge is not available, it retries without blocking Fallout 4's main thread.

## Prototype responsibilities

- acquire one designated Bloatfly actor
- read the minimal observation set defined by protocol v1
- send observations to the external bridge
- receive action commands
- translate actions into safe game-side behavior
- restore fallback behavior when the controller is unavailable

## Implementation order

1. build and load the plugin successfully
2. verify the bridge connection in logs
3. identify one designated Bloatfly reliably
4. sample and serialize protocol-v1 observations
5. round-trip observations through `bridge.mock_controller`
6. apply only one or two actions first
7. add timeout/stale-command handling
8. expand to the full MVP action set

No neural-model code should live in the plugin. The game-facing layer only implements transport, observation collection, and action execution.
