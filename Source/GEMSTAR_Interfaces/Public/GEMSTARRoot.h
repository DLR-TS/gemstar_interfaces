// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "OSITrafficLightDataDispatcher.h"
#include "SensorConfigurationDispatcher.h"
#include "SensorDataDispatcher.h"
#include "TrafficLightDataDispatcher.h"
#include "VehicleDataDispatcher.h"
#include "GEMSTARManager.h"
#include "LocalTransformUpdater.h"

#include "Misc/Timespan.h"
#include "Containers/Ticker.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GEMSTARRoot.generated.h"

USTRUCT(BlueprintType)
struct FInnerConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	TMap<FString, FString> Entries;
};

struct FDataFlowCacheEntry {
	AActor* Consumer;
	FString Producer;
	GEMSTARInterfaceType InterfaceName;
};

struct FMountOnCacheEntry {
	FString NameByConfiguration;
	AActor* Actor;
};

UCLASS()
class GEMSTAR_INTERFACES_API AGEMSTARRoot : public AActor, public IGEMSTARManager
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGEMSTARRoot();

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	TMap<FString, FInnerConfig> RegisterManagerAndGetConfiguration(AActor* Manager);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void SetIniFile(const FString& IniFile);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void LoadIni();

	virtual bool MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor) override;
	virtual bool RegisterDataflow_Implementation(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName) override;
	bool ConfigureDataFlow(bool EndOfTickOne = false);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FString DEFAULTGEMSTARCONFIGURATION = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() + "DefaultGemstar.ini");
	TMap<FString, FInnerConfig> simulationConfiguration;
	TArray<AActor*> Managers;
	TArray<FDataFlowCacheEntry> CachedDataFlows;
	TArray<FMountOnCacheEntry> CachedMounts;
	//Get the configurations for this manager (can be several configs, like several sensors)
	TMap<FString, FInnerConfig> GetConfigurations(const FString& Manager);
	bool bDataFlowsConfigured = false;
	bool bMountsConfigured = false;
	bool ConfigureMounts();
	bool bFirstTick = true;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FString GemstarConfiguration = DEFAULTGEMSTARCONFIGURATION;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GEMSTAR")
	bool IniLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseSUMO = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseESMINI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseTrafficLight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseLog = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseROS = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseSimVehicles = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseLidar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool UseVisualizer = false;
};
