/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarExperienceManagerComponent.h"

#include "Engine/AssetManager.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "Subsystems/GemstarSimulationSubsystem.h"
#include "Subsystems/GemstarExperienceDefinition.h"
#include "Utils/GemstarInterfacesLog.h"
#include "Subsystems/GemstarGameFeatureSubsystem.h"
#include "Subsystems/GemstarWorldSettings.h"
#include "Subsystems/BulkGameInstanceSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

UGemstarExperienceManagerComponent::UGemstarExperienceManagerComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGemstarExperienceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UBulkGameInstanceSubsystem* Bulk = GameInstance
		? GameInstance->GetSubsystem<UBulkGameInstanceSubsystem>() : nullptr;
	if (Bulk != nullptr && !Bulk->PrepareForWorld(World)) {
		return;
	}
	UGemstarGameFeatureSubsystem* Features = GameInstance ? GameInstance->GetSubsystem<UGemstarGameFeatureSubsystem>() : nullptr;

	if (Features == nullptr) {
		UE_LOG(GemstarInterfaces, Warning, TEXT("GEMSTAR: no GameFeature subsystem; starting simulation directly."));
		StartGemstarSimulation();
		return;
	}

	FeatureSubsystem = Features;
	Features->OnStartupFeaturesReady.AddUObject(this, &ThisClass::HandleFeaturesReady);

	// Everything an experience must do before features activate is data on the experience
	LoadState = EGemstarLoadState::WaitingForAssets;
	ActiveExperience = ResolveExperience(ExperienceSource);
	ApplyExperienceConfigOverrides();
	RequestExperiencePreloads();

	// The asset barrier sits above activation because activation is what injects the loaders that
	// spawn the managers: a scenario must not start on assets that are still loading.
	if (ArePendingLoadsComplete()) {
		ActivateFeaturesAndActions();
	}
}

void UGemstarExperienceManagerComponent::ApplyExperienceConfigOverrides()
{
	if (ActiveExperience == nullptr || ActiveExperience->ConfigOverrides.IsEmpty()) {
		return;
	}

	UGemstarSimulationSubsystem* Simulation = UGemstarSimulationSubsystem::Get(GetWorld());
	if (Simulation == nullptr) {
		return;
	}

	for (const FGemstarConfigOverride& Override : ActiveExperience->ConfigOverrides) {
		Simulation->OverrideConfiguration(Override.Section, Override.Key, Override.Value);
	}
	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR experience '%s' applied %d configuration override(s)."),
	       *ActiveExperience->GetName(), ActiveExperience->ConfigOverrides.Num());
}

void UGemstarExperienceManagerComponent::RequestExperiencePreloads()
{
	if (ActiveExperience == nullptr || ActiveExperience->PreloadAssets.IsEmpty()) {
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(ActiveExperience->PreloadAssets.Num());
	
	for (const TSoftObjectPtr<UObject>& Asset : ActiveExperience->PreloadAssets) {

		if (!Asset.IsNull()) {
			Paths.Add(Asset.ToSoftObjectPath());
		}
	}
	if (Paths.IsEmpty()) {
		return;
	}

	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR preload: requested %d asset(s) for experience '%s'."),
	       Paths.Num(), *ActiveExperience->GetName());
	RegisterPendingLoad(UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths));
}

void UGemstarExperienceManagerComponent::HandlePreloadComplete()
{
	if (LoadState != EGemstarLoadState::WaitingForAssets) {
		return;
	}

	if (ArePendingLoadsComplete()) {
		ActivateFeaturesAndActions();
	}
}

void UGemstarExperienceManagerComponent::ActivateFeaturesAndActions()
{
	if (LoadState != EGemstarLoadState::WaitingForAssets || FeatureSubsystem == nullptr) {
		return;
	}
	LoadState = EGemstarLoadState::ActivatingFeatures;

	TArray<FString> Features;
	
	if (ActiveExperience != nullptr) {
		Features = ActiveExperience->ResolveFeatures();
		UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR experience '%s' (from %s) activates %d feature(s)."),
		       *ActiveExperience->GetName(), ExperienceSource, Features.Num());
	}

	FeatureSubsystem->ActivateFeatures(Features);
	ActivateExperienceActions();

	if (FeatureSubsystem->AreStartupFeaturesReady()) {
		StartGemstarSimulation();
	}
}

void UGemstarExperienceManagerComponent::HandleFeaturesReady()
{
	// Fires for whatever the subsystem is doing; only meaningful once this map has activated.
	if (LoadState == EGemstarLoadState::ActivatingFeatures) {
		StartGemstarSimulation();
	}
}

void UGemstarExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Actions run only after activation, so a map that stopped earlier has nothing to unwind
	// and the component's own preload handle is released by emptying PendingLoads below.
	if (LoadState == EGemstarLoadState::ActivatingFeatures || LoadState == EGemstarLoadState::Ready) {
		DeactivateExperienceActions();

		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr) {
			const UBulkGameInstanceSubsystem* Bulk = GameInstance->GetSubsystem<UBulkGameInstanceSubsystem>();

			if (UGemstarGameFeatureSubsystem* Features =
					GameInstance->GetSubsystem<UGemstarGameFeatureSubsystem>()) {
				if (Bulk == nullptr || !Bulk->IsScenarioTransitionPending()) {
					Features->DeactivateStartupFeatures();
				}
			}
		}
	}

	LoadState = EGemstarLoadState::Unloaded;
	PendingLoads.Empty();

	Super::EndPlay(EndPlayReason);
}

