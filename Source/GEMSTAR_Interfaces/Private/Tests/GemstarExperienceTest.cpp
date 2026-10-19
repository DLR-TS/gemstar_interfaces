/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Experience/GemstarExperienceSet.h"
#include "Subsystems/GemstarExperienceDefinition.h"

#include "Engine/AssetManager.h"
#include "Engine/AssetManagerTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	UGemstarExperienceSet* MakeSet(std::initializer_list<const TCHAR*> Names)
	{
		UGemstarExperienceSet* Set = NewObject<UGemstarExperienceSet>();

		for (const TCHAR* Name : Names) {
			Set->GameFeaturesToEnable.Add(Name);
		}
		return Set;
	}

	UGemstarExperienceDefinition* MakeExperience(std::initializer_list<const TCHAR*> Names)
	{
		UGemstarExperienceDefinition* Experience = NewObject<UGemstarExperienceDefinition>();

		for (const TCHAR* Name : Names) {
			Experience->GameFeaturesToEnable.Add(Name);
		}
		return Experience;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceWithoutSetsTest,
	"GEMSTAR.Experience.WithoutSets",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceWithoutSetsTest::RunTest(const FString& Parameters)
{
	UGemstarExperienceDefinition* Experience = MakeExperience({ TEXT("ue_lidar_feature"), TEXT("ue_visualizer_feature") });
	const TArray<FString> Features = Experience->ResolveFeatures();

	if (TestEqual(TEXT("own entries are returned"), Features.Num(), 2)) {
		TestEqual(TEXT("in order, first"), Features[0], TEXT("ue_lidar_feature"));
		TestEqual(TEXT("in order, second"), Features[1], TEXT("ue_visualizer_feature"));
	}

	TestEqual(TEXT("an empty experience resolves to nothing"),
		MakeExperience({})->ResolveFeatures().Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceSetOrderTest,
	"GEMSTAR.Experience.SetsComeFirst",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceSetOrderTest::RunTest(const FString& Parameters)
{
	UGemstarExperienceDefinition* Experience = MakeExperience({ TEXT("ue_m_map_feature") });
	Experience->ExperienceSets.Add(MakeSet({ TEXT("ue_esmini_feature"), TEXT("ue_vehicle_feature") }));

	const TArray<FString> Features = Experience->ResolveFeatures();

	if (TestEqual(TEXT("set entries and own entries are both present"), Features.Num(), 3)) {
		TestEqual(TEXT("set first"), Features[0], TEXT("ue_esmini_feature"));
		TestEqual(TEXT("set second"), Features[1], TEXT("ue_vehicle_feature"));
		TestEqual(TEXT("own entry last"), Features[2], TEXT("ue_m_map_feature"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceDeduplicationTest,
	"GEMSTAR.Experience.Deduplication",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceDeduplicationTest::RunTest(const FString& Parameters)
{
	UGemstarExperienceDefinition* Experience = MakeExperience({ TEXT("ue_lidar_feature") });
	Experience->ExperienceSets.Add(MakeSet({ TEXT("ue_lidar_feature"), TEXT("ue_camera_feature") }));

	const TArray<FString> Features = Experience->ResolveFeatures();
	TestEqual(TEXT("a name in a set and the experience appears once"), Features.Num(), 2);

	UGemstarExperienceDefinition* Overlapping = MakeExperience({});
	Overlapping->ExperienceSets.Add(MakeSet({ TEXT("ue_esmini_feature"), TEXT("ue_lidar_feature") }));
	Overlapping->ExperienceSets.Add(MakeSet({ TEXT("ue_lidar_feature"), TEXT("ue_logger_feature") }));

	TestEqual(TEXT("a name in two sets appears once"),
		Overlapping->ResolveFeatures().Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceNullSetTest,
	"GEMSTAR.Experience.NullSetIsSkipped",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceNullSetTest::RunTest(const FString& Parameters)
{
	UGemstarExperienceDefinition* Experience = MakeExperience({ TEXT("ue_m_map_feature") });
	Experience->ExperienceSets.Add(nullptr);
	Experience->ExperienceSets.Add(MakeSet({ TEXT("ue_esmini_feature") }));

	AddExpectedError(TEXT("has an unset experience set"), EAutomationExpectedErrorFlags::Contains, 1);

	const TArray<FString> Features = Experience->ResolveFeatures();

	if (TestEqual(TEXT("the valid set and own entries still resolve"), Features.Num(), 2)) {
		TestEqual(TEXT("valid set first"), Features[0], TEXT("ue_esmini_feature"));
		TestEqual(TEXT("own entry last"), Features[1], TEXT("ue_m_map_feature"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceScanPathsTest,
	"GEMSTAR.Experience.ScanPaths",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceScanPathsTest::RunTest(const FString& Parameters)
{
	// Experiences are selected by name, so nothing references them: they resolve only if the
	// Asset Manager was told to scan for them. This pins that configuration, which lives in ini
	// files and is otherwise only discovered by a run silently falling back.
	const auto ScanPathsFor = [this](const TCHAR* TypeName) -> TArray<FString>
	{
		FPrimaryAssetTypeInfo Info;
		
		if (!UAssetManager::Get().GetPrimaryAssetTypeInfo(FPrimaryAssetType(TypeName), Info)) {
			AddError(FString::Printf(TEXT("primary asset type '%s' is not registered"), TypeName));
			return {};
		}
		return Info.AssetScanPaths;
	};

	const TArray<FString> DefinitionPaths = ScanPathsFor(TEXT("GemstarExperienceDefinition"));
	TestTrue(TEXT("the project's experience directory is scanned"),
		DefinitionPaths.ContainsByPredicate([](const FString& Path)
			{ return Path.Contains(TEXT("/Game/Experiences")); }));

	const TArray<FString> SetPaths = ScanPathsFor(TEXT("GemstarExperienceSet"));
	TestTrue(TEXT("experience sets are scanned too, else a referenced set resolves null"),
		SetPaths.Num() > 0);

	// A plugin shipping its own experiences adds its directory from its own Config/DefaultGame.ini.
	// If this fails, that merge does not happen and the entry belongs in the GameFeatureData asset
	// instead.
	TestTrue(TEXT("ue_m_map_feature's own experience directory is scanned"),
		DefinitionPaths.ContainsByPredicate([](const FString& Path)
			{ return Path.Contains(TEXT("ue_m_map_feature")); }));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarExperienceEsminiConflictTest,
	"GEMSTAR.Experience.EsminiConflict",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarExperienceEsminiConflictTest::RunTest(const FString& Parameters)
{
	UGemstarExperienceDefinition* Conflicting =
		MakeExperience({ TEXT("ue_esmini_feature"), TEXT("ue_esmini_control_feature") });

	FDataValidationContext Context;
	TestEqual(TEXT("naming both esmini features is invalid"),
		Conflicting->IsDataValid(Context), EDataValidationResult::Invalid);

	UGemstarExperienceDefinition* Fine = MakeExperience({ TEXT("ue_esmini_feature") });
	FDataValidationContext CleanContext;
	TestNotEqual(TEXT("naming one is fine"),
		Fine->IsDataValid(CleanContext), EDataValidationResult::Invalid);

	UGemstarExperienceDefinition* ViaSet = MakeExperience({ TEXT("ue_esmini_control_feature") });
	ViaSet->ExperienceSets.Add(MakeSet({ TEXT("ue_esmini_feature") }));
	FDataValidationContext SetContext;
	TestEqual(TEXT("the conflict is caught across a set too"),
		ViaSet->IsDataValid(SetContext), EDataValidationResult::Invalid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
