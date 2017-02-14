// © 2016 - 2017 Daniel Bortfeld

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

	UPROPERTY()
		UArrowComponent* Origin;

	UPROPERTY()
		UCameraComponent* Camera;

private:

	UPROPERTY()
		class AEtosPlayerController* myPlayerController;

	UPROPERTY()
		bool bRotateCamera = false;

	UPROPERTY()
		bool bPanCamera = false;
	
	UPROPERTY()
		bool bPanCamera_IE_Repeat = false;

	UPROPERTY()
		FVector2D panningStartPosition;

	UPROPERTY(EditDefaultsOnly)
		float panningSpeed = 1.5f;

	UPROPERTY(EditDefaultsOnly)
		float panningThreshold = 20;

	UPROPERTY(EditDefaultsOnly)
		float cameraZAngle = 0.f;

	UPROPERTY(EditDefaultsOnly)
		float cameraHeightAngle = 27.5f;

	UPROPERTY(EditDefaultsOnly)
		float cameraHeightAngleMin = 10;

	UPROPERTY(EditDefaultsOnly)
		float cameraHeightAngleMax = 70;

	UPROPERTY(EditDefaultsOnly)
		float rotationSpeed = 4.0f;

	UPROPERTY(EditDefaultsOnly)
		float cameraRadius = 2048.f;

	UPROPERTY(EditDefaultsOnly)
		float zoomSpeed = 768.f;

	UPROPERTY(EditDefaultsOnly)
		float radiusMin = 256.f;

	UPROPERTY(EditDefaultsOnly)
		float radiusMax = 1792.f;

	UPROPERTY(EditDefaultsOnly)
		float movementSpeed = 100.f;

	//idk what that is good for
	UPROPERTY(EditDefaultsOnly)
		float edgeForwardAxis = 0.f;

	//idk what that is good for
	UPROPERTY(EditDefaultsOnly)
		float edgeRightAxis = 0.f;

public:
	// Sets default values for this pawn's properties
	ARTSCamera();

	virtual void Tick(float DeltaTime) override;

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
		void StartPanning(FKey key);

	UFUNCTION()
		void Pan(FKey key);

	UFUNCTION()
		void StopPanning(FKey key);

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
