/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "Subsystems/GemstarExperienceDefinition.h"

#include "GameFeatureAction.h"
#include "Utils/GemstarInterfacesLog.h"
#include "Subsystems/GemstarSimulationSubsystem.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

const FPrimaryAssetType UGemstarExperienceDefinition::PrimaryAssetType(TEXT("GemstarExperienceDefinition"));

FPrimaryAssetId UGemstarExperienceDefinition::GetPrimaryAssetId() const {
	return FPrimaryAssetId(PrimaryAssetType, FName(GetName()));
}

TArray<FString> UGemstarExperienceDefinition::ResolveFeatures() const
{
	TArray<FString> Features;

	for (const TObjectPtr<UGemstarExperienceSet>& Set : ExperienceSets) {
		
		if (!IsValid(Set)) {
			UE_LOG(GemstarInterfaces, Warning, TEXT("Experience '%s' has an unset experience set."), *GetName());
			continue;
		}
		
		for (const FString& Feature : Set->GameFeaturesToEnable) {
			Features.AddUnique(Feature);
		}
	}

	for (const FString& Feature : GameFeaturesToEnable) {
		Features.AddUnique(Feature);
	}

	AddTaggedFeatures(Features);

	return Features;
}

void UGemstarExperienceDefinition::AddTaggedFeatures(TArray<FString>& Features) const
{

	if (AcceptsFeatureTags.IsEmpty()) {
		return;
	}

	for (const TPair<FString, TArray<FString>>& Declared :
		UGemstarSimulationSubsystem::FeatureTagsByPlugin())
	{
		for (const FString& Tag : Declared.Value) {
			
			if (AcceptsFeatureTags.ContainsByPredicate([&Tag](const FString& Accepted)
				{ return Accepted.Equals(Tag, ESearchCase::IgnoreCase); })) {

				if (!Features.Contains(Declared.Key)) {
					Features.Add(Declared.Key);
					UE_LOG(GemstarInterfaces, Log, TEXT("GEMSTAR experience '%s' also activates %s, tag '%s'."),
						*GetName(), *Declared.Key, *Tag);
				}
				break;
			}
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UGemstarExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	const TArray<FString> Features = ResolveFeatures();

	if (Features.Contains(TEXT("ue_esmini_feature")) &&
	    Features.Contains(TEXT("ue_esmini_control_feature")))
	{
		Context.AddError(NSLOCTEXT("Gemstar", "EsminiConflict",
			"ue_esmini_feature and ue_esmini_control_feature are mutually exclusive: both spawn a "
			"manager for the [esmini] section and share one esmini library. Name only one."));
		Result = EDataValidationResult::Invalid;
	}

	for (int32 i = 0; i < Actions.Num(); ++i) {

		if (Actions[i] == nullptr) {
			Context.AddError(FText::Format(
				NSLOCTEXT("Gemstar", "NullAction", "Actions[{0}] is unset. Remove the entry or "
				          "choose an action class."),
				FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		const UGameFeatureAction* Action = Actions[i];
		
		if (Action->IsDataValid(Context) == EDataValidationResult::Invalid) {
			Result = EDataValidationResult::Invalid;
		}
	}

	for (int32 i = 0; i < PreloadAssets.Num(); ++i) {

		if (PreloadAssets[i].IsNull()) {
			Context.AddError(FText::Format(
				NSLOCTEXT("Gemstar", "EmptyPreload", "PreloadAssets[{0}] is empty."),
				FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
		}
	}

	for (int32 i = 0; i < ConfigOverrides.Num(); ++i) {
		
		if (ConfigOverrides[i].Section.IsEmpty() || ConfigOverrides[i].Key.IsEmpty()) {
			Context.AddError(FText::Format(
				NSLOCTEXT("Gemstar", "EmptyOverrideEntry",
				          "ConfigOverrides[{0}] needs both a Section and a Key."),
				FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif