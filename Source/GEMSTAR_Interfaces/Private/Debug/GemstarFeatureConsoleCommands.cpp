/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

// --- GEMSTAR Game Feature console commands -----------------
//   gemstar.ListFeatures
//   gemstar.ActivateFeature   <featureName>   -> Active
//   gemstar.DeactivateFeature <featureName>   -> Loaded   (NOT Registered)
//   gemstar.UnloadFeature     <featureName>   -> Registered
//
//        gemstar.ListFeatures
//            ue_visualizer_feature            Active       (built-in)
//            ue_lidar_feature                 Active       (built-in)
//            ue_camera_feature                Registered   (built-in)
//            ...
//
//        gemstar.DeactivateFeature ue_visualizer_feature
//            LogTemp: GEMSTAR unregistered manager: VisualizerManager
//            -- point cloud disappears, simulation keeps running
//
//        gemstar.ListFeatures
//            ue_visualizer_feature            Loaded       (built-in)
//
//        gemstar.ActivateFeature ue_visualizer_feature
//            LogTemp: Activated GEMSTAR visualizer feature
//            -- point cloud returns
//
//        gemstar.UnloadFeature ue_visualizer_feature
//        gemstar.ListFeatures
//            ue_visualizer_feature            Registered   (built-in)
//
// Lifecycle:  Registered --activate--> Active --deactivate--> Loaded --unload--> Registered
// Deactivating undoes actions but leaves modules resident (Loaded);
// only UnloadFeature detaches them. Reactivating from Loaded is cheap.
//
// IMPORTANT: every command is asynchronous, it returns immediately and
// completes via a delegate. Re-run ListFeatures a moment later for the
// settled state, an in-flight command shows Deactivating/Unloading/Activating.
// Errors surface in the completion callback, not the console return.
// --------------------------------------------------------------------------


#include "Utils/GemstarInterfacesLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFeatureTypes.h"
#include "GameFeaturesSubsystem.h"
#include "Subsystems/GemstarGameFeatureSubsystem.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING

namespace
{
	UGemstarGameFeatureSubsystem* GemstarFeatures(UWorld* World)
	{
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UGemstarGameFeatureSubsystem>() : nullptr;
	}

	bool RequireName(const TArray<FString>& Args, const TCHAR* Usage)
	{
		if (Args.Num() >= 1){
			return true;
		}

		UE_LOG(GemstarInterfaces, Warning, TEXT("usage: %s"), Usage);
		return false;
	}

	void DeactivateAnyFeature(UWorld* World, const FString& PluginName, bool bUnload)
	{
		UGemstarGameFeatureSubsystem* Features = GemstarFeatures(World);

		if (Features && Features->DeactivateFeature(PluginName, bUnload)) {
			return;
		}

		UGameFeaturesSubsystem& Engine = UGameFeaturesSubsystem::Get();
		FString PluginURL;
		if (!Engine.GetPluginURLByName(PluginName, PluginURL))
		{
			UE_LOG(GemstarInterfaces, Error, TEXT("Unknown Game Feature Plugin: %s"), *PluginName);
			return;
		}

		if (Engine.GetPluginState(PluginURL) == EGameFeaturePluginState::Active)
		{
			Engine.DeactivateGameFeaturePlugin(PluginURL);
		}

		if (bUnload)
		{
			Engine.UnloadGameFeaturePlugin(PluginURL, /*bKeepRegistered=*/true);
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs GGemstarListFeatures(
	TEXT("gemstar.ListFeatures"),
	TEXT("Print every Game Feature plugin and its current state."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			UGameFeaturesSubsystem::Get().ForEachGameFeature(
				[](FGameFeatureInfo&& Info)
				{
					UE_LOG(GemstarInterfaces, Display, TEXT("%-32s %-12s %s"),
						*Info.Name,
						*UE::GameFeatures::ToString(Info.CurrentState),
						Info.bLoadedAsBuiltIn ? TEXT("(built-in)") : TEXT(""));
				});
		}));

static FAutoConsoleCommandWithWorldAndArgs GGemstarActivateFeature(
	TEXT("gemstar.ActivateFeature"),
	TEXT("Activate one feature: gemstar.ActivateFeature ue_visualizer_feature"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!RequireName(Args, TEXT("gemstar.ActivateFeature <plugin_name>"))) { 
                return; 
            }
			if (UGemstarGameFeatureSubsystem* Features = GemstarFeatures(World)) {
				Features->ActivateFeatures({ Args[0] });
			}
		}));

static FAutoConsoleCommandWithWorldAndArgs GGemstarDeactivateFeature(
	TEXT("gemstar.DeactivateFeature"),
	TEXT("Deactivate one feature, leaving it Loaded: gemstar.DeactivateFeature ue_visualizer_feature"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!RequireName(Args, TEXT("gemstar.DeactivateFeature <plugin_name>"))) { 
                return; 
            }
			DeactivateAnyFeature(World, Args[0], /*bUnload=*/false);
		}));

static FAutoConsoleCommandWithWorldAndArgs GGemstarUnloadFeature(
	TEXT("gemstar.UnloadFeature"),
	TEXT("Deactivate a feature and unload its modules: gemstar.UnloadFeature ue_visualizer_feature"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!RequireName(Args, TEXT("gemstar.UnloadFeature <plugin_name>"))) { 
                return; 
            }
			DeactivateAnyFeature(World, Args[0], /*bUnload=*/true);
		}));

#endif // !UE_BUILD_SHIPPING