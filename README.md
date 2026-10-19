# GEMSTAR Interfaces

[![DLR-TS](https://img.shields.io/badge/developed%20by-DLR%20TS-orange.svg?style=flat&colorA=E1523D&colorB=007D8A)](https://www.dlr.de/en/ts/about-us/the-institute-of-transportation-systems)

`GEMSTAR_Interfaces` contains the shared contracts used by GEMSTAR GameFeature plugins.

Use this plugin for data contracts shared by GEMSTAR features.

## Interfaces

Currently this plugin provides shared interfaces for:

- OSI traffic light data as ground truth
- traffic light data
- sensor configuration
- sensor data with OSI `SensorView` and `SensorData`
- vehicle data
- render target camera sources

The Open Simulation Interface ([OSI](https://github.com/OpenSimulationInterface/open-simulation-interface)) is integrated so GEMSTAR plugins can read and write OSI messages.\
OSI Version: `3.8.0`

## Installation

Add the plugin to your Unreal project:

```sh
mkdir Plugins
cd Plugins
git clone https://github.com/DLR-TS/gemstar_interfaces.git
```

Or add it as a submodule:

```sh
git submodule add https://github.com/DLR-TS/gemstar_interfaces.git Plugins/gemstar_interfaces
```

## Third-party dependencies

Linux artefacts are built through Docker:

```sh
./Scripts/build-dependencies.sh
```

Windows artefacts are built natively with Git, CMake, and Visual Studio 2022:

```powershell
.\Scripts\build-dependencies.ps1
```

# Configuration

`AGEMSTARRoot` must exist in every GEMSTAR map. GameFeature loaders attach to the root actor and read the configured GEMSTAR ini file.

Use the root to:

- load the GEMSTAR ini
- share configuration with managers
- cache and resolve data flows
- mount spawned sensors or feature actors to configured targets

## Render Target Camera Source

Camera-like features can implement `UGEMSTARRenderTargetSource` to expose a render target to other features.

Implement:

- `GetGEMSTARRenderTarget()`
- `GetGEMSTARRenderTargetFrameId()`

## Adding A Classic GEMSTAR Interface

Use the existing pattern when a payload is not serialized Protobuf and needs a direct Unreal interface:

1. Create a `UInterface`.
2. Create a `USTRUCT` update payload.
3. Add dispatcher registration methods.
4. Implement the receiver in the consuming manager.

Existing examples include vehicle data, sensor data, and traffic light data.


## Bulk Mode

The bulk mode is a game instance subsystem that look for scenarios in a given path and replay than in a sequence. Each scenario player calls the subsystem when it is done and triggers the loading of the next scenario.

General usage is described in - [Bulk Mode](https://github.com/DLR-TS/gemstar/Documentation/BulkMode.md)
 

## Contact

opensource-ts@dlr.de

This project uses the Open Simulation Interface ([OSI](https://github.com/OpenSimulationInterface/open-simulation-interface)), which is licensed under the Mozilla Public License 2.0.
