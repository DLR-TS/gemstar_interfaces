/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarGameState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Subsystems/GemstarExperienceManagerComponent.h"

AGemstarGameState::AGemstarGameState()
{
	ExperienceManager = CreateDefaultSubobject<UGemstarExperienceManagerComponent>(
		TEXT("ExperienceManager"));
}
void AGemstarGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	// Receiver must exist before any BeginPlay can activate a feature
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this, true);
}

void AGemstarGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}
