/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "Sensors/GemstarSensor.h"

AGemstarSensor::AGemstarSensor()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AGemstarSensor::BeginPlay()
{
	Super::BeginPlay();
	TickActive = true;
	PublishingActive = true;
	bReportedNoDataTarget = false;
}

void AGemstarSensor::Tick(float DeltaTime) 
{
}

void AGemstarSensor::SetWeatherData(const FWeatherData& NewWeatherData)
{
	CurrentWeatherData = NewWeatherData;
	HasWeatherData = true;
}

void AGemstarSensor::SetTickActive(bool Active)
{
	TickActive = Active;
}

void AGemstarSensor::SetPublishingActive(bool Active)
{
	PublishingActive = Active;
}

void AGemstarSensor::AddDataTarget(FDataTarget& dataTarget) 
{
	SensorConfiguration.DataTargets.AddUnique(dataTarget);
}

void AGemstarSensor::RemoveDataTarget(FDataTarget& dataTarget) 
{
	SensorConfiguration.DataTargets.Remove(dataTarget);

	if (IsValid(dataTarget.Actor)) {
		dataTarget.Actor->RemoveTickPrerequisiteActor(this);
	}
}

void AGemstarSensor::SetSensorConfiguration(FSensorConfigData NewSensorConfiguration)
{
	this->SensorConfiguration = NewSensorConfiguration;
}

void AGemstarSensor::UpdateTransformAfterAttachment_Implementation() 
{
	SetActorRelativeTransform(SensorConfiguration.Offset);
}

void AGemstarSensor::SendData(osi3::SensorView& SensorView)
{
	if (SensorConfiguration.DataTargets.Num() == 0) {

		if (!bReportedNoDataTarget) {
			bReportedNoDataTarget = true;
			UE_LOG(GemstarInterfaces, Warning, TEXT("Sensor data target not configured! %s"), *GetName());
		}
		return;
	}
	bReportedNoDataTarget = false;
	FSensorView sensorViewData;
	sensorViewData.sensorview = &SensorView;

	for (int32 i = SensorConfiguration.DataTargets.Num() - 1; i >= 0; --i) {
		FDataTarget dataTarget = SensorConfiguration.DataTargets[i];

		if (!IsValid(dataTarget.Actor)) {
			SensorConfiguration.DataTargets.RemoveAt(i);
		}
		else if (!dataTarget.Actor->IsUnreachable()) {
			ISensorDataUpdate::Execute_SensorViewDataUpdate(dataTarget.Actor, sensorViewData, dataTarget.TargetIndex, nullptr);
		}
	}
}