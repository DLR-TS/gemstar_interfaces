// Fill out your copyright notice in the Description page of Project Settings.


#include "GEMSTARRoot.h"

// Sets default values
AGEMSTARRoot::AGEMSTARRoot()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGEMSTARRoot::BeginPlay()
{
	Super::BeginPlay();
	SetActorTransform(FTransform()); //init the position always on 0 0 0
}

// Called every frame
void AGEMSTARRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Tick of GEMSTAR runs after all managers and
	//after all spawned sensors
	//Dataflows can be configured in first step because everything
	//is spawned by the managers during their first step
	if (!bMountsConfigured) {
		bMountsConfigured = ConfigureMounts();
	}
	if (!bDataFlowsConfigured) {
		bDataFlowsConfigured = ConfigureDataFlow(true);
	}
	if (bFirstTick) {
		bFirstTick = false;
		if (bMountsConfigured && bDataFlowsConfigured) {
			UE_LOG(LogTemp, Log, TEXT("GEMSTAR IS READY FOR SIMULATION!"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("GEMSTAR IS NOT READY FOR SIMULATION!"));
		}
	}
}

void AGEMSTARRoot::SetIniFile(const FString& IniFile) {
	GemstarConfiguration = IniFile;
}

void AGEMSTARRoot::LoadIni()
{
	if (IniLoaded)
	{
		return;
	}
	if (FPaths::IsRelative(GemstarConfiguration)) {
		GemstarConfiguration = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + GemstarConfiguration);
	}
	UE_LOG(LogTemp, Log, TEXT("Load GEMSTAR configuration from: %s"), *GemstarConfiguration);

	TArray<FString> Sections;
	GConfig->GetSectionNames(GemstarConfiguration, Sections);

	for (const FString& Section : Sections)
	{
		FInnerConfig InnerConfig;
		TArray<FString> SectionData;
		if (GConfig->GetSection(*Section, SectionData, GemstarConfiguration))
		{
			for (const FString& Entry : SectionData)
			{
				FString Key, Value;
				if (Entry.Split(TEXT("="), &Key, &Value))
				{
					InnerConfig.Entries.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
				}
			}
		}
		simulationConfiguration.Add(Section, InnerConfig);
	}
	IniLoaded = true;
}

TMap<FString, FInnerConfig> AGEMSTARRoot::RegisterManagerAndGetConfiguration(AActor* Manager) {
	Managers.Add(Manager);
	//first tick of GEMSTARRoot registers the dataflows
	//managers shall tick after their spawned actors
	AddTickPrerequisiteActor(Manager);
	LoadIni();
	FString ClassName = Manager->GetClass()->GetName();
	if (ClassName.Contains("sumo"))
	{
		return GetConfigurations("sumo");
	}
	else if (ClassName.Contains("esmini"))
	{
		return GetConfigurations("esmini");
	}
	else if (ClassName.Contains("log"))
	{
		return GetConfigurations("log");
	}
	else if (ClassName.Contains("vehicle"))
	{
		return GetConfigurations("vehicle");
	}
	else if (ClassName.Contains("camera"))
	{
		return GetConfigurations("camera");
	}
	else if (ClassName.Contains("lidar"))
	{
		return GetConfigurations("lidar");
	}
	else if (ClassName.Contains("visualizer"))
	{
		return GetConfigurations("visualizer");
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("GEMSTAR ROOT: Could not match: %s"), *ClassName);
		return TMap<FString, FInnerConfig>();
	}
}

bool AGEMSTARRoot::ConfigureDataFlow(bool EndOfTickOne) {
	bool success = false;
	for (int32 i = CachedDataFlows.Num() - 1; i >= 0; --i) {
		FDataFlowCacheEntry& flow = CachedDataFlows[i];
		success = false;
		for (auto& manager : Managers) {
			success = IGEMSTARManager::Execute_RegisterDataflow(manager, flow.Consumer, flow.Producer, flow.InterfaceName);
			if (success) {
				UE_LOG(LogTemp, Log, TEXT("GEMSTAR registered dataflow: from %s to %s"), *flow.Producer, *flow.Consumer->GetName());
				CachedDataFlows.RemoveAt(i);
				break;
			}
		}
		if (!success && EndOfTickOne)
			UE_LOG(LogTemp, Error, TEXT("GEMSTAR registered dataflow can not be set up: from %s to %s"), *flow.Producer, *flow.Consumer->GetName());
	}
	return CachedDataFlows.Num() == 0;
}

bool AGEMSTARRoot::ConfigureMounts() {
	bool success = false;
	for (int32 i = CachedMounts.Num() - 1; i >= 0; --i) {
		FMountOnCacheEntry& mount = CachedMounts[i];
		success = false;
		for (auto& manager : Managers) {
			success = IGEMSTARManager::Execute_MountOn(manager, mount.NameByConfiguration, mount.Actor);
			if (success) {
				UE_LOG(LogTemp, Log, TEXT("GEMSTAR mounted: %s on %s"), *mount.NameByConfiguration, *mount.Actor->GetName());
				CachedMounts.RemoveAt(i);
				ILocalTransformUpdater::Execute_UpdateTransformAfterAttachment(mount.Actor);
				break;
			}
		}
		if (!success)
			UE_LOG(LogTemp, Error, TEXT("GEMSTAR can not mount: %s on %s"), *mount.NameByConfiguration, *mount.Actor->GetName());
	}
	return CachedMounts.Num() == 0;
}


TMap<FString, FInnerConfig> AGEMSTARRoot::GetConfigurations(const FString& name) {
	TMap<FString, FInnerConfig> configurations;
	for (const TPair<FString, FInnerConfig>& Pair : simulationConfiguration)
	{
		if (Pair.Key.Contains(name)) {
			configurations.Add(Pair.Key, Pair.Value);
		}
	}
	if (configurations.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Maybe ok, but no configuration found for: %s"), *name);
	}
	return configurations;
}

bool AGEMSTARRoot::RegisterDataflow_Implementation(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName) {
	FDataFlowCacheEntry entry;
	entry.Consumer = consumer;
	entry.Producer = producer;
	entry.InterfaceName = interfaceName;
	CachedDataFlows.Add(entry);
	ConfigureDataFlow();
	return true;//returns true because the flow is cached and will be added if producer is available
}

bool AGEMSTARRoot::MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor) {
	FMountOnCacheEntry entry;
	entry.NameByConfiguration = NameByConfiguration;
	entry.Actor = Actor;
	CachedMounts.Add(entry);
	return true;
}
