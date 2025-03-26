// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "Obstacle.h"
#include "GameUnit.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

struct FNode
{
	FVector2D Position;
	float FScore;

	FNode() : Position(FVector2D::ZeroVector), FScore(0.0f) {} // Costruttore di Default
	FNode(FVector2D InPosition, float InFScore) : Position(InPosition), FScore(InFScore) {}

	bool operator<(const FNode& Other) const
	{
		return FScore > Other.FScore; // Min-Heap: priorità al nodo con FScore più basso
	}
};

UCLASS()
class BOH_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameField();
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AObstacle> ObstacleClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Size;
	// tile padding percentage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CellPadding;
	// tile size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NextCellPositionMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstaclePercentage;
	// keeps track of tiles
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;
	//given a position returns a tile
	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	TArray<AGameUnit*> UnitsArray;

	bool IsGameFieldValid();
	int32 FreeTiles();
	int32 AccessibleTiles();
	void ExploreTile(ATile& Target, int32& c, TArray<FVector2D>& ca);
	TArray<ATile*> ReachableTiles(FVector2D Pos, int32 Range, int32 Start);
	void ShowReachableTiles(FVector2D Pos, int32 Range, int32 Start);
	TArray<ATile*> AttackableTiles(FVector2D Pos, int32 Range, int32 Start);
	void ShowAttackableTiles(FVector2D Pos, int32 Range, int32 Start);
	void SetAllTilesWhite();
	void MoveUnitTo(AGameUnit* Unit, FVector2D Dest);
	void Attack(AGameUnit* Attacker, AGameUnit* Attacked);
	TArray<FVector2D> FindPath(FVector2D Start, FVector2D Goal);
	void ShowPath(TArray<FVector2D> Path);
	void CheckWin();
	UFUNCTION()
	void ResetField();
	virtual void FieldBuilder();
	FString ConvertCoord(FVector2D Coord);
	void MoveAlongPath(AGameUnit* Unit, TArray<FVector2D> Path, int32 Step, TFunction<void()> OnComplete);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
