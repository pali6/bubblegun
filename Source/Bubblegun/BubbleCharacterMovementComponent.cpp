// Fill out your copyright notice in the Description page of Project Settings.


#include "Curves/CurveFloat.h"
#include "BubbleCharacterMovementComponent.h"

float UBubbleCharacterMovementComponent::GetGravityZ() const
{
	if (GravityCurve)
	{
		return UMovementComponent::GetGravityZ() * GravityCurve->GetFloatValue(GravityTimer);
	}
	else
	{
		return Super::GetGravityZ();
	}
}

bool UBubbleCharacterMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	GravityTimer = 0.f;
	return Super::DoJump(bReplayingMoves, DeltaTime);
}

void UBubbleCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	GravityTimer = IsFalling()
		? GravityTimer + DeltaTime
		: 0.f;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
