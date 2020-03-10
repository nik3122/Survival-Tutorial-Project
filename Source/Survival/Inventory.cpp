// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "Pickups.h"
#include "SurvivalCharacter.h"
#include "StorageContainer.h"

#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

// Sets default values for this component's properties
UInventory::UInventory()
{
	bReplicates = true;

	InventorySize = 16;
}


// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UInventory::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventory, Items);
}

bool UInventory::AddItem(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Items.Add(Item);
		Item->InInventory(true);

		for (APickups* Pickup : Items)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item: %s"), *Pickup->GetName());
		}
		UE_LOG(LogTemp, Warning, TEXT("END OF ITEMS"));
	}
	return false;
}

void UInventory::DropAllInventory()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (APickups* Pickup : Items)
		{
			DropItem(Pickup);
		}
		Items.Empty();
	}
}

void UInventory::TransferItem(AActor* ToActor, APickups* Item)
{
	Server_TransferItem(ToActor, Item);
}

bool UInventory::Server_TransferItem_Validate(AActor* ToActor, APickups* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_TransferItem_Implementation(AActor* ToActor, APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (ToActor)
		{
			if (AStorageContainer* Container = Cast<AStorageContainer>(ToActor))
			{
				if (UInventory* CInventory = Container->GetInventoryComponent())
				{
					CInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}
			else if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(ToActor))
			{
				if (UInventory* CInventory = Character->GetInventoryComponent())
				{
					CInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}
		}
	}
}

bool UInventory::Server_ReceiveItem_Validate(APickups* Item)
{
	if (Item) return true;
	else return false;
}

void UInventory::Server_ReceiveItem_Implementation(APickups* Item)
{
	if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(GetOwner()))
	{
		if (AStorageContainer* Container = Character->GetOpenedContainer())
		{
			UE_LOG(LogTemp, Warning, TEXT("TRANSFERRING ITEM TO CHARACTER"));
			Container->GetInventoryComponent()->TransferItem(Character, Item);
		}
	}
}

bool UInventory::CheckIfClientHasItem(APickups* Item)
{
	for (APickups* Pickup : Items)
	{
		if (Pickup == Item)
		{
			return true;
		}
	}
	return false;
}

bool UInventory::RemoveItemFromInventory(APickups* Item)
{
	int32 Counter = 0;
	for (APickups* Pickup : Items)
	{
		if (Pickup == Item)
		{
			Items.RemoveAt(Counter);
			return true;
		}
		++Counter;
	}
	return false;
}

bool UInventory::Server_DropItem_Validate(APickups* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_DropItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FVector Location = GetOwner()->GetActorLocation();
		Location.X += FMath::RandRange(-50.0f, 100.0f);
		Location.Y += FMath::RandRange(-50.0f, 100.0f);
		FVector EndRay = Location;
		EndRay.Z -= 500.0f;

		FHitResult HitResult;
		FCollisionObjectQueryParams ObjQuery;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());

		GetWorld()->LineTraceSingleByObjectType
		(
			OUT HitResult,
			Location,
			EndRay,
			ObjQuery,
			CollisionParams
		);

		if (HitResult.ImpactPoint != FVector::ZeroVector)
		{
			Location = HitResult.ImpactPoint;
		}

		Item->SetActorLocation(Location);
		Item->InInventory(false);

		RemoveItemFromInventory(Item);
	}
}

void UInventory::DropItem(APickups* Item)
{
	Server_DropItem(Item);
}

void UInventory::RemoveItem(APickups* Item)
{
	RemoveItemFromInventory(Item);
}

bool UInventory::Server_UseItem_Validate(APickups* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_UseItem_Implementation(APickups* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(GetOwner()))
		{
			Item->UseItem(Player);
			RemoveItemFromInventory(Item);
		}
	}
}

void UInventory::UseItem(APickups* Item)
{
	Server_UseItem(Item);
}

TArray<class APickups*> UInventory::GetInventoryItems()
{
	return Items;
}

int32 UInventory::GetCurrentInventoryCount()
{
	return Items.Num() - 1;
}

int32 UInventory::GetInventorySize()
{
	return InventorySize - 1;
}
