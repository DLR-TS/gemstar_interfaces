// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SensorDataUpdate.h"
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SensorConfigurationData.generated.h"

UENUM(BlueprintType)
enum class SensorType : uint8 {
	CAMERA, LIDAR, RADAR
};

USTRUCT(BlueprintType)
struct FLidarConfigurationData {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewHorizontal = PI / 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewVertical = PI / 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfRaysHorizontal = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfRaysVertical = 10;
};

USTRUCT(BlueprintType)
struct FRadarConfigurationData {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewHorizontal = PI / 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewVertical = PI / 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfRaysHorizontal = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfRaysVertical = 10;
};

USTRUCT(BlueprintType)
struct FCameraConfigurationData {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewHorizontal = PI / 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	double FieldOfViewVertical = PI / 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfPixelsHorizontal = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	float NumberOfPixelsVertical = 10;
};

USTRUCT(BlueprintType)
struct FSensorConfigData {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FName ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FTransform Offset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	AActor* Vehicle = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	SensorType Type = SensorType::CAMERA;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FCameraConfigurationData Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FRadarConfigurationData Radar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FLidarConfigurationData Lidar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	FString Mount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SensorConfiguration")
	TArray<AActor*> DataTargets;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USensorConfigurationData : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API ISensorConfigurationData
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void SpawnSensor(FSensorConfigData& sensorConfiguration);
};
