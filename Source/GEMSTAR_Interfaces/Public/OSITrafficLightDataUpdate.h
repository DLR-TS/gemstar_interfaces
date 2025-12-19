// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "osi_groundtruth.pb.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OSITrafficLightDataUpdate.generated.h"

USTRUCT(BlueprintType)
struct FGroundTruthWrapper {
	GENERATED_BODY()
	osi3::GroundTruth* gt;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UOSITrafficLightDataUpdate : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API IOSITrafficLightDataUpdate
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void OSITrafficLightDataUpdate(FGroundTruthWrapper& trafficLightData);
};
