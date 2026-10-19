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
#include "GemstarConfigOverride.generated.h"

// Carried by an experience instead of a ini file
USTRUCT()
struct FGemstarConfigOverride
{
	GENERATED_BODY()

	// Section without brackets: "esmini", "vehicles", "lidar:sensor1".
	UPROPERTY(EditAnywhere, Category = "GEMSTAR")
	FString Section;

	UPROPERTY(EditAnywhere, Category = "GEMSTAR")
	FString Key;

	UPROPERTY(EditAnywhere, Category = "GEMSTAR")
	FString Value;
};
