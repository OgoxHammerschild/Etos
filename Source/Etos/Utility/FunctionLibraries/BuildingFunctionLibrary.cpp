// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Etos/Buildings/Base/Building.h"
#include "Etos/Buildings/Path.h"
#include "BuildingFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

inline void UBuildingFunctionLibrary::CalcVectors(UPARAM(DisplayName = "X-Extend") float XExtend, UPARAM(DisplayName = "Y-Extend") float YExtend, UPARAM(DisplayName = "Z-Extend") float ZExtend, bool negateX1, bool negateY1, bool negateX2, bool negateY2, EOffsetDirections StartOffset, EOffsetDirections GoalOffset, FVector& OutStart, FVector& OutGoal, float ZHeight)
{
	FVector2D TopLeftOffset = FVector2D(-50, -50);
	FVector2D TopRightOffset = FVector2D(50, -50);
	FVector2D BotRightOffset = FVector2D(50, 50);
	FVector2D BotLeftOffset = FVector2D(-50, 50);

	float X1 = XExtend;
	float Y1 = YExtend;
	float X2 = XExtend;
	float Y2 = YExtend;

	if (negateX1)
	{
		X1 *= -1;
	}
	if (negateY1)
	{
		Y1 *= -1;
	}
	if (negateX2)
	{
		X2 *= -1;
	}
	if (negateY2)
	{
		Y2 *= -1;
	}

	switch (StartOffset)
	{
	case EOffsetDirections::TopLeft:
		OutStart = MakeVector(X1, Y1, TopLeftOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::TopRight:
		OutStart = MakeVector(X1, Y1, TopRightOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::BotLeft:
		OutStart = MakeVector(X1, Y1, BotLeftOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::BotRight:
		OutStart = MakeVector(X1, Y1, BotRightOffset, ZHeight - ZExtend);
		break;
	default:
		break;
	}

	switch (GoalOffset)
	{
	case EOffsetDirections::TopLeft:
		OutGoal = MakeVector(X2, Y2, TopLeftOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::TopRight:
		OutGoal = MakeVector(X2, Y2, TopRightOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::BotLeft:
		OutGoal = MakeVector(X2, Y2, BotLeftOffset, ZHeight - ZExtend);
		break;
	case EOffsetDirections::BotRight:
		OutGoal = MakeVector(X2, Y2, BotRightOffset, ZHeight - ZExtend);
		break;
	default:
		break;
	}
}

inline bool UBuildingFunctionLibrary::FindPath(const ABuilding* Source, const ABuilding* Target)
{
	if (Target && Source)
	{
		if (Target == Source)
		{
			return true;
		}

		TArray<ABuilding*> openList = TArray<ABuilding*>();
		openList.Empty(Source->Data.PathConnections.Num());

		openList.Append(Source->Data.PathConnections);

		for (int32 i = 0; i < openList.Num(); ++i)
		{
			if (openList[i] == Target)
			{
				return true;
			}

			if (APath* current = dynamic_cast<APath*, ABuilding>(openList[i]))
			{
				for (ABuilding* building : current->Connections)
				{
					openList.AddUnique(building);
				}
			}
		}
	}
	return false;
}

bool UBuildingFunctionLibrary::FindPath(const ABuilding * Source, const ABuilding * Target, APath *out StartPath, APath *out GoalPath)
{
	static bool bFindStart = false;

	if (Target && Source)
	{
		if (Target == Source)
		{
			return true;
		}

		TArray<ABuilding*> openList = TArray<ABuilding*>();
		openList.Empty(Source->Data.PathConnections.Num());

		openList.Append(Source->Data.PathConnections);

		if (openList.Num() > 0 &&
			openList[0] == Target)
		{
			if (bFindStart)
			{
				bFindStart = false;
				return true;
			}
			bFindStart = true;

			return FindPath(Target, Source, GoalPath, StartPath);
		}

		for (int32 i = 0; i < openList.Num(); ++i)
		{
			if (APath* const current = dynamic_cast<APath*, ABuilding>(openList[i]))
			{
				for (ABuilding* const building : current->Connections)
				{
					if (building == Target)
					{
						GoalPath = current;
						if (bFindStart)
						{
							bFindStart = false;
							return true;
						}
						bFindStart = true;

						return FindPath(Target, Source, GoalPath, StartPath);
					}
					openList.AddUnique(building);
				}
			}
		}
	}
	StartPath = nullptr;
	GoalPath = nullptr;
	return false;
}

FORCEINLINE FVector UBuildingFunctionLibrary::GetNextGridLocation(const FVector & location, const FVector2Di & size, const float & heightOffset)
{
	float X = UKismetMathLibrary::Round(location.X / 100) * 100;
	float Y = UKismetMathLibrary::Round(location.Y / 100) * 100;
	float Z = location.Z + heightOffset;

	if (size.X % 2 == 0)
	{
		X += 50;
	}

	if (size.Y % 2 == 0)
	{
		Y += 50;
	}

	return FVector(X, Y, Z);
}

FORCEINLINE FVector UBuildingFunctionLibrary::MakeVector(float X, float Y, FVector2D offset, float Z)
{
	return FVector(X + offset.X, Y + offset.Y, Z);
}
