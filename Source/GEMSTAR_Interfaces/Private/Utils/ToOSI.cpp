/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "Utils/ToOSI.h"

ToOSI::ToOSI()
{
}

ToOSI::~ToOSI()
{
}

osi3::BaseStationary* ToOSI::BaseStationary(FTransform transform) {
	osi3::BaseStationary* baseStationary = new osi3::BaseStationary();
	baseStationary->set_allocated_position(Position(transform.GetLocation()));
	baseStationary->set_allocated_orientation(Orientation3d(transform.GetRotation()));
	baseStationary->set_allocated_dimension(Dimension(transform.GetScale3D()));
	return baseStationary;
}

osi3::Vector3d* ToOSI::Position(FVector vector) {
	osi3::Vector3d* vector3d = new osi3::Vector3d();
	//change hands and cm to m
	vector3d->set_x(vector.X * 0.01);
	vector3d->set_y(-vector.Y * 0.01);
	vector3d->set_z(vector.Z * 0.01);
	return vector3d;
}

osi3::Orientation3d* ToOSI::Orientation3d(FQuat quat) {
	osi3::Orientation3d* orientation = new osi3::Orientation3d();
	FRotator rotator = quat.Rotator();
	orientation->set_yaw(-rotator.Yaw * PI / 180.0);
	orientation->set_pitch(rotator.Pitch * PI / 180.0);
	orientation->set_roll(rotator.Roll * PI / 180.0);
	return orientation;
}

osi3::Dimension3d* ToOSI::Dimension(FVector dimension) {
	osi3::Dimension3d* dim = new osi3::Dimension3d();
	dim->set_length(dimension.X);
	dim->set_width(-dimension.Y);
	dim->set_height(dimension.Z);
	return dim;
}

osi3::BaseStationary* ToOSI::toOSIBaseStationary(const AActor* actor)
{
	osi3::BaseStationary* base = new osi3::BaseStationary();
	if (actor)
	{
		FVector Origin;
		FVector Extent;

		actor->GetActorBounds(false, Origin, Extent);

		base->mutable_dimension()->set_height(Extent.Z * 0.01);
		base->mutable_dimension()->set_width(Extent.Y * 0.01);
		base->mutable_dimension()->set_length(Extent.X * 0.01);
		base->mutable_position()->set_z(Origin.Z * 0.01);
		base->mutable_position()->set_y(-Origin.Y * 0.01);
		base->mutable_position()->set_z(Origin.X * 0.01);

		auto rotation = actor->GetActorRotation();

		base->mutable_orientation()->set_pitch(rotation.Pitch * PI / 180.f);
		base->mutable_orientation()->set_yaw(-rotation.Yaw * PI / 180.f);
		base->mutable_orientation()->set_roll(rotation.Roll * PI / 180.f);
	}

	return base;
}

osi3::BaseMoving* ToOSI::toOSIBaseMoving(const AActor* actor)
{
	osi3::BaseMoving* base = new osi3::BaseMoving();
	if (actor)
	{
		FVector Origin;
		FVector Extent;

		actor->GetActorBounds(false, Origin, Extent);

		base->mutable_dimension()->set_height(Extent.Z * 0.01);
		base->mutable_dimension()->set_width(Extent.Y * 0.01);
		base->mutable_dimension()->set_length(Extent.X * 0.01);
		base->mutable_position()->set_x(Origin.X * 0.01);
		base->mutable_position()->set_y(-Origin.Y * 0.01);
		base->mutable_position()->set_z(Origin.Z * 0.01);

		auto rotation = actor->GetActorRotation();

		base->mutable_orientation()->set_pitch(rotation.Pitch * PI / 180.f);
		base->mutable_orientation()->set_yaw(-rotation.Yaw * PI / 180.f);
		base->mutable_orientation()->set_roll(rotation.Roll * PI / 180.f);

		base->mutable_acceleration()->set_x(0);
		base->mutable_acceleration()->set_y(0);
		base->mutable_acceleration()->set_z(0);

		base->mutable_orientation_acceleration()->set_pitch(0);
		base->mutable_orientation_acceleration()->set_roll(0);
		base->mutable_orientation_acceleration()->set_yaw(0);

		FVector velocity = actor->GetVelocity();
		base->mutable_velocity()->set_x(velocity.X);
		base->mutable_velocity()->set_y(-velocity.Y);
		base->mutable_velocity()->set_z(velocity.Z);
	}

	return base;
}

osi3::MountingPosition* ToOSI::toMountingPosition(FSensorConfigData& sensorConfigData)
{
	osi3::MountingPosition* mounting_position = new osi3::MountingPosition();
	auto* orientation = mounting_position->mutable_orientation();
	orientation->set_roll(sensorConfigData.Offset.GetRotation().X * PI / 180.0);
	orientation->set_pitch(sensorConfigData.Offset.GetRotation().Y * PI / 180.0);
	orientation->set_yaw(sensorConfigData.Offset.GetRotation().Z * PI / -180.0);
	auto* position = mounting_position->mutable_position();
	position->set_x(sensorConfigData.Offset.GetLocation().X * 0.01);
	position->set_y(-sensorConfigData.Offset.GetLocation().Y * 0.01);
	position->set_z(sensorConfigData.Offset.GetLocation().Z * 0.01);
	return mounting_position;
}
