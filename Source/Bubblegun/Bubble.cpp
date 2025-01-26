// Fill out your copyright notice in the Description page of Project Settings.


#include "Bubble.h"

#include "Templates/Tuple.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <MathUtil.h>
#include <Kismet/GameplayStatics.h>

struct MeshRepr {
	TArray<FVector> Positions;

	TArray<TPair<int32, int32>> Edges;

	TArray<TTuple<int32, int32, int32>> Faces;

	void RescaleToSphere(double radius) {
		for (int32 i = 0; i < Positions.Num(); i++) {
			Positions[i] = Positions[i].GetSafeNormal() * radius;
		}
	}

	static MeshRepr GetOctahedron() {
		TArray<FVector> positions{
			{ 1, 0, 0 },
			{ -1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, -1, 0 },
			{ 0, 0, 1 },
			{ 0, 0, -1 }
		};

		TArray<TPair<int32, int32>> edges{
			{ 0, 2 },
			{ 0, 3 },
			{ 0, 4 },
			{ 0, 5 },
			{ 1, 2 },
			{ 1, 3 },
			{ 1, 4 },
			{ 1, 5 },
			{ 2, 4 },
			{ 2, 5 },
			{ 3, 4 },
			{ 3, 5 }
		};

		TArray<TTuple<int32, int32, int32>> faces{
			{ 0, 2, 4 },
			{ 0, 4, 3 },
			{ 0, 3, 5 },
			{ 0, 5, 2 },
			{ 1, 4, 2 },
			{ 1, 3, 4 },
			{ 1, 5, 3 },
			{ 1, 2, 5 }
		};

		return MeshRepr{
			positions,
			edges,
			faces
		};
	}

	static MeshRepr GetIcosahedron() {
		double phi = (1 + FMath::Sqrt(5.0)) / 2;
		TArray<FVector> positions{
			{ phi, 1, 0 }, { -phi, 1, 0 }, { phi, -1, 0 }, { -phi, -1, 0 },
			{ 1, 0, phi }, { 1, 0, -phi }, { -1, 0, phi }, { -1, 0, -phi },
			{ 0, phi, 1 }, { 0, -phi, 1 }, { 0, phi, -1 }, { 0, -phi, -1 }
		};
		TArray<TPair<int32, int32>> edges;
		edges.Reserve(30);
		TArray<TTuple<int32, int32, int32>> faces{
			{ 0, 8, 4 }, { 0, 5, 10 }, { 2, 4, 9 }, { 2, 11, 5 }, { 1, 6, 8 },
			{ 1, 10, 7 }, { 3, 9, 6 }, { 3, 7, 11 }, { 0, 10, 8 }, { 1, 8, 10 },
			{ 2, 9, 11 }, { 3, 11, 9 }, { 4, 2, 0 }, { 5, 0, 2 }, { 6, 1, 3 },
			{ 7, 3, 1 }, { 8, 6, 4 }, { 9, 4, 6 }, { 10, 5, 7 }, { 11, 7, 5 }
		};
		for (auto& face : faces) {
			FVector3d v0 = positions[face.Get<0>()];
			FVector3d v1 = positions[face.Get<1>()];
			FVector3d v2 = positions[face.Get<2>()];
			FVector3d normal = FVector3d::CrossProduct(v1 - v0, v2 - v0).GetSafeNormal();
			checkf(FVector3d::DotProduct(v0, normal) >= 0, TEXT("Icosahedron face %d %d %d is not CCW"), face.Get<0>(), face.Get<1>(), face.Get<2>());
			if (face.Get<0>() < face.Get<1>()) edges.Add({ face.Get<0>(), face.Get<1>() });
			if (face.Get<1>() < face.Get<2>()) edges.Add({ face.Get<1>(), face.Get<2>() });
			if (face.Get<2>() < face.Get<0>()) edges.Add({ face.Get<2>(), face.Get<0>() });
		}
		checkf(edges.Num() == 30, TEXT("Icosahedron has %d edges, expected 30"), edges.Num());
		return MeshRepr{
			positions,
			edges,
			faces
		};
	}

	void Subdivide() {
		TArray<TPair<int32, int32>> newEdges;
		TArray<TTuple<int32, int32, int32>> newFaces;
		TMap<TPair<int32, int32>, int32> edgeToVertex;
		for (auto edge : Edges) {
			int32 v0 = edge.Get<0>();
			int32 v1 = edge.Get<1>();
			int32 newVertexIndex = Positions.Num();
			Positions.Add((Positions[v0] + Positions[v1]) / 2);
			edgeToVertex.Add(TPair<int32, int32>{FMathf::Min(v0, v1), FMathf::Max(v0, v1)}, newVertexIndex);
		}
		for (auto face : Faces) {
			auto [v0, v1, v2] = face;
			int32 v3 = edgeToVertex[TPair<int32, int32>{FMathf::Min(v0, v1), FMathf::Max(v0, v1)}];
			int32 v4 = edgeToVertex[TPair<int32, int32>{FMathf::Min(v1, v2), FMathf::Max(v1, v2)}];
			int32 v5 = edgeToVertex[TPair<int32, int32>{FMathf::Min(v2, v0), FMathf::Max(v2, v0)}];
			if (v0 < v3) newEdges.Add({ v0, v3 });
			if (v3 < v5) newEdges.Add({ v3, v5 });
			if (v5 < v0) newEdges.Add({ v5, v0 });
			if (v1 < v4) newEdges.Add({ v1, v4 });
			if (v4 < v3) newEdges.Add({ v4, v3 });
			if (v3 < v1) newEdges.Add({ v3, v1 });
			if (v2 < v5) newEdges.Add({ v2, v5 });
			if (v5 < v4) newEdges.Add({ v5, v4 });
			if (v4 < v2) newEdges.Add({ v4, v2 });
			if (v3 < v4) newEdges.Add({ v3, v4 });
			if (v4 < v5) newEdges.Add({ v4, v5 });
			if (v5 < v3) newEdges.Add({ v5, v3 });
			newFaces.Add({ v0, v3, v5 });
			newFaces.Add({ v1, v4, v3 });
			newFaces.Add({ v2, v5, v4 });
			newFaces.Add({ v3, v4, v5 });
		}
		Edges = newEdges;
		Faces = newFaces;
	}

	static MeshRepr GetSphere(float radius, int32 nSubdivisions, bool bUseIcosahedron = false) {
		auto mesh = bUseIcosahedron ? GetIcosahedron() : GetOctahedron();
		for (int32 i = 0; i < nSubdivisions; i++) {
			mesh.Subdivide();
		}
		mesh.RescaleToSphere(radius);
		return mesh;
	}
};

static FRandomStream BubbleRandomStream = FRandomStream();

// Sets default values
ABubble::ABubble()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BubbleMesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("BubbleMesh"));
	RootComponent = BubbleMesh;
	//BubbleMesh->SetMobility(EComponentMobility::Movable);
	//BubbleMesh->SetSimulatePhysics(true);
	BubbleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BubbleMesh->EnableComplexAsSimpleCollision();
	BubbleMesh->bEnableComplexCollision = true;
	BubbleMesh->ColorMode = EDynamicMeshComponentColorOverrideMode::None;

	BubbleMesh->GetBodyInstance()->bUseCCD = true;
}

// Called when the game starts or when spawned
void ABubble::BeginPlay()
{
	Super::BeginPlay();

	BubbleMesh->OnComponentHit.AddDynamic(this, &ABubble::OnHit);
	BubbleMesh->OnComponentBeginOverlap.AddDynamic(this, &ABubble::OnOverlapBegin);
	BubbleMesh->OnComponentEndOverlap.AddDynamic(this, &ABubble::OnOverlapEnd);
	
	Generate();
}

// Called every frame
void ABubble::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCenterOfMass();

	if (BigNoiseChangeTimer <= 0 || BubbleRandomStream.GetFraction() < DeltaTime * (1.0 - BigNoiseChangeTimer / BigNoiseChangeInterval)) {
		BigNoiseVector = BubbleRandomStream.GetUnitVector();
		BigNoiseChangeTimer = BigNoiseChangeInterval;
	}
	BigNoiseChangeTimer -= DeltaTime;

	double deltaTime = FMathf::Min(DeltaTime, 1 / 15.0);

	double VertexDisplacementSum = 0;
	int VertexCount = 0;

	TArray<FVector3d> forces;
	forces.Init(FVector3d::Zero(), BubbleMesh->GetDynamicMesh()->GetMeshRef().MaxVertexID());
	BubbleMesh->GetDynamicMesh()->ProcessMesh(
		[&](const FDynamicMesh3& Mesh) {
			for (int32 i = 0; i < Mesh.MaxVertexID(); i++) {
				FVector3d force{ 0, 0, 0 };

				FVector3d pos = Mesh.GetVertex(i);
				FVector3d airPressureForce = (pos - CenterOfMass).GetSafeNormal();
				airPressureForce = (airPressureForce + FVector(Mesh.GetVertexNormal(i))).GetSafeNormal();
				double comDistance = (pos - CenterOfMass).Size();
				airPressureForce *= 1 / (comDistance * comDistance) * AirPressureForce;
				force += airPressureForce;

				Mesh.EnumerateVertexEdges(i, [&](int32 edgeId) {
					auto edge = Mesh.GetEdge(edgeId);
					int32 neigh = edge.Vert.A == i ? edge.Vert.B : edge.Vert.A;
					FVector3d neighPos = Mesh.GetVertex(neigh);
					FVector3d springForce = (neighPos - pos).GetSafeNormal();
					double edgeLength = (neighPos - pos).Size();
					double targetLength = TargetEdgeLengths[edgeId] * Radius / InitialRadius;
					springForce *= (edgeLength - targetLength) * SpringCoefficient;
					force += springForce;
				});

				force += BubbleRandomStream.GetUnitVector() * ForceNoiseMagnitude;
				force += (2.0 * FMath::Abs(FVector3d::DotProduct(pos - CenterOfMass, BigNoiseVector)) - 1) * (CenterOfMass - pos).GetSafeNormal() * ForceBigNoiseMagnitude;
				force += GlobalForce / deltaTime;

				forces[i] = force;

				VertexDisplacementSum += (pos - CenterOfMass).Size();
				VertexCount++;
			}
		});
	ActualRadius = VertexDisplacementSum / VertexCount;
	
	GlobalForce = FVector3d::Zero();

	FDynamicMesh3* mesh = BubbleMesh->GetDynamicMesh()->GetMeshPtr();
	int index = -1;
	TArray<AActor*> actorsToRemove;
	for (auto& push : CurrentPushes) {
		index++;
		auto [faceIndex, velocityDelta, distance] = push.Value;
		AActor* actor = push.Key;

		if (!IsValid(actor)) {
			actorsToRemove.Add(actor);
			continue;
		}
		
		double actorDistance = (actor->GetActorLocation() - (GetActorLocation() + CenterOfMass)).Size();
		if (actorDistance > distance * 1.5) {
			actorsToRemove.Add(actor);
			continue;
		}

		auto hitFace = mesh->GetTriangle(faceIndex);
		int v0i = hitFace.A;
		int v1i = hitFace.B;
		int v2i = hitFace.C;
		FVector3d vertexVelocityDelta = velocityDelta / 3 * ImpactVertexPushStrength;
		VertexVelocities[v0i] += vertexVelocityDelta;
		VertexVelocities[v1i] += vertexVelocityDelta;
		VertexVelocities[v2i] += vertexVelocityDelta;

		GlobalForce += velocityDelta * ImpactGlobalPushStrength;
	}
	for (auto actor : actorsToRemove) {
		CurrentPushes.Remove(actor);
	}

	FVector3d totalBounce = FVector3d::Zero();
	BubbleMesh->GetDynamicMesh()->EditMesh(
		[&](FDynamicMesh3& Mesh) {
			for (int32 i = 0; i < Mesh.MaxVertexID(); i++) {
				double mass = 0;
				Mesh.EnumerateVertexTriangles(i, [&](int32 face) {
					FVector3d v0, v1, v2;
					Mesh.GetTriVertices(face, v0, v1, v2);
					double faceArea = FMath::Abs(FVector3d::CrossProduct(v1 - v0, v2 - v0).Size() / 2);
					mass += faceArea;
				});
				mass /= 3;
				FVector3d vel = VertexVelocities[i];
				FVector3d force = forces[i];
				vel += force * deltaTime;
				VertexVelocities[i] = vel * VelocityDamping;
				FVector3d newPos = Mesh.GetVertex(i) + vel * deltaTime;
				auto params = FCollisionQueryParams::DefaultQueryParam;
				params.AddIgnoredActor(this);
				FHitResult Hit;
				FVector3d ActorPos = GetActorLocation();
				if (GetWorld()->LineTraceSingleByChannel(Hit, Mesh.GetVertex(i) + ActorPos, newPos + ActorPos, ECC_WorldDynamic, params)) {
					//Mesh.SetVertex(i, Mesh.GetVertex(i) - vel * deltaTime);
					// proper reflection taking Hit.ImpactNormal into account
					FVector3d bounce = -FVector3d::DotProduct(VertexVelocities[i], Hit.ImpactNormal) * Hit.ImpactNormal;
					VertexVelocities[i] = (VertexVelocities[i] + 1.9 * bounce);
					totalBounce += bounce;
				}
				else {
					Mesh.SetVertex(i, newPos);
				}
			}
		}
	);
	GlobalForce += totalBounce * GlobalBounceMultiplier;

	UpdateNormals();
}