UGemstarExperienceDefinition* UGemstarExperienceManagerComponent::ResolveExperience(
	const TCHAR*& OutSource)
{
	// Three sources, most specific first.

	// 1. Command line, so a batch run needs no file or asset edits.
	FString CommandLineName;
	if (FParse::Value(FCommandLine::Get(), TEXT("GemstarExperience="), CommandLineName)) {

		if (UGemstarExperienceDefinition* Experience = FindExperienceByName(CommandLineName)) {
			OutSource = TEXT("command line");
			return Experience;
		}
		UE_LOG(GemstarInterfaces, Error,
		       TEXT("GEMSTAR experience '%s' from -GemstarExperience= could not be resolved."),
		       *CommandLineName);
	}

	// 2. Create experience from map's ini file entries. This enables full external control.
	FString ConfigFile = AGemstarWorldSettings::ConfigFor(GetWorld());
	if (ConfigFile != "") {
		FString BulkActiveFeature;
		FString BulkInactiveFeature;
		FConfigFile LocalConfig;
		if (!FPaths::FileExists(ConfigFile)) {
			ConfigFile = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + ConfigFile);
		}
		LocalConfig.Read(ConfigFile);
		if (!LocalConfig.IsEmpty()) {
			if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr) {
				if (const UBulkGameInstanceSubsystem* Bulk = GameInstance->GetSubsystem<UBulkGameInstanceSubsystem>()) {
					FString ScenarioPath;
					EBulkScenarioType ScenarioType;
					if (Bulk->GetActiveScenario(ScenarioPath, ScenarioType)) {
						BulkActiveFeature = ScenarioType == EBulkScenarioType::Sumo ? TEXT("ue_sumo_feature") : TEXT("ue_esmini_feature");
						BulkInactiveFeature = ScenarioType == EBulkScenarioType::Sumo ? TEXT("ue_esmini_feature") : TEXT("ue_sumo_feature");
						FConfigFile Sidecar;
						Sidecar.Read(FPaths::ChangeExtension(ScenarioPath, TEXT("ini")));
						TArray<FString> SectionNames;
						Sidecar.GetKeys(SectionNames);
						for (const FString& SectionName : SectionNames) {
							if (const FConfigSection* Section = Sidecar.FindSection(SectionName)) {
								LocalConfig.Add(SectionName, *Section);
							}
						}
					}
				}
			}
		}
		else {
			UE_LOG(GemstarInterfaces, Error, TEXT("GEMSTAR: Could not read experience configuration %s"), *ConfigFile);
		}

		if (!LocalConfig.IsEmpty()) {
			ActiveExperience = NewObject<UGemstarExperienceDefinition>();
			TObjectPtr<UGemstarExperienceSet> ExperienceSet = NewObject<UGemstarExperienceSet>();

			TArray<FString> SectionNames;
			LocalConfig.GetKeys(SectionNames);
			for (const FString& SectionName : SectionNames) {
				if (SectionName.Equals(TEXT("Gemstar"), ESearchCase::IgnoreCase)) {
					continue;
				}
				if (!BulkInactiveFeature.IsEmpty() && (SectionName.Equals(BulkInactiveFeature, ESearchCase::IgnoreCase) || SectionName.StartsWith(BulkInactiveFeature + TEXT(":"), ESearchCase::IgnoreCase))) {
					continue;
				}
				if (!SectionName.Contains(":")) {
					ExperienceSet->GameFeaturesToEnable.Add(SectionName);
				}
				else {
					FString FeatureName;
					FString InstanceName;
					SectionName.Split(":", &FeatureName, &InstanceName);
					ExperienceSet->GameFeaturesToEnable.Add(FeatureName);
				}
			}
			if (!BulkActiveFeature.IsEmpty()) {
				ExperienceSet->GameFeaturesToEnable.AddUnique(BulkActiveFeature);
			}
			ActiveExperience->ExperienceSets.Add(ExperienceSet);
			OutSource = TEXT(".ini file");
			return ActiveExperience;
		}
	}

	// 3. The map's own World Settings. A reference, so it cannot go stale like a name can.
	if (UGemstarExperienceDefinition* Experience = AGemstarWorldSettings::ExperienceFor(GetWorld())) {
		OutSource = TEXT("World Settings");
		return Experience;
	}

	UE_LOG(GemstarInterfaces, Error,
	       TEXT("GEMSTAR: no experience selected. Set World Settings > GEMSTAR > Default "
	            "Experience on this map, or pass -GemstarExperience=<name>."));

	return nullptr;
}

