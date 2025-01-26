// Copyright Epic Games, Inc. All Rights Reserved.

#include "BubblegunCharacter.h"
#include "BubblegunProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BubblegunWeaponComponent.h"
#include "BubbleGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Misc/Optional.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ABubblegunCharacter

ABubblegunCharacter::ABubblegunCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	CameraHolderComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraHolder"));
	CameraHolderComponent->SetupAttachment(GetCapsuleComponent());
	CameraHolderComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	CameraHolderComponent->bUsePawnControlRotation = true;
	CameraHolderComponent->TargetArmLength = 0.f;


	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(CameraHolderComponent);
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
}

//////////////////////////////////////////////////////////////////////////// Input

void ABubblegunCharacter::InputDash()
{
	if (!CanDash())
	{
		return;
	}

	DashCooldownTimer = DashCooldown;
	DashTimer = DashDuration;
	PreDashJumpCount = JumpCurrentCount;

	FVector2D Input = LastInput.IsSet()
		? LastInput.GetValue()
		: FVector2D(0, 1.f);

	FVector MoveDir = FVector::ZeroVector;
	MoveDir += GetActorRightVector() * Input.X;
	MoveDir += GetActorForwardVector() * Input.Y;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetCharacterMovement()->Velocity = MoveDir * DashInitialSpeed;
}

void ABubblegunCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABubblegunCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABubblegunCharacter::InputMove);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABubblegunCharacter::InputLook);

		// Dashing
		EnhancedInputComponent->BindAction(DashInputAction, ETriggerEvent::Triggered, this, &ABubblegunCharacter::InputDash);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABubblegunCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHeadBob(DeltaTime);
	UpdateCameraOffset(DeltaTime);
	UpdateDash(DeltaTime);
	LastInput.Reset();
}

void ABubblegunCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	LandedCameraTimer = 0.f;
}

void ABubblegunCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (BubblegunClass)
	{
		WeaponComp = NewObject<UBubblegunWeaponComponent>(this, BubblegunClass);
		WeaponComp->CastShadow = false;
		WeaponComp->AttachWeapon(this, FName(TEXT("GripPoint")));
		WeaponComp->RegisterComponent();
	}

	if (AltWeaponClass)
	{
		LeftWeaponComp = NewObject<UBubblegunWeaponComponent>(this, AltWeaponClass);
		LeftWeaponComp->CastShadow = false;
		LeftWeaponComp->AttachWeapon(this, FName(TEXT("GripPointLeft")));
		LeftWeaponComp->RegisterComponent();
	}
}

void ABubblegunCharacter::FireSecondaryWeapon()
{
	if (!LeftWeaponComp)
	{
		return;
	}

	LeftWeaponComp->FireProjectile();
	LeftWeaponComp->PlayFireSound();
	LeftWeaponComp->SpawnFireVFX();
	AltFireShot.Broadcast(LeftWeaponComp->GetCooldownRemaining());
}

void ABubblegunCharacter::UpdateHeadBob(float DeltaTime)
{
	if (LastInput.IsSet() && GetCharacterMovement()->IsWalking())
	{
		HeadBobTimer += DeltaTime * HeadBobFrequency;
		HeadBobTimer = FMath::Modulo(HeadBobTimer, 1.f);
		HeadBobTransitionTimer += DeltaTime / HeadBobTransitionTime;
	}
	else
	{
		HeadBobTransitionTimer -= DeltaTime / HeadBobTransitionTime;
	}

	HeadBobTransitionTimer = FMath::Clamp(HeadBobTransitionTimer, 0.f, 1.f);

	if (HeadBobCurve)
	{
		FVector HeadBobPosition = HeadBobCurve->GetVectorValue(HeadBobTimer) * HeadBobTransitionTimer;
		FirstPersonCameraComponent->SetRelativeLocation(HeadBobPosition * HeadBobPositionIntensity);

		FVector EulerRotation = HeadBobCurve->GetVectorValue(HeadBobTimer) * HeadBobRotationIntensity;
		EulerRotation *= HeadBobTransitionTimer;
		FQuat HeadBobRotation = FQuat::MakeFromEuler(FVector(0.f, EulerRotation.Z, EulerRotation.Y));
		FirstPersonCameraComponent->SetRelativeRotation(HeadBobRotation);
	}
}

void ABubblegunCharacter::UpdateCameraOffset(float DeltaTime)
{
	if (!LandedCameraCurve)
	{
		return;
	}
	
	// No more than a minute of falling animation, cmon
	const float SAFE_MAX = 60;
	if (LandedCameraTimer < SAFE_MAX)
	{
		LandedCameraTimer += DeltaTime;
	}

	// In Meters
	float CameraOffset = LandedCameraCurve->GetFloatValue(LandedCameraTimer) * 100; 
	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, CameraOffset));
}

void ABubblegunCharacter::UpdateDash(float DeltaTime)
{
	if (DashTimer >= 0.f)
	{
		FVector Dir = GetCharacterMovement()->Velocity.GetSafeNormal();
		float StoppingSpeed = GetCharacterMovement()->MaxWalkSpeed;
		float T = 1.f - DashTimer / DashDuration;
		GetCharacterMovement()->Velocity = Dir * FMath::Lerp(DashInitialSpeed, StoppingSpeed, T * T * T);

		// Just finished dashing
		if (DashTimer < DeltaTime)
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
			GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetClampedToSize(0.f, GetCharacterMovement()->MaxWalkSpeed);
			JumpCurrentCount = PreDashJumpCount;
		}

		DashTimer -= DeltaTime;
		return;
	}

	if (DashCooldownTimer >= 0.f)
	{
		DashCooldownTimer -= DeltaTime;
	}
}


void ABubblegunCharacter::InputMove(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// add movement 
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
	LastInput = MovementVector;
}

void ABubblegunCharacter::InputLook(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	float MouseSensitivity = 1.f;
	if (UBubbleGameInstance* GameInstance = Cast<UBubbleGameInstance>(GetGameInstance()))
	{
		MouseSensitivity = GameInstance->MouseSensitivity;
	}

	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
	AddControllerPitchInput(LookAxisVector.Y * MouseSensitivity);

}