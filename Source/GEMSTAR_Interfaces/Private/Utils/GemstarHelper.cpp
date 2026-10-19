/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "Utils/GemstarHelper.h"

#include "Utils/GemstarInterfacesLog.h"

GemstarHelper::GemstarHelper()
{
}

GemstarHelper::~GemstarHelper()
{
}

TArray<FString> GemstarHelper::SplitIniString(const FString& Input) {
	TArray<FString> Strings;
	if (Input.IsEmpty()) {
		return Strings;
	}
	FString TrimmedInput = Input.TrimStartAndEnd();
	if (TrimmedInput.StartsWith(TEXT("[")) && TrimmedInput.EndsWith(TEXT("]")))
	{
		FString InputWithoutBrackets = TrimmedInput.Mid(1, TrimmedInput.Len() - 2);
		TArray<FString> Parts;
		InputWithoutBrackets.ParseIntoArray(Parts, TEXT(","), true);
		for (FString& Part : Parts)
		{
			Strings.Add(Part.TrimStartAndEnd());
		}
	}
	else {
		Strings.Add(TrimmedInput);
	}
	return Strings;
}

GemstarInterfaceType GemstarHelper::FStringToGEMSTARInterfaceEnum(const FString& Name) {
	if (Name.Equals(TEXT("SENSORVIEW"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::SENSORVIEW;
	if (Name.Equals(TEXT("SENSORDATA"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::SENSORDATA;
	if (Name.Equals(TEXT("TRAFFICUPDATE"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::TRAFFICUPDATE;
	if (Name.Equals(TEXT("TRAFFICLIGHTUPDATE"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::TRAFFICLIGHTUPDATE;
	if (Name.Equals(TEXT("VRUUPDATE"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::VRUUPDATE;
	if (Name.Equals(TEXT("WEATHERDATA"), ESearchCase::IgnoreCase)) return GemstarInterfaceType::WEATHERDATA;

	UE_LOG(GemstarInterfaces, Warning, TEXT("Unkown Enum name: %s"), *Name);
	return GemstarInterfaceType::SENSORVIEW;
}

FString GemstarHelper::GetAbsolutePathOfGameFeatureFile(const FString& RelativeFilePath, const FString& PluginName)
{
	// Get the plugin directory
	FString PluginBaseDir = IPluginManager::Get().FindPlugin(PluginName)->GetBaseDir();

	// Combine with relative path inside the GameFeature
	FString FullPath = FPaths::Combine(PluginBaseDir, RelativeFilePath);

	// Convert to absolute path
	return FPaths::ConvertRelativePathToFull(FullPath);
}
