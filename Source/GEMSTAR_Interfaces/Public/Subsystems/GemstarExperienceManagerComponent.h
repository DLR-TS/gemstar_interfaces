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

#include "Components/GameStateComponent.h"
#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "GemstarExperienceManagerComponent.generated.h"

class UGemstarExperienceDefinition;
class UGemstarGameFeatureSubsystem;

UENUM()
enum class EGemstarLoadState : uint8
{
	Unloaded,
	WaitingForAssets,     // preload requested by an experience action; nothing has activated yet
	ActivatingFeatures,   // plugins activating; managers spawn here
	Ready,                // READY logged
	Failed                // a preload was cancelled; features deliberately not activated
};

UCLASS()
class GEMSTAR_INTERFACES_API UGemstarExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UGemstarExperienceManagerComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	// Manager selects its own experience. Two sources, both assets.
	UGemstarExperienceDefinition* ResolveExperience(const TCHAR*& OutSource);
	static UGemstarExperienceDefinition* FindExperienceByName(const FString& Name);

	void ApplyExperienceConfigOverrides();   // experience data, before features read configuration
	void RequestExperiencePreloads();        // experience data, before anything activates
	void RegisterPendingLoad(TSharedPtr<FStreamableHandle> Handle);
	bool ArePendingLoadsComplete();          // the asset barrier, above activation
	void HandlePreloadComplete();
	void ActivateFeaturesAndActions();
	void HandleFeaturesReady();
	void StartGemstarSimulation();

	void ActivateExperienceActions();
	void DeactivateExperienceActions();

	UPROPERTY()
	TObjectPtr<UGemstarExperienceDefinition> ActiveExperience;

	const TCHAR* ExperienceSource = TEXT("none");

	UPROPERTY()
	TObjectPtr<UGemstarGameFeatureSubsystem> FeatureSubsystem;

	TArray<TSharedPtr<FStreamableHandle>> PendingLoads;

	EGemstarLoadState LoadState = EGemstarLoadState::Unloaded;
};