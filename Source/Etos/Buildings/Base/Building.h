// © 2016 - 2017 Daniel Bortfeld

// NOTE: preprocessing is not supported in UHT (#if #ifndef ...)
// only CPP, !CPP, WITH_EDITOR and WITH_EDITORONLY_DATA are supported

#pragma once

class ABuilding;
class APath;
class AResourcePopup;
class UBoxCollider;
class AMarketBarrow;
#include "Etos/FunctionLibraries/StructLibrary.h"
#include "Etos/ObjectPool/ObjectPool.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

DECLARE_DYNAMIC_DELEGATE(FDelayedActionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBuildingDelegate, ABuilding*, sender);

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

	//UPROPERTY(VisibleAnywhere)
	//	UObjectPool* MarketBarrowPool;

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
