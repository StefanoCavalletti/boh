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
	GameMode->PassIfForced(); //DEVO FARE RETURN SE PASS IF FORCED RITORNA TRUE
	if (GameMode->CurrentPlayer == 0)
		return; // SE PASSA TERMINO
	if (GameMode->IsGameOver)
		return; // return se è finito il giuoco
	TArray<AGameUnit*> MyUnits;
	TArray<AGameUnit*> EnemyUnits;
	for (AGameUnit* Unit : GameMode->GField->UnitsArray) {
		if (Unit->Owner == 1) {
			MyUnits.Add(Unit);
		}
		else {
			if(Unit->HealtPoints > 0) EnemyUnits.Add(Unit);
		}
	}
	AGameUnit* EnemyBrawler = nullptr;
	for (AGameUnit* Unit : MyUnits) {
		if (Unit->HealtPoints > 0) {
			if (Unit->bCanMove) {
				switch (Unit->UnitType) {
					case EUnits::SNIPER:
						
						for (AGameUnit* GU : EnemyUnits) {
							if (GU->UnitType == EUnits::BRAWLER) EnemyBrawler = GU;
						}
						if (GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange, 0).IsEmpty())
						{
							TArray<TArray<FVector2D>> Paths;
							TArray<FVector2D> Neighbors = {
								FVector2D(+1, 0),
								FVector2D(-1, 0),
								FVector2D(0, +1),
								FVector2D(0, -1),
							};
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
								//ToGo = MinPath[Unit->MovementRange + 1];
								ToGo = MinPath[Unit->MovementRange];
							}
							GameMode->GField->MoveUnitTo(Unit, ToGo);
							GameMode->PassIfForced();
						}
						else if (EnemyBrawler and (FVector2D::Distance(Unit->GridPosition, EnemyBrawler->GridPosition) == 1.0f))
						{
							//if (FVector2D::Distance(Unit->GridPosition, EnemyBrawler->GridPosition) == 1.0f) {
								TArray<ATile*> AccessibleTiles = GameMode->GField->ReachableTiles(Unit->GridPosition, Unit->MovementRange, 0);
								ATile* Tile;
								do {
									Tile = AccessibleTiles[FMath::RandRange(0, AccessibleTiles.Num() - 1)];
								} while (FVector2D::Distance(Tile->GetGridPosition(), EnemyBrawler->GridPosition) == 1.0f);
								GameMode->GField->MoveUnitTo(Unit, Tile->GetGridPosition());
							//}
						}
						else
						{
							TArray<TArray<FVector2D>> Paths;
							TArray<FVector2D> Neighbors = {
								FVector2D(+1, 0),
								FVector2D(-1, 0),
								FVector2D(0, +1),
								FVector2D(0, -1),
							};
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
							if (MinPath.Num() > 10) {
								if (MinPath.Num() - 1 <= Unit->MovementRange) {
									ToGo = MinPath.Last();
								}
								else {
									//ToGo = MinPath[Unit->MovementRange + 1];
									ToGo = MinPath[Unit->MovementRange];
								}
								GameMode->GField->MoveUnitTo(Unit, ToGo);
								GameMode->PassIfForced();
							}
							else {
								Unit->bCanMove = false;
								OnTurn();
							}
						}
					break;

					case EUnits::BRAWLER:
						//Priorità andare sotto quello piu vicino
						if (GameMode->GField->AttackableTiles(Unit->GridPosition, Unit->AttackRange, 0).IsEmpty()) {
							TArray<TArray<FVector2D>> Paths;
							TArray<FVector2D> Neighbors = {
								FVector2D(+1, 0),
								FVector2D(-1, 0),
								FVector2D(0, +1),
								FVector2D(0, -1),
							};
							for (AGameUnit* Enemy : EnemyUnits) {
								for (FVector2D Neig : Neighbors) {
									if(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition + Neig).Num() != 0)
										Paths.Add(GameMode->GField->FindPath(Unit->GridPosition, Enemy->GridPosition+Neig));
								}
							}
							if (Paths.Num() == 0) return;
							UE_LOG(LogTemp, Warning, TEXT("PATHS DIM %d"), Paths.Num());
							TArray<FVector2D> MinPath = Paths.Last(); // SE l'ultimo è 0 è un problema, devo risolvere
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
								//ToGo = MinPath[Unit->MovementRange + 1];
								ToGo = MinPath[Unit->MovementRange];
							}
							GameMode->GField->MoveUnitTo(Unit, ToGo);
							GameMode->PassIfForced();
						}
						else {
							Unit->bCanMove = false;
							OnTurn();
						}
					break;
				}
				/*TArray<ATile*> AccessibleTiles = GameMode->GField->ReachableTiles(Unit->GridPosition, Unit->MovementRange, 0);
				UE_LOG(LogTemp, Warning, TEXT("Accessible tiles %d"), AccessibleTiles.Num());
				ATile* Tile = AccessibleTiles[FMath::RandRange(0, AccessibleTiles.Num() - 1)];
				GameMode->GField->MoveUnitTo(Unit, Tile->GetGridPosition());
				GameMode->PassIfForced();*/
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
							GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, GameMode, Unit, UnitToAttack]()
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

void ASmartPlayer::OnWin()
{
}

void ASmartPlayer::OnLose()
{
}

void ASmartPlayer::OnPlacing()
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

