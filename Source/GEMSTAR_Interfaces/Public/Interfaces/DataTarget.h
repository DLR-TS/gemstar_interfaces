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
#include "DataTarget.generated.h"

USTRUCT(BlueprintType)
struct FDataTarget {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	AActor* Actor = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	int TargetIndex = 0;

	bool operator==(const FDataTarget& Other) const
	{
		return Actor == Other.Actor
			&& TargetIndex == Other.TargetIndex;
	}
};
