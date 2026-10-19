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

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/Weather/WeatherEffectTypes.h"
#include "WeatherDataUpdate.generated.h"

USTRUCT(BlueprintType)
struct GEMSTAR_INTERFACES_API FWeatherData {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FName ID = FName(TEXT("weather")); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FGEMSTARWeatherState WeatherState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FGEMSTARTimeOfDayInfo TimeOfDayInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	TMap<FString, FString> SensorWeatherParameters;
};

UINTERFACE(MinimalAPI)
class UWeatherDataUpdate : public UInterface
{
	GENERATED_BODY()
};

class GEMSTAR_INTERFACES_API IWeatherDataUpdate
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GEMSTAR")
	void WeatherDataUpdate(FWeatherData& data, int32 targetIndex);
};


