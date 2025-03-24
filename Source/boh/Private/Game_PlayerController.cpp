// Fill out your copyright notice in the Description page of Project Settings.


#include "Game_PlayerController.h"
#include "Components/InputComponent.h"
#include "HumanPlayer.h"

AGame_PlayerController::AGame_PlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void AGame_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(GameContext, 0);
	}
}

void AGame_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AGame_PlayerController::ClickOnGrid);
	}
}

void AGame_PlayerController::ClickOnGrid()
{
	const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
	if (IsValid(HumanPlayer))
	{
		HumanPlayer->OnClick();
	}
}
