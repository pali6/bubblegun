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

	static MeshRepr GetSphere(float radius, int32 nSubdivisions) {
		auto mesh = GetOctahedron();
		for (int32 i = 0; i < nSubdivisions; i++) {
			mesh.Subdivide();
		}
		mesh.RescaleToSphere(radius);
		return mesh;
	}
};



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

}

void ABubble::Generate() {
	if (IsValid(BubbleMesh)) {
		BubbleMesh->DestroyComponent();
	}

	auto mesh = MeshRepr::GetSphere(Radius, Subdivisions);

	BubbleMesh = NewObject<UDynamicMeshComponent>(this);
	BubbleMesh->RegisterComponent();
	BubbleMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	BubbleMesh->SetMobility(EComponentMobility::Movable);
	BubbleMesh->SetSimulatePhysics(true);
	BubbleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BubbleMesh->SetOverrideRenderMaterial(BubbleMaterial);

	FDynamicMesh3 dynMesh{ true, false, false, false };
	for (const auto& pos : mesh.Positions) {
		dynMesh.AppendVertex(pos);
	}
	for (auto face : mesh.Faces) {
		dynMesh.AppendTriangle(face.Get<1>(), face.Get<0>(), face.Get<2>());
	}

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