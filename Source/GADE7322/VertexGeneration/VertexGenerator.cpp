// Fill out your copyright notice in the Description page of Project Settings.


#include "VertexGeneration/VertexGenerator.h"

// Sets default values
AVertexGenerator::AVertexGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	RootComponent = ProcMesh;
	ProcMesh->bUseAsyncCooking = true;
}

void AVertexGenerator::UpdateTerrain()
{
    TArray<float> Heightmap = GenerateHeightmap();
    ApplyHeightmap(Heightmap);
}

// Called when the game starts or when spawned
void AVertexGenerator::BeginPlay()
{
	Super::BeginPlay();
	BuildGrid();
	UpdateTerrain();
}

// Called every frame
void AVertexGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVertexGenerator::BuildGrid()
{
    int32 N = GridSize;
    BaseVertices.SetNum(N * N);
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // create flat grid of vertices
    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = 0; j < N; ++j)
        {
            int32 Idx = i * N + j;
            float X = j * CellSize;
            float Y = i * CellSize;
            BaseVertices[Idx] = FVector(X, Y, 0.f);
            UVs.Add(FVector2D((float)j / (N - 1), (float)i / (N - 1)));
            Normals.Add(FVector::UpVector);
        }
    }

    // create triangles from vertices
    for (int32 i = 0; i < N - 1; ++i)
    {
        for (int32 j = 0; j < N - 1; ++j)
        {
            int32 Idx = i * N + j;
            Triangles.Append({ Idx, Idx + N, Idx + N + 1, Idx, Idx + N + 1, Idx + 1 });
        }
    }

    // create mesh section
    ProcMesh->CreateMeshSection(0, BaseVertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}


TArray<float> AVertexGenerator::GenerateHeightmap()
{
    int32 N = GridSize;
    TArray<float> Heightmap;
    Heightmap.Init(0.f, N * N);

    float TotalWeight = 0.f;
    for (auto& L : VertexLayers) if (L.bEnabled) TotalWeight += L.Weight;
    if (TotalWeight <= 0.f) return Heightmap;

    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = 0; j < N; ++j)
        {
            float Sum = 0.f;
            for (const auto& L : VertexLayers)
            {
                if (!L.bEnabled) continue;
                float lx = i / (L.Scale * GlobalZoom);
                float ly = j / (L.Scale * GlobalZoom);
                Sum += L.Weight * FractalNoise(lx + L.Seed, ly + L.Seed, L);
            }
            Heightmap[i * N + j] = Sum / TotalWeight;
        }
    }
    return Heightmap;
}

float AVertexGenerator::FractalNoise(float X, float Y, const FVertexLayer& Layer)
{
    float Value = 0.f;
    float Amplitude = 1000.f;
    float Frequency = 1.f;
    float MaxVal = 0.f;

    for (int32 o = 0; o < Layer.Octaves; ++o)
    {
        float N = FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency));
        Value += Amplitude * N;
        MaxVal += Amplitude/100;
        Amplitude *= Layer.Persistence;
        Frequency *= Layer.Lacunarity;
    }
    return (MaxVal > 0.f) ? (Value / MaxVal) : 0.f;
}

void AVertexGenerator::ApplyHeightmap(const TArray<float>& Heightmap)
{
    int32 N = GridSize;
    TArray<FVector> Vertices = BaseVertices;

    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = 0; j < N; ++j)
        {
            int32 Idx = i * N + j;
            FVector V = Vertices[Idx];
            V.Z = Heightmap[Idx] * CellSize;  // scale height
            Vertices[Idx] = V;
        }
    }

    // update mesh
    ProcMesh->UpdateMeshSection(0, Vertices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>());
}

