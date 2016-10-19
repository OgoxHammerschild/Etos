// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "RTSCamera.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ARTSCamera::ARTSCamera()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;

	Origin = CreateDefaultSubobject<UArrowComponent>("Origin");
	Origin->SetupAttachment(RootComponent);
	Origin->SetRelativeRotation(FRotator(90, 0, 0));
	Origin->ArrowColor = FColor(255, 0, 0, 255);
	Origin->ArrowSize = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(256, 0, 256));
}

// Called when the game starts or when spawned
void ARTSCamera::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARTSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARTSCamera::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ARTSCamera::ZoomIn);
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ARTSCamera::ZoomOut);
	InputComponent->BindAction("RotateCamera", IE_Pressed, this, &ARTSCamera::Rotate);
	InputComponent->BindAction("RotateCamera", IE_Released, this, &ARTSCamera::StopRotation);
	InputComponent->BindAxis("Turn", this, &ARTSCamera::Turn);
	InputComponent->BindAxis("LookUp", this, &ARTSCamera::LookUp);
	InputComponent->BindAxis("MoveForward", this, &ARTSCamera::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ARTSCamera::MoveRight);
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

void ARTSCamera::Turn(float axisValue)
{
	if (bRotateCamera)
	{
		cameraZAngle += (axisValue*rotationSpeed);
		UpdateCameraLocationAndRotation();
	}
}

void ARTSCamera::LookUp(float axisValue)
{
	if (bRotateCamera)
	{
		cameraHeightAngle = UKismetMathLibrary::Clamp(cameraHeightAngle + (axisValue*rotationSpeed), cameraHeightAngleMin, cameraHeightAngleMax);
		UpdateCameraLocationAndRotation();
	}
}

void ARTSCamera::MoveForward(float axisValue)
{
	float distance = (axisValue + edgeForwardAxis)*movementSpeed;

	FRotator viewRotation = FRotator(0, Camera->GetComponentRotation().Yaw, 0);
	FVector forwardVector = UKismetMathLibrary::GetForwardVector(viewRotation);

	FVector newLocation = GetActorLocation() + (forwardVector * distance);

	SetActorLocation(newLocation);
}

void ARTSCamera::MoveRight(float axisValue)
{
	float distance = (axisValue + edgeRightAxis)*movementSpeed;

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
