// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VertexLayer.h"
#include "VertexGenerator.generated.h"

UCLASS()
class GADE7322_API AVertexGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVertexGenerator();

	//parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 GridSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float GlobalZoom = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	TArray<FVertexLayer> VertexLayers;


	//procedural mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMeshComponent* ProcMesh;

	// regenerate (call from UI)
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void UpdateTerrain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	void BuildGrid();
	TArray<float> GenerateHeightmap();
	float FractalNoise(float X, float Y, const FVertexLayer& Layer);
	void ApplyHeightmap(const TArray<float>& Heightmap);

	// cache base vertices
	TArray<FVector> BaseVertices;

};
