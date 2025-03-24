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
	// get the game instance reference
	//GameInstance = Cast<UTTT_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
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
	//GameMode->CurrentGameState = EGameState::WaitingSelection;
}

void AHumanPlayer::OnClick()
{
	FHitResult Hit = FHitResult(ForceInit);
	// GetHitResultUnderCursor function sends a ray from the mouse position and gives the corresponding hit results
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);

	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	switch (GameMode->CurrentGameState) {
		
		case EGameState::PlacingBrawler:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
				{
					if (CurrTile->TileStatus == ETileStatus::EMPTY)
					{
						UE_LOG(LogTemp, Warning, TEXT("CLICK BRAWLER %s"), *UEnum::GetValueAsString(GameMode->CurrentGameState));
						FVector Location = CurrTile->GetActorLocation() + FVector(0,0,2);
						AGameUnit* Brawler = GetWorld()->SpawnActor<AGameUnit>(GameMode->BlueBrawlerActor, Location, FRotator::ZeroRotator);
						CurrTile->TileStatus = ETileStatus::OCCUPIED;

						Brawler->SetUpUnit(EUnits::BRAWLER, GameMode->CurrentPlayer, CurrTile->GetGridPosition());
						GameMode->GField->UnitsArray.Add(Brawler);
						
						GameMode->CurrentGameState = EGameState::WaitingSelection;
						UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
						GameInstance->RemoveBrawlerButton();
						GameMode->PlayerToPlace.Remove(EUnits::BRAWLER);

						IsMyTurn = false;

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

		case EGameState::PlacingSniper:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
				{
					if (CurrTile->TileStatus == ETileStatus::EMPTY)
					{
						UE_LOG(LogTemp, Warning, TEXT("CLICK SNIPER"));
						FVector Location = CurrTile->GetActorLocation() + FVector(0,0,2);
						AGameUnit* Sniper = GetWorld()->SpawnActor<AGameUnit>(GameMode->BlueSniperActor, Location, FRotator::ZeroRotator);
						CurrTile->TileStatus = ETileStatus::OCCUPIED;
						
						Sniper->SetUpUnit(EUnits::SNIPER, GameMode->CurrentPlayer, CurrTile->GetGridPosition());
						GameMode->GField->UnitsArray.Add(Sniper);
						
						GameMode->CurrentGameState = EGameState::WaitingSelection;
						UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
						GameInstance->RemoveSniperButton();
						GameMode->PlayerToPlace.Remove(EUnits::SNIPER);

						IsMyTurn = false;

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

		case EGameState::WaitingAction:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				UE_LOG(LogTemp, Warning, TEXT("MIO TURNO HIT!!"));
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor()))
				{
					UE_LOG(LogTemp, Warning, TEXT("PRESO GAMEUNIT"));
					if (GameUnit->Owner == GameMode->CurrentPlayer) {
						UE_LOG(LogTemp, Warning, TEXT("MIO GAMEUNIT"));
						FVector2D Position = GameUnit->GridPosition;
						if (GameUnit->bCanMove) {
							UE_LOG(LogTemp, Warning, TEXT("PUO MUOVERE"));
							GameMode->CurrentGameState = EGameState::MovingUnit;
							SelectedUnit = GameUnit;
							GameMode->GField->ShowReachableTiles(Position, GameUnit->MovementRange, 0);
						}
						if (GameUnit->bCanAttack and !GameMode->GField->AttackableTiles(GameUnit->GridPosition, GameUnit->AttackRange, 0).IsEmpty()) {
							UE_LOG(LogTemp, Warning, TEXT("PUO ATTACCARE"));
							GameMode->CurrentGameState = EGameState::MovingUnit;
							SelectedUnit = GameUnit;
							GameMode->GField->ShowAttackableTiles(Position, GameUnit->AttackRange, 0);
						}
					}
				}
				//IsMyTurn = false;
			}
		break;

		case EGameState::MovingUnit:
			if (Hit.bBlockingHit and IsMyTurn)
			{
				UE_LOG(LogTemp, Warning, TEXT("MIO TURNO MOVING HIT!!"));
				if (ATile* Tile = Cast<ATile>(Hit.GetActor()))
				{
					if (Tile->TileColor == ETileColor::GREEN and Tile->TileStatus == ETileStatus::EMPTY) {
						//Muovi il giocatore
						//FVector2D StartPosition = SelectedUnit->GridPosition;
						FVector2D EndPosition = Tile->GetGridPosition();
						GameMode->GField->MoveUnitTo(SelectedUnit, EndPosition); //muove la gameunit
						GameMode->GField->SetAllTilesWhite();
						GameMode->CurrentGameState = EGameState::WaitingAction;
						SelectedUnit = nullptr;
						GameMode->PassIfForced();
					}
				}
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor()))
				{
					if (SelectedUnit != nullptr and SelectedUnit->Owner == GameUnit->Owner and SelectedUnit->UnitType == GameUnit->UnitType) {
					//if (SelectedUnit != nullptr and SelectedUnit == GameUnit) {
						GameMode->GField->SetAllTilesWhite();
						GameMode->CurrentGameState = EGameState::WaitingAction;
						SelectedUnit = nullptr;
					}
				}
				if (AGameUnit* GameUnit = Cast<AGameUnit>(Hit.GetActor()))
				{
					ATile** Tile = GameMode->GField->TileMap.Find(GameUnit->GridPosition);
					if (SelectedUnit != nullptr and SelectedUnit->Owner != GameUnit->Owner and (*Tile)->TileColor == ETileColor::RED) {
						//ATTACCA
						GameMode->GField->Attack(SelectedUnit, GameUnit);
						GameMode->GField->SetAllTilesWhite();
						GameMode->CurrentGameState = EGameState::WaitingAction;
						SelectedUnit->bCanAttack = false;
						SelectedUnit->bCanMove = false;
						SelectedUnit = nullptr;
						if (!GameMode->IsGameOver) {
							GameMode->PassIfForced();
						}
						else {
							//GameMode->GField->ResetField();
						}
					}
				}
				//IsMyTurn = false;
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



