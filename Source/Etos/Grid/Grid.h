// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Grid.generated.h"

UCLASS()
class ETOS_API AGrid : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
		UInstancedStaticMeshComponent* GridMesh;

public:

	UPROPERTY(EditAnywhere)
		bool bConstruct;

	UPROPERTY(EditAnywhere)
		float TileSize;

	UPROPERTY(EditAnywhere)
		int32 Width;

	UPROPERTY(EditAnywhere)
		int32 Height;

public:
	// Sets default values for this actor's properties
	AGrid();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	//virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
};
