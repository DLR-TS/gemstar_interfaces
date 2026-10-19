/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "Utils/StandaloneWeatherParser.h"

#include "Utils/StandaloneWeatherParser.h"


FString UStandaloneWeatherParser::FindMatchingWeatherFile(const FString& FilePath)
{
    const FString Directory = FPaths::GetPath(FilePath);
    const FString Stem = FPaths::GetBaseFilename(FilePath);

    TArray<FString> WeatherFiles;

    IFileManager::Get().FindFiles(
        WeatherFiles,
        *FPaths::Combine(Directory, TEXT("weather_*.xml")),
        true,   // Files
        false   // Directories
    );

    // Prefer weather_<stem>.xml
    const FString DesiredFilename =
        FString::Printf(TEXT("weather_%s.xml"), *Stem);

    for (const FString& Filename : WeatherFiles)
    {
        if (Filename.Equals(DesiredFilename, ESearchCase::IgnoreCase))
        {
            return FPaths::Combine(Directory, Filename);
        }
    }

    // If there is only one weather_*.xml, use that one.
    if (WeatherFiles.Num() == 1)
    {
        return FPaths::Combine(Directory, WeatherFiles[0]);
    }

    // None found, or multiple found without a matching stem.
    return FString();
}

bool UStandaloneWeatherParser::ParseEnvironmentFile(
    const FString& FilePath,
    FStandaloneWeatherData& OutData,
    FString& OutError)
{
    OutData = FStandaloneWeatherData();
    OutError.Reset();

    FXmlFile XmlFile(
        FilePath,
        EConstructMethod::ConstructFromFile);

    if (!XmlFile.IsValid())
    {
        OutError = FString::Printf(
            TEXT("Could not parse XML file '%s': %s"),
            *FilePath,
            *XmlFile.GetLastError());

        return false;
    }

    const FXmlNode* RootNode = XmlFile.GetRootNode();

    if (!RootNode)
    {
        OutError = TEXT("XML file has no root node.");
        return false;
    }

    const FXmlNode* EnvironmentNode =
        FindFirstNode(RootNode, TEXT("Environment"));

    if (EnvironmentNode)
    {
        if (const FXmlNode* TimeOfDayNode =
            GetDirectChild(EnvironmentNode, TEXT("TimeOfDay")))
        {
            ParseTimeOfDayNode(
                TimeOfDayNode,
                OutData);
        }

        if (const FXmlNode* WeatherNode =
            GetDirectChild(EnvironmentNode, TEXT("Weather")))
        {
            ParseWeatherNode(
                WeatherNode,
                OutData);
        }

        if (const FXmlNode* RoadConditionNode =
            GetDirectChild(EnvironmentNode, TEXT("RoadCondition")))
        {
            ParseRoadConditionNode(
                RoadConditionNode,
                OutData);
        }

        return true;
    }

    // Still allow standalone <Weather> files.
    if (const FXmlNode* WeatherNode =
        FindFirstNode(RootNode, TEXT("Weather")))
    {
        ParseWeatherNode(
            WeatherNode,
            OutData);

        return true;
    }

    OutError = TEXT(
        "Could not find an <Environment> or <Weather> element.");

    return false;
}


bool UStandaloneWeatherParser::ParseEnvironmentNode(
        const FXmlNode* EnvironmentNode,
        FStandaloneWeatherData& OutData,
        FString& OutError)
        {
               OutData = FStandaloneWeatherData();
    OutError.Reset();

    if (EnvironmentNode)
    {
        if (const FXmlNode* TimeOfDayNode =
            GetDirectChild(EnvironmentNode, TEXT("TimeOfDay")))
        {
            ParseTimeOfDayNode(
                TimeOfDayNode,
                OutData);
        }

        if (const FXmlNode* WeatherNode =
            GetDirectChild(EnvironmentNode, TEXT("Weather")))
        {
            ParseWeatherNode(
                WeatherNode,
                OutData);
        }

        if (const FXmlNode* RoadConditionNode =
            GetDirectChild(EnvironmentNode, TEXT("RoadCondition")))
        {
            ParseRoadConditionNode(
                RoadConditionNode,
                OutData);
        }

        return true;
    }

    // Still allow standalone <Weather> files.
    if (const FXmlNode* WeatherNode =
        FindFirstNode(EnvironmentNode, TEXT("Weather")))
    {
        ParseWeatherNode(
            WeatherNode,
            OutData);

        return true;
    }

    OutError = TEXT(
        "Could not find an <Environment> or <Weather> element.");

    return false;
        }


