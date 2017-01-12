// © 2016 - 2017 Daniel Bortfeld

// NOTE: preprocessing is not supported in UHT (#if #ifndef ...)
// only CPP, !CPP, WITH_EDITOR and WITH_EDITORONLY_DATA are supported

#pragma once

class ABuilding;
class APath;
class AResourcePopup;
class UBoxCollider;
class AMarketBarrow;

#include "Etos/Utility/InOut.h"
#include "Etos/Utility/Structs/BuildingData.h"
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

	UPROPERTY()
		TSubclassOf<AResourcePopup> ResourcePopupList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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
		bool bIsActive = true;

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

	// Destroy this Building.
	virtual void Demolish();

	virtual void BeginDestroy() override;

	virtual void OnBuild();

	// @resource = EResource::None returns the produced resource
	virtual FResource HandOutResource( EResource in resource = EResource::None);

	virtual void ReceiveResource( FResource in resource);

	void ResetStoredResources();

	void SetFoundationSize(int32 in width, int32 in height);

	int32 GetBarrowsInUse();

	// for loading savegame only
	void SetBarrowsInUse(int32 in amount);

	void DecreaseBarrowsInUse();

	// Whether the other building has the resources this building needs
	bool HasNeededResources(ABuilding* other);

	// Whether the other building offers the specified resource 
	bool HasResource(ABuilding* other, EResource in resource);

	// Whether this building offers the specified resource 
	virtual	bool HasResource(EResource in resource);

	bool IsActive();

	void SetActive(bool in isActive);

	bool TryReturningToPool(AMarketBarrow* barrow);

	void GetOverlappingBulidings(TArray<ABuilding*> out OverlappingBuildings);

	// Spawns a popup at the building position
	void SpawnResourcePopup(FVector in Offset, UTexture2D* ResourceIcon, FText in Text, FLinearColor in TextColor);

	AResourcePopup* SpawnResourcePopupList(FVector in Offset);

	bool operator<(ABuilding in B) const;

protected:

	virtual void CreateTracePoints();

	virtual void RelocateTracePoints();

	virtual void GetSurroundingBuildings();

	virtual void BindDelayAction();

	virtual void SpendUpkeep(float in DeltaTime);

	virtual void SendMarketBarrow_Internal(ABuilding* targetBuilding,  EResource in orderedResource,  FVector in spawnLocation,  FVector in targetLocation);

	void InitOccupiedBuildSpace();

	void InitOccupiedBuildSpace_Custom();

	void InitOccupiedBuildSpace_Internal(UBoxComponent* collider);

	TArray<ABuilding*> GetBuildingsInRange();

	void CallDelayAction(float in pastTime, float in delayDuration = 1);

	// Spawns the +1 popup with the produced resource icon 
	void SpawnResourcePopup(FVector in Offset = FVector(0, 0, 200));

	AEtosPlayerController * GetMyPlayerController();

	void RefreshBuildingsInRadius();

	UFUNCTION()
		void OnBuildingDestroyed(AActor* DestroyedActor);

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

	void DetermineOrderedResource(EResource out OrderedResource, ABuilding* TargetBuilding);

	AResourcePopup* SpawnResourcePopup_Internal(FVector in Offset, TSubclassOf<AResourcePopup> in AlternativeBlueprint = nullptr);
};
