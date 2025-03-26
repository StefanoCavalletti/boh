// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "MyGameModeBase.h"
#include "GameUnit.h"

void UMyGameInstance::SetUpStarting()
{
	GameWidgetStarting = CreateWidget<UUserWidget>(GetWorld(), WidgetStartingClass);
	GameWidgetStarting->AddToViewport();
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	UButton* StartButton = Cast<UButton>(GameWidgetStarting->GetWidgetFromName(TEXT("start")));
	if (StartButton) {
		UE_LOG(LogTemp, Warning, TEXT("Button trovato"));
		StartButton->OnClicked.AddDynamic(GameMode, &AMyGameModeBase::StartGame);
	}
	UE_LOG(LogTemp, Warning, TEXT("SetUpStarting OK"));
}

void UMyGameInstance::SetUpGame()
{
	GameWidget = CreateWidget<UUserWidget>(GetWorld(), GameWidgetClass);
	GameWidget->AddToViewport();
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	UButton* ResetButton = Cast<UButton>(GameWidget->GetWidgetFromName(TEXT("ResetButton")));
	ResetButton->OnClicked.AddDynamic(GameMode->GField, &AGameField::ResetField);
	LogScrollBox = Cast<UScrollBox>(GameWidget->GetWidgetFromName(TEXT("LogBox")));
}

void UMyGameInstance::SetUpPlacing()
{
	GameWidgetPlacing = CreateWidget<UUserWidget>(GetWorld(), WidgetPlacingClass);
	GameWidgetPlacing->AddToViewport();
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	UButton* BrawlerButton = Cast<UButton>(GameWidgetPlacing->GetWidgetFromName(TEXT("BrawlerButton")));
	UButton* SniperButton = Cast<UButton>(GameWidgetPlacing->GetWidgetFromName(TEXT("SniperButton")));
	BrawlerButton->OnClicked.AddDynamic(GameMode, &AMyGameModeBase::SetPlacingUnitTypeBrawler);
	SniperButton->OnClicked.AddDynamic(GameMode, &AMyGameModeBase::SetPlacingUnitTypeSniper);
}

void UMyGameInstance::SetUpTurns()
{
	GameWidgetTurns = CreateWidget<UUserWidget>(GetWorld(), WidgetTurnsClass);
	GameWidgetTurns->AddToViewport();
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	UButton* PassButton = Cast<UButton>(GameWidgetTurns->GetWidgetFromName(TEXT("PassButton")));
	PassButton->OnClicked.AddDynamic(GameMode, &AMyGameModeBase::MyTurnSafePass);
	UpdateStats();
}

void UMyGameInstance::UpdateStats()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	UTextBlock* PlayerBrawlerStats = Cast<UTextBlock>(GameWidgetTurns->GetWidgetFromName(TEXT("PlayerBrawlerStats")));
	UTextBlock* PlayerSniperStats = Cast<UTextBlock>(GameWidgetTurns->GetWidgetFromName(TEXT("PlayerSniperStats")));
	UTextBlock* ComputerBrawlerStats = Cast<UTextBlock>(GameWidgetTurns->GetWidgetFromName(TEXT("ComputerBrawlerStats")));
	UTextBlock* ComputerSniperStats = Cast<UTextBlock>(GameWidgetTurns->GetWidgetFromName(TEXT("ComputerSniperStats")));
	for (AGameUnit* Unit : GameMode->GField->UnitsArray) {
		switch (Unit->UnitType) {
		case EUnits::SNIPER:
			if (Unit->Owner == 0) {
				PlayerSniperStats->SetText(FText::FromString(FString::Printf(TEXT("Human Sniper HP: %d"), Unit->HealtPoints)));
			}
			else if (Unit->Owner == 1) {
				ComputerSniperStats->SetText(FText::FromString(FString::Printf(TEXT("Computer Sniper HP: %d"), Unit->HealtPoints)));
			}
			break;

		case EUnits::BRAWLER:
			if (Unit->Owner == 0) {
				PlayerBrawlerStats->SetText(FText::FromString(FString::Printf(TEXT("Human Brawler HP: %d"), Unit->HealtPoints)));
			}
			else if (Unit->Owner == 1) {
				ComputerBrawlerStats->SetText(FText::FromString(FString::Printf(TEXT("Computer Brawler HP: %d"), Unit->HealtPoints)));
			}
			break;
		}
	}
}

void UMyGameInstance::RemoveBrawlerButton()
{
	UButton* BrawlerButton = Cast<UButton>(GameWidgetPlacing->GetWidgetFromName(TEXT("BrawlerButton")));
	BrawlerButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UMyGameInstance::RemoveSniperButton()
{
	UButton* SniperButton = Cast<UButton>(GameWidgetPlacing->GetWidgetFromName(TEXT("SniperButton")));
	SniperButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UMyGameInstance::RemoveWidgets()
{
	if(GameWidget)
		GameWidget->RemoveFromParent();
	if(GameWidgetPlacing)
		GameWidgetPlacing->RemoveFromParent();
	if(GameWidgetTurns)
		GameWidgetTurns->RemoveFromParent();
}

void UMyGameInstance::RemoveStartButton()
{
	GameWidgetStarting->RemoveFromParent();
}

void UMyGameInstance::SetMessagge(FString Mex)
{
	UTextBlock* TextBlock_0 = Cast<UTextBlock>(GameWidget->GetWidgetFromName(TEXT("Message")));
	TextBlock_0->SetText(FText::FromString(Mex));
}

void UMyGameInstance::AddLogMessage(FString Message)
{
	if (!LogScrollBox) return;

	// Create new text block for the message
	UTextBlock* NewMessage = NewObject<UTextBlock>(this);
	NewMessage->SetText(FText::FromString(Message));
	NewMessage->Font.Size = 18; 
	LogScrollBox->AddChild(NewMessage);

	// Add to logs 
	LogMessages.Add(NewMessage);

	// Remove oldest message
	if (LogMessages.Num() > MaxLogLines)
	{
		UTextBlock* OldMessage = LogMessages[0];
		LogScrollBox->RemoveChild(OldMessage);
		LogMessages.RemoveAt(0);
	}
	// Scroll to last message
	LogScrollBox->ScrollToEnd();
}
