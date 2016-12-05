// © 2016 - 2017 Daniel Bortfeld

// NOTE: preprocessing is not supported in UHT (#if #ifndef ...)
// only CPP, !CPP, WITH_EDITOR and WITH_EDITORONLY_DATA are supported

#pragma once

class ABuilding;
class APath;
class AResourcePopup;
class UBoxCollider;
class AMarketBarrow;
#include "Etos/ObjectPool/ObjectPool.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

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
	Food,
	Coal,
	Iron,
	IronOre, 

	EResource_MAX
};

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

public:

	FResource(EResource Type = EResource::None, int32 Amount = 0, UTexture2D* Icon = nullptr)
	{
		if (Type != EResource::None)
		{
			this->Type = Type;
			this->Amount = Amount;
			this->Icon = Icon;
		}
	}

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

	// The name of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Name = FName(TEXT("New Building"));
	// The icon of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* BuildingIcon;

	// placement of the building

	// Whether this building is held by the mouse of the player
	// true means it's not bulit and it follows the mouse
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsHeld = false;
	// Whether something blocks the buildings position in buildmode (when it's held and not built)
	// true means you can't build here
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bPositionIsBlocked = false;
	// Whether this building was built
	// Its built when the resources are spend for building and when 
	// it was placed in the world
	// It calls its delay action when built
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsBuilt = false;
	// The resources required to build this building
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FResource> BuildCost;
	// The paths this building could be connected with when it was built right here
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<APath*> PossibleConnections;
	// The bulidings that could be in the radius of this building if it was built right here
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<ABuilding*> PossibleBuildingsInRadius;

	// producing resources

	// The Money this buildings needs per minute
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Upkeep;
	// The first resource needed for production
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FResource NeededResource1;
	// The second resource needed for production
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FResource NeededResource2;
	// The resource this building produces (or already has produced)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FResource ProducedResource;
	// The time it takes for a building to produce a resource 
	// (or call its respective delay action)
	// <= 0 means dont call a delay action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ProductionTime;
	// The maximum amount of each needed and produced resources this 
	// building can store
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxStoredResources;

	// getting resources to other buildings

	// The buildings inside the radius of this buildings
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<ABuilding*> BuildingsInRadius;
	// Whether there is already a market barrow on the way to this building
	// to get the resources
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bBarrowIsOnTheWay;
	// The Path/Road tiles connected to this buildling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<APath*> PathConnections;
	// The radius of the building in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
		UBoxCollider* OccupiedBuildSpace_Custom;

	UPROPERTY()
		TArray<USceneComponent*> TracePoints;

	UPROPERTY()
		USphereComponent* Radius;

public:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMarketBarrow> BP_MarketBarrow;

	UPROPERTY()
		TSubclassOf<AResourcePopup> ResourcePopup;

	UPROPERTY(EditAnywhere)
		FBuildingData Data;

	// Width in Tiles
	UPROPERTY(EditAnywhere)
		int32 Width;

	// Height in Tiles
	UPROPERTY(EditAnywhere)
		int32 Height;

protected:

	const bool bUseCustomBoxCollider = true;

	UPROPERTY()
		AEtosPlayerController* MyPlayerController;

	UPROPERTY()
		FDelayedActionDelegate Action;

	UPROPERTY()
		FBuildingDelegate BuildEvent;

	UPROPERTY(VisibleAnywhere)
		int32 BarrowsInUse = 0;

	UPROPERTY(EditAnywhere)
		int32 MaxBarrows = 1;

	UPROPERTY(VisibleAnywhere)
		FObjectPool MarketBarrowPool = FObjectPool();

private:

	UPROPERTY()
		float pastDelayTimerTime = 0;

	UPROPERTY()
		bool bMovedOnce = false;

	// amount of current collisions
	UPROPERTY()
		int32 collisionCount = 0;

	UPROPERTY()
		float currentUpkeepDebts = 0;

	UPROPERTY()
		bool bIsActive;

	UPROPERTY()
		TArray<ABuilding*> collisions;

public:
	// Sets default values for this actor's properties
	ABuilding();

	virtual void PostInitProperties() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	virtual void BeginDestroy() override;

	virtual void OnBuild();

	// @resource = EResource::None returns the produced resource
	virtual FResource HandOutResource(const EResource& resource = EResource::None);

	virtual void ReceiveResource(const FResource& resource);

	void ResetStoredResources();

	void SetFoundationSize(int32 width, int32 height);

	void DecreaseBarrowsInUse();

	// Whether the other building has the resources this building needs
	bool HasNeededResources(ABuilding* other);

	// Whether the other building offers the specified resource 
	bool HasResource(ABuilding* other, EResource resource);

	// Whether this building offers the specified resource 
	virtual	bool HasResource(EResource resource);

	bool IsActive();

	void SetActive(bool isActive);

	bool TryReturningToPool(AMarketBarrow* barrow);

	void GetOverlappingBulidings(TArray<ABuilding*>& OverlappingBuildings);

	bool operator<(const ABuilding& B) const;

protected:

	virtual void CreateTracePoints();

	virtual void RelocateTracePoints();

	virtual void GetSurroundingBuildings();

	virtual void BindDelayAction();

	virtual void SpendUpkeep(float DeltaTime);

	virtual void SendMarketBarrow_Internal(ABuilding* targetBuilding, const EResource& orderedResource, const FVector& spawnLocation, const FVector& targetLocation);

	void InitOccupiedBuildSpace();

	void InitOccupiedBuildSpace_Custom();

	void InitOccupiedBuildSpace_Internal(UBoxComponent* collider);

	TArray<ABuilding*> GetBuildingsInRange();

	void CallDelayAction(float pastTime, float delayDuration = 1);

	void SpawnResourcePopup(FVector offset = FVector(0, 0, 200));

	AEtosPlayerController * GetMyPlayerController();

	void RefreshBuildingsInRadius();

private:

	UFUNCTION()
		void BuildSpace_OnBeginOverlap_Custom(UBoxCollider* other);

	UFUNCTION()
		void BuildSpace_OnEndOverlap_Custom(UBoxCollider* other);

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

	void GetNeededResources();

	// Whether this building has the resources it needs
	bool HasNeededResources();

	void RemoveANeededResource();

	void MoveToMouseLocation();

	void DetermineOrderedResource(EResource& OrderedResource, ABuilding* TargetBuilding);
};
