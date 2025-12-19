// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "osi_common.pb.h"
#include "osi_sensorviewconfiguration.pb.h"

#include "SensorConfigurationData.h"

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
