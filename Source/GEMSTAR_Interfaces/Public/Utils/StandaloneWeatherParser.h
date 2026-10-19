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
#include "UObject/Object.h"
#include "XmlFile.h"
#include "XmlNode.h"
#include "StandaloneWeatherParser.generated.h"


USTRUCT(BlueprintType)
struct FStandaloneWeatherData
{
    GENERATED_BODY()
    // ----------------------------
    // Weather
    // ----------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasTemperature = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double TemperatureCelsius = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather|Raw")
    double TemperatureKelvin = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasAtmosphericPressure = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double AtmosphericPressureHectoPascal = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather|Raw")
    double AtmosphericPressurePascal = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasFractionalCloudCover = false;
    FString FractionalCloudCover;
    float  FractionalCloudCover_normalized;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasFog = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double VisualRangeMeters = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasPrecipitation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    FString PrecipitationType;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double PrecipitationIntensityMmPerHour = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    bool bHasWind = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double WindSpeedKilometersPerHour = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather")
    double WindDirectionDegrees = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather|Raw")
    double WindSpeedMetersPerSecond = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Weather|Raw")
    double WindDirectionRadians = 0.0;
    // ----------------------------
    // TimeOfDay
    // ----------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Environment|Time")
    bool bHasTimeOfDay = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Time")
    bool bTimeOfDayAnimation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Time")
    FString TimeOfDayDateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Time")
    FDateTime ParsedTimeOfDay;
    // ----------------------------
    // RoadCondition
    // ----------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Environment|Road")
    bool bHasRoadCondition = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment|Road")
    double FrictionScaleFactor = 1.0;
};
/**
 * 
 */
UCLASS(BlueprintType)
class GEMSTAR_INTERFACES_API UStandaloneWeatherParser : public UObject
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Environment")
    static bool ParseEnvironmentFile(
        const FString& FilePath,
        FStandaloneWeatherData& OutData,
        FString& OutError);

    static bool ParseEnvironmentNode(const FXmlNode* Root, FStandaloneWeatherData& OutData, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    static FString FindMatchingWeatherFile(const FString& FilePath);


    static const FXmlNode* FindFirstNode(
        const FXmlNode* Node,
        const FString& Tag);
private:

    static const FXmlNode* GetDirectChild(
        const FXmlNode* Node,
        const FString& Tag);

    static bool TryGetDoubleAttribute(
        const FXmlNode* Node,
        const FString& Attribute,
        double& OutValue);

    static void ParseWeatherNode(
        const FXmlNode* WeatherNode,
        FStandaloneWeatherData& OutData);

    static void ParseTimeOfDayNode(
        const FXmlNode* TimeOfDayNode,
        FStandaloneWeatherData& OutData);

    static void ParseRoadConditionNode(
        const FXmlNode* RoadConditionNode,
        FStandaloneWeatherData& OutData);
};

float OktasToCloudCoverage(const FString& Oktas);