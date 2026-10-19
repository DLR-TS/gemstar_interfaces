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
#include "UObject/Interface.h"
#include "VRUDataUpdate.generated.h"

USTRUCT(BlueprintType)
struct FVRUDataUpdateData {
	GENERATED_BODY()
	FName Provider;
	FName Name;
	FTransform Position;
	FVector BoundingBox;
	double speed;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVRUDataUpdate : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GEMSTAR_INTERFACES_API IVRUDataUpdate
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void VRUDataUpdate(FVRUDataUpdateData& data);
};