void ABubble::Generate() {
	// BubbleMesh->SetOverrideRenderMaterial(BubbleMaterial);
	BubbleMesh->SetMaterial(0, BubbleMaterial);

	Radius = InitialRadius;
	auto mesh = MeshRepr::GetSphere(Radius, Subdivisions, true);

	FDynamicMesh3 dynMesh{ true, true, false, false };
	dynMesh.EnableVertexColors(FVector4f{ 0, 1, 0, 1 });
	dynMesh.EnableAttributes();
	dynMesh.Attributes()->EnablePrimaryColors();
	auto ColorOverlay = dynMesh.Attributes()->PrimaryColors();

	for (const auto& pos : mesh.Positions) {
		dynMesh.AppendVertex(pos);
		ColorOverlay->AppendElement(FVector4f{ 0, 1, 0, 1 });
	}
	for (auto face : mesh.Faces) {
		int id = dynMesh.AppendTriangle(face.Get<1>(), face.Get<0>(), face.Get<2>());
		ColorOverlay->SetTriangle(id, UE::Geometry::FIndex3i{ face.Get<1>(), face.Get<0>(), face.Get<2>() });
	}

	for (auto edge : dynMesh.GetEdgesBuffer()) {
		FVector3d v0 = dynMesh.GetVertex(edge.Vert.A);
		FVector3d v1 = dynMesh.GetVertex(edge.Vert.B);
		TargetEdgeLengths.Add((v1 - v0).Size());
	}

	VertexVelocities.Init(FVector3d::Zero(), dynMesh.MaxVertexID());

	AverageVertexArea = 0;
	for (int32 i = 0; i < dynMesh.MaxVertexID(); i++) {
		double faceAreaSum = 0;
		int faceCount = 0;
		dynMesh.EnumerateVertexTriangles(i, [&](int32 face) {
			FVector3d v0, v1, v2;
			dynMesh.GetTriVertices(face, v0, v1, v2);
			double faceArea = FMath::Abs(FVector3d::CrossProduct(v1 - v0, v2 - v0).Size() / 2);
			faceAreaSum += faceArea;
			faceCount++;
		});
		double vertexArea = faceAreaSum / faceCount / 3.0;
		AverageVertexArea += vertexArea;
	}
	AverageVertexArea /= dynMesh.MaxVertexID();

	UDynamicMesh* dynamicMesh = NewObject<UDynamicMesh>();
	dynamicMesh->SetMesh(MoveTemp(dynMesh));

	BubbleMesh->SetDynamicMesh(dynamicMesh);

	UpdateNormals();

	if (bRandomizeColor)
		RandomizeColor();
}

