// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SURVIVAL_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventory();

protected:
	UPROPERTY(Replicated)
		TArray<class APickups*> Items;

	UPROPERTY(EditAnywhere)
		int32 InventorySize;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_DropItem(class APickups* Item);
	bool Server_DropItem_Validate(class APickups* Item);
	void Server_DropItem_Implementation(class APickups* Item);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_UseItem(class APickups* Item);
	bool Server_UseItem_Validate(class APickups* Item);
	void Server_UseItem_Implementation(class APickups* Item);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_TransferItem(class AActor* ToActor, class APickups* Item);
	bool Server_TransferItem_Validate(class AActor* ToActor, class APickups* Item);
	void Server_TransferItem_Implementation(class AActor* ToActor, class APickups* Item);

	bool CheckIfClientHasItem(class APickups* Item);
	bool RemoveItemFromInventory(class APickups* Item);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void Server_ReceiveItem(class APickups* Item);
	bool Server_ReceiveItem_Validate(class APickups* Item);
	void Server_ReceiveItem_Implementation(class APickups* Item);

public:
	bool AddItem(class APickups* Item);
	void DropAllInventory();

	UFUNCTION(BlueprintCallable)
		void TransferItem(class AActor* ToActor, class APickups* Item);

	UFUNCTION(BlueprintCallable)
		void DropItem(class APickups* Item);

	void RemoveItem(class APickups* Item);

	UFUNCTION(BlueprintCallable)
		void UseItem(class APickups* Item);

	UFUNCTION(BlueprintCallable)
		TArray<class APickups*> GetInventoryItems();

	UFUNCTION(BlueprintCallable)
		int32 GetCurrentInventoryCount();

	UFUNCTION(BlueprintCallable)
		int32 GetInventorySize();
};