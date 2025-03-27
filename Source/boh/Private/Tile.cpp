// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	TileStatus = ETileStatus::EMPTY;
	TileColor = ETileColor::WHITE;

}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

void ATile::ChangeColor(FLinearColor NewColor)
{
	if (!StaticMeshComponent) return;

	// create dynamic material
	if (!DynamicMaterial)
	{
		DynamicMaterial = StaticMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (DynamicMaterial)
	{
		// Set color of dynamic material
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
	}
}

// Called every frame
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

