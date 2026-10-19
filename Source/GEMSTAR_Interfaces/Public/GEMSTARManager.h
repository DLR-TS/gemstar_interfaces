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

#include "Interfaces/DataTarget.h"
#include "Utils/GemstarHelper.h"
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GemstarManager.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGemstarManager : public UInterface
{
	GENERATED_BODY()
};

using FGemstarChannelHandler = TFunction<bool(FDataTarget&, const FString&)>;

/**
 * Generic manger functions
 */
class GEMSTAR_INTERFACES_API IGemstarManager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
		UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	bool RegisterDataflow(FDataTarget& dataTarget, FString& producer,
		GemstarInterfaceType interfaceName);
	virtual bool RegisterDataflow_Implementation(FDataTarget& dataTarget, FString& producer,
		GemstarInterfaceType interfaceName);

	// Mirrors RegisterDataflow: what ConfigureDataFlow wired, UnregisterManager unwires.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	bool UnregisterDataflow(FDataTarget& dataTarget, GemstarInterfaceType interfaceName);
	virtual bool UnregisterDataflow_Implementation(FDataTarget& dataTarget, GemstarInterfaceType interfaceName);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	bool MountOn(const FString& NameByConfiguration, AActor* Actor);
	virtual bool MountOn_Implementation(const FString& NameByConfiguration, AActor* Actor) { return false; }

	void ConnectInputs(const FString* Input, UObject* GemstarSimulationSubsystem, FDataTarget& dataTarget,
		GemstarInterfaceType type);

protected:
	void AcceptChannel(GemstarInterfaceType Channel, FGemstarChannelHandler OnWire,
		FGemstarChannelHandler OnUnwire = nullptr);

private:
	TMap<GemstarInterfaceType, FGemstarChannelHandler> WireHandlers;
	TMap<GemstarInterfaceType, FGemstarChannelHandler> UnwireHandlers;
};
