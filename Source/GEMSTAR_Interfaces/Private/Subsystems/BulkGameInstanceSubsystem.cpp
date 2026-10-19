/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/BulkGameInstanceSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Utils/GemstarInterfacesLog.h"
#include "Subsystems/GemstarSimulationSubsystem.h"
#include "Subsystems/GemstarGameFeatureSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "XmlFile.h"

void UBulkGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString Folder;
	FString DefaultLevelPath;
	if (!ReadBulkConfiguration(Folder, DefaultLevelPath)) {
		return;
	}

	DiscoverMaps();
	DiscoverScenarios(Folder, DefaultLevelPath);
	if (Scenarios.IsEmpty()) {
		UE_LOG(GemstarInterfaces, Warning,
			TEXT("GEMSTAR bulk: no .xosc or .sumocfg files found in %s."), *Folder);
		return;
	}

	bActive = true;
}

void UBulkGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UBulkGameInstanceSubsystem::GetActiveScenario(FString& OutPath, EBulkScenarioType& OutType) const
{
	if (!bActive || !Scenarios.IsValidIndex(CurrentScenario)) {
		return false;
	}

	const FBulkScenario& Scenario = Scenarios[CurrentScenario];
	OutPath = Scenario.FilePath;
	OutType = Scenario.Type;
	return true;
}

bool UBulkGameInstanceSubsystem::PrepareForWorld(UWorld* World)
{
	if (!bActive || World == nullptr) {
		return true;
	}

	const FString CurrentLevel = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
	if (CurrentScenario == INDEX_NONE) {
		if (DefaultLevel.IsNull()) {
			DefaultLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(CurrentLevel));
		}
		CurrentScenario = 0;
	}

	const FBulkScenario& Scenario = Scenarios[CurrentScenario];
	const TSoftObjectPtr<UWorld>& Target = Scenario.Level.IsNull() ? DefaultLevel : Scenario.Level;

	if (FName(*Target.ToSoftObjectPath().GetLongPackageName()) != FName(*CurrentLevel)) {
		UGameplayStatics::OpenLevel(World, FName(*Target.ToSoftObjectPath().GetLongPackageName()));
		return false;
	}
	return true;
}

bool UBulkGameInstanceSubsystem::ReadBulkConfiguration(FString& OutFolder, FString& OutDefaultLevel)
{
	FConfigFile Config;
	Config.Read(FPaths::ProjectDir() / TEXT("Simulation/DefaultGemstar.ini"));
	if (Config.IsEmpty()) {
		return false;
	}

	const FConfigSection* Bulk = Config.FindSection(TEXT("Gemstar"));
	const FConfigValue* Path = Bulk ? Bulk->Find(TEXT("BulkPath")) : nullptr;
	if (Path == nullptr) {
		return false;
	}

	OutFolder = Path->GetValue().TrimStartAndEnd();
	if (OutFolder.IsEmpty()) {
		return false;
	}
	if (FPaths::IsRelative(OutFolder)) {
		OutFolder = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / OutFolder);
	}
	FPaths::NormalizeDirectoryName(OutFolder);
	if (!IFileManager::Get().DirectoryExists(*OutFolder)) {
		UE_LOG(GemstarInterfaces, Error, TEXT("GEMSTAR bulk: scenario folder does not exist: %s"), *OutFolder);
		return false;
	}

	if (const FConfigValue* Default = Bulk->Find(TEXT("DefaultLevel"))) {
		OutDefaultLevel = Default->GetValue().TrimStartAndEnd();
		DefaultLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(OutDefaultLevel));
	}
	if (DefaultLevel.IsNull() && GetWorld() != nullptr) {
		DefaultLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName())));
	}
	return true;
}

void UBulkGameInstanceSubsystem::DiscoverMaps()
{
	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> Assets;
	AssetRegistry.Get().GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), Assets, true);

	for (const FAssetData& Asset : Assets) {
		Maps.Add(Asset.AssetName.ToString().ToLower(),
			TSoftObjectPtr<UWorld>(Asset.ToSoftObjectPath()));
	}
}

