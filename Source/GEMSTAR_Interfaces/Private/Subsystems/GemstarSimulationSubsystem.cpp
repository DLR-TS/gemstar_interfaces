/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarSimulationSubsystem.h"

#include "Interfaces/IPluginManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "Utils/GemstarInterfacesLog.h"
#include "Subsystems/GemstarWorldSettings.h"
#include "Subsystems/BulkGameInstanceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Utils/LocalTransformUpdater.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

UGemstarSimulationSubsystem* UGemstarSimulationSubsystem::Get(const UWorld* World)
{
	return World ? World->GetSubsystem<UGemstarSimulationSubsystem>() : nullptr;
}

bool UGemstarSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) {
		return false;
	}
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && World->IsGameWorld();
}

FString UGemstarSimulationSubsystem::PluginNameOf(const AActor* Manager)
{
	if (Manager == nullptr || Manager->GetClass() == nullptr) {
		return FString();
	}

	const FString Module = FPackageName::GetShortName(Manager->GetClass()->GetOutermost()->GetName());

	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(*Module))) {
		return Plugin->GetName();
	}
	return FString();
}

void UGemstarSimulationSubsystem::ReportClaim(const FString& PluginName, const FString& Identifier)
{
	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR registered manager: %s as \"%s\""),
		PluginName.IsEmpty() ? TEXT("<no plugin>") : *PluginName, *Identifier);

	if (PluginName.IsEmpty() || Identifier.IsEmpty()) {
		return;
	}

	FString Subject = PluginName;
	Subject.RemoveFromStart(TEXT("ue_"));
	Subject.RemoveFromEnd(TEXT("_feature"));

	if (Subject.Contains(Identifier, ESearchCase::IgnoreCase) ||
		Identifier.Contains(Subject, ESearchCase::IgnoreCase)) {
		return;
	}

	UE_LOG(GemstarInterfaces, Warning,
		TEXT("GEMSTAR: plugin '%s' claims configuration \"%s\" -- the two names have no visible "
			"relationship. Intended, or has one of them drifted?"),
		*PluginName, *Identifier);
}

void UGemstarSimulationSubsystem::ReportUnregisteredManagers()
{
	UWorld* World = GetWorld();

	if (World == nullptr) {
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It) {
		AActor* Candidate = *It;

		if (!Candidate->Implements<UGemstarManager>()) {
			continue;
		}

		const bool bRegistered = Managers.ContainsByPredicate(
			[Candidate](const FRegisteredManager& Registered) { return Registered.Manager == Candidate; });

		if (bRegistered) {
			continue;
		}

		UE_LOG(GemstarInterfaces, Warning,
			TEXT("GEMSTAR: %s is a manager but never registered -- it holds no configuration and "
				"no dataflow can reach it. A manager placed in the map is not the one the feature "
				"spawns; remove it, or have whatever placed it call Configure."),
			*Candidate->GetName());
	}
}

bool UGemstarSimulationSubsystem::ReportScenarioFinished(AActor* Manager)
{
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UBulkGameInstanceSubsystem* Bulk = GameInstance
		? GameInstance->GetSubsystem<UBulkGameInstanceSubsystem>() : nullptr;
	if (Manager == nullptr || Bulk == nullptr || !Bulk->IsActive()) {
		return false;
	}

	Bulk->OnScenarioFinished();
	return true;
}

TMap<FString, FInnerConfig> UGemstarSimulationSubsystem::RegisterManagerAndGetConfiguration(
	AActor* Manager, FString Identifier)
{
	const FString PluginName = PluginNameOf(Manager);
	Managers.Add({ Manager, Identifier, PluginName });
	ReportClaim(PluginName, Identifier);
	LoadIni();
	TMap<FString, FInnerConfig> Configurations = GetConfigurations(Identifier);

	// New arrival may be the producer or mount target something pending waits for
	TryResolvePending(false);

	return Configurations;
}

