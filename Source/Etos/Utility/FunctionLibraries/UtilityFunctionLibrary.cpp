// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "UtilityFunctionLibrary.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/Game/EtosHUD.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

#define DEFINE_ENUM_ISVALID(EnumType) bool UUtilityFunctionLibrary::IsValid(EnumType value) { return Enum::IsValid(value); }

TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::BuildingObjectType;// = InitBuildingObjectType();
TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::FloorObjectType;// = InitFloorObjectType();

TMap <EResource, FIcon> UUtilityFunctionLibrary::ResourceIcons;// = TMap<EResource, FIcon>();

//DEFINE_ENUM_ISVALID(EResource);
//DEFINE_ENUM_ISVALID(EResidentLevel);
//DEFINE_ENUM_ISVALID(EResidentNeed);

UUtilityFunctionLibrary::UUtilityFunctionLibrary()
{
	BuildingObjectType = InitBuildingObjectType();
	FloorObjectType = InitFloorObjectType();
	ResourceIcons = TMap<EResource, FIcon>();
}

FORCEINLINE AEtosGameMode* UUtilityFunctionLibrary::GetEtosGameMode(UObject* WorldContextObject)
{
	if (UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		AEtosGameMode* gm = dynamic_cast<AEtosGameMode*, AGameMode>(World->GetAuthGameMode());
		return gm;
	}
	return nullptr;
}

FORCEINLINE AEtosHUD * UUtilityFunctionLibrary::GetEtosHUD(UObject* WorldContextObject, int32 PlayerIndex)
{
	AEtosHUD* hud = dynamic_cast<AEtosHUD*, AHUD>(GetEtosPlayerController(WorldContextObject, PlayerIndex)->GetHUD());
	return hud;
}

FORCEINLINE AEtosPlayerController * UUtilityFunctionLibrary::GetFirstEtosPlayerController(UObject* WorldContextObject)
{
	if (UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		AEtosPlayerController* pc = dynamic_cast<AEtosPlayerController*, APlayerController>(World->GetFirstPlayerController());
		return pc;
	}
	return nullptr;
}

FORCEINLINE AEtosPlayerController * UUtilityFunctionLibrary::GetEtosPlayerController(UObject* WorldContextObject, int32 PlayerIndex)
{
	if (UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		uint32 Index = 0;
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = *Iterator;
			if (Index == PlayerIndex)
			{
				return dynamic_cast<AEtosPlayerController*, APlayerController>(PlayerController);
			}
			Index++;
		}
	}
	return nullptr;
}

FORCEINLINE ASimpleCollisionManager * UUtilityFunctionLibrary::GetEtosCollisionManager(UObject * WorldContextObject)
{
	if (AEtosGameMode* const gm = GetEtosGameMode(WorldContextObject))
	{
		return gm->CollisionManager;
	}
	return nullptr;
}

UTexture2D * UUtilityFunctionLibrary::GetDefaultTexture()
{
	return AEtosGameMode::GetDefaultTexture();
}

UTexture2D * UUtilityFunctionLibrary::EnsureTexture(UTexture2D * Texture)
{
	if (Texture->IsValidLowLevel())
	{
		return Texture;
	}

	return GetDefaultTexture();
}

UTexture2D * UUtilityFunctionLibrary::GetIcon(EResource Resource, EIconSize Size)
{
	if (!Enum::IsValid(Resource))
	{
		return GetDefaultTexture();
	}

	if (ResourceIcons.Num() > 0)
	{
		switch (Size)
		{
		case EIconSize::Smol:
			return EnsureTexture(ResourceIcons[Resource].Icon_Smol);
		case EIconSize::Big:
			return EnsureTexture(ResourceIcons[Resource].Icon_Big);
		default:
			return GetDefaultTexture();
		}
	}
	return GetDefaultTexture();
}

bool UUtilityFunctionLibrary::TraceSingleForBuildings(UObject* WorldContextObject, const FVector & Start, const FVector & End, FHitResult & HitResult)
{
	return UKismetSystemLibrary::LineTraceSingleForObjects(WorldContextObject, Start, End, BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true, FLinearColor(98, 147, 238));
}

bool UUtilityFunctionLibrary::TraceMultiForBuildings(UObject* WorldContextObject, const FVector & Start, const FVector & End, TArray<FHitResult>& HitResults)
{
	return UKismetSystemLibrary::LineTraceMultiForObjects(WorldContextObject, Start, End, BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true, FLinearColor::Blue);
}

bool UUtilityFunctionLibrary::TraceSingleForFloor(UObject* WorldContextObject, const FVector & Start, const FVector & End, FHitResult & Hit)
{
	return UKismetSystemLibrary::LineTraceSingleForObjects(WorldContextObject, Start, End, FloorObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
}

bool UUtilityFunctionLibrary::BP_TraceSingleAtMousePosition(UObject * WorldContextObject, FHitResult & Hit, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes, float Range)
{
	return TraceSingleAtMousePosition(WorldContextObject, Hit, Range, ObjectTypes);
}

inline bool UUtilityFunctionLibrary::TraceSingleAtMousePosition(UObject * WorldContextObject, FHitResult & Hit, float Range, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes)
{
	if (UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		if (APlayerController* const PlayerController = World->GetFirstPlayerController())
		{
			FVector MouseLocation;
			FVector MouseDirection;
			PlayerController->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
			MouseDirection *= Range;

			if (ObjectTypes.Num() > 0)
			{
				return UKismetSystemLibrary::LineTraceSingleForObjects(WorldContextObject, MouseLocation, MouseLocation + MouseDirection, ObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
			}
			else
			{
				return UKismetSystemLibrary::LineTraceSingleForObjects(WorldContextObject, MouseLocation, MouseLocation + MouseDirection, FloorObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
			}
		}
	}
	return false;
}

FString UUtilityFunctionLibrary::ConvertBoolToString(const bool & b)
{
	return b ? FString(TEXT("true")) : FString(TEXT("false"));
}

template<typename TEnum>
FString UUtilityFunctionLibrary::ConvertEnumValueToString(const FString& EnumName, TEnum Value)
{
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	if (!enumPtr)
	{
		return FString("Invalid Name");
	}

	return enumPtr->GetEnumName((int32)Value);
}

bool UUtilityFunctionLibrary::IsValidR(EResource value)
{
	return Enum::IsValid(value);
}

bool UUtilityFunctionLibrary::IsValidL(EResidentLevel value)
{
	return Enum::IsValid(value);
}

bool UUtilityFunctionLibrary::IsValidN(EResidentNeed value)
{
	return Enum::IsValid(value);
}

TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::InitBuildingObjectType()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> buildingObjectType;
	buildingObjectType.Init(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1/*Building*/), 1);

	return buildingObjectType;
}

TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::InitFloorObjectType()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> floorObjectType;
	floorObjectType.Init(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2/*Floor*/), 1);

	return floorObjectType;
}
