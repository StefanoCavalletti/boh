// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "SmartPlayer.h"
#include "MyGameInstance.h"
#include "Game_PlayerController.h"

AMyGameModeBase::AMyGameModeBase() {
	PlayerControllerClass = AGame_PlayerController::StaticClass();
	DefaultPawnClass = AHumanPlayer::StaticClass();
	FieldSize = 25;
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	AHumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();

	if (!IsValid(HumanPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn of type '%s' was found."), *AHumanPlayer::StaticClass()->GetName());
		return;
	}

	if (GameFieldClass != nullptr)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GField->Size = FieldSize;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
	}

	float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
	float Zposition = 3500.0f;
	FVector CameraPos(CameraPosX, CameraPosX, Zposition);
	HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());

	// Human Player = 0
	Players.Add(HumanPlayer);
	
	//RANDOM PLAYER
	//auto* AI = GetWorld()->SpawnActor<ARandomPlayer>(FVector(), FRotator());

	//SMART(?) PLAYER
	auto* AI = GetWorld()->SpawnActor<ASmartPlayer>(FVector(), FRotator());
	
	// AI player = 1
	Players.Add(AI);
	
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->SetUpStarting();
}

void AMyGameModeBase::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("StartingGame"));
	IsGameOver = false;
	this->CurrentGameState = EGameState::WaitingSelection;
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->RemoveStartButton();
	GameInstance->SetUpGame();
	GField->FieldBuilder();
	PlayerToPlace = { EUnits::BRAWLER,  EUnits::SNIPER };
	AIToPlace = { EUnits::BRAWLER,  EUnits::SNIPER };
	CurrentPlayer = FMath::RandRange(0, 1); // Se 0 inizia Human, altrimenti AI
	if(CurrentPlayer==0){
		GameInstance->SetMessagge(TEXT("Human Starts Placing"));
		//TextBlock_0->SetText(FText::FromString(TEXT("Human starts")));
		this->PlaceUnits();
		
	} else {
		//TextBlock_0->SetText(FText::FromString(TEXT("AI starts")));
		GameInstance->SetMessagge(TEXT("Computer Starts Placing"));
		this->PlaceUnits();
	}
}

void AMyGameModeBase::PlaceUnits()
{
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->SetUpPlacing();
	Players[CurrentPlayer]->OnPlacing();
}

void AMyGameModeBase::SetupAndStartTurns()
{
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->SetUpTurns();
	this->NextPlayerTurn();
}

void AMyGameModeBase::PassIfForced()
{
	bool bCanStillDoSomething = false;
	for (AGameUnit* Unit : GField->UnitsArray) {
		if (Unit->Owner == CurrentPlayer) {
			if (Unit->HealtPoints >0 and (Unit->bCanMove or (Unit->bCanAttack and !GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange ,0).IsEmpty()))) {
				bCanStillDoSomething = true;
			}
		}
	}
	if (!bCanStillDoSomething) {
		SafePass();
	}
}

void AMyGameModeBase::SafePass()
{
	for (AGameUnit* Unit : GField->UnitsArray) {
		if (Unit->Owner == CurrentPlayer) {
			if (Unit->HealtPoints > 0) {
				Unit->bCanAttack = true;
				Unit->bCanMove = true;
			}
		}
	}
	GField->SetAllTilesWhite();
	CurrentGameState = EGameState::WaitingAction;
	NextPlayerTurn();
}

void AMyGameModeBase::SetPlacingUnitTypeBrawler()
{
	UE_LOG(LogTemp, Warning, TEXT("CLICK BUTTON BRAWLER"));
	if (CurrentGameState == EGameState::WaitingSelection or CurrentGameState == EGameState::PlacingSniper or CurrentGameState == EGameState::ComputerPlacing) {
		UE_LOG(LogTemp, Warning, TEXT("PLACING BRAWLER"));
		CurrentGameState = EGameState::PlacingBrawler;
	}
}

void AMyGameModeBase::SetPlacingUnitTypeSniper()
{
	UE_LOG(LogTemp, Warning, TEXT("CLICK BUTTON SNIPER"));
	if (CurrentGameState == EGameState::WaitingSelection or CurrentGameState == EGameState::PlacingBrawler or CurrentGameState == EGameState::ComputerPlacing) {
		UE_LOG(LogTemp, Warning, TEXT("PLACING SNIPER"));
		CurrentGameState = EGameState::PlacingSniper;
	}
}

int32 AMyGameModeBase::GetNextPlayer(int32 Player)
{
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

void AMyGameModeBase::NextPlayerPlace()
{
	CurrentPlayer = GetNextPlayer(CurrentPlayer);
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	if (CurrentPlayer == 0) {
		GameInstance->SetMessagge(TEXT("Human Player places"));
	}
	else if (CurrentPlayer == 1) {
		GameInstance->SetMessagge(TEXT("Computer places"));
	}
	Players[CurrentPlayer]->OnPlacing();
}

void AMyGameModeBase::NextPlayerTurn()
{
	if (IsGameOver == true) {
		return;
	}
	CurrentPlayer = GetNextPlayer(CurrentPlayer);
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	if (CurrentPlayer == 0) {
		GameInstance->SetMessagge(TEXT("Human Player plays"));
	}
	else if (CurrentPlayer == 1) {
		GameInstance->SetMessagge(TEXT("Computer plays"));
	}
	Players[CurrentPlayer]->OnTurn();
}

FVector AMyGameModeBase::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY)
{
	return GField->GetRelativeLocationByXYPosition(InX,InY);
}

void AMyGameModeBase::MyTurnSafePass()
{
	if (CurrentPlayer == 0 and (CurrentGameState == EGameState::WaitingAction)) {
		SafePass();
	}
}