void UGemstarSimulationSubsystem::UnregisterManager(AActor* Manager)
{
	if (Manager == nullptr) {
		return;
	}

	for (int32 i = ResolvedDataFlows.Num() - 1; i >= 0; --i) {
		FResolvedDataFlow& Flow = ResolvedDataFlows[i];
		AActor* Producer = Flow.Producer.Get();

		if (Flow.DataTarget.Actor == Manager) {
			if (Producer != nullptr && Producer != Manager) {
				IGemstarManager::Execute_UnregisterDataflow(Producer, Flow.DataTarget, Flow.InterfaceName);
			}
			ResolvedDataFlows.RemoveAt(i);
		}
		// Departing or already-gone producer takes its receiver lists with it
		else if (Producer == Manager || Producer == nullptr) {
			ResolvedDataFlows.RemoveAt(i);
		}
	}

	// Pending flows too, or the retry loop polls a destroyed actor forever
	for (int32 i = CachedDataFlows.Num() - 1; i >= 0; --i) {
		if (CachedDataFlows[i].DataTarget.Actor == Manager) {
			CachedDataFlows.RemoveAt(i);
		}
	}

	for (int32 i = CachedMounts.Num() - 1; i >= 0; --i) {
		if (CachedMounts[i].Actor == Manager) {
			CachedMounts.RemoveAt(i);
		}
	}

	Managers.RemoveAll([Manager](const FRegisteredManager& Registered)
		{ return Registered.Manager == Manager; });
	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR unregistered manager: %s"), *Manager->GetName());
}

bool UGemstarSimulationSubsystem::RegisterDataflow_Implementation(FDataTarget& dataTarget,
	FString& producer, GemstarInterfaceType interfaceName)
{
	CachedDataFlows.Add({ dataTarget, producer, interfaceName, false });
	TryResolvePending(false);
	return true;
}

bool UGemstarSimulationSubsystem::MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor)
{
	CachedMounts.Add({ NameByConfiguration, Actor, false });
	TryResolvePending(false);
	return true;
}

void UGemstarSimulationSubsystem::TryResolvePending(bool bReportStragglers)
{
	ConfigureDataFlow(bReportStragglers);
	ConfigureMounts(bReportStragglers);
}

bool UGemstarSimulationSubsystem::IsTickable() const
{
	return CachedDataFlows.Num() > 0 || CachedMounts.Num() > 0;
}

void UGemstarSimulationSubsystem::Tick(float DeltaTime)
{
	// Straggler reporting starts on the second retry tick, once every first-frame spawn had its chance
	++TicksSinceFirstManager;
	TryResolvePending(TicksSinceFirstManager > 1);
}

bool UGemstarSimulationSubsystem::ConfigureDataFlow(bool bReportStragglers)
{
	bool success = false;
	for (int32 i = CachedDataFlows.Num() - 1; i >= 0; --i) {
		FDataFlowCacheEntry& flow = CachedDataFlows[i];
		success = false;

		for (const FRegisteredManager& Registered : Managers) {
			AActor* manager = Registered.Manager;
			success = IGemstarManager::Execute_RegisterDataflow(manager, flow.DataTarget, flow.Producer, flow.InterfaceName);

			if (success) {
				UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR registered dataflow: from %s to %s%s"),
					*flow.Producer, *flow.DataTarget.Actor->GetName(),
					flow.bReported ? TEXT(" (resolved after retry)") : TEXT(""));
				ResolvedDataFlows.Add({ flow.DataTarget, flow.InterfaceName, manager });
				CachedDataFlows.RemoveAt(i);
				break;
			}
		}

		if (!success && bReportStragglers && !flow.bReported) {
			flow.bReported = true;
			UE_LOG(GemstarInterfaces, Error,
				TEXT("GEMSTAR dataflow not set up yet: from %s to %s. Retrying silently every "
					"tick. If it never resolves, the producer's feature is not active -- add it "
					"to the experience or [GameFeatures] in the configuration this map reads."),
				*flow.Producer, *flow.DataTarget.Actor->GetName());
		}
	}
	return CachedDataFlows.Num() == 0;
}

bool UGemstarSimulationSubsystem::ConfigureMounts(bool bReportStragglers)
{
	bool success = false;
	for (int32 i = CachedMounts.Num() - 1; i >= 0; --i) {
		FMountOnCacheEntry& mount = CachedMounts[i];
		success = false;

		for (const FRegisteredManager& Registered : Managers) {
			AActor* manager = Registered.Manager;
			success = IGemstarManager::Execute_MountOn(manager, mount.NameByConfiguration, mount.Actor);
			if (success) {
				UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR mounted: %s on %s%s"),
					*mount.Actor->GetName(), *mount.NameByConfiguration,
					mount.bReported ? TEXT(" (resolved after retry)") : TEXT(""));

				// `mount` is a reference INTO CachedMounts; take what is needed before RemoveAt.
				AActor* const MountedActor = mount.Actor;
				CachedMounts.RemoveAt(i);
				ILocalTransformUpdater::Execute_UpdateTransformAfterAttachment(MountedActor);
				break;
			}
		}

		if (!success && bReportStragglers && !mount.bReported) {
			mount.bReported = true;
			UE_LOG(GemstarInterfaces, Log,
				TEXT("GEMSTAR can not mount yet: %s on %s, Retrying silently every tick"),
				*mount.Actor->GetName(), *mount.NameByConfiguration);
		}
	}
	return CachedMounts.Num() == 0;
}