const FXmlNode* UStandaloneWeatherParser::FindFirstNode(
    const FXmlNode* Node,
    const FString& Tag)
{
    if (!Node)
    {
        return nullptr;
    }

    if (Node->GetTag() == Tag)
    {
        return Node;
    }

    for (const FXmlNode* Child : Node->GetChildrenNodes())
    {
        if (const FXmlNode* Found =
            FindFirstNode(Child, Tag))
        {
            return Found;
        }
    }

    return nullptr;
}

const FXmlNode* UStandaloneWeatherParser::GetDirectChild(
    const FXmlNode* Node,
    const FString& Tag)
{
    if (!Node)
    {
        return nullptr;
    }

    for (const FXmlNode* Child : Node->GetChildrenNodes())
    {
        if (Child->GetTag() == Tag)
        {
            return Child;
        }
    }

    return nullptr;
}

bool UStandaloneWeatherParser::TryGetDoubleAttribute(
    const FXmlNode* Node,
    const FString& Attribute,
    double& OutValue)
{
    if (!Node)
    {
        return false;
    }

    const FString Value =
        Node->GetAttribute(Attribute);

    if (Value.IsEmpty())
    {
        return false;
    }

    return LexTryParseString(
        OutValue,
        *Value);
}

void UStandaloneWeatherParser::ParseWeatherNode(
    const FXmlNode* WeatherNode,
    FStandaloneWeatherData& OutData)
{
    if (!WeatherNode)
    {
        return;
    }

    OutData.bHasTemperature =
        TryGetDoubleAttribute(
            WeatherNode,
            TEXT("temperature"),
            OutData.TemperatureKelvin);

    if (OutData.bHasTemperature)
    {
        OutData.TemperatureCelsius =
            OutData.TemperatureKelvin - 273.15;
    }

    OutData.bHasAtmosphericPressure =
        TryGetDoubleAttribute(
            WeatherNode,
            TEXT("atmosphericPressure"),
            OutData.AtmosphericPressurePascal);

    if (OutData.bHasAtmosphericPressure)
    {
        OutData.AtmosphericPressureHectoPascal =
            OutData.AtmosphericPressurePascal / 100.0;
    }

    OutData.FractionalCloudCover = WeatherNode->GetAttribute(
        TEXT("fractionalCloudCover"));
    if (!OutData.FractionalCloudCover.IsEmpty()) {
        OutData.bHasFractionalCloudCover = true;
        OutData.FractionalCloudCover_normalized = OktasToCloudCoverage(OutData.FractionalCloudCover);
    }

    if (const FXmlNode* FogNode =
        GetDirectChild(
            WeatherNode,
            TEXT("Fog")))
    {
        OutData.bHasFog =
            TryGetDoubleAttribute(
                FogNode,
                TEXT("visualRange"),
                OutData.VisualRangeMeters);
    }

    if (const FXmlNode* PrecipitationNode =
        GetDirectChild(
            WeatherNode,
            TEXT("Precipitation")))
    {
        OutData.PrecipitationType =
            PrecipitationNode->GetAttribute(
                TEXT("precipitationType"));

        const bool bValidPrecipitationType = OutData.PrecipitationType.Equals(TEXT("Rain"), ESearchCase::IgnoreCase) ||
            OutData.PrecipitationType.Equals(TEXT("Snow"), ESearchCase::IgnoreCase) ||
            OutData.PrecipitationType.Equals(TEXT("Dry"), ESearchCase::IgnoreCase);

        OutData.bHasPrecipitation = bValidPrecipitationType;

        if (OutData.bHasPrecipitation) {
            TryGetDoubleAttribute(
                PrecipitationNode,
                TEXT("precipitationIntensity"),
                OutData.PrecipitationIntensityMmPerHour);
        }
    }

    if (const FXmlNode* WindNode =
        GetDirectChild(
            WeatherNode,
            TEXT("Wind")))
    {
        const bool bHasSpeed =
            TryGetDoubleAttribute(
                WindNode,
                TEXT("speed"),
                OutData.WindSpeedMetersPerSecond);

        const bool bHasDirection =
            TryGetDoubleAttribute(
                WindNode,
                TEXT("direction"),
                OutData.WindDirectionRadians);

        OutData.bHasWind =
            bHasSpeed && bHasDirection;

        if (bHasSpeed)
        {
            OutData.WindSpeedKilometersPerHour =
                OutData.WindSpeedMetersPerSecond * 3.6;
        }

        if (bHasDirection)
        {
            OutData.WindDirectionDegrees =
                FMath::RadiansToDegrees(
                    OutData.WindDirectionRadians);
        }
    }
}