void ABubble::UpdateNormals() {
	auto ColorOverlay = BubbleMesh->GetDynamicMesh()->GetMeshRef().Attributes()->PrimaryColors();
	BubbleMesh->GetDynamicMesh()->EditMesh(
		[&](FDynamicMesh3& Mesh) {
			for (int32 i = 0; i < Mesh.MaxVertexID(); i++) {
				FVector3d faceNormalSum = FVector3d::Zero();
				double faceAreaSum = 0.0;
				int faceCount = 0;
				Mesh.EnumerateVertexTriangles(i, [&](int32 faceID) {
					FVector3d v0, v1, v2;
					Mesh.GetTriVertices(faceID, v0, v1, v2);
					double faceArea = FMath::Abs(FVector3d::CrossProduct(v1 - v0, v2 - v0).Size() / 2);
					faceAreaSum += faceArea;
					FVector3d faceNormal = -FVector3d::CrossProduct(v1 - v0, v2 - v0).GetSafeNormal();
					faceNormalSum += faceNormal;
					faceCount++;
					});
				FVector3d normal = faceNormalSum / faceCount;
				Mesh.SetVertexNormal(i, FVector3f(normal));

				double vertexArea = faceAreaSum / faceCount / 3.0;
				Mesh.SetVertexColor(i, FVector4f(vertexArea / AverageVertexArea / 4.0, 0.0, 0.0, 1.0));
				ColorOverlay->SetElement(i, FVector4f(vertexArea / AverageVertexArea / 4.0, 0.0, 1.0, 1.0));
			}
		}
	);
	BubbleMesh->NotifyMeshUpdated();
}

void ABubble::UpdateCenterOfMass() {
	BubbleMesh->GetDynamicMesh()->ProcessMesh(
		[&](const FDynamicMesh3& Mesh) {
			FVector3d centerOfMass = FVector3d::Zero();
			double totalArea = 0;
			for (auto triangleIndex : Mesh.GetTrianglesBuffer()) {
				FVector3d v0 = Mesh.GetVertex(triangleIndex.A);
				FVector3d v1 = Mesh.GetVertex(triangleIndex.B);
				FVector3d v2 = Mesh.GetVertex(triangleIndex.C);
				FVector3d faceCenter = (v0 + v1 + v2) / 3;
				double faceArea = FMath::Abs(FVector3d::CrossProduct(v1 - v0, v2 - v0).Size() / 2);
				centerOfMass += faceCenter * faceArea;
				totalArea += faceArea;
			}
			CenterOfMass = centerOfMass / totalArea;
		}
	);
}

void ABubble::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if (!IsValid(OtherActor) || OtherActor == this || !IsValid(OtherComp))
		return;

	FDynamicMesh3* mesh = BubbleMesh->GetDynamicMesh()->GetMeshPtr();
	int hitFaceIndex = Hit.FaceIndex;
	if (hitFaceIndex == INDEX_NONE)
	{
		FVector TraceStart = Hit.ImpactPoint + Hit.ImpactNormal * 10.0f;
		FVector TraceEnd = Hit.ImpactPoint - Hit.ImpactNormal * 10.0f;

		FCollisionQueryParams QueryParams;
		QueryParams.bReturnFaceIndex = true;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(OtherActor);

		FHitResult TraceHit;
		bool bHit = GetWorld()->LineTraceSingleByChannel(TraceHit, TraceEnd, TraceStart, ECC_WorldDynamic, QueryParams);

		if (bHit && TraceHit.GetActor() == this)
		{
			int32 HitFaceIndex = TraceHit.FaceIndex;
			if (HitFaceIndex != INDEX_NONE)
			{
				hitFaceIndex = HitFaceIndex;
			}
		}
	}
	if (hitFaceIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("No hit face found"));
		return;
	}

	auto hitFace = mesh->GetTriangle(hitFaceIndex);
	int v0i = hitFace.A;
	int v1i = hitFace.B;
	int v2i = hitFace.C;

	FVector3d velocityDelta = FVector3d(Hit.ImpactNormal);

	UE_LOG(LogTemp, Warning, TEXT("Hit face %d %d %d with velocity delta %s, current velocity is %s"), v0i, v1i, v2i, *velocityDelta.ToString(), *VertexVelocities[v0i].ToString());

	FVector3d vertexVelocityDelta = velocityDelta / 3 * ImpactVertexPushStrength * HitSingleVertexFactor(OtherActor);
	VertexVelocities[v0i] += vertexVelocityDelta;
	VertexVelocities[v1i] += vertexVelocityDelta;
	VertexVelocities[v2i] += vertexVelocityDelta;

	GlobalForce += velocityDelta * ImpactGlobalPushStrength * HitGlobalFactor(OtherActor);

	CurrentPushes.Add(OtherActor, TTuple<int32, FVector, double>{hitFaceIndex, velocityDelta, (OtherActor->GetActorLocation() - (GetActorLocation() + CenterOfMass)).Size()});
}

