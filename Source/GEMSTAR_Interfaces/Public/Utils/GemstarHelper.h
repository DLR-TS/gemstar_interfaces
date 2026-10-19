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

#include "Interfaces/IPluginManager.h"
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class GemstarInterfaceType : uint8 {
	SENSORVIEW,
	SENSORDATA,
	TRAFFICUPDATE,
	TRAFFICLIGHTUPDATE,
	VRUUPDATE,
	WEATHERDATA
};

class GEMSTAR_INTERFACES_API GemstarHelper
{
public:
	GemstarHelper();
	~GemstarHelper();

	//Split String into parts seperated by ","
	static TArray<FString> SplitIniString(const FString& Input);
	static GemstarInterfaceType FStringToGEMSTARInterfaceEnum(const FString& name);
	static FString GetAbsolutePathOfGameFeatureFile(const FString& RelativeFilePath,
		const FString& PluginName);
};
