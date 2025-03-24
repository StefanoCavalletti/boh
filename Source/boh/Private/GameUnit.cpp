// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUnit.h"

// Sets default values
AGameUnit::AGameUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bCanMove = true;
	bCanAttack = true;
}

void AGameUnit::SetUpUnit(EUnits UT, int32 Player, FVector2D Pos)
{
	UnitType = UT;
	Owner = Player;
	GridPosition = Pos;
	switch (UT) {
	case EUnits::BRAWLER:
		MovementRange = 6;
		AttackRange = 1;
		MinDamage = 1;
		MaxDamage = 6;
		HealtPoints = 40;
		break;

	case EUnits::SNIPER:
		MovementRange = 3;
		AttackRange = 10;
		MinDamage = 4;
		MaxDamage = 8;
		HealtPoints = 20;
		break;
	}
}

// Called when the game starts or when spawned
void AGameUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGameUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGameUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