void ABubble::RandomizeColor() {
	auto Material = BubbleMaterial->GetMaterial();
	// Create a dynamic material instance
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);

	if (DynamicMaterial)
	{
		FVector NewColor = FVector(BubbleRandomStream.GetFraction(), BubbleRandomStream.GetFraction(), BubbleRandomStream.GetFraction());
		NewColor /= 2.0;
		//NewColor *= 1.0 / NewColor.Size() * (1.0 - 0.5 * (1.0 - NewColor.Size()));
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
		BubbleMesh->SetMaterial(0, DynamicMaterial);
	}
}

void ABubble::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!IsValid(OtherActor) || OtherActor == this || !IsValid(OtherComp))
		return;
	UE_LOG(LogTemp, Warning, TEXT("Overlap begin with %s"), *OtherActor->GetName());
}

void ABubble::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (!IsValid(OtherActor) || OtherActor == this || !IsValid(OtherComp))
		return;
	UE_LOG(LogTemp, Warning, TEXT("Overlap end with %s"), *OtherActor->GetName());
}

void ABubble::GrowBubble(double Amount) {
	double NewRadius = Radius + Amount;
	AirPressureForce = AirPressureForce * FMath::Pow(NewRadius / Radius, 4);
	Radius = NewRadius;
}

void ABubble::Pop() {
	if (PopSound) {
		UGameplayStatics::PlaySoundAtLocation(this, PopSound, GetActorLocation() + CenterOfMass);
	}

	double PopForce = ActualRadius;

	// explosive force
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(HitResults, GetActorLocation() + CenterOfMass, GetActorLocation() + CenterOfMass, FQuat::Identity, ECC_WorldDynamic, FCollisionShape::MakeSphere(PopForce * PopForceBase), QueryParams);
	for (auto& Hit : HitResults) {
		if (!IsValid(Hit.GetActor()) || Hit.GetActor() == this)
			continue;
		if (Cast<ABubble>(Hit.GetActor()))
			continue;

		FVector Direction = (Hit.ImpactPoint - (GetActorLocation() + CenterOfMass)).GetSafeNormal();
		double Distance = (Hit.ImpactPoint - (GetActorLocation() + CenterOfMass)).Size();

		double PopForce2 = FMath::Max(0, FMath::Pow(PopForce - 30, 2.5));
		auto Character = Cast<ACharacter>(Hit.GetActor());
		if (Character) {
			Character->GetCharacterMovement()->AddImpulse(Direction * PopForce2 * PopForceBase / Distance, true);
			UE_LOG(LogTemp, Warning, TEXT("Character hit"));
		}
		else {
			auto Primitive = Cast<UPrimitiveComponent>(Hit.GetActor()->GetRootComponent());
			if (Primitive && Primitive->IsSimulatingPhysics()) {
				Primitive->AddImpulseAtLocation(Direction * PopForce2 * PopForceBase / Distance, Hit.ImpactPoint);
				UE_LOG(LogTemp, Warning, TEXT("Primitive %s hit"), *Hit.GetActor()->GetName());
			}
		}
	}

	Destroy();
}