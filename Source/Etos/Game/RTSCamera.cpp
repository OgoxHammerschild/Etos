// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "RTSCamera.h"
#include "Kismet/KismetMathLibrary.h"
#include"Etos/Game/EtosPlayerController.h"

// Sets default values
ARTSCamera::ARTSCamera()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;

	Origin = CreateDefaultSubobject<UArrowComponent>("Origin");
	Origin->SetupAttachment(RootComponent);
	Origin->SetRelativeRotation(FRotator(90, 0, 0));
	Origin->ArrowColor = FColor(255, 0, 0, 255);
	Origin->ArrowSize = 0.2f;
	Origin->bRenderInMainPass = false;
	Origin->bReceivesDecals = false;
	Origin->bHiddenInGame = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(256, 0, 256));
}

void ARTSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPanCamera_IE_Repeat)
		Pan(FKey());
}

// Called to bind functionality to input
void ARTSCamera::SetupPlayerInputComponent(UInputComponent* inputComponent)
{
	Super::SetupPlayerInputComponent(inputComponent);

	inputComponent->BindAction("ZoomIn", IE_Pressed, this, &ARTSCamera::ZoomIn);
	inputComponent->BindAction("ZoomOut", IE_Pressed, this, &ARTSCamera::ZoomOut);
	inputComponent->BindAction("RotateCamera", IE_Pressed, this, &ARTSCamera::Rotate);
	inputComponent->BindAction("RotateCamera", IE_Released, this, &ARTSCamera::StopRotation);
	inputComponent->BindAxis("Turn", this, &ARTSCamera::Turn);
	//inputComponent->BindAxis("LookUp", this, &ARTSCamera::LookUp);
	inputComponent->BindAxis("MoveForward", this, &ARTSCamera::MoveForward);
	inputComponent->BindAxis("MoveRight", this, &ARTSCamera::MoveRight);
	inputComponent->BindAction("PanCamera", IE_Pressed, this, &ARTSCamera::StartPanning);
	//inputComponent->BindAction("PanCamera", IE_Repeat, this, &ARTSCamera::Pan); // not working for mouse button
	inputComponent->BindAction("PanCamera", IE_Released, this, &ARTSCamera::StopPanning);
}

void ARTSCamera::OnConstruction(const FTransform & Transform)
{
	UpdateCameraLocationAndRotation();
}

void ARTSCamera::ZoomIn(FKey key)
{
	cameraRadius = UKismetMathLibrary::Clamp(cameraRadius - zoomSpeed, radiusMin, radiusMax);
	UpdateCameraLocationAndRotation();
}

void ARTSCamera::ZoomOut(FKey key)
{
	cameraRadius = UKismetMathLibrary::Clamp(cameraRadius + zoomSpeed, radiusMin, radiusMax);
	UpdateCameraLocationAndRotation();
}

void ARTSCamera::Rotate(FKey key)
{
	bRotateCamera = true;
}

void ARTSCamera::StopRotation(FKey key)
{
	bRotateCamera = false;
}

void ARTSCamera::StartPanning(FKey key)
{
	bPanCamera_IE_Repeat = true;

	FHitResult hit;
	if (Util::TraceSingleAtMousePosition(this, hit))
	{
		myPlayerController = Util::GetFirstEtosPlayerController(this);
		if (myPlayerController)
		{
			myPlayerController->GetMousePosition(panningStartPosition.X, panningStartPosition.Y);
		}
	}
}

void ARTSCamera::Pan(FKey key)
{
	if (myPlayerController)
	{
		FVector2D currentPosition;
		myPlayerController->GetMousePosition(currentPosition.X, currentPosition.Y);

		if (!bPanCamera)
		{
			if (FVector2D::Distance(currentPosition, panningStartPosition) > panningThreshold)
			{
				bPanCamera = true;
			}
		}
		else
		{
			FVector2D panDirection = currentPosition - panningStartPosition;
			panDirection.Normalize();

			MoveForward(-panDirection.Y * panningSpeed);
			MoveRight(panDirection.X * panningSpeed);
		}
	}
}

void ARTSCamera::StopPanning(FKey key)
{
	bPanCamera = false;
	bPanCamera_IE_Repeat = false;
}

void ARTSCamera::Turn(float axisValue)
{
	if (bRotateCamera)
	{
		cameraZAngle += (axisValue * rotationSpeed * UGameplayStatics::GetWorldDeltaSeconds(this));
		UpdateCameraLocationAndRotation();
	}
}

void ARTSCamera::LookUp(float axisValue)
{
	if (bRotateCamera)
	{
		cameraHeightAngle = UKismetMathLibrary::Clamp(cameraHeightAngle + (axisValue * rotationSpeed * UGameplayStatics::GetWorldDeltaSeconds(this)), cameraHeightAngleMin, cameraHeightAngleMax);
		UpdateCameraLocationAndRotation();
	}
}

void ARTSCamera::MoveForward(float axisValue)
{
	float distance = (axisValue + edgeForwardAxis) * movementSpeed * UGameplayStatics::GetWorldDeltaSeconds(this);

	FRotator viewRotation = FRotator(0, Camera->GetComponentRotation().Yaw, 0);
	FVector forwardVector = UKismetMathLibrary::GetForwardVector(viewRotation);

	FVector newLocation = GetActorLocation() + (forwardVector * distance);

	SetActorLocation(newLocation);
}

void ARTSCamera::MoveRight(float axisValue)
{
	float distance = (axisValue + edgeRightAxis) * movementSpeed * UGameplayStatics::GetWorldDeltaSeconds(this);

	FRotator viewRotation = FRotator(0, Camera->GetComponentRotation().Yaw, 0);
	FVector rightVector = UKismetMathLibrary::GetRightVector(viewRotation);

	FVector newLocation = GetActorLocation() + (rightVector * distance);

	SetActorLocation(newLocation);
}

FORCEINLINE void ARTSCamera::UpdateCameraLocationAndRotation()
{
	FVector newLocation = GetPointOnASphere(cameraRadius, cameraZAngle, cameraHeightAngle);
	Camera->SetRelativeLocation(newLocation);

	FVector start = Camera->GetRelativeTransform().GetLocation();
	FVector target = FVector::ZeroVector;
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(start, target);
	Camera->SetRelativeRotation(newRotation);
}

FORCEINLINE FVector ARTSCamera::GetPointOnASphere(float radius, float angleAroundZ, float heightAngle)
{
	float sinZ = UKismetMathLibrary::DegSin(angleAroundZ);
	float cosZ = UKismetMathLibrary::DegCos(angleAroundZ);
	float sinHeight = UKismetMathLibrary::DegSin(heightAngle);
	float cosHeight = UKismetMathLibrary::DegCos(heightAngle);

	float x = radius * cosZ * sinHeight;
	float y = radius * sinZ * sinHeight;
	float z = radius * cosHeight;

	return FVector(x, y, z);
}
