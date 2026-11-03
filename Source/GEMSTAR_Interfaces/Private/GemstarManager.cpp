/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "GemstarManager.h"

#include "Utils/GemstarInterfacesLog.h"

// Add default functionality here for any IGemstarManager functions that are not pure virtual.

void IGemstarManager::AcceptChannel(GemstarInterfaceType Channel, FGemstarChannelHandler OnWire,
	FGemstarChannelHandler OnUnwire)
{
	if (OnWire) {
		WireHandlers.Add(Channel, MoveTemp(OnWire));
	}

	if (OnUnwire) {
		UnwireHandlers.Add(Channel, MoveTemp(OnUnwire));
	}
}

bool IGemstarManager::RegisterDataflow_Implementation(FDataTarget& dataTarget, FString& producer,
	GemstarInterfaceType interfaceName)
{
	if (FGemstarChannelHandler* Handler = WireHandlers.Find(interfaceName)) {
		return (*Handler)(dataTarget, producer);
	}
	return false;
}

bool IGemstarManager::UnregisterDataflow_Implementation(FDataTarget& dataTarget, GemstarInterfaceType interfaceName)
{
	static const FString NoProducer;

	if (FGemstarChannelHandler* Handler = UnwireHandlers.Find(interfaceName)) {
		return (*Handler)(dataTarget, NoProducer);
	}
	return false;
}

void IGemstarManager::ConnectInputs(const FString* Input, UObject* GemstarSimulationSubsystem,
	FDataTarget& DataTarget, GemstarInterfaceType Type) {
	if (Input == nullptr || GemstarSimulationSubsystem == nullptr) {
		UE_LOG(GemstarInterfaces, Log, TEXT("Input or GemstarSimulationSubsystem is null."));
		return;
	}
	TArray<FString> Parts = GemstarHelper::SplitIniString(*Input);
	for (FString& Part : Parts)
	{
		IGemstarManager::Execute_RegisterDataflow(GemstarSimulationSubsystem, DataTarget, Part, Type);
	}
}
