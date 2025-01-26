// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BubbleGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BUBBLEGUN_API UBubbleGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MouseSensitivity = 1.f;
	
};
