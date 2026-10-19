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
#include "Interfaces/DataTarget.h"
#include "GemstarManager.h"
#include "Utils/GemstarInnerConfig.h"
#include "Subsystems/WorldSubsystem.h"
#include "GemstarSimulationSubsystem.generated.h"

struct FDataFlowCacheEntry {
	FDataTarget DataTarget;
	FString Producer;
	GemstarInterfaceType InterfaceName;
	bool bReported = false;
};

// Flow ConfigureDataFlow wired: UnregisterManager unwires exactly these
struct FResolvedDataFlow {
	FDataTarget DataTarget;
	GemstarInterfaceType InterfaceName;
	TWeakObjectPtr<AActor> Producer;
};

struct FMountOnCacheEntry {
	FString NameByConfiguration;
	AActor* Actor;
	bool bReported = false;
};

struct FRegisteredManager {
	AActor* Manager = nullptr;
	FString Identifier;
	FString PluginName;
};

// turns feature names into plugin activations
// tracks pending count
// fires OnStartupFeatureReady
// deactivates on teardown
UCLASS()
class GEMSTAR_INTERFACES_API UGemstarSimulationSubsystem : public UTickableWorldSubsystem, public IGemstarManager
{
	GENERATED_BODY()

public:
	static UGemstarSimulationSubsystem* Get(const UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	TMap<FString, FInnerConfig> RegisterManagerAndGetConfiguration(AActor* Manager, FString Identifier);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void UnregisterManager(AActor* Manager);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void SetIniFile(const FString& IniFile);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void LoadIni();

	// The path actually loaded, for messages that have to name it
	const FString& GetConfigurationPath();

	static FString PluginNameOf(const AActor* Manager);

	static void ForEachFeatureDefaults(TFunctionRef<void(const FString&, const FConfigFile&)> Visit);

	static TMap<FString, TArray<FString>> FeatureTagsByPlugin();

	void ReportUnregisteredManagers();
	bool ReportScenarioFinished(AActor* Manager);

private:
	// Prints the claim, and warns when the two names have no visible relationship
	static void ReportClaim(const FString& PluginName, const FString& Identifier);

public:

	// Applied by UGemstarActionConfigOverride before features read their configuration
	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void OverrideConfiguration(const FString& Section, const FString& Key, const FString& Value);

	// Receiving ends of ConnectInputs and mount requests; cached until a manager satisfies them
	virtual bool RegisterDataflow_Implementation(FDataTarget& dataTarget, FString& producer,
		GemstarInterfaceType interfaceName) override;
	virtual bool MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor) override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UGemstarSimulationSubsystem, STATGROUP_Tickables);
	}

private:
	// Ordering-free by design: every registration immediately tries to satisfy what is pending,
	// so first-frame resolution does not depend on tick scheduling. The tick is only a retry net
	void TryResolvePending(bool bReportStragglers);
	bool ConfigureDataFlow(bool bReportStragglers);
	bool ConfigureMounts(bool bReportStragglers);

	// World Settings first, the project default last
	FString ResolveConfiguredPath() const;
	void RecoverForeignPath();

	TMap<FString, FInnerConfig> GetConfigurations(const FString& Name);
	void MergeConfigFile(const FConfigFile& Config);
	void MergeFeatureDefaults();

	TArray<FRegisteredManager> Managers;
	TArray<FDataFlowCacheEntry> CachedDataFlows;
	TArray<FResolvedDataFlow> ResolvedDataFlows;
	TArray<FMountOnCacheEntry> CachedMounts;
	TMap<FString, FInnerConfig> simulationConfiguration;
	FString GemstarConfiguration;
	bool IniLoaded = false;
	int32 TicksSinceFirstManager = 0;
};