UGemstarExperienceDefinition* UGemstarExperienceManagerComponent::FindExperienceByName(const FString& Name)
{
	const FPrimaryAssetId ExperienceId(UGemstarExperienceDefinition::PrimaryAssetType, FName(*Name));
	UAssetManager& AssetManager = UAssetManager::Get();

	if (UGemstarExperienceDefinition* Resident =
			Cast<UGemstarExperienceDefinition>(AssetManager.GetPrimaryAssetObject(ExperienceId))){
		return Resident;
	}
	const FSoftObjectPath ExperiencePath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	return Cast<UGemstarExperienceDefinition>(ExperiencePath.TryLoad());
}

void UGemstarExperienceManagerComponent::ActivateExperienceActions()
{
	if (ActiveExperience == nullptr || ActiveExperience->Actions.IsEmpty()) {
		return;
	}

	FGameFeatureActivatingContext Context;

	if (const FWorldContext* WorldContext = GetWorld()->GetGameInstance()->GetWorldContext()) {
		Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
	}

	int32 ActionCount = 0;

	for (UGameFeatureAction* Action : ActiveExperience->Actions) {

		if (Action == nullptr) {
			continue; // IsDataValid flags these in the editor; do not crash a run over one.
		}

		// All three phases together, after activation: an action is either whole or absent.
		Action->OnGameFeatureRegistering();
		Action->OnGameFeatureLoading();
		Action->OnGameFeatureActivating(Context);
		++ActionCount;
	}

	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR experience '%s' ran %d action(s)."),
	       *ActiveExperience->GetName(), ActionCount);
}

void UGemstarExperienceManagerComponent::DeactivateExperienceActions()
{
	if (ActiveExperience == nullptr) {
		return;
	}

	FGameFeatureDeactivatingContext Context(
		ActiveExperience->GetName(),
		[](FStringView PauserTag)
		{
			UE_LOG(GemstarInterfaces, Verbose, TEXT("GEMSTAR: experience action paused deactivation: %s"),
			       *FString(PauserTag));
		});

	if (const FWorldContext* WorldContext = GetWorld() && GetWorld()->GetGameInstance()
			? GetWorld()->GetGameInstance()->GetWorldContext() : nullptr) {
		Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
	}

	for (int32 i = ActiveExperience->Actions.Num() - 1; i >= 0; --i) {

		if (UGameFeatureAction* Action = ActiveExperience->Actions[i]) {
			Action->OnGameFeatureDeactivating(Context);
			Action->OnGameFeatureUnloading();
			Action->OnGameFeatureUnregistering();
		}
	}

	ActiveExperience = nullptr;
}

void UGemstarExperienceManagerComponent::RegisterPendingLoad(TSharedPtr<FStreamableHandle> Handle)
{
	// A resident asset never fires the completion delegate, so an already-loaded handle is not
	// something to wait for.
	if (!Handle.IsValid() || Handle->HasLoadCompleted()) {
		return;
	}

	PendingLoads.Add(Handle);
	Handle->BindCompleteDelegate(
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandlePreloadComplete));
}

bool UGemstarExperienceManagerComponent::ArePendingLoadsComplete()
{
	for (int32 i = PendingLoads.Num() - 1; i >= 0; --i) {

		if (!PendingLoads[i].IsValid()) {
			PendingLoads.RemoveAt(i);
			continue;
		}

		// A cancelled load leaves content missing. Measurements taken on partial content look
		// valid and are not, so this stops activation instead of degrading quietly.
		if (PendingLoads[i]->WasCanceled()) {
			TArray<FSoftObjectPath> Requested;
			PendingLoads[i]->GetRequestedAssets(Requested);
			UE_LOG(GemstarInterfaces, Error,
				TEXT("GEMSTAR: preload cancelled for experience '%s' (%d asset(s), first %s). "
					"Features are NOT activated and this map will not run."),
				ActiveExperience ? *ActiveExperience->GetName() : TEXT("none"),
				Requested.Num(),
				Requested.IsEmpty() ? TEXT("unnamed") : *Requested[0].ToString());
			LoadState = EGemstarLoadState::Failed;
			PendingLoads.Empty();
			return false;
		}

		if (PendingLoads[i]->HasLoadCompleted()) {
			PendingLoads.RemoveAt(i);
		}
	}

	if (PendingLoads.IsEmpty()) {
		return true;
	}

	TArray<FSoftObjectPath> Waiting;
	PendingLoads[0]->GetRequestedAssets(Waiting);
	UE_LOG(GemstarInterfaces, Log,
		TEXT("GEMSTAR: holding activation for %d preload request(s); first is %s."),
		PendingLoads.Num(),
		Waiting.IsEmpty() ? TEXT("unnamed") : *Waiting[0].ToString());
	return false;
}

void UGemstarExperienceManagerComponent::StartGemstarSimulation()
{
	if (LoadState == EGemstarLoadState::Ready) {
		return;
	}
	LoadState = EGemstarLoadState::Ready;
	UE_LOG(GemstarInterfaces, Log, TEXT("##### GEMSTAR IS READY FOR SIMULATION! #####"));

	if (UGemstarSimulationSubsystem* Simulation = UGemstarSimulationSubsystem::Get(GetWorld())) {
		Simulation->ReportUnregisteredManagers();
	}
}
