/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#pragma once
#include "osi_common.pb.h"
#include "osi_sensorviewconfiguration.pb.h"

#include "Interfaces/Sensor/SensorConfigurationData.h"

#include "Math/UnrealMathUtility.h"

#include "CoreMinimal.h"

/**
 *
 */
class GEMSTAR_INTERFACES_API ToOSI
{
public:
	ToOSI();
	~ToOSI();
	static osi3::BaseStationary* BaseStationary(FTransform transform);
	static osi3::BaseStationary* toOSIBaseStationary(const AActor* actor);
	static osi3::BaseMoving* toOSIBaseMoving(const AActor* actor);
	static osi3::Vector3d* Position(FVector vector);
	static osi3::Orientation3d* Orientation3d(FQuat quat);
	static osi3::Dimension3d* Dimension(FVector dimension);
	static osi3::MountingPosition* toMountingPosition(FSensorConfigData& sensorConfigData);
};
