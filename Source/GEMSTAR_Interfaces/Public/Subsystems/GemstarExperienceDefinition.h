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
#include "Experience/GemstarConfigOverride.h"
#include "Experience/GemstarExperienceSet.h"
#include "GemstarExperienceDefinition.generated.h"

class UGameFeatureAction;

UCLASS(BlueprintType)
class GEMSTAR_INTERFACES_API UGemstarExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:	
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
	TArray<TObjectPtr<UGemstarExperienceSet>> ExperienceSets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience",
		meta = (GetOptions = "GEMSTAR_Interfaces.GemstarFeatureOptions.AvailableFeatures"))
	TArray<FString> GameFeaturesToEnable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience",
		meta = (GetOptions = "GEMSTAR_Interfaces.GemstarFeatureOptions.AvailableFeatureTags"))
	TArray<FString> AcceptsFeatureTags;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Experience")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// Loaded before any feature activates. List only content this plugin owns: naming another
	// feature's assets here is a cross-plugin edge, and the reference validators reject it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience|Preload")
	TArray<TSoftObjectPtr<UObject>> PreloadAssets;

	// Applied to the loaded configuration before any feature reads it
	UPROPERTY(EditDefaultsOnly, Category = "Experience|Configuration")
	TArray<FGemstarConfigOverride> ConfigOverrides;

	TArray<FString> ResolveFeatures() const;

private:
	// Tag source, applied last by ResolveFeatures
	void AddTaggedFeatures(TArray<FString>& Features) const;

public:

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
