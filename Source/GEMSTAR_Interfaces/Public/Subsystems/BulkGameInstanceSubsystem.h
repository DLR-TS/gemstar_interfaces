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
#include "BulkGameInstanceSubsystem.generated.h"

enum class EBulkScenarioType : uint8
{
	Esmini,
	Sumo
};

struct FBulkScenario
{
	FString FilePath;
	TSoftObjectPtr<UWorld> Level;
	EBulkScenarioType Type = EBulkScenarioType::Esmini;
};

UCLASS()
class GEMSTAR_INTERFACES_API UBulkGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsActive() const { return bActive; }
	bool IsScenarioTransitionPending() const { return bScenarioTransitionPending; }
	bool GetActiveScenario(FString& OutPath, EBulkScenarioType& OutType) const;
	bool PrepareForWorld(UWorld* World);
	void OnScenarioFinished();

private:
	void LoadNextLevel();
	void DiscoverMaps();
	void DiscoverScenarios(const FString& Folder, const FString& DefaultLevelPath);
	void TravelToMap(const TSoftObjectPtr<UWorld>& Map);
	bool ReadBulkConfiguration(FString& OutFolder, FString& OutDefaultLevel);

	TArray<FBulkScenario> Scenarios;
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;
	TSoftObjectPtr<UWorld> DefaultLevel;
	int32 CurrentScenario = INDEX_NONE;
	bool bActive = false;
	bool bScenarioTransitionPending = false;
};
