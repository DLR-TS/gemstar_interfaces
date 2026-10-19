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
#include "osi_sensorview.pb.h"
#include "osi_sensordata.pb.h"
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SensorDataUpdate.generated.h"

USTRUCT(BlueprintType)
struct FCameraData {
	GENERATED_BODY()
	TArray<uint8> DataPackage;
};

USTRUCT(BlueprintType)
struct FLidarData {
	GENERATED_BODY()
	TArray<uint8> DataPackage;
};

USTRUCT(BlueprintType)
struct FRadarData {
	GENERATED_BODY()
	TArray<uint8> DataPackage;
};

USTRUCT(BlueprintType)
struct FSensorView {
	GENERATED_BODY()
	FName ID;
	FCameraData camera;
	FLidarData lidar;
	FRadarData radar;
	osi3::SensorView* sensorview = nullptr;
};

USTRUCT(BlueprintType)
struct FSensorData {
	GENERATED_BODY()
	FName ID;
	osi3::SensorData* sensordata = nullptr;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USensorDataUpdate : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API ISensorDataUpdate
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/*
	raw sensor information (e.g. reflections)
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void SensorViewDataUpdate(FSensorView& sensorViewData, int targetIndex, UTextureRenderTarget2D* cameraRenderTarget);

	/*
	processed sensor information (e.g. point clouds)
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void SensorDataUpdate(FSensorData& sensorData, int targetIndex);
};
