// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BOH_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	public:
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
		TSubclassOf<UUserWidget> WidgetStartingClass;

		UPROPERTY()
		UUserWidget* GameWidgetStarting;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
		TSubclassOf<UUserWidget> GameWidgetClass;
		/** Riferimento al Widget creato */
		UPROPERTY()
		UUserWidget* GameWidget;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
		TSubclassOf<UUserWidget> WidgetPlacingClass;

		UPROPERTY()
		UUserWidget* GameWidgetPlacing;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
		TSubclassOf<UUserWidget> WidgetTurnsClass;

		UPROPERTY()
		UUserWidget* GameWidgetTurns;

		int32 HumanScore = 0;

		int32 ComputerScore = 0;

		UPROPERTY(meta = (BindWidget))
		UScrollBox* LogScrollBox;

		TArray<UTextBlock*> LogMessages;
		const int32 MaxLogLines = 10; // Numero massimo di righe nel log

		void SetUpStarting();

		void SetUpGame();

		void SetUpPlacing();

		void SetUpTurns();

		void UpdateStats();

		void RemoveBrawlerButton();

		void RemoveSniperButton();

		void RemoveWidgets();

		void RemoveStartButton();

		void SetMessagge(FString Mex);

		void AddLogMessage(FString Message);
}; 
