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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	float InitialRadius = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bubble")
	FVector GlobalForce = FVector::Zero();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bubble")
	double AverageVertexArea = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double ImpactVertexPushStrength = 300.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double ImpactGlobalPushStrength = 75.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	double GlobalBounceMultiplier = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bubble")
	bool bRandomizeColor = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bubble")
	double ActualRadius = 0;

	TMap<AActor*, TTuple<int32, FVector, double>> CurrentPushes;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Bubble")
	void Generate();

	void UpdateNormals();

	void UpdateCenterOfMass();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void RandomizeColor();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Bubble")
	void GrowBubble(double Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Bubble")
	float HitSingleVertexFactor(AActor* HitActor);

	float HitSingleVertexFactor_Implementation(AActor* HitActor) { return 1.0; }

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Bubble")
	float HitGlobalFactor(AActor* HitActor);

	float HitGlobalFactor_Implementation(AActor* HitActor) { return 1.0; }
};
