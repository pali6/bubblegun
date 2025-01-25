// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "BubblegunCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UCurveVector;
class UBubblegunWeaponComponent;
class USpringArmComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAltFireShot, float, TimeLeft);

UCLASS(config=Game)
class ABubblegunCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraHolderComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Dash Input Action */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* DashInputAction;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	UCurveVector* HeadBobCurve;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	UCurveFloat* LandedCameraCurve;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	float HeadBobRotationIntensity = 1.f;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	float HeadBobPositionIntensity = 1.f;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	float HeadBobFrequency = 2.f;

	UPROPERTY(EditAnywhere, Category = HeadBob)
	float HeadBobTransitionTime = 0.3f;

	UPROPERTY(EditAnywhere, Category = Input)
	float DashInitialSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, Category = Input)
	float DashDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = Input)
	float DashCooldown = 1.0f;

	UPROPERTY(EditAnywhere, Category = Input)
	TSubclassOf<UBubblegunWeaponComponent> BubblegunClass;

	UPROPERTY(EditAnywhere, Category = Input)
	TSubclassOf<UBubblegunWeaponComponent> AltWeaponClass;

	UPROPERTY(VisibleAnywhere)
	UBubblegunWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere)
	UBubblegunWeaponComponent* LeftWeaponComp;

	float HeadBobTimer = 0.f;
	float HeadBobTransitionTimer = 0.f;
	TOptional<FVector2D> LastInput;

	float DashTimer = -1.f;
	float DashCooldownTimer = -1.f;
	int PreDashJumpCount = 0;

	float LandedCameraTimer = 0.f;

public:

	ABubblegunCharacter();

protected:

	void InputMove(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);
	void InputDash();

	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	// For animation notify... the animation is already playing.
	UFUNCTION(BlueprintCallable)
	void FireSecondaryWeapon();

	bool IsDashing() const { return DashTimer >= 0.f; }
	bool CanDash() const { return DashCooldownTimer < 0.f && !IsDashing(); }

	UPROPERTY(BlueprintAssignable)
	FAltFireShot AltFireShot;

private:
	void UpdateHeadBob(float DeltaTime);
	void UpdateCameraOffset(float DeltaTime);
	void UpdateDash(float DeltaTime);
};

