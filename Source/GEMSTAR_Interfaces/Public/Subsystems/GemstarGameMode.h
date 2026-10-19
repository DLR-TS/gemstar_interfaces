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
#include "GameFramework/GameModeBase.h"
#include "GemstarGameMode.generated.h"

// Makes GemstarGameState selectable
UCLASS()
class GEMSTAR_INTERFACES_API AGemstarGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGemstarGameMode();
};