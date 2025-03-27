// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartPlayer.h"
#include "Tile.h"
#include "MyGameModeBase.h"
#include "GameUnit.h"

// Sets default values
ASmartPlayer::ASmartPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASmartPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASmartPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASmartPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASmartPlayer::OnTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("SMART COMPUTER TURN"));
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->CurrentGameState = EGameState::WaitingAction;
	GameMode->PassIfForced(); 
	if (GameMode->CurrentPlayer == 0)
		return; // If is not my turn anymore i return
	if (GameMode->IsGameOver)
		return; // If the game is over i return
	TArray<AGameUnit*> MyUnits;
	TArray<AGameUnit*> EnemyUnits;
	AGameUnit* EnemyBrawler = nullptr;
	TArray<FVector2D> Neighbors = {FVector2D(+1, 0), FVector2D(-1, 0), FVector2D(0, +1), FVector2D(0, -1)};
	for (AGameUnit* Unit : GameMode->GField->UnitsArray) {
		if (Unit->Owner == 1) {
			MyUnits.Add(Unit);
		}
		else {
			if(Unit->HealtPoints > 0) EnemyUnits.Add(Unit);
		}
	}
	for (AGameUnit* Unit : MyUnits) { // Search for my alive units
		if (Unit->HealtPoints > 0) {
			if (Unit->bCanMove) { // If it can move
				switch (Unit->UnitType) {
					//SNIPER MOVEMENT LOGIC
					case EUnits::SNIPER:
						for (AGameUnit* GU : EnemyUnits) {
							if (GU->UnitType == EUnits::BRAWLER) EnemyBrawler = GU;
						}
						//If there are no enemys in range in attack range, move towards nearest enemy
						if (GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange).IsEmpty())
						{
							TArray<TArray<FVector2D>> Paths;
							for (AGameUnit* Enemy : EnemyUnits) {
								for (FVector2D Neig : Neighbors) {
									if (GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig).Num() != 0)
										Paths.Add(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig));
								}
							}
							if (Paths.Num() == 0) return;
							UE_LOG(LogTemp, Warning, TEXT("PATHS DIM %d"), Paths.Num());
							TArray<FVector2D> MinPath = Paths.Last();
							for (TArray<FVector2D> Pa : Paths) {
								UE_LOG(LogTemp, Warning, TEXT("PATH DIM %d"), Pa.Num());
								if (Pa.Num() < MinPath.Num() and Pa.Num() > 1) {
									MinPath = Pa;
								}
							}
							FVector2D ToGo;
							UE_LOG(LogTemp, Warning, TEXT("MINPATH DIM %d"), MinPath.Num());
							if (MinPath.Num() - 1 <= Unit->MovementRange) {
								ToGo = MinPath.Last();
							}
							else {
								ToGo = MinPath[Unit->MovementRange];
							}
							GameMode->GField->MoveUnitTo(Unit, ToGo);
							GameMode->PassIfForced();
						}		//If the enemy brawler is in counter attack range, move out before shooting, avoiding counter attack
						else if (EnemyBrawler and (FVector2D::Distance(Unit->GridPosition, EnemyBrawler->GridPosition) == 1.0f))
						{
								TArray<ATile*> AccessibleTiles = GameMode->GField->ReachableTiles(Unit->GridPosition, Unit->MovementRange);
								ATile* Tile;
								do {
									Tile = AccessibleTiles[FMath::RandRange(0, AccessibleTiles.Num() - 1)];
									if (AccessibleTiles.Num() == 1) break;
								} while (FVector2D::Distance(Tile->GetGridPosition(), EnemyBrawler->GridPosition) == 1.0f);
								GameMode->GField->MoveUnitTo(Unit, Tile->GetGridPosition());
						}
						else {   //if there are enemys in attack range, get closer but not too much
							TArray<TArray<FVector2D>> Paths;
							
							for (AGameUnit* Enemy : EnemyUnits) 
							{
								for (FVector2D Neig : Neighbors) 
								{
									if (GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig).Num() != 0)
										Paths.Add(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig));
								}
							}
							if (Paths.Num() == 0) return;
							UE_LOG(LogTemp, Warning, TEXT("PATHS DIM %d"), Paths.Num());
							TArray<FVector2D> MinPath = Paths.Last();
							for (TArray<FVector2D> Pa : Paths)
							{
								UE_LOG(LogTemp, Warning, TEXT("PATH DIM %d"), Pa.Num());
								if (Pa.Num() < MinPath.Num() and Pa.Num() > 1)
								{
									MinPath = Pa;
								}
							}
							FVector2D ToGo;
							UE_LOG(LogTemp, Warning, TEXT("MINPATH DIM %d"), MinPath.Num());
							if (MinPath.Num() > 10) {
								if (MinPath.Num() - 1 <= Unit->MovementRange) {
									ToGo = MinPath.Last();
								}
								else {
									ToGo = MinPath[Unit->MovementRange];
								}
								GameMode->GField->MoveUnitTo(Unit, ToGo);
								GameMode->PassIfForced();
							} else { // otherwise just don't move 
								Unit->bCanMove = false;
								OnTurn();
							}
						}
					break;

					// BRAWLER MOVEMENT LOGIC
					case EUnits::BRAWLER:
						//Walk to the closest enemy
						if (GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange).IsEmpty()) {
							TArray<TArray<FVector2D>> Paths;
							
							for (AGameUnit* Enemy : EnemyUnits) {
								for (FVector2D Neig : Neighbors) {
									if(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig).Num() != 0)
										Paths.Add(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition+Neig));
								}
							}
							if (Paths.Num() == 0) return;
							UE_LOG(LogTemp, Warning, TEXT("PATHS DIM %d"), Paths.Num());
							TArray<FVector2D> MinPath = Paths.Last();
							for (TArray<FVector2D> Pa : Paths) {
								UE_LOG(LogTemp, Warning, TEXT("PATH DIM %d"), Pa.Num());
								if (Pa.Num() < MinPath.Num() and Pa.Num() > 1) {
									MinPath = Pa;
								}
							}
							FVector2D ToGo;
							UE_LOG(LogTemp, Warning, TEXT("MINPATH DIM %d"), MinPath.Num());
							if (MinPath.Num() - 1 <= Unit->MovementRange) {
								ToGo = MinPath.Last();
							}
							else {
								ToGo = MinPath[Unit->MovementRange];
							}
							GameMode->GField->MoveUnitTo(Unit, ToGo);
							GameMode->PassIfForced();
						}
						else {//if already in attack range just don't move
							Unit->bCanMove = false;
							OnTurn();
						}
					break;
				}
				return;
			}
			else if (Unit->bCanAttack)
			{ // Attack random unit in attack range
				TArray<ATile*> AttackableTiles = GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange);
				UE_LOG(LogTemp, Warning, TEXT("Attackable tiles %d"), AttackableTiles.Num());
				if (!AttackableTiles.IsEmpty()) {
					ATile* TileToAttack = AttackableTiles[FMath::RandRange(0, AttackableTiles.Num() - 1)];
					for (AGameUnit* UnitToAttack : GameMode->GField->UnitsArray) {
						if (UnitToAttack->GridPosition == TileToAttack->GetGridPosition() and UnitToAttack->Owner != GameMode->CurrentPlayer) {
							GameMode->GField->ShowAttackableTiles(Unit->GridPosition, Unit->AttackRange);
							GameMode->CurrentGameState = EGameState::MovingUnit;
							FTimerHandle TimerHandle;
							GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, GameMode, Unit, UnitToAttack]()
								{
									GameMode->GField->SetAllTilesWhite();
									GameMode->GField->Attack(Unit, UnitToAttack);
									this->OnTurn();
								}), 1.5f, false);
							return;
						}
					}
				}
			}
		}
	}
	//If the game is not over i check i check if the turn is over
	if (!GameMode->IsGameOver) {
		GameMode->PassIfForced();
	}
}

