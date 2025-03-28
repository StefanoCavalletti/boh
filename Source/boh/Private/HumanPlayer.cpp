// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "MyGameModeBase.h"
#include "MyGameInstance.h"
#include "Tile.h"
#include "GameUnit.h"
#include "GameField.h"

// Sets default values
AHumanPlayer::AHumanPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// create a camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//set the camera as RootComponent
	SetRootComponent(Camera);
	// default init values
	PlayerNumber = -1;
	SelectedUnit = nullptr;
	IsMyTurn = false;

}

// Called when the game starts or when spawned
void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHumanPlayer::OnTurn()
{
	IsMyTurn = true;
	UE_LOG(LogTemp, Warning, TEXT("PLAYER TURN"));
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->PassIfForced();
	GameMode->CurrentGameState = EGameState::WaitingAction;
}

void AHumanPlayer::OnPlacing()
{
	IsMyTurn = true;
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!(GameMode->CurrentGameState == EGameState::PlacingSniper or GameMode->CurrentGameState == EGameState::PlacingBrawler)) {
		GameMode->CurrentGameState = EGameState::WaitingSelection;
	}
}

void AHumanPlayer::OnClick()
{
	
	FHitResult Hit = FHitResult(ForceInit);
	// GetHitResultUnderCursor function sends a ray from the mouse position and gives the corresponding hit results
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);

	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode->IsGameOver) return; // If the game is over do nothing
	switch (GameMode->CurrentGameState) {
		// Click acts different based on the state of the game
		//If i am placing the brawler
		case EGameState::PlacingBrawler:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
				{
					if (CurrTile->TileStatus == ETileStatus::EMPTY) // Checks if the tile is empty
					{
						UE_LOG(LogTemp, Warning, TEXT("CLICK BRAWLER %s"), *UEnum::GetValueAsString(GameMode->CurrentGameState));
						//Spawn the brawler actor
						FVector Location = CurrTile->GetActorLocation() + FVector(0,0,2);
						AGameUnit* Brawler = GetWorld()->SpawnActor<AGameUnit>(GameMode->BlueBrawlerActor, Location, FRotator::ZeroRotator);
						CurrTile->TileStatus = ETileStatus::OCCUPIED;
						//Set up the gameUnit
						Brawler->SetUpUnit(EUnits::BRAWLER, GameMode->CurrentPlayer, CurrTile->GetGridPosition());
						GameMode->GField->UnitsArray.Add(Brawler);
						
						GameMode->CurrentGameState = EGameState::WaitingSelection;
						UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
						GameInstance->RemoveBrawlerButton();
						GameMode->PlayerToPlace.Remove(EUnits::BRAWLER);

						IsMyTurn = false;
						//If placing is over start game otherwise continue placing
						if (GameMode->AIToPlace.IsEmpty() and GameMode->PlayerToPlace.IsEmpty()) {
							GameMode->SetupAndStartTurns();
						}
						else
						{
							GameMode->NextPlayerPlace();
						}
					}
				}
			}
		break;
		//If i am placing the sniper
		case EGameState::PlacingSniper:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
				{
					if (CurrTile->TileStatus == ETileStatus::EMPTY) // check if the tile is empty
					{
						UE_LOG(LogTemp, Warning, TEXT("CLICK SNIPER"));
						// Spawn sniper actor
						FVector Location = CurrTile->GetActorLocation() + FVector(0,0,2);
						AGameUnit* Sniper = GetWorld()->SpawnActor<AGameUnit>(GameMode->BlueSniperActor, Location, FRotator::ZeroRotator);
						CurrTile->TileStatus = ETileStatus::OCCUPIED;
						//Set up GameUnit
						Sniper->SetUpUnit(EUnits::SNIPER, GameMode->CurrentPlayer, CurrTile->GetGridPosition());
						GameMode->GField->UnitsArray.Add(Sniper);
						
						GameMode->CurrentGameState = EGameState::WaitingSelection;
						UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
						GameInstance->RemoveSniperButton();
						GameMode->PlayerToPlace.Remove(EUnits::SNIPER);

						IsMyTurn = false;
						//If placing is over start game otherwise continue placing
						if (GameMode->AIToPlace.IsEmpty() and GameMode->PlayerToPlace.IsEmpty()) {
							GameMode->SetupAndStartTurns();
						}
						else
						{
							GameMode->NextPlayerPlace();
						}
					}
				}
			}
		break;
		//If the game is started and i did not click on a GameUnit yet
		case EGameState::WaitingAction:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				UE_LOG(LogTemp, Warning, TEXT("MIO TURNO HIT!!"));
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor())) // If i did hit a GameUnit
				{
					UE_LOG(LogTemp, Warning, TEXT("PRESO GAMEUNIT"));
					if (GameUnit->Owner == GameMode->CurrentPlayer) {  // If the GameUnit was mine
						UE_LOG(LogTemp, Warning, TEXT("MIO GAMEUNIT"));
						FVector2D Position = GameUnit->GridPosition;
						if (GameUnit->bCanMove) { // If the unit can move show the movement range
							UE_LOG(LogTemp, Warning, TEXT("PUO MUOVERE"));
							GameMode->CurrentGameState = EGameState::UnitSelected;
							SelectedUnit = GameUnit;
							GameMode->GField->ShowReachableTiles(Position, GameUnit->MovementRange);
						}
						if (GameUnit->bCanAttack and !GameMode->GField->AttackableTiles(GameUnit->GridPosition, GameUnit->AttackRange).IsEmpty()) {
							UE_LOG(LogTemp, Warning, TEXT("PUO ATTACCARE")); // if the unit can attack show the attack range
							GameMode->CurrentGameState = EGameState::UnitSelected;
							SelectedUnit = GameUnit;
							GameMode->GField->ShowAttackableTiles(Position, GameUnit->AttackRange);
						}
					}
				}
			}
		break;
		//If i had selected a GameUnit
		case EGameState::UnitSelected:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				UE_LOG(LogTemp, Warning, TEXT("MIO TURNO MOVING HIT!!"));
				if (ATile* Tile = Cast<ATile>(Hit.GetActor())) // If i click on a tile
				{
					if (Tile->TileColor == ETileColor::GREEN and Tile->TileStatus == ETileStatus::EMPTY) { //If the tile is in movement range
						// Move the Unit to the tile i clicked
						FVector2D EndPosition = Tile->GetGridPosition();
						GameMode->GField->SetAllTilesWhite();
						GameMode->GField->MoveUnitTo(SelectedUnit, EndPosition); //Moves the unit
						SelectedUnit = nullptr;
						GameMode->PassIfForced();
					}
				}
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor()))
				{
					//If i click on the same unit again close the ranges
					if (SelectedUnit != nullptr and SelectedUnit->Owner == GameUnit->Owner and SelectedUnit->UnitType == GameUnit->UnitType) {
						GameMode->GField->SetAllTilesWhite();
						GameMode->CurrentGameState = EGameState::WaitingAction;
						SelectedUnit = nullptr;
					}
				}
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor()))
				{
					//if i click on a enemy unit attack it (if is in range)
					ATile** Tile = GameMode->GField->TileMap.Find(GameUnit->GridPosition);
					if (SelectedUnit != nullptr and SelectedUnit->Owner != GameUnit->Owner and (*Tile)->TileColor == ETileColor::RED) {
						//Attack
						GameMode->GField->Attack(SelectedUnit, GameUnit);
						GameMode->GField->SetAllTilesWhite();
						GameMode->CurrentGameState = EGameState::WaitingAction;
						SelectedUnit->bCanAttack = false;
						SelectedUnit->bCanMove = false;
						SelectedUnit = nullptr;
						if (!GameMode->IsGameOver) {
							GameMode->PassIfForced();
						}
					}
				}
			}
		break;
	}
}

void AHumanPlayer::OnWin()
{
}

void AHumanPlayer::OnLose()
{
}



