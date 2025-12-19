// Fill out your copyright notice in the Description page of Project Settings.


#include "GEMSTARManager.h"

// Add default functionality here for any IGEMSTARManager functions that are not pure virtual.

void IGEMSTARManager::ConnectInputs(const FString* Input, UObject* GEMSTARRoot, AActor* me, GEMSTARInterfaceType type) {
	if (Input == nullptr) {
		return;
	}
	FString TrimmedInput = Input->TrimStartAndEnd();
	if (TrimmedInput.StartsWith(TEXT("[")) && TrimmedInput.EndsWith(TEXT("]")))
	{
		FString InputWithoutBrackets = TrimmedInput.Mid(1, TrimmedInput.Len() - 2);
		TArray<FString> Parts;
		InputWithoutBrackets.ParseIntoArray(Parts, TEXT(","), true);
		for (FString& Part : Parts)
		{
			FString managerToConnect = Part.TrimStartAndEnd();
			IGEMSTARManager::Execute_RegisterDataflow(GEMSTARRoot, me, managerToConnect, type);
		}
	}
	else {
		IGEMSTARManager::Execute_RegisterDataflow(GEMSTARRoot, me, TrimmedInput, type);
	}
}
