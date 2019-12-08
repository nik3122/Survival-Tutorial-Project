// Fill out your copyright notice in the Description page of Project Settings.


#include "StorageContainer.h"
#include "Inventory.h"

#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
// Sets default values
AStorageContainer::AStorageContainer()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComponent");
	Inventory = CreateDefaultSubobject<UInventory>("InventryComponent");

	bReplicates = true;
	IsOpen = false;
}

// Called when the game starts or when spawned
void AStorageContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStorageContainer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStorageContainer, IsOpen);
}

UInventory* AStorageContainer::GetInventoryComponent()
{
	return Inventory;
}

void AStorageContainer::OnRep_ChestOpened()
{
	if (IsOpen)
	{
		MeshComp->PlayAnimation(OpenAnimation, false);
	}
	else
	{
		MeshComp->PlayAnimation(CloseAnimation, false);
	}
}

void AStorageContainer::OpenedChest(bool Opened)
{
	Server_OpenedChest(Opened);
}

bool AStorageContainer::IsChestOpen()
{
	return IsOpen;
}

bool AStorageContainer::Server_OpenedChest_Validate(bool Opened)
{
	return true;
}

void AStorageContainer::Server_OpenedChest_Implementation(bool Opened)
{
	if (Role == ROLE_Authority)
	{
		IsOpen = Opened;
	}
}
