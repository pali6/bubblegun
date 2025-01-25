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

	UPROPERTY(EditAnywhere, Category = "Bubble")
	float Radius = 100.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	int Subdivisions = 3;

	// dynamic mesh
	UPROPERTY(VisibleAnywhere, Category = "Bubble")
	UDynamicMeshComponent* BubbleMesh;

	// material instance of the bubble
	UPROPERTY(EditAnywhere, Category = "Bubble")
	UMaterialInstance* BubbleMaterial;

	UPROPERTY(VisibleAnywhere, Category = "Bubble")
	FVector3d CenterOfMass;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double AirPressureForce = 500000;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double SpringCoefficient = 10.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double VelocityDamping = 0.999;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double ForceNoiseMagnitude = 20.0;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double ForceBigNoiseMagnitude = 0.05;

	UPROPERTY(EditAnywhere, Category = "Bubble")
	double BigNoiseChangeInterval = 0.5;

	UPROPERTY(VisibleAnywhere, Category = "Bubble")
	FVector3d BigNoiseVector;

	UPROPERTY(VisibleAnywhere, Category = "Bubble")
	double BigNoiseChangeTimer = 0.0;

	TArray<double> TargetEdgeLengths;

	TArray<FVector3d> VertexVelocities;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Generate();

	void UpdateNormals();

	void UpdateCenterOfMass();
};
