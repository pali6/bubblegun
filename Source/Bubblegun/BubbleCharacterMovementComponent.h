// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BubbleCharacterMovementComponent.generated.h"

class UCurveFloat;

/**
 * 
 */
UCLASS()
class BUBBLEGUN_API UBubbleCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* GravityCurve;

	float GravityTimer;

public:

	// BEGIN UCharacterMovement Interface
	float GetGravityZ() const override;
	bool DoJump(bool bReplayingMoves, float DeltaTime) override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// END UCharacterMovement Interface

};
