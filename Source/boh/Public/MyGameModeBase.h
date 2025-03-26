// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "GameField.h"
#include "PlayerInterface.h"
#include "MyGameModeBase.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	PlacingBrawler    UMETA(DisplayName = "PlacingBrawler"),
	PlacingSniper    UMETA(DisplayName = "PlacingSniper"),
	ComputerPlacing UMETA(DisplayName = "ComputerPlacing"),
	MovingUnit     UMETA(DisplayName = "MovingUnit"),
	WaitingSelection  UMETA(DisplayName = "WaitingSelection"),
	WaitingAction	UMETA(DisplayName = "WaitingAction")
};


class AActor;

UCLASS()
class BOH_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void StartGame();

	void PlaceUnits();

	void SetupAndStartTurns();

	void PassIfForced();

	UFUNCTION()
	void SafePass();

	// tracks if the game is over
	bool IsGameOver;

	// array of player interfaces
	int32 CurrentPlayer;

	TArray<IPlayerInterface*> Players;

	UPROPERTY(BlueprintReadWrite, Category = "Game")
	EGameState CurrentGameState;

	TArray<EUnits> PlayerToPlace;

	TArray<EUnits> AIToPlace;

	// TSubclassOf is a template class that provides UClass type safety.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UFUNCTION()
	void SetPlacingUnitTypeBrawler();

	UFUNCTION()
	void SetPlacingUnitTypeSniper();

	// field size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	// reference to a GameField object
	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> BlueSniperActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> BlueBrawlerActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> RedSniperActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> RedBrawlerActor;

	int32 GetNextPlayer(int32 Player);
	
	void NextPlayerPlace();

	void NextPlayerTurn();

	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY);

	UFUNCTION()
	void MyTurnSafePass();

	AMyGameModeBase();
};
