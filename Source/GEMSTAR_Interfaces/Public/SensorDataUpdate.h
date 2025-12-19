// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "osi_sensorview.pb.h"
#include "osi_sensordata.pb.h"
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SensorDataUpdate.generated.h"

USTRUCT(BlueprintType)
struct FLidarData {
	GENERATED_BODY()
	TArray<uint8> DataPackage;
};

USTRUCT(BlueprintType)
struct FSensorView {
	GENERATED_BODY()
	FName ID;
	FLidarData lidar;
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
	raw sensor information (reflections)
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void SensorViewDataUpdate(FSensorView& sensorViewData);

	/*
	processed sensor information (point clouds)
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void SensorDataUpdate(FSensorData& sensorData);
};
