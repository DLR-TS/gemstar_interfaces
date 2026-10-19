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
#include "GameFramework/Actor.h"
#include "Interfaces/Sensor/SensorConfigurationData.h"
#include "Interfaces/Sensor/SensorDataDispatcher.h"
#include "Utils/LocalTransformUpdater.h"
#include "Interfaces/Weather/WeatherDataUpdate.h"
#include "Utils/GemstarInterfacesLog.h"
#include "GemstarSensor.generated.h"
/**
 * 
 */

UCLASS()
class GEMSTAR_INTERFACES_API AGemstarSensor : public AActor,
	public ISensorDataDispatcher,
	public ILocalTransformUpdater
{
	GENERATED_BODY()
public:
	AGemstarSensor();

	void Tick(float time) override;

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void SetSensorConfiguration(FSensorConfigData NewSensorConfiguration);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void AddDataTarget(FDataTarget& dataTarget);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void RemoveDataTarget(FDataTarget& dataTarget);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void SetWeatherData(const FWeatherData& NewWeatherData);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void SetTickActive(bool Active);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void SetPublishingActive(bool Active);

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	virtual void UpdateTransformAfterAttachment_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	FSensorConfigData SensorConfiguration;

	UPROPERTY(BlueprintReadOnly, Category = "GEMSTAR")
	bool HasWeatherData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FWeatherData CurrentWeatherData;
	UPROPERTY(BlueprintReadOnly, Category = "GEMSTAR")
	bool TickActive;

	UPROPERTY(BlueprintReadOnly, Category = "GEMSTAR")
	bool PublishingActive;

	virtual void SendData(osi3::SensorView& SensorView);
	bool bReportedNoDataTarget = false;
};