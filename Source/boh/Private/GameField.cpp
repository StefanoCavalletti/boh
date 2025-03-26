// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "MyGameModeBase.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Math/UnrealMathUtility.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Sort.h" // Includi questa libreria per l'ordinamento


// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Size = 25;
	TileSize = 120.f;
	CellPadding = 0.1f;
	ObstaclePercentage = 0.2f; // Example
	NextCellPositionMultiplier = (TileSize + TileSize * CellPadding) / TileSize;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay(); 
	//FieldBuilder();
	UE_LOG(LogTemp, Warning, TEXT("QUI!!!!!!!!!!!!!!!!!!!!"));
}

void AGameField::FieldBuilder() {
	// Spawns the 25x25 tiles
	for (int32 IndexX = 0 ; IndexX < Size; IndexX++) {
		for (int32 IndexY = 0; IndexY < Size; IndexY++) {
			FVector Location = AGameField::GetRelativeLocationByXYPosition(IndexX, IndexY);
			ATile* Obj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
			const float TileScale = TileSize / 100.f;
			const float Zscaling = 0.2f;
			Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
			Obj->SetGridPosition(IndexX, IndexY);
			TileArray.Add(Obj);
			TileMap.Add(FVector2D(IndexX, IndexY), Obj);
		}
	}
	int32 ObstacleNumber = int32((Size * Size) * ObstaclePercentage);
	for (int32 Index = 0; Index < ObstacleNumber; Index++) {
		/* GETTING RANDOM X AND Y COORDINATES */
		int32 IndexX = rand() % Size;
		int32 IndexY = rand() % Size;
		
		/* CHECK IF VALID*/
		ATile* tile = *TileMap.Find(FVector2D(IndexX, IndexY));
		if (tile->TileStatus == ETileStatus::OCCUPIED) {
			Index--;
			continue; // Salta il resto del loop e riprova con nuove coordinate
		}
		tile->TileStatus = ETileStatus::OCCUPIED;
		if(IsGameFieldValid()){
			/* SPAWING OBSTACLE ACTOR*/
			FVector Location = AGameField::GetRelativeLocationByXYPosition(IndexX, IndexY);
			AObstacle* Obj = GetWorld()->SpawnActor<AObstacle>(ObstacleClass, Location, FRotator::ZeroRotator);
			const float TileScale = TileSize / 100.f;
			const float Zscaling = 0.2f;
			Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("X: %d , Y: %d , POS INVALIDA"), IndexX, IndexY);
			tile->TileStatus = ETileStatus::EMPTY;
			Index--;
		}
	}
}

FString AGameField::ConvertCoord(FVector2D Coord)
{
	// Controllo che X sia nel range corretto
	if (Coord.X < 0 || Coord.X >= 25)
	{
		return TEXT("Invalid");
	}

	// Convertire il numero in lettera
	TCHAR Letter = 'A' + static_cast<int32>(Coord.X);

	// Creare la stringa nel formato "LETTERA NUMERO"
	FString Result = FString::Printf(TEXT("%c%d"), Letter, static_cast<int32>(Coord.Y+1));

	return Result;
}


bool AGameField::IsGameFieldValid() {
	if (FreeTiles() == AccessibleTiles()) {
		return true;
	}
	else {
		return false;
	}
}

int32 AGameField::FreeTiles() {
	int32 Count = 0;
	for (int32 IX = 0; IX < Size; IX++) {
		for (int32 IY = 0; IY < Size; IY++) {
			ATile* tile = *TileMap.Find(FVector2D(IX, IY));
			if (tile->TileStatus == ETileStatus::EMPTY) {
				Count++;
			}
		}
	}
	return Count;
}

int32 AGameField::AccessibleTiles() {
	/*Get first free tile*/
	ATile* obj = nullptr;
	for (int32 i = 0; i < TileArray.Num(); i++) {
		if ((TileArray[i]->TileStatus == ETileStatus::EMPTY)) {
			obj = TileArray[i];
			break;
		}
	}
	/*------------------*/
	int32 Count = 0;
	TArray<FVector2D> Counted;
	//Counted.Add(obj->GetGridPosition());
	ExploreTile(*obj, Count, Counted);
	return Count;
}

void AGameField::ExploreTile(ATile& Target, int32& c, TArray<FVector2D>& ca) {
	FVector2D pos = Target.GetGridPosition();
	int32 x = int(pos.X);
	int32 y = int(pos.Y);

	ca.Add(FVector2D(x, y));
	c++;  //Incremento siccome non la ho già contata

	for (int i = -1; i <= 1; i++) { // Chiamo ricorsivamente sulle celle valide vicine
		for (int j = -1; j <= 1; j++) {
			if ((i != 0 && j == 0) || (j != 0 && i == 0)) { // SOLO MOVIMENTI DIRETTI NO OBLIQUI
				if (x + i >= 0 && x + i < Size && y + j >= 0 && y + j < Size) {
					ATile** TilePtr = TileMap.Find(FVector2D(x + i, y + j));
					if (TilePtr && *TilePtr && (*TilePtr)->TileStatus == ETileStatus::EMPTY) {
						if (!ca.Contains(FVector2D(x + i, y + j))) {
							ExploreTile(**TilePtr, c, ca);
						}
					}
				}
			}
		}
	}
}

TArray<ATile*> AGameField::ReachableTiles(FVector2D Pos, int32 Range, int32 Start)
{
	TArray<ATile*> Tiles;
	if (Start <= Range) {
		ATile** Tile = TileMap.Find(Pos);
		if (((*Tile)->TileStatus == ETileStatus::EMPTY) or /*((*Tile)->TileStatus == ETileStatus::GREEN) or */(Start == 0)) { // Voglio illuminare quella sotto anche se (ovviamente) è occupata
			int32 x = Pos.X, y = Pos.Y;
			Tiles.AddUnique(*Tile);
			for (int i = -1; i <= 1; i++) { // Chiamo ricorsivamente sulle celle valide vicine
				for (int j = -1; j <= 1; j++) {
					if ((i != 0 && j == 0) || (j != 0 && i == 0)) { // SOLO MOVIMENTI DIRETTI NO OBLIQUI
						if (x + i >= 0 && x + i < Size && y + j >= 0 && y + j < Size) {
							//Tiles.Append(ReachableTiles(FVector2D(x + i, y + j), Range, Start + 1));
							TArray<ATile*> AT = ReachableTiles(FVector2D(x + i, y + j), Range, Start + 1);
							for (ATile* TT : AT) {
								Tiles.AddUnique(TT);
							}
							//ReachableTiles(FVector2D(x + i, y + j), Range, Start + 1);
						}
					}
				}
			}
		}
	}
	return Tiles;
}

void AGameField::ShowReachableTiles(FVector2D Pos, int32 Range, int32 Start)
{
	TArray<ATile*> Tiles = ReachableTiles(Pos, Range, Start);
	for (ATile* Tile : Tiles) {
		Tile->ChangeColor(FLinearColor::Green);
		Tile->TileColor = ETileColor::GREEN;
	}
}

TArray<ATile*> AGameField::AttackableTiles(FVector2D Pos, int32 Range, int32 Start)
{
	TArray<ATile*> Tiles;
	if (Start <= Range) {
		ATile** Tile = TileMap.Find(Pos);
		int32 x = Pos.X, y = Pos.Y;
		if ((*Tile)->TileStatus == ETileStatus::OCCUPIED) {
			FVector2D Pos = (*Tile)->GetGridPosition();
			for (int32 Index = 0; Index < UnitsArray.Num(); Index++) {
				AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
				if (UnitsArray[Index]->GridPosition == Pos and UnitsArray[Index]->Owner != GameMode->CurrentPlayer) {
					Tiles.AddUnique((*Tile));
				}
			}
		}
		for (int i = -1; i <= 1; i++) { // Chiamo ricorsivamente sulle celle valide vicine
			for (int j = -1; j <= 1; j++) {
				if ((i != 0 && j == 0) || (j != 0 && i == 0)) { // SOLO MOVIMENTI DIRETTI NO OBLIQUI
					if (x + i >= 0 && x + i < Size && y + j >= 0 && y + j < Size) {
						//Tiles.Append(AttackableTiles(FVector2D(x + i, y + j), Range, Start + 1));
						TArray<ATile*> AT = AttackableTiles(FVector2D(x + i, y + j), Range, Start + 1);
						for (ATile* TT : AT) {
							Tiles.AddUnique(TT);
						}
						//AttackableTiles(FVector2D(x + i, y + j), Range, Start + 1);
					}
				}
			}
		}
	}
	return Tiles;
}

void AGameField::ShowAttackableTiles(FVector2D Pos, int32 Range, int32 Start) {
	TArray<ATile*> Tiles = AttackableTiles(Pos, Range, Start);
	for (ATile* Tile : Tiles) {
		Tile->ChangeColor(FLinearColor::Red);
		Tile->TileColor = ETileColor::RED;
	}
}

void AGameField::SetAllTilesWhite()
{
	for (int32 IX = 0; IX < Size; IX++) {
		for (int32 IY = 0; IY < Size; IY++) {
			ATile* tile = *TileMap.Find(FVector2D(IX, IY));
			if (tile->TileColor == ETileColor::GREEN or tile->TileColor == ETileColor::RED) {
				tile->TileColor = ETileColor::WHITE;
				tile->ChangeColor(FLinearColor::White);
			}
		}
	}
}

void AGameField::MoveUnitTo(AGameUnit* Unit, FVector2D Dest)
{
	if (!Unit) return;
	FString Player = Unit->Owner == 0 ? TEXT("HP: ") : TEXT("CP: ");
	FString UT = Unit->UnitType == EUnits::SNIPER ? TEXT("S ") : TEXT("B ");
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->AddLogMessage(Player + UT + ConvertCoord(Unit->GridPosition) + TEXT(" -> ") + ConvertCoord(Dest));
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	GameMode->CurrentGameState = EGameState::MovingUnit;

	(*TileMap.Find(Unit->GridPosition))->TileStatus = ETileStatus::EMPTY;
	TArray<FVector2D> BestPath = FindPath(Unit->GridPosition, Dest);

	for (const FVector2D& Vector : BestPath)
	{
		UE_LOG(LogTemp, Log, TEXT("TILE COORD: X = %f, Y = %f"), Vector.X, Vector.Y);
	}
	// Aspetta che MoveAlongPath finisca prima di aggiornare lo stato dell'unità
	if (GameMode->CurrentPlayer == 1) {
		//Set the reachable tiles to green
		ShowReachableTiles(Unit->GridPosition, Unit->MovementRange, 0);
		FTimerHandle TimerHandle;
		//Wait before putting them back to white
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, Unit, BestPath, Dest, GameMode]()
			{
				//Set the tile back to white
				SetAllTilesWhite();
				//Set the path tiles to green
				ShowPath(BestPath);
				MoveAlongPath(Unit, BestPath, 0, [this, Unit, Dest, GameMode]()
					{
						(*TileMap.Find(Dest))->TileStatus = ETileStatus::OCCUPIED;
						Unit->GridPosition = Dest;
						Unit->bCanMove = false;
						UE_LOG(LogTemp, Warning, TEXT("UNIT MOVEMENT COMPLETED!"));
						GameMode->Players[GameMode->CurrentPlayer]->OnTurn();
					});
			}), 2.0f, false);
	}
	else {
		//Set the path tiles to green
		ShowPath(BestPath);
		MoveAlongPath(Unit, BestPath, 0, [this, Unit, Dest, GameMode]()
			{
				(*TileMap.Find(Dest))->TileStatus = ETileStatus::OCCUPIED;
				Unit->GridPosition = Dest;
				Unit->bCanMove = false;
				UE_LOG(LogTemp, Warning, TEXT("UNIT MOVEMENT COMPLETED!"));
				GameMode->Players[GameMode->CurrentPlayer]->OnTurn();
			});
	}
	return;
}

void AGameField::MoveAlongPath(AGameUnit* Unit, TArray<FVector2D> Path, int32 Step, TFunction<void()> OnComplete)
{
	if (Step >= Path.Num())
	{
		if (OnComplete)
		{
			AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
			GameMode->CurrentGameState = EGameState::WaitingAction;
			OnComplete(); // Chiama la funzione di callback alla fine del percorso
		}
		return;
	}

	FVector2D NextTile = Path[Step];

	// Imposta il timer per il prossimo movimento
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, Unit, Path, Step, OnComplete]()
		{
			UE_LOG(LogTemp, Warning, TEXT("FOLLOWING PATH %d"), Step);
			Unit->SetActorLocation(GetRelativeLocationByXYPosition(Path[Step].X, Path[Step].Y) + FVector(0, 0, 2));
			ATile** Tile = TileMap.Find(Path[Step]);
			(*Tile)->ChangeColor(FLinearColor::White);
			(*Tile)->TileColor = ETileColor::WHITE;
			// Chiamata ricorsiva per il passo successivo
			MoveAlongPath(Unit, Path, Step + 1, OnComplete);
		}), 0.3f, false);
}


void AGameField::Attack(AGameUnit* Attacker, AGameUnit* Attacked)
{
	FString Player = Attacker->Owner == 0 ? TEXT("HP: ") : TEXT("CP: ");
	FString UT = Attacker->UnitType == EUnits::SNIPER ? TEXT("S ") : TEXT("B ");
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
	int32 CounterDamage = 0;
	Attacked->HealtPoints -= Damage;
	if (Attacker->UnitType == EUnits::SNIPER) {
		switch (Attacked->UnitType) {
			case EUnits::SNIPER:
				CounterDamage = FMath::RandRange(1, 3);
				Attacker->HealtPoints -= CounterDamage;
			break;
			case EUnits::BRAWLER:
				UE_LOG(LogTemp, Warning, TEXT("BROOOOOOOOOOOOOO DIST %f"), FVector2D::Distance(Attacker->GridPosition, Attacked->GridPosition));
				if (FVector2D::Distance(Attacker->GridPosition, Attacked->GridPosition) == 1.0f) {
					CounterDamage =  FMath::RandRange(1, 3);
					Attacker->HealtPoints -= CounterDamage;
				}
			break;
		}
	}
	GameInstance->AddLogMessage(Player + UT + ConvertCoord(Attacked->GridPosition) + FString::Printf(TEXT(" %d C %d"), Damage, CounterDamage));
	UE_LOG(LogTemp, Warning, TEXT("Attacked HP %d"), Attacked->HealtPoints);
	if (Attacked->HealtPoints <= 0) {
		Attacked->HealtPoints = 0;
		Attacked->Destroy();
		ATile** Tile = TileMap.Find(Attacked->GridPosition);
		(*Tile)->TileStatus = ETileStatus::EMPTY;
		Attacked->GridPosition = FVector2D(-1000, -1000);
	}
	if (Attacker->HealtPoints <= 0) {
		Attacker->HealtPoints = 0;
		Attacker->Destroy();
		ATile** Tile = TileMap.Find(Attacker->GridPosition);
		(*Tile)->TileStatus = ETileStatus::EMPTY;
		Attacker->GridPosition = FVector2D(-1000, -1000);
	}
	Attacker->bCanAttack = false;
	GameInstance->UpdateStats();
	CheckWin();
}



TArray<FVector2D> AGameField::FindPath(FVector2D Start, FVector2D Goal)
{
	TArray<FVector2D> Path;

	// Coda con priorità emulata tramite TArray
	TArray<FNode> OpenSet;
	OpenSet.Add(FNode(Start, 0.0f));  // Aggiungi il nodo iniziale con punteggio 0

	// Mappe per il tracciamento del percorso
	TMap<FVector2D, FVector2D> CameFrom;
	TMap<FVector2D, float> GScore;
	TMap<FVector2D, float> FScore;

	GScore.Add(Start, 0.0f);
	FScore.Add(Start, FVector2D::Distance(Start, Goal));

	while (OpenSet.Num() > 0)
	{
		// Ordina la coda in base al FScore
		Algo::Sort(OpenSet, [](const FNode& A, const FNode& B) {
			return A.FScore < B.FScore; // Min-Heap
			});

		FNode CurrentNode = OpenSet[0];
		OpenSet.RemoveAt(0);  // Rimuove il primo nodo (quello con il punteggio più basso)

		if (CurrentNode.Position == Goal) // Se raggiungiamo il Goal, ricostruiamo il percorso
		{
			FVector2D Current = Goal;
			while (CameFrom.Contains(Current))
			{
				Path.Insert(Current, 0);  // Aggiungi alla lista del percorso
				Current = CameFrom[Current];
			}
			Path.Insert(Start, 0);  // Aggiungi il punto di partenza
			return Path;
		}

		// Direzioni possibili (Su, Giù, Sinistra, Destra)
		TArray<FVector2D> Neighbors = {
			FVector2D(CurrentNode.Position.X + 1, CurrentNode.Position.Y),
			FVector2D(CurrentNode.Position.X - 1, CurrentNode.Position.Y),
			FVector2D(CurrentNode.Position.X, CurrentNode.Position.Y + 1),
			FVector2D(CurrentNode.Position.X, CurrentNode.Position.Y - 1),
		};

		for (const FVector2D& Neighbor : Neighbors)
		{
			if (!TileMap.Contains(Neighbor)) continue; // Ignora se fuori dalla mappa
			if (TileMap[Neighbor]->TileStatus == ETileStatus::OCCUPIED) continue; // Ignora se è un ostacolo

			float TentativeGScore = GScore[CurrentNode.Position] + 1.0f; // Costo base di movimento

			if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
			{
				CameFrom.Add(Neighbor, CurrentNode.Position);
				GScore.Add(Neighbor, TentativeGScore);
				float H = FVector2D::Distance(Neighbor, Goal);
				float F = TentativeGScore + H;
				FScore.Add(Neighbor, F);
				OpenSet.Add(FNode(Neighbor, F)); // Aggiungi il nodo alla coda
			}
		}
	}
	return Path; // Se nessun percorso trovato, restituisce un array vuoto
}

void AGameField::ShowPath(TArray<FVector2D> Path)
{
	for (FVector2D Coord : Path) {
		ATile** Tile = TileMap.Find(Coord);
		(*Tile)->ChangeColor(FLinearColor::Green);
		(*Tile)->TileColor = ETileColor::GREEN;
	}
}

void AGameField::CheckWin()
{
	int32 DeadHuman = 0, DeadComputer = 0;
	for (AGameUnit* Unit : UnitsArray) {
		if (Unit->HealtPoints <= 0 and Unit->Owner == 0) {
			DeadHuman++;
		}
		if (Unit->HealtPoints <= 0 and Unit->Owner == 1) {
			DeadComputer++;
		}
	}
	if (DeadHuman >= 2 and DeadComputer >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Pareggio"))
		GameInstance->SetMessagge(TEXT("Pareggio"));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		//ResetField();
		return;
	}
	if (DeadHuman >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Vince Computer"))
		GameInstance->SetMessagge(TEXT("Vince Computer"));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		//ResetField();
		return;
	}
	if (DeadComputer >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Vince Human"))
		GameInstance->SetMessagge(TEXT("Vince Human "));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		//ResetField();
		return;
	}
	return;
}

void AGameField::ResetField()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GetWorld()) return;
	if (GameMode->CurrentGameState == EGameState::ComputerPlacing or GameMode->CurrentGameState == EGameState::MovingUnit) return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ObstacleClass, FoundActors);

	for (AActor* Actor : FoundActors) {
		if (Actor) {
			Actor->Destroy();
		}
	}

	for (ATile* Tile : TileArray) {
		if (Tile) {
			Tile->Destroy();
		}
	}
	for (AGameUnit* Unit : UnitsArray) {
		if (Unit->GridPosition != FVector2D(-1000, -1000)) {
			Unit->Destroy();
		}
	}
	//TileArray = TArray<ATile*>();
	TileArray.Empty();
	TileMap = TMap<FVector2D, ATile*>();
	//UnitsArray = TArray<AGameUnit*>();
	UnitsArray.Empty();
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->RemoveWidgets();
	GameMode->StartGame();
}


FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const 
{
	return TileSize * NextCellPositionMultiplier * FVector(InX, InY, 0);
}

// Called every frame
void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

