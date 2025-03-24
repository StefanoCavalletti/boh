// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BOH_API AGame_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AGame_PlayerController();
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* GameContext;


	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ClickAction;

	void ClickOnGrid();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
