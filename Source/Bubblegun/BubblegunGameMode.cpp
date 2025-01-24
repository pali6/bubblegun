// Copyright Epic Games, Inc. All Rights Reserved.

#include "BubblegunGameMode.h"
#include "BubblegunCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABubblegunGameMode::ABubblegunGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
