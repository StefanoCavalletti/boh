// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "GameUnit.h"
#include "Camera/CameraComponent.h"
#include "HumanPlayer.generated.h"

UCLASS()
class BOH_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHumanPlayer();
	// camera component attacched to player pawn
	UCameraComponent* Camera;
	AGameUnit* SelectedUnit;
	// game instance reference
	//UTTT_GameInstance* GameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool IsMyTurn;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;
	virtual void OnPlacing() override;

	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();

};
