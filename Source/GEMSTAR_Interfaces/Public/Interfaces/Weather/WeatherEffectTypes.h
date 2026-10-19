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
#include "WeatherEffectTypes.generated.h"

namespace GemstarWeatherTypes {
	static const FName Clear(TEXT("Clear"));
	static const FName Fog(TEXT("Fog"));
	static const FName Rain(TEXT("Rain"));
	static const FName Snow(TEXT("Snow"));
}

namespace GemstarPrecipitationType {
	static const FName None(TEXT("None"));
	static const FName Dry(TEXT("Dry"));
	static const FName Rain(TEXT("Rain"));
	static const FName Snow(TEXT("Snow"));
}

namespace GEMSTARSensorWeatherParameter {
	// Lidar spacific weather parameters
	static const FName LidarCoefficient(TEXT("LidarCoefficient"));
	static const FName ReflectivityThreshold(TEXT("ReflectivityThreshold"));
	// Camera spacific weather parameters

	// Radae spacific weather parameters

}

UENUM(BlueprintType)
enum class EGEMSTARSensorType : uint8 {
	Unknown = 0 UMETA(DispalyName = "Unknown"),
	Lidar = 1 UMETA(DispalyName = "Lidar"),
	Camera = 2 UMETA(DispalyName = "Camera"),
	Radar = 3 UMETA(DispalyName = "Radar")
};

USTRUCT(BlueprintType)
struct GEMSTAR_INTERFACES_API FGEMSTARTimeOfDayInfo {
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasTimeOfDay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FString ScenarioDateTime; // OpenSCENARIO dateTime

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double TimeOfDayInSeconds = 0.0; // DateTime in seconds since midnight [0,86400)
};

USTRUCT(BlueprintType)
struct GEMSTAR_INTERFACES_API FGEMSTARWeatherState {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bWeatherEffectEnabled = false;

	//osi3::EnvironmentalCondtions EnvironmentalCondtions; // Environmental stat from OSI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FName ActiveWeatherType = NAME_None;
	//--------------------------
	// Fog Visibility/ MOR
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasMOR = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double MOR = 0.0; 	// Meteorological Optical Range in meters
	//--------------------------
	// Precipitation
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasPrecipitation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FName PrecipitationType = GemstarPrecipitationType::None; 	// The intensity of the precipitation [mm/h]

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double PrecipitationIntensity = 0.0;
	//--------------------------
	// Temperature and Pressure
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasTemperature = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double Temperature = 0.0; 	// Temperature in Kelvin

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasPressure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double AtmosphericPressure = 0.0; 	// Atmospheric Pressure in Pascal
	//--------------------------
	// Wind
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasWind = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double WindSpeed = 0.0; 	// Wind speed in m/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double WindDirection = 0.0; 	// Wind direction in radians
	//--------------------------
	// Sun/ Light
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasSun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double SunIlluminance = 0.0; 	// light strength in lux

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double SunAzimuth = 0.0; 	// Sun azimuth in radians

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	double SunElevation = 0.0; 	// Position of the Sun: Elevation angle in radians
	//--------------------------
	// Cloud Cover
	//--------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bHasFractionalCloudCover = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	float FractionalCloudCover = 0.0f; 	// FractionalCloudCover stored as oktas
};