void UGemstarSimulationSubsystem::SetIniFile(const FString& IniFile)
{
	GemstarConfiguration = IniFile;
	IniLoaded = false;
}

const FString& UGemstarSimulationSubsystem::GetConfigurationPath()
{
	LoadIni();
	return GemstarConfiguration;
}

FString UGemstarSimulationSubsystem::ResolveConfiguredPath() const
{
	FString Path;

	if (const AGemstarWorldSettings* Settings =
		GetWorld() ? Cast<AGemstarWorldSettings>(GetWorld()->GetWorldSettings()) : nullptr) {
		Path = Settings->GemstarConfiguration;
	}

	if (Path.IsEmpty()) {
		UE_LOG(GemstarInterfaces, Warning,
			TEXT("GEMSTAR: map sets no configuration path in World Settings; using the project "
				"default."));
	}

	if (Path.IsEmpty()) {
		Path = TEXT("Simulation/DefaultGemstar.ini");
	}

	if (FPaths::IsRelative(Path)) {
		Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + Path);
	}
	return Path;
}

void UGemstarSimulationSubsystem::RecoverForeignPath()
{
	if (FPaths::FileExists(GemstarConfiguration)) {
		return;
	}

	FString Normalised = GemstarConfiguration;
	Normalised.ReplaceInline(TEXT("\\"), TEXT("/"));

	static const TCHAR* Anchors[] = { TEXT("Plugins/"), TEXT("Config/") };

	for (const TCHAR* Anchor : Anchors) {
		const int32 Index = Normalised.Find(Anchor, ESearchCase::IgnoreCase, ESearchDir::FromStart);

		if (Index == INDEX_NONE) {
			continue;
		}

		const FString Relative = Normalised.RightChop(Index);
		const FString Candidate = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + Relative);

		if (!FPaths::FileExists(Candidate)) {
			continue;
		}

		UE_LOG(GemstarInterfaces, Warning,
			TEXT("GEMSTAR configuration path was saved on another machine: %s. Recovered it, but "
				"fix the map: set World Settings > GEMSTAR > Gemstar Configuration to '%s'."),
			*GemstarConfiguration, *Relative);

		GemstarConfiguration = Candidate;
		return;
	}
}

void UGemstarSimulationSubsystem::LoadIni()
{
	if (IniLoaded) {
		return;
	}

	if (GemstarConfiguration.IsEmpty()) {
		GemstarConfiguration = ResolveConfiguredPath();
	}
	else if (FPaths::IsRelative(GemstarConfiguration)) {
		GemstarConfiguration = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + GemstarConfiguration);
	}
	RecoverForeignPath();

	UE_LOG(GemstarInterfaces, Log, TEXT("Load GEMSTAR configuration from: %s"), *GemstarConfiguration);

	simulationConfiguration.Empty();

	//Feature-shipped defaults first, so the scenario ini holds only overrides
	//Commented method out, because behavior adds inputs even if no input is definded in scenario ini
	//MergeFeatureDefaults();

	FConfigFile LocalConfig;
	LocalConfig.Read(GemstarConfiguration);

	if (LocalConfig.IsEmpty()) {
		UE_LOG(GemstarInterfaces, Error, TEXT("Could not read GEMSTAR configuration from: %s"), *GemstarConfiguration);
	}

	MergeConfigFile(LocalConfig);

	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr) {
		if (const UBulkGameInstanceSubsystem* Bulk = GameInstance->GetSubsystem<UBulkGameInstanceSubsystem>()) {
			FString ScenarioPath;
			EBulkScenarioType ScenarioType;
			if (Bulk->GetActiveScenario(ScenarioPath, ScenarioType)) {
				FConfigFile Sidecar;
				Sidecar.Read(FPaths::ChangeExtension(ScenarioPath, TEXT("ini")));
				if (!Sidecar.IsEmpty()) {
					MergeConfigFile(Sidecar);
				}

				const TCHAR* InactiveFeature = ScenarioType == EBulkScenarioType::Sumo
					? TEXT("ue_esmini_feature") : TEXT("ue_sumo_feature");
				for (auto It = simulationConfiguration.CreateIterator(); It; ++It) {
					if (It.Key().Equals(InactiveFeature, ESearchCase::IgnoreCase) ||
						It.Key().StartsWith(FString(InactiveFeature) + TEXT(":"), ESearchCase::IgnoreCase)) {
						It.RemoveCurrent();
					}
				}

				const TCHAR* ActiveFeature = ScenarioType == EBulkScenarioType::Sumo
					? TEXT("ue_sumo_feature") : TEXT("ue_esmini_feature");
				FInnerConfig& Player = simulationConfiguration.FindOrAdd(ActiveFeature);
				if (ScenarioType == EBulkScenarioType::Sumo) {
					Player.Entries.Add(TEXT("Parameters"), FString::Printf(TEXT("-c \"%s\""), *ScenarioPath));
				}
				else {
					Player.Entries.Add(TEXT("Path"), ScenarioPath);
				}
			}
		}
	}
	IniLoaded = true;
}

