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
#include "Subsystems/GameInstanceSubsystem.h"
#include "GemstarGameFeatureSubsystem.generated.h"

namespace UE::GameFeatures { struct FResult; }

DECLARE_MULTICAST_DELEGATE(FOnGemstarStartupFeaturesReady);
DECLARE_LOG_CATEGORY_EXTERN(GemstarGameFeatureSubsystem, Log, All);

UCLASS()
class GEMSTAR_INTERFACES_API UGemstarGameFeatureSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:	
	void ActivateFeatures(const TArray<FString>& PluginNames);
	bool DeactivateFeature(const FString& PluginName, bool bUnload);
	void DeactivateStartupFeatures(FSimpleDelegate Completion = FSimpleDelegate());
	bool AreStartupFeaturesReady() const { return PendingActivations == 0; };
	FOnGemstarStartupFeaturesReady OnStartupFeaturesReady;

private:
	void ActiveFeature(const FString& PluginName);
	void OnFeatureActivationCompleted(const UE::GameFeatures::FResult& Result, FString PluginName, FString PluginURL);
	void FinishOneActivation();
	int32 PendingActivations = 0;
	TMap<FString, FString> ActivePluginURLs;
};