void UStandaloneWeatherParser::ParseTimeOfDayNode(
    const FXmlNode* TimeOfDayNode,
    FStandaloneWeatherData& OutData)
{
    if (!TimeOfDayNode)
    {
        return;
    }

    const FString AnimationString =
        TimeOfDayNode->GetAttribute(
            TEXT("animation"));

    OutData.bTimeOfDayAnimation =
        AnimationString.Equals(
            TEXT("true"),
            ESearchCase::IgnoreCase);

    OutData.TimeOfDayDateTime =
        TimeOfDayNode->GetAttribute(
            TEXT("dateTime"));

    if (!OutData.TimeOfDayDateTime.IsEmpty())
    {
        FDateTime ParsedDateTime;

        if (FDateTime::ParseIso8601(
            *OutData.TimeOfDayDateTime,
            ParsedDateTime))
        {
            OutData.ParsedTimeOfDay =
                ParsedDateTime;
            OutData.bHasTimeOfDay = true;
        }
    }
}

void UStandaloneWeatherParser::ParseRoadConditionNode(
    const FXmlNode* RoadConditionNode,
    FStandaloneWeatherData& OutData)
{
    if (!RoadConditionNode)
    {
        return;
    }

    OutData.bHasRoadCondition =
        TryGetDoubleAttribute(
            RoadConditionNode,
            TEXT("frictionScaleFactor"),
            OutData.FrictionScaleFactor);
}

float OktasToCloudCoverage(const FString& Oktas)
{
    if (Oktas == TEXT("zeroOktas"))  return 0.0f;
    if (Oktas == TEXT("oneOktas"))   return 1.0f / 8.0f;
    if (Oktas == TEXT("twoOktas"))   return 2.0f / 8.0f;
    if (Oktas == TEXT("threeOktas")) return 3.0f / 8.0f;
    if (Oktas == TEXT("fourOktas"))  return 4.0f / 8.0f;
    if (Oktas == TEXT("fiveOktas"))  return 5.0f / 8.0f;
    if (Oktas == TEXT("sixOktas"))   return 6.0f / 8.0f;
    if (Oktas == TEXT("sevenOktas")) return 7.0f / 8.0f;
    if (Oktas == TEXT("eightOktas")) return 1.0f;

    // nineOktas = sky obscured.
    // Choose appropriate behavior for your weather system.
    if (Oktas == TEXT("nineOktas"))  return 1.0f;

    return 0.0f;
}