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
#include "GameFramework/WorldSettings.h"
#include "GemstarWorldSettings.generated.h"

class UGemstarExperienceDefinition;

// DefaultExperience: what runs
// GemstarConfiguration: which ini
// both per map
UCLASS()
class GEMSTAR_INTERFACES_API AGemstarWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	// Unset means "fall back to the ini" 
	// Full resolution order in
	UPROPERTY(EditAnywhere, Category = "GEMSTAR")
	TSoftObjectPtr<UGemstarExperienceDefinition> DefaultExperience;
    static UGemstarExperienceDefinition* ExperienceFor(const UWorld* World);
	static const FString ConfigFor(const UWorld* World);

	// The ini this map reads, project-relative. Empty falls back to the root actor's property
	// then the project default. Wins over the actor property when set
	UPROPERTY(EditAnywhere, Category = "GEMSTAR")
	FString GemstarConfiguration;
};