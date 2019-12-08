// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageContainer.generated.h"

UCLASS()
class SURVIVAL_API AStorageContainer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStorageContainer();

protected:
	UPROPERTY(EditAnywhere)
		class USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
		class UInventory* Inventory;

	UPROPERTY(EditAnywhere)
		class UAnimationAsset* OpenAnimation;

	UPROPERTY(EditAnywhere)
		class UAnimationAsset* CloseAnimation;

	UPROPERTY(ReplicatedUsing = OnRep_ChestOpened)
		bool IsOpen;

	UFUNCTION()
		void OnRep_ChestOpened();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_OpenedChest(bool Opened);
	bool Server_OpenedChest_Validate(bool Opened);
	void Server_OpenedChest_Implementation(bool Opened);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	class UInventory* GetInventoryComponent();

	void OpenedChest(bool Opened);
	bool IsChestOpen();
};