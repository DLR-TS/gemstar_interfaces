/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarWorldSettings.h"

#include "Engine/World.h"
#include "Subsystems/GemstarExperienceDefinition.h"
#include "Utils/GemstarInterfacesLog.h"

UGemstarExperienceDefinition* AGemstarWorldSettings::ExperienceFor(const UWorld* World)
{
	if (World == nullptr) {
		return nullptr;
	}

	const AGemstarWorldSettings* Settings = Cast<AGemstarWorldSettings>(World->GetWorldSettings());
	
	if (Settings == nullptr || Settings->DefaultExperience.IsNull()) {
		return nullptr;
	}

	UGemstarExperienceDefinition* Experience = Settings->DefaultExperience.LoadSynchronous();
	
	if (Experience == nullptr) {
		UE_LOG(GemstarInterfaces, Error,
		       TEXT("World Settings names an experience that failed to load: %s"),
		       *Settings->DefaultExperience.ToString());
	}
	
	return Experience;
}

const FString AGemstarWorldSettings::ConfigFor(const UWorld* World) {
	if (World == nullptr) {
		return TEXT("Simulation/DefaultGemstar.ini");
	}

	const AGemstarWorldSettings* Settings = Cast<AGemstarWorldSettings>(World->GetWorldSettings());

	if (Settings == nullptr) {
		return "";
	}

	FString Configuration = Settings->GemstarConfiguration;

	if (Configuration == "") {
		UE_LOG(GemstarInterfaces, Error,
			TEXT("World Settings has no configuration"));
	}
	else {
		return Configuration;
	}

	return "";
}
