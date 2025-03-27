// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "MyGameModeBase.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Math/UnrealMathUtility.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Sort.h" 


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
			continue; // Skip loop and try new coords
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
	// From number to letter
	TCHAR Letter = 'A' + static_cast<int32>(Coord.X);

	// Create string 
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
	//Get the number of empty tiles
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
	ExploreTile(*obj, Count, Counted);
	return Count;
}

void AGameField::ExploreTile(ATile& Target, int32& c, TArray<FVector2D>& ca) {
	FVector2D pos = Target.GetGridPosition();
	int32 x = int(pos.X);
	int32 y = int(pos.Y);

	ca.Add(FVector2D(x, y));
	c++;  //Increment since it's a empty tile

	for (int i = -1; i <= 1; i++) { // Call on the near cells
		for (int j = -1; j <= 1; j++) {
			if ((i != 0 && j == 0) || (j != 0 && i == 0)) { // Only up, down, left, right
				if (x + i >= 0 && x + i < Size && y + j >= 0 && y + j < Size) {
					ATile** TilePtr = TileMap.Find(FVector2D(x + i, y + j));
					if (TilePtr && *TilePtr && (*TilePtr)->TileStatus == ETileStatus::EMPTY) {
						if (!ca.Contains(FVector2D(x + i, y + j))) { // if i did not visit yet
							ExploreTile(**TilePtr, c, ca);
						}
					}
				}
			}
		}
	}
}

TArray<ATile*> AGameField::ReachableTiles(FVector2D Pos, int32 Range)
{
	TArray<ATile*> Tiles;
	TSet<FVector2D> Visited;
	TQueue<TPair<FVector2D, int32>> Queue; 

	Queue.Enqueue(TPair<FVector2D, int32>(Pos, 0));

	while (!Queue.IsEmpty())
	{
		TPair<FVector2D, int32> Current;
		Queue.Dequeue(Current);

		FVector2D CurrentPos = Current.Key;
		int32 CurrentStep = Current.Value;

		// Skip if out of range
		if (CurrentStep > Range)
			continue;

		// Search tile on the map
		ATile** Tile = TileMap.Find(CurrentPos);
		if (!Tile) continue;

		// Avoid multiple visit on the same node
		if (Visited.Contains(CurrentPos))
			continue;

		Visited.Add(CurrentPos);
		Tiles.AddUnique(*Tile); // add the tile to the list

		// Up, down, left, right
		TArray<FVector2D> Directions = {
			FVector2D(1, 0),
			FVector2D(-1, 0),
			FVector2D(0, 1),
			FVector2D(0, -1),
		};

		for (const FVector2D& Dir : Directions)
		{
			FVector2D NextPos = CurrentPos + Dir;

			// If not visited yet
			if (!Visited.Contains(NextPos) && TileMap.Contains(NextPos))
			{
				// If the tile is empty or is the initial one
				ATile** NextTile = TileMap.Find(NextPos);
				if (NextTile && ((*NextTile)->TileStatus == ETileStatus::EMPTY || CurrentStep == 0))
				{
					Queue.Enqueue(TPair<FVector2D, int32>(NextPos, CurrentStep + 1));
				}
			}
		}
	}

	return Tiles;
}


void AGameField::ShowReachableTiles(FVector2D Pos, int32 Range)
{
	// Change reachable tiles color to green
	TArray<ATile*> Tiles = ReachableTiles(Pos, Range);
	for (ATile* Tile : Tiles) {
		Tile->ChangeColor(FLinearColor::Green);
		Tile->TileColor = ETileColor::GREEN;
	}
}

TArray<ATile*> AGameField::AttackableTiles(FVector2D Pos, int32 Range)
{
	TArray<ATile*> Tiles;
	TSet<FVector2D> Visited; 
	TQueue<TPair<FVector2D, int32>> Queue; 

	Queue.Enqueue(TPair<FVector2D, int32>(Pos, 0));

	while (!Queue.IsEmpty())
	{
		TPair<FVector2D, int32> Current;
		Queue.Dequeue(Current);

		FVector2D CurrentPos = Current.Key;
		int32 CurrentStep = Current.Value;

		// Skip if out of range
		if (CurrentStep > Range)
			continue;

		// Search tile on the map
		ATile** Tile = TileMap.Find(CurrentPos);
		if (!Tile) continue;

		// Avoid multiple visit on the same node
		if (Visited.Contains(CurrentPos))
			continue;

		Visited.Add(CurrentPos);

		// If there's an enemy on the tile add the tile to the list
		if ((*Tile)->TileStatus == ETileStatus::OCCUPIED)
		{
			FVector2D TileGridPos = (*Tile)->GetGridPosition();
			AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());

			for (AGameUnit* Unit : UnitsArray)
			{
				if (Unit->GridPosition == TileGridPos && Unit->Owner != GameMode->CurrentPlayer)
				{
					Tiles.AddUnique(*Tile);
					break; // No need to check the other units (Max 1 unit for tile)
				}
			}
		}

		// Up, down, left, right
		TArray<FVector2D> Directions = {
			FVector2D(1, 0),
			FVector2D(-1, 0),
			FVector2D(0, 1),
			FVector2D(0, -1),
		};

		for (const FVector2D& Dir : Directions)
		{
			FVector2D NextPos = CurrentPos + Dir;

			if (!Visited.Contains(NextPos) && TileMap.Contains(NextPos)) // Avoid loops
			{
				Queue.Enqueue(TPair<FVector2D, int32>(NextPos, CurrentStep + 1));
			}
		}
	}

	return Tiles;
}



void AGameField::ShowAttackableTiles(FVector2D Pos, int32 Range) {
	TArray<ATile*> Tiles = AttackableTiles(Pos, Range);
	//Change attackable tiles color to red
	for (ATile* Tile : Tiles) {
		Tile->ChangeColor(FLinearColor::Red);
		Tile->TileColor = ETileColor::RED;
	}
}

void AGameField::SetAllTilesWhite()
{
	//Set all the tiles color to white
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
	if (GameMode->CurrentPlayer == 1) {
		//Set the reachable tiles to green
		ShowReachableTiles(Unit->GridPosition, Unit->MovementRange);
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
{   //If i am at the end of the path terminate
	if (Step >= Path.Num())
	{
		if (OnComplete)
		{
			AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
			GameMode->CurrentGameState = EGameState::WaitingAction;
			OnComplete(); // Callback function at the end of the path
		}
		return;
	}

	FVector2D NextTile = Path[Step];

	// Set the timer for next step
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, Unit, Path, Step, OnComplete]()
		{
			UE_LOG(LogTemp, Warning, TEXT("FOLLOWING PATH %d"), Step);
			Unit->SetActorLocation(GetRelativeLocationByXYPosition(Path[Step].X, Path[Step].Y) + FVector(0, 0, 2));
			ATile** Tile = TileMap.Find(Path[Step]);
			(*Tile)->ChangeColor(FLinearColor::White);
			(*Tile)->TileColor = ETileColor::WHITE;
			// next step 
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
	// Counter attack logic
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
	// Add attack log to log history
	GameInstance->AddLogMessage(Player + UT + ConvertCoord(Attacked->GridPosition) + FString::Printf(TEXT(" %d C %d"), Damage, CounterDamage));
	UE_LOG(LogTemp, Warning, TEXT("Attacked HP %d"), Attacked->HealtPoints);
	// Check if a unit died
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
	// Update the unit stats on screen
	GameInstance->UpdateStats();
	// Check if the game is over
	CheckWin();
}



TArray<FVector2D> AGameField::FindPath(FVector2D Start, FVector2D Goal)
{
	TArray<FVector2D> Path;

	TArray<FNode> OpenSet;
	OpenSet.Add(FNode(Start, 0.0f));  // Starting node score 0

	// Maps to keep track of path
	TMap<FVector2D, FVector2D> CameFrom;
	TMap<FVector2D, float> GScore;
	TMap<FVector2D, float> FScore;

	GScore.Add(Start, 0.0f);
	FScore.Add(Start, FVector2D::Distance(Start, Goal));

	while (OpenSet.Num() > 0)
	{
		// Sort by FScore
		Algo::Sort(OpenSet, [](const FNode& A, const FNode& B) {
			return A.FScore < B.FScore; // Min-Heap
			});

		FNode CurrentNode = OpenSet[0];
		OpenSet.RemoveAt(0);  // Remove first node (lowest score)

		if (CurrentNode.Position == Goal) // If goal reached
		{
			FVector2D Current = Goal;
			while (CameFrom.Contains(Current))
			{
				Path.Insert(Current, 0);  // Add to list 
				Current = CameFrom[Current];
			}
			Path.Insert(Start, 0);  // Add starting coords
			return Path;
		}

		// Up, down, right, left
		TArray<FVector2D> Neighbors = {
			FVector2D(CurrentNode.Position.X + 1, CurrentNode.Position.Y),
			FVector2D(CurrentNode.Position.X - 1, CurrentNode.Position.Y),
			FVector2D(CurrentNode.Position.X, CurrentNode.Position.Y + 1),
			FVector2D(CurrentNode.Position.X, CurrentNode.Position.Y - 1),
		};

		for (const FVector2D& Neighbor : Neighbors)
		{
			if (!TileMap.Contains(Neighbor)) continue; // Ignore invalid positions
			if (TileMap[Neighbor]->TileStatus == ETileStatus::OCCUPIED) continue; // If is occupied ignore

			float TentativeGScore = GScore[CurrentNode.Position] + 1.0f; //Base cost

			if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
			{
				CameFrom.Add(Neighbor, CurrentNode.Position);
				GScore.Add(Neighbor, TentativeGScore);
				float H = FVector2D::Distance(Neighbor, Goal);
				float F = TentativeGScore + H;
				FScore.Add(Neighbor, F);
				OpenSet.Add(FNode(Neighbor, F)); // Add node to queue
			}
		}
	}
	return Path; 
}

void AGameField::ShowPath(TArray<FVector2D> Path)
{
	// Change the path color to green
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
	// If all the units are dead it's a draw
	if (DeadHuman >= 2 and DeadComputer >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Pareggio"))
		GameInstance->SetMessagge(TEXT("Pareggio"));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		return;
	}
	// If human player units are dead computer wins
	if (DeadHuman >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Vince Computer"))
		GameInstance->SetMessagge(TEXT("Vince Computer"));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		return;
	}
	// If computer units are dead human wins
	if (DeadComputer >= 2) {
		UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		UE_LOG(LogTemp, Warning, TEXT("Vince Human"))
		GameInstance->SetMessagge(TEXT("Vince Human "));
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		GameMode->IsGameOver = true;
		return;
	}
	return;
}

void AGameField::ResetField()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GetWorld()) return;
	if (!GameMode->IsGameOver) {
		if (GameMode->CurrentGameState == EGameState::ComputerPlacing or GameMode->CurrentGameState == EGameState::MovingUnit) return;
		if (GameMode->CurrentPlayer != 0) return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ObstacleClass, FoundActors);
	// Remove all actors 
	for (AActor* Actor : FoundActors) {
		if (Actor) {
			Actor->Destroy();
		}
	}
	// Remove all tiles
	for (ATile* Tile : TileArray) {
		if (Tile) {
			Tile->Destroy();
		}
	}
	// Remove all units
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

