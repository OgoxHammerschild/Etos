// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/Actor.h"
#include "Building.generated.h"
class ABuilding;
class APath;

DECLARE_DYNAMIC_DELEGATE(FDelayedActionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBuildingDelegate, ABuilding*, sender);

UENUM(BlueprintType)
enum class EResource : uint8
{
	None,
	Money,
	Wood,
	Tool,
	Stone,

	EResource_MAX
};

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resource)
		EResource Type = EResource::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resource)
		int32 Amount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resource)
		UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FBuildingData
{
	GENERATED_BODY()

public:

	// building info

	UPROPERTY(EditAnywhere)
		FName Name = FName(TEXT("New Building"));
	UPROPERTY(EditAnywhere)
		UTexture2D* BuildingIcon;


	// placement of the building

	UPROPERTY(VisibleAnywhere)
		bool bIsHeld = false;
	UPROPERTY(VisibleAnywhere)
		bool bPositionIsBlocked = false;
	UPROPERTY(VisibleAnywhere)
		bool bIsBuilt = false;
	UPROPERTY(VisibleAnywhere)
		TArray<FResource> BuildCost;
	UPROPERTY(VisibleAnywhere)
		TArray<APath*> PossibleConnections;
	UPROPERTY(VisibleAnywhere)
		TArray<ABuilding*> PossibleBuildingsInRadius;


	// producing resources

	UPROPERTY(EditAnywhere)
		FResource NeededResource1;
	UPROPERTY(EditAnywhere)
		FResource NeededResource2;
	UPROPERTY(EditAnywhere)
		FResource ProducedResource;
	UPROPERTY(EditAnywhere)
		float ProductionTime;
	UPROPERTY(EditAnywhere)
		int32 MaxStoredResources;


	// getting resources to other buildings

	UPROPERTY(VisibleAnywhere)
		TArray<ABuilding*> BuildingsInRadius;
	UPROPERTY(VisibleAnywhere)
		bool bBarrowIsOnTheWay;
	UPROPERTY(VisibleAnywhere)
		TArray<APath*> PathConnections;
	UPROPERTY(EditAnywhere)
		float Radius;
};

UCLASS()
class ETOS_API ABuilding : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* BuildingMesh;

	UPROPERTY(EditDefaultsOnly)
		UStaticMeshComponent* FoundationMesh;

	UPROPERTY()
		UBoxComponent* OccupiedBuildSpace;

	UPROPERTY()
		TArray<USceneComponent*> TracePoints;

	UPROPERTY()
		USphereComponent* Radius;

public:

	UPROPERTY(EditAnywhere)
		FBuildingData Data;

	// Width in Tiles
	UPROPERTY(EditAnywhere)
		int32 Width;

	// Height in Tiles
	UPROPERTY(EditAnywhere)
		int32 Height;

protected:

	UPROPERTY()
		FDelayedActionDelegate Action;

	UPROPERTY()
		FBuildingDelegate BuildEvent;

private:

	UPROPERTY()
		float pastDelayTimerTime = 0;

	UPROPERTY()
		bool bMovedOnce = false;

	UPROPERTY()
		TArray<AActor*> collisions = TArray<AActor*>();

public:
	// Sets default values for this actor's properties
	ABuilding();

	virtual void PostInitProperties() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	//virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	virtual void BeginDestroy() override;

	virtual void OnBuild();

	void ResetStoredResources();

	void SetFoundationSize(int32 width, int32 height);

protected:

	virtual void CreateTracePoints();

	virtual void RelocateTracePoints();

	virtual void GetSurroundingBuildings();

	virtual void BindDelayAction();

	void InitOccupiedBuildSpace();

	TArray<ABuilding*> GetBuildingsInRange();

	bool TraceSingleForBuildings(FVector Start, FVector End, FHitResult& HitResult);

	bool TraceMultiForBuildings(FVector Start, FVector End, TArray<FHitResult>& HitResults);

	bool TraceSingleForFloor(FVector Start, FVector End, FHitResult& Hit);

	void CallDelayAction(float pastTime, float delayDuration = 1);

private:

	UFUNCTION()
		void BuildSpace_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
		void BuildSpace_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void BuildingEnteredRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
		void BuildingLeftRadius(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void AddResource();

	UFUNCTION()
		void AddNewBuildingInRange(ABuilding* buildingInRange);

	void MoveToMouseLocation();
};
