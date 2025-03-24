// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameUnit.generated.h"

UENUM(BlueprintType)
enum class EUnits : uint8
{
	BRAWLER     UMETA(DisplayName = "BRAWLER"),
	SNIPER      UMETA(DisplayName = "SNIPER")
};

UCLASS()
class BOH_API AGameUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameUnit();
	int32 MovementRange;
	int32 AttackRange;
	int32 MinDamage;
	int32 MaxDamage;
	int32 HealtPoints;
	EUnits UnitType;
	int32 Owner;
	FVector2D GridPosition;
	bool bCanMove;
	bool bCanAttack;

	void SetUpUnit(EUnits UT, int32 Player, FVector2D Pos);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
