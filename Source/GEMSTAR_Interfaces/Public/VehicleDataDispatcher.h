// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "VehicleDataUpdate.h"

#include "VehicleDataDispatcher.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVehicleDataDispatcher : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API IVehicleDataDispatcher
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void RegisterVehicleDataReceiver(AActor* vehicleDataReceiver);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void UnregisterVehicleDataReceiver(AActor* vehicleDataReceiver);
};
