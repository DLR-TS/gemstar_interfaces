// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GEMSTARManager.generated.h"

UENUM(BlueprintType)
enum class GEMSTARInterfaceType : uint8 {
	SENSORVIEW,
	SENSORDATA,
	TRAFFICUPDATE,
	TRAFFICLIGHTUPDATE
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGEMSTARManager : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API IGEMSTARManager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	bool RegisterDataflow(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName);
	virtual bool RegisterDataflow_Implementation(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName) { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	bool MountOn(const FString& NameByConfiguration, AActor* Actor);
	virtual bool MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor) { return false; }

	void ConnectInputs(const FString* Input, UObject* GEMSTARRoot, AActor* me, GEMSTARInterfaceType type);
};
