// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DynamicMeshComponent.h"
#include "Bubble.generated.h"

UCLASS()
class BUBBLEGUN_API ABubble : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ABubble();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	float Radius = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bubble")
	int Subdivisions = 3;

	// dynamic mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bubble")
	UDynamicMeshComponent* BubbleMesh;

	// material instance of the bubble
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bubble")
	UMaterialInstance* BubbleMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bubble")
	FVector CenterOfMass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double AirPressureForce = 500000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double SpringCoefficient = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double VelocityDamping = 0.999;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double ForceNoiseMagnitude = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double ForceBigNoiseMagnitude = 0.08;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double BigNoiseChangeInterval = 0.5;

	UPROPERTY(VisibleAnywhere, Category = "Bubble")
	FVector3d BigNoiseVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bubble")
	double BigNoiseChangeTimer = 0.0;

	TArray<double> TargetEdgeLengths;

	TArray<FVector3d> VertexVelocities;

	FVector3d GlobalForce = FVector3d::Zero();

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double AverageVertexArea = 0.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double ImpactVertexPushStrength = 300.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double ImpactGlobalPushStrength = 75.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double GlobalBounceMultiplier = 0.5;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	bool bRandomizeColor = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Generate();

	void UpdateNormals();

	void UpdateCenterOfMass();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void RandomizeColor();
};