void ASmartPlayer::OnWin()
{
}

void ASmartPlayer::OnLose()
{
}

void ASmartPlayer::OnPlacing()
{ // random placing
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->CurrentGameState = EGameState::ComputerPlacing;
	FTimerHandle TimerHandle;
	//Wait a little
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{	// Place a random unit in a random free tile
			AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
			EUnits UnitToPlace = GameMode->AIToPlace[FMath::RandRange(0, GameMode->AIToPlace.Num() - 1)];
			ATile** RandTile = nullptr;
			int32 IndexX = 0, IndexY = 0;
			do {
				IndexX = FMath::RandRange(0, GameMode->FieldSize - 1);
				IndexY = FMath::RandRange(0, GameMode->FieldSize - 1);
				RandTile = GameMode->GField->TileMap.Find(FVector2D(IndexX, IndexY));
			} while ((*RandTile)->TileStatus == ETileStatus::OCCUPIED);		//look for a random empty tile

			FVector Location = (*RandTile)->GetActorLocation() + FVector(0, 0, 2);
			(*RandTile)->TileStatus = ETileStatus::OCCUPIED;

			AGameUnit* UnitPtr = nullptr;

			switch (UnitToPlace) { // Place the unit
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
			//If placing is over start game otherwise continue placing
			if (GameMode->AIToPlace.IsEmpty() and GameMode->PlayerToPlace.IsEmpty()) {
				GameMode->SetupAndStartTurns();
			}
			else
			{
				GameMode->NextPlayerPlace();
			}
		}, 1.5f, false);
}