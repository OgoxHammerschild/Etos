// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "RTSCamera.generated.h"

UCLASS()
class ETOS_API ARTSCamera : public APawn
{
	GENERATED_BODY()

public:

	UPROPERTY()
		USceneComponent* Root;

	UPROPERTY(EditAnywhere)
		UArrowComponent* Origin;

	UPROPERTY()
		UCameraComponent* Camera;

private:

	UPROPERTY()
		bool bRotateCamera = false;

	UPROPERTY()
		float cameraZAngle = 0.f;

	UPROPERTY()
		float cameraHeightAngle = 45.f;

	UPROPERTY()
		float cameraHeightAngleMin = 10;

	UPROPERTY()
		float cameraHeightAngleMax = 90;

	UPROPERTY()
		float rotationSpeed = 4.0f;

	UPROPERTY()
		float cameraRadius = 2048.f;

	UPROPERTY()
		float zoomSpeed = 256.f;

	UPROPERTY()
		float radiusMin = 256.f;

	UPROPERTY()
		float radiusMax = 2048.f;

	UPROPERTY()
		float movementSpeed = 10.f;

	UPROPERTY()
		float edgeForwardAxis = 0.f;

	UPROPERTY()
		float edgeRightAxis = 0.f;

public:
	// Sets default values for this pawn's properties
	ARTSCamera();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	UFUNCTION()
		void ZoomIn(FKey key);

	UFUNCTION()
		void ZoomOut(FKey key);

	UFUNCTION()
		void Rotate(FKey key);

	UFUNCTION()
		void StopRotation(FKey key);

	UFUNCTION()
		void Turn(float axisValue);

	UFUNCTION()
		void LookUp(float axisValue);

	UFUNCTION()
		void MoveForward(float axisValue);

	UFUNCTION()
		void MoveRight(float axisValue);

	void UpdateCameraLocationAndRotation();

	FVector GetPointOnASphere(float radius, float angleAroundZ, float heightAngle);
};
