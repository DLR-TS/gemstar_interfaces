/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
// Pins each GetOptions path to its function. The editor resolves these only when an asset is
// opened, and an unresolvable path degrades to a plain text box rather than failing anything.

#include "Experience/GemstarExperienceSet.h"
#include "Subsystems/GemstarExperienceDefinition.h"

#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGemstarFeatureOptionPathsTest,
	"GEMSTAR.Experience.OptionPathsResolve",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FGemstarFeatureOptionPathsTest::RunTest(const FString& Parameters)
{
	const auto PinsAStaticFunction = [this](const UClass* Owner, const TCHAR* PropertyName)
	{
		const FProperty* Property = Owner->FindPropertyByName(PropertyName);
		if (Property == nullptr)
		{
			AddError(FString::Printf(TEXT("%s has no property %s"), *Owner->GetName(), PropertyName));
			return;
		}

		const FString Path = Property->GetMetaData(TEXT("GetOptions"));
		if (Path.IsEmpty())
		{
			AddError(FString::Printf(TEXT("%s::%s declares no GetOptions"),
				*Owner->GetName(), PropertyName));
			return;
		}

		// The two conditions PropertyEditorUtils::GetPropertyOptions ensures on.
		const UFunction* Options = FindObject<UFunction>(nullptr, *Path, EFindObjectFlags::ExactClass);
		if (TestNotNull(*FString::Printf(TEXT("'%s' resolves"), *Path), Options))
		{
			TestTrue(*FString::Printf(TEXT("'%s' is static"), *Path),
				Options->HasAnyFunctionFlags(FUNC_Static));
		}
	};

	PinsAStaticFunction(UGemstarExperienceSet::StaticClass(), TEXT("GameFeaturesToEnable"));
	PinsAStaticFunction(UGemstarExperienceDefinition::StaticClass(), TEXT("GameFeaturesToEnable"));
	PinsAStaticFunction(UGemstarExperienceDefinition::StaticClass(), TEXT("AcceptsFeatureTags"));

	// The same lookup on a name that does not exist, so a pass above means resolution, not luck.
	TestNull(TEXT("a path to nothing resolves to nothing"),
		FindObject<UFunction>(nullptr,
			TEXT("GEMSTAR_Interfaces.GemstarFeatureOptions.NoSuchFunction"),
			EFindObjectFlags::ExactClass));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