void UBulkGameInstanceSubsystem::DiscoverScenarios(const FString& Folder, const FString& DefaultLevelPath)
{
	if (!DefaultLevelPath.IsEmpty()) {
		DefaultLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(DefaultLevelPath));
	}

	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Folder, TEXT("*.*"), true, false);
	Files.Sort([](const FString& A, const FString& B) { return A < B; });

	for (const FString& File : Files) {
		const FString Extension = FPaths::GetExtension(File, true).ToLower();
		EBulkScenarioType Type;
		if (Extension == TEXT(".xosc")) {
			Type = EBulkScenarioType::Esmini;
		}
		else if (Extension == TEXT(".sumocfg")) {
			Type = EBulkScenarioType::Sumo;
		}
		else {
			continue;
		}

		FBulkScenario& Scenario = Scenarios.AddDefaulted_GetRef();
		Scenario.FilePath = FPaths::ConvertRelativePathToFull(File);
		Scenario.Type = Type;
		FXmlFile XmlFile(Scenario.FilePath, EConstructMethod::ConstructFromFile);
		if (!XmlFile.IsValid()) {
			UE_LOG(GemstarInterfaces, Warning,
				TEXT("GEMSTAR bulk: could not parse %s: %s. Using the default level."),
				*Scenario.FilePath, *XmlFile.GetLastError());
			continue;
		}

		const FXmlNode* Root = XmlFile.GetRootNode();
		FString MapFile;
		if (Type == EBulkScenarioType::Sumo) {
			const FXmlNode* Network = Root ? Root->FindChildNode(TEXT("input")) : nullptr;
			const FXmlNode* Reference = Network ? Network->FindChildNode(TEXT("net-file")) : nullptr;
			MapFile = Reference ? Reference->GetAttribute(TEXT("value")).TrimStartAndEnd() : FString();
		}
		else {
			const FXmlNode* Network = Root ? Root->FindChildNode(TEXT("RoadNetwork")) : nullptr;
			const FXmlNode* Reference = Network ? Network->FindChildNode(TEXT("LogicFile")) : nullptr;
			MapFile = Reference ? Reference->GetAttribute(TEXT("filepath")).TrimStartAndEnd() : FString();
		}

		if (MapFile.IsEmpty()) {
			if (Type == EBulkScenarioType::Esmini) {
				UE_LOG(GemstarInterfaces, Warning,
					TEXT("GEMSTAR bulk: no logical map reference in %s. Skip this scenario."), *Scenario.FilePath);
				Scenarios.Pop();
				continue;
			}
			UE_LOG(GemstarInterfaces, Warning,
				TEXT("GEMSTAR bulk: no logical map reference in %s. Using the default level."), *Scenario.FilePath);
			continue;
		}

		FPaths::NormalizeFilename(MapFile);
		FString Stem = FPaths::GetBaseFilename(MapFile).ToLower();
		if (Type == EBulkScenarioType::Sumo && MapFile.EndsWith(TEXT(".net.xml"), ESearchCase::IgnoreCase)) {
			Stem.RemoveFromEnd(TEXT(".net"));
		}
		if (const TSoftObjectPtr<UWorld>* Map = Maps.Find(Stem)) {
			Scenario.Level = *Map;
		}
	}
}

void UBulkGameInstanceSubsystem::LoadNextLevel()
{
	++CurrentScenario;
	if (!Scenarios.IsValidIndex(CurrentScenario)) {
		bActive = false;
		UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR bulk: all scenarios finished."));
		if (GIsEditor)
		{
			// Running inside Unreal Editor
			UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("Exit"));
		}
		else
		{
			// Running as packaged game or standalone
			UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
		}
		return;
	}

	const FBulkScenario& Scenario = Scenarios[CurrentScenario];
	const TSoftObjectPtr<UWorld>& Target = Scenario.Level.IsNull() ? DefaultLevel : Scenario.Level;
	TravelToMap(Target);
}

void UBulkGameInstanceSubsystem::TravelToMap(const TSoftObjectPtr<UWorld>& Map)
{
	if (Map.IsNull()) {
		UE_LOG(GemstarInterfaces, Log,
			TEXT("GEMSTAR bulk: no matching or default level for %s."), *Scenarios[CurrentScenario].FilePath);
		LoadNextLevel();
		return;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName(*Map.ToSoftObjectPath().GetLongPackageName()));
}

void UBulkGameInstanceSubsystem::OnScenarioFinished()
{
	if (!bActive || bScenarioTransitionPending) {
		return;
	}

	bScenarioTransitionPending = true;
	UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR bulk: scenario finished, deactivating features before loading the next scenario."));
	UGameInstance* GameInstance = GetGameInstance();
	UGemstarGameFeatureSubsystem* Features = GameInstance
		? GameInstance->GetSubsystem<UGemstarGameFeatureSubsystem>() : nullptr;
	if (Features) {
		Features->DeactivateStartupFeatures(FSimpleDelegate::CreateWeakLambda(this, [this]() {
			bScenarioTransitionPending = false;
			LoadNextLevel();
		}));
	}
	else {
		bScenarioTransitionPending = false;
		LoadNextLevel();
	}
}
