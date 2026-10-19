/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Experience/GemstarFeatureOptions.h"

#include "GameFeaturesSubsystem.h"
#include "Subsystems/GemstarSimulationSubsystem.h"


TArray<FString> UGemstarFeatureOptions::AvailableFeatures() 
{
    TArray<FString> Names;

    UGameFeaturesSubsystem::Get().ForEachGameFeature(
        [&Names](FGameFeatureInfo&& Info) { Names.Add(MoveTemp(Info.Name)); }
    );
    
    Names.Sort();
    return Names;
}

TArray<FString> UGemstarFeatureOptions::AvailableFeatureTags() 
{
    TArray<FString> Tags;

    for (const TPair<FString, TArray<FString>>& Declared : UGemstarSimulationSubsystem::FeatureTagsByPlugin()) {

        for (const FString& Tag : Declared.Value) {
            Tags.AddUnique(Tag);
        }
    }
    
    Tags.Sort();
    return Tags;
}