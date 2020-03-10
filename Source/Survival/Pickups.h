// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickups.generated.h"

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	EWater UMETA(DisplayName = "Water"),
	EFood UMETA(DisplayName = "Food"),
	EHealth UMETA(DisplayName = "Health"),
	EKey UMETA(DisplayName = "Key"),
	EGreneade UMETA(DisplayName = "Grenade")
};

UCLASS()
class SURVIVAL_API APickups : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickups();

protected:
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
		class UTexture2D* Icon;

	UPROPERTY(EditAnywhere)
		float IncreaseAmount;

	UPROPERTY(EditAnywhere, Category = "Enums")
		EPickupType PickupType;

	UPROPERTY(ReplicatedUsing = OnRep_PickedUp)
		bool ObjectPickedUp;

	UFUNCTION()
		void OnRep_PickedUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void UseItem(class ASurvivalCharacter* Player);
	void InInventory(bool In);
	UFUNCTION(BlueprintCallable)
	class UTexture2D* GetItemIcon();
};
