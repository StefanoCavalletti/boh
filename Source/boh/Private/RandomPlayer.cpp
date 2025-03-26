// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "Tile.h"
#include "MyGameModeBase.h"
#include "GameUnit.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARandomPlayer::OnTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("RANDOM AI TURN"));
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode->IsGameOver) return; 
	GameMode->PassIfForced(); 
	if (GameMode->CurrentPlayer == 0) return;// SE PASSA TERMINO
	for (AGameUnit* Unit : GameMode->GField->UnitsArray) {
		if (Unit->Owner == GameMode->CurrentPlayer and Unit->HealtPoints > 0) {
			if (Unit->bCanMove) {
				TArray<ATile*> AccessibleTiles = GameMode->GField->ReachableTiles(Unit->GridPosition, Unit->MovementRange, 0);
				UE_LOG(LogTemp, Warning, TEXT("Accessible tiles %d"), AccessibleTiles.Num());
				ATile* Tile = AccessibleTiles[FMath::RandRange(0, AccessibleTiles.Num() - 1)];
				GameMode->GField->MoveUnitTo(Unit, Tile->GetGridPosition());
				GameMode->PassIfForced();
				return;
			}
			else if (Unit->bCanAttack)
			{
				TArray<ATile*> AttackableTiles = GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange, 0);
				UE_LOG(LogTemp, Warning, TEXT("Attackable tiles %d"), AttackableTiles.Num());
				if (!AttackableTiles.IsEmpty()) {
					ATile* TileToAttack = AttackableTiles[FMath::RandRange(0, AttackableTiles.Num() - 1)];
					for (AGameUnit* UnitToAttack : GameMode->GField->UnitsArray) {
						if (UnitToAttack->GridPosition == TileToAttack->GetGridPosition() and UnitToAttack->Owner != GameMode->CurrentPlayer) {
							GameMode->GField->ShowAttackableTiles(Unit->GridPosition, Unit->AttackRange, 0);
							FTimerHandle TimerHandle;
							GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, GameMode,Unit,UnitToAttack]()
							{
								GameMode->GField->SetAllTilesWhite();
								GameMode->GField->Attack(Unit, UnitToAttack);
								//GameMode->PassIfForced();
								this->OnTurn();
							}), 1.5f, false);
							return;
						}
					}
				}
			}
		}
	}
	if (!GameMode->IsGameOver) {
		//GameMode->NextPlayerTurn();
		GameMode->PassIfForced();
	}
	else {
		//GameMode->GField->ResetField();
	}
}

void ARandomPlayer::OnWin()
{
}

void ARandomPlayer::OnLose()
{
}

void ARandomPlayer::OnPlacing()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->CurrentGameState = EGameState::ComputerPlacing;
	FTimerHandle TimerHandle;
	//ritarda tutto di 3 secondi
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{	// effettivo corpo della funzione
			AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
			EUnits UnitToPlace = GameMode->AIToPlace[FMath::RandRange(0, GameMode->AIToPlace.Num() - 1)];
			ATile** RandTile = nullptr;
			int32 IndexX = 0, IndexY = 0;
			do {
				IndexX = FMath::RandRange(0, GameMode->FieldSize - 1);
				IndexY = FMath::RandRange(0, GameMode->FieldSize - 1);
				RandTile = GameMode->GField->TileMap.Find(FVector2D(IndexX, IndexY));
			} while ((*RandTile)->TileStatus == ETileStatus::OCCUPIED);
			
			FVector Location = (*RandTile)->GetActorLocation() + FVector(0, 0, 2);
			(*RandTile)->TileStatus = ETileStatus::OCCUPIED;
			
			AGameUnit* UnitPtr = nullptr;

			switch (UnitToPlace) {
				case EUnits::BRAWLER:
					UnitPtr = GetWorld()->SpawnActor<AGameUnit>(GameMode->RedBrawlerActor, Location, FRotator::ZeroRotator);
					GameMode->AIToPlace.Remove(EUnits::BRAWLER);
					UnitPtr->SetUpUnit(EUnits::BRAWLER, GameMode->CurrentPlayer, FVector2D(IndexX, IndexY));
					GameMode->GField->UnitsArray.Add(UnitPtr);
					break;

				case EUnits::SNIPER:
					UnitPtr = GetWorld()->SpawnActor<AGameUnit>(GameMode->RedSniperActor, Location, FRotator::ZeroRotator);
					GameMode->AIToPlace.Remove(EUnits::SNIPER);
					UnitPtr->SetUpUnit(EUnits::SNIPER, GameMode->CurrentPlayer, FVector2D(IndexX, IndexY));
					GameMode->GField->UnitsArray.Add(UnitPtr);
					break;
			}
			if (GameMode->AIToPlace.IsEmpty() and GameMode->PlayerToPlace.IsEmpty()) {
				GameMode->SetupAndStartTurns();
			}
			else
			{
				GameMode->NextPlayerPlace();
			}
		}, 1.5f, false); 
}