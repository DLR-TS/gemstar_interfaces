/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarGameFeatureSubsystem.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturePluginOperationResult.h"
#include "Utils/GemstarInterfacesLog.h"

void UGemstarGameFeatureSubsystem::ActivateFeatures(const TArray<FString>& PluginNames)
{
	PendingActivations = PluginNames.Num();
	
	if (PendingActivations == 0) {
		OnStartupFeaturesReady.Broadcast();
		return;
	}

	for (const FString& PluginName : PluginNames) {
		ActiveFeature(PluginName);
	}
}

bool UGemstarGameFeatureSubsystem::DeactivateFeature(const FString& PluginName, bool bUnload)
{
	const FString* PluginURL = ActivePluginURLs.Find(PluginName);

	if (!PluginURL) {
		return false;
	}
	const FString URL = *PluginURL;

	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR: deactivating %s"), *PluginName);

	UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(
		URL,
		FGameFeaturePluginDeactivateComplete::CreateWeakLambda(
			this,
			[this, PluginName, URL, bUnload](const UE::GameFeatures::FResult& Result) {
				
				if (Result.HasError()) {
					UE_LOG(GemstarInterfaces, Error, TEXT("Could not deactivate %s: %s"), *PluginName, *Result.GetError());
					return;
				}
				ActivePluginURLs.Remove(PluginName);

				if (bUnload) {
					UGameFeaturesSubsystem::Get().UnloadGameFeaturePlugin(URL, true);
				}
			}
		)
	);

	return true;
}

void UGemstarGameFeatureSubsystem::DeactivateStartupFeatures(FSimpleDelegate Completion)
{
	TArray<FString> PluginNames;
	ActivePluginURLs.GetKeys(PluginNames);

	if (PluginNames.Num() > 0) {
		UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR: deactivating %d startup feature(s)"), PluginNames.Num());
	}

	if (PluginNames.IsEmpty()) {
		Completion.ExecuteIfBound();
		return;
	}

	TSharedRef<int32> Remaining = MakeShared<int32>(PluginNames.Num());
	for (const FString& PluginName : PluginNames) {
		const FString URL = ActivePluginURLs.FindChecked(PluginName);
		UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(URL,
			FGameFeaturePluginDeactivateComplete::CreateWeakLambda(this,
				[this, PluginName, Completion, Remaining](const UE::GameFeatures::FResult& Result) mutable {
					if (Result.HasError()) {
						UE_LOG(GemstarInterfaces, Error, TEXT("Could not deactivate %s: %s"), *PluginName, *Result.GetError());
					}
					else {
						ActivePluginURLs.Remove(PluginName);
					}
					if (--(*Remaining) == 0) {
						Completion.ExecuteIfBound();
					}
				}));
	}
}

void UGemstarGameFeatureSubsystem::ActiveFeature(const FString& PluginName)
{
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	FString PluginURL;

	if ( !Subsystem.GetPluginURLByName(PluginName, PluginURL) ) {
		UE_LOG(GemstarInterfaces, Error, TEXT("Unknown Game Feature Plugin: %s"), *PluginName);
		FinishOneActivation();
		return;
	}

	if ( Subsystem.IsGameFeaturePluginActive(PluginURL, true) ) {
		ActivePluginURLs.Add(PluginName, PluginURL);
		FinishOneActivation();
		return;
	}
	// The callback finishes this one 
	// counting it here would only count the request
	Subsystem.LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnFeatureActivationCompleted, PluginName, PluginURL));
}


void UGemstarGameFeatureSubsystem::OnFeatureActivationCompleted(const UE::GameFeatures::FResult& Result, FString PluginName, FString PluginURL)
{
	if (Result.HasError()) {
		UE_LOG(GemstarInterfaces, Error, TEXT("Activation of %s failed: %s"), *PluginName, *Result.GetError());
	}
	else {
		ActivePluginURLs.Add(PluginName, PluginURL);
		UE_LOG(GemstarInterfaces, Log, TEXT("Activated Game Feature: %s"), *PluginName);
	}

	// Outside the branch: 
	//        a failed activation must release the gate, 
	//        or READY never fires
	FinishOneActivation();
}

void UGemstarGameFeatureSubsystem::FinishOneActivation()
{
	PendingActivations = FMath::Max(0, PendingActivations - 1);

	if (PendingActivations == 0) {
		OnStartupFeaturesReady.Broadcast();
	}
}
