# GEMSTAR Interfaces

[![DLR-TS](https://img.shields.io/badge/developed%20by-DLR%20TS-orange.svg?style=flat&colorA=E1523D&colorB=007D8A)](https://www.dlr.de/en/ts/about-us/the-institute-of-transportation-systems)

This plugin defines the interfaces between all [GEMSTAR](https://github.com/DLR-TS/gemstar) Gamefeature Plugins.
The Open Simulation Interface ([OSI](https://github.com/OpenSimulationInterface/open-simulation-interface)) is integrated to enable all GEMSTAR Plugins to read and write OSI messages.

## Interfaces

Currently interfaces are defined for:
- OSI Traffic Light Data (as Ground Truth)
- Traffic Light Data
- Sensor Configuration
- Sensor Data (OSI SensorView & SensorData)
- Vehicle Data

# Getting Started

Add the plugin to your Unreal project:

```sh
mkdir Plugins
cd Plugins

git clone https://github.com/DLR-TS/gemstar_interfaces.git
```
or:
```sh
git submodule add https://github.com/DLR-TS/gemstar_interfaces.git
```

# Configuration

**GEMSTAR Root**\
GEMSTAR Root is the heart of GEMSTAR and needs to be present in any GEMSTAR map.\
All Gamefeature Plugins spawn their Loader on the BP_GEMSTAR_Root.\
Enable the features in the editor or by configuration file.

**Utils**\
Some util function are made available for all plugins.
Mainly conversions from Unreal Engine data types to OSI data types.

# How to add an interface

## 1. Define interface (Example: EsminiVehicleDataUpdate.h)

Create: inherit from UInterface\
create USTRUCT UpdateData\
Create method with (optional) parameters

## 2. Define dispatcher (Example: EsminiVehicleDataDispatcher.h)

Create: inherit from UInterface\
Createt Register und Unregister methods

## 3. Integrate Dispatcher (Example: esminiActor.h)

class ESMINIRUNTIME_API AesminiActor : public AActor, public IEsminiVehicleDataDispatcher\
Syntax anders:\
IEsminiVehicleDataUpdate::Execute_VehicleDataUpdate(i, newMessage);

## 4. Integrate receiver (Example: VehicleManager.h)

class SIMVEHICLES_API AVehicleManager : public AActor, public IEsminiVehicleDataUpdate\
Syntax anders:\
virtual void VehicleDataUpdate_Implementation(FVehicleDataUpdateData data) override;

---------------
**Contact:** opensource-ts@dlr.de

This project uses the Open Simulation Interface ([OSI](https://github.com/OpenSimulationInterface/open-simulation-interface)), which is licensed under the
Mozilla Public License 2.0.