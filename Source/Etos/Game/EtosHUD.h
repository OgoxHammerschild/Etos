// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/HUD.h"
#include "EtosHUD.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API AEtosHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UInGameUI> wInGameUI;

private:

	UPROPERTY()
		UInGameUI* inGameUI;

public:

	UInGameUI* GetInGameUI();

	virtual void BeginDestroy() override;
};
