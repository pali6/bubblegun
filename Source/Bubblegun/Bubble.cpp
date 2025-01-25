// Fill out your copyright notice in the Description page of Project Settings.


#include "Bubble.h"

#include "Templates/Tuple.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "DynamicMesh/DynamicMesh3.h"
#include <MathUtil.h>

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


}

// Called when the game starts or when spawned
void ABubble::BeginPlay()
{
	Super::BeginPlay();
	
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

	TArray<FVector3d> forces;
	forces.Init(FVector3d::Zero(), BubbleMesh->GetDynamicMesh()->GetMeshRef().MaxVertexID());
	BubbleMesh->GetDynamicMesh()->ProcessMesh(
		[&](const FDynamicMesh3& Mesh) {
			for (int32 i = 0; i < Mesh.MaxVertexID(); i++) {
				FVector3d force{ 0, 0, 0 };

				FVector3d pos = Mesh.GetVertex(i);
				FVector3d airPressureForce = (pos - CenterOfMass).GetSafeNormal();
				double comDistance = (pos - CenterOfMass).Size();
				airPressureForce *= 1 / (comDistance * comDistance) * AirPressureForce;
				force += airPressureForce;

				Mesh.EnumerateVertexEdges(i, [&](int32 edgeId) {
					auto edge = Mesh.GetEdge(edgeId);
					int32 neigh = edge.Vert.A == i ? edge.Vert.B : edge.Vert.A;
					FVector3d neighPos = Mesh.GetVertex(neigh);
					FVector3d springForce = (neighPos - pos).GetSafeNormal();
					double edgeLength = (neighPos - pos).Size();
					springForce *= (edgeLength - TargetEdgeLengths[edgeId]) * SpringCoefficient;
					force += springForce;
				});

				force += BubbleRandomStream.GetUnitVector() * ForceNoiseMagnitude;
				force += (2.0 * FMath::Abs(FVector3d::DotProduct(pos - CenterOfMass, BigNoiseVector)) - 1) * (CenterOfMass - pos).GetSafeNormal() * ForceBigNoiseMagnitude;

				forces[i] = force;
			}
		});

	double deltaTime = FMathf::Min(DeltaTime, 1 / 15.0);
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
				Mesh.SetVertex(i, Mesh.GetVertex(i) + vel * deltaTime);
			}
		}
	);

	UpdateNormals();
}

void ABubble::Generate() {
	if (IsValid(BubbleMesh)) {
		BubbleMesh->DestroyComponent();
	}

	auto mesh = MeshRepr::GetSphere(Radius, Subdivisions, true);

	BubbleMesh = NewObject<UDynamicMeshComponent>(this);
	BubbleMesh->RegisterComponent();
	BubbleMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	//BubbleMesh->SetMobility(EComponentMobility::Movable);
	//BubbleMesh->SetSimulatePhysics(true);
	BubbleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BubbleMesh->EnableComplexAsSimpleCollision();
	BubbleMesh->bEnableComplexCollision = true;
	BubbleMesh->SetOverrideRenderMaterial(BubbleMaterial);

	FDynamicMesh3 dynMesh{ true, false, false, false };
	for (const auto& pos : mesh.Positions) {
		dynMesh.AppendVertex(pos);
	}
	for (auto face : mesh.Faces) {
		dynMesh.AppendTriangle(face.Get<1>(), face.Get<0>(), face.Get<2>());
	}

	for (auto edge : dynMesh.GetEdgesBuffer()) {
		FVector3d v0 = dynMesh.GetVertex(edge.Vert.A);
		FVector3d v1 = dynMesh.GetVertex(edge.Vert.B);
		TargetEdgeLengths.Add((v1 - v0).Size());
	}

	VertexVelocities.Init(FVector3d::Zero(), dynMesh.MaxVertexID());

	UDynamicMesh* dynamicMesh = NewObject<UDynamicMesh>();
	dynamicMesh->SetMesh(MoveTemp(dynMesh));

	BubbleMesh->SetDynamicMesh(dynamicMesh);

	UpdateNormals();
}

void ABubble::UpdateNormals() {
	BubbleMesh->GetDynamicMesh()->EditMesh(
		[&](FDynamicMesh3& Mesh) {
			for (int32 i = 0; i < Mesh.MaxVertexID(); i++) {
				FVector3d faceNormalSum = FVector3d::Zero();
				int faceCount = 0;
				Mesh.EnumerateVertexTriangles(i, [&](int32 faceID) {
					FVector3d v0, v1, v2;
					Mesh.GetTriVertices(faceID, v0, v1, v2);
					FVector3d faceNormal = -FVector3d::CrossProduct(v1 - v0, v2 - v0).GetSafeNormal();
					faceNormalSum += faceNormal;
					faceCount++;
					});
				FVector3d normal = faceNormalSum / faceCount;
				Mesh.SetVertexNormal(i, FVector3f(normal));
			}
		}
	);
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