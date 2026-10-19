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
#include "GameFramework/GameStateBase.h"
#include "GemstarGameState.generated.h"

class UGemstarExperienceManagerComponent;

// Registers as component receiver in PreInitializeComponents
// Owns the experience manager component
UCLASS()
class GEMSTAR_INTERFACES_API AGemstarGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGemstarGameState();

	UGemstarExperienceManagerComponent* GetExperienceManager() const { return ExperienceManager; }

protected:
	// The injection target for feature AddComponents actions
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

private:
	UPROPERTY()
	TObjectPtr<UGemstarExperienceManagerComponent> ExperienceManager;
};