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

#include "CoreMinimal.h"
#include "TrafficLightDataUpdate.h"
#include "UObject/Interface.h"
#include "TrafficLightDataDispatcher.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTrafficLightDataDispatcher : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GEMSTAR_INTERFACES_API ITrafficLightDataDispatcher
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void RegisterTrafficLight(AActor* trafficLight);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void UnregisterTrafficLight(AActor* trafficLight);
};
