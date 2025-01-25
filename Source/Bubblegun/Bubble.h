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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Generate();

	void UpdateNormals();
};
