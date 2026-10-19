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
#include "Engine/DataAsset.h"
#include "GemstarExperienceSet.generated.h"

UCLASS(BlueprintType)
class GEMSTAR_INTERFACES_API UGemstarExperienceSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience",
		meta = (GetOptions = "GEMSTAR_Interfaces.GemstarFeatureOptions.AvailableFeatures"))
	TArray<FString> GameFeaturesToEnable;
};