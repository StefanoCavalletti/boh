// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM()
enum class ETileStatus : uint8
{
	EMPTY         UMETA(DisplayName = "Empty"),
	OCCUPIED      UMETA(DisplayName = "Occupied"),
};

UENUM()
enum class ETileColor : uint8
{
	WHITE		  UMETA(DisplayName = "White"),
	GREEN	      UMETA(DisplayName = "Green"),
	RED			  UMETA(DisplayName = "Red"),
};

UCLASS()
class BOH_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ETileStatus TileStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ETileColor TileColor;

	// set the (x, y) position
	void SetGridPosition(const double InX, const double InY);

	// get the (x, y) position
	FVector2D GetGridPosition();

	UFUNCTION(BlueprintCallable)
	void ChangeColor(FLinearColor NewColor);

	bool bIsInMoveRange;
	bool bIsInAttackRange;

private:
	UMaterialInstanceDynamic* DynamicMaterial;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