void UGemstarSimulationSubsystem::MergeConfigFile(const FConfigFile& Config)
{
	for (const TPair<FString, FConfigSection>& SectionPair : Config) {
		const FString& SectionName = SectionPair.Key;
		const FConfigSection& Section = SectionPair.Value;

		FInnerConfig& InnerConfig = simulationConfiguration.FindOrAdd(SectionName);

		for (const TPair<FName, FConfigValue>& Entry : Section) {
			const FString Key = Entry.Key.ToString();
			const FString Value = Entry.Value.GetValue();

			InnerConfig.Entries.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
		}
	}
}

void UGemstarSimulationSubsystem::ForEachFeatureDefaults(
	TFunctionRef<void(const FString&, const FConfigFile&)> Visit)
{
	TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetEnabledPlugins();
	Plugins.Sort([](const TSharedRef<IPlugin>& A, const TSharedRef<IPlugin>& B)
		{ return A->GetName() < B->GetName(); });

	for (const TSharedRef<IPlugin>& Plugin : Plugins) {
		const FString DefaultsPath = Plugin->GetBaseDir() / TEXT("Config/FeatureDefaults.ini");

		if (!FPaths::FileExists(DefaultsPath)) {
			continue;
		}

		FConfigFile Defaults;
		Defaults.Read(DefaultsPath);
		Visit(Plugin->GetName(), Defaults);
	}
}

TMap<FString, TArray<FString>> UGemstarSimulationSubsystem::FeatureTagsByPlugin()
{
	TMap<FString, TArray<FString>> TagsByPlugin;

	ForEachFeatureDefaults([&TagsByPlugin](const FString& PluginName, const FConfigFile& Defaults)
		{
			const FConfigSection* Section = Defaults.FindSection(TEXT("gemstar.feature"));

			if (Section == nullptr) {
				return;
			}
			const FConfigValue* Declared = Section->Find(TEXT("JoinsExperiencesTagged"));

			if (Declared == nullptr) {
				return;
			}

			TArray<FString> Tags;
			Declared->GetValue().ParseIntoArray(Tags, TEXT(","), true);

			for (FString& Tag : Tags) {
				Tag.TrimStartAndEndInline();
			}

			if (Tags.Num() > 0) {
				TagsByPlugin.Add(PluginName, MoveTemp(Tags));
			}
		});

	return TagsByPlugin;
}

void UGemstarSimulationSubsystem::MergeFeatureDefaults()
{
	ForEachFeatureDefaults([this](const FString& PluginName, const FConfigFile& Defaults)
		{
			MergeConfigFile(Defaults);
			UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR defaults: %s (%d section(s))"),
				*PluginName, Defaults.Num());
		});
}

TMap<FString, FInnerConfig> UGemstarSimulationSubsystem::GetConfigurations(const FString& Name)
{
	TMap<FString, FInnerConfig> configurations;

	for (const TPair<FString, FInnerConfig>& Pair : simulationConfiguration) {
		if (Pair.Key.Contains(Name)) {
			configurations.Add(Pair.Key, Pair.Value);
		}
	}

	if (configurations.Num() == 0) {
		UE_LOG(GemstarInterfaces, Warning, TEXT("Maybe ok, but no configuration found for: %s"), *Name);
	}
	return configurations;
}


void UGemstarSimulationSubsystem::OverrideConfiguration(
	const FString& Section, const FString& Key, const FString& Value)
{
	// LoadIni first: the first load clears the map, so an earlier override would be erased
	LoadIni();

	FInnerConfig& Inner = simulationConfiguration.FindOrAdd(Section);
	Inner.Entries.Add(Key, Value);

	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR config override: [%s] %s = %s"), *Section, *Key, *Value);
}
