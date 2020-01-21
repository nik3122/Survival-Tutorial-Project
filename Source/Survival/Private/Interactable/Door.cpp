// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Interactable/Door.h"
#include "SurvivalCharacter.h"
#include "Inventory.h"
#include "Public/Interactable/DoorKey.h"

#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
ADoor::ADoor()
{
	MeshComp = CreateDefaultSubobject <UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComp;

	OpenRotation = FRotator::ZeroRotator;
	ClosedRotation = FRotator::ZeroRotator;
	bDoorOpen = false;
	bDoorLocked = false;
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority)
		SetReplicates(true);

	ClosedRotation = this->GetActorRotation();

	GetWorld()->GetTimerManager().SetTimer(THDoor, this, &ADoor::RotateDoor, 0.02f, true);
	GetWorld()->GetTimerManager().PauseTimer(THDoor);
}

void ADoor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, bDoorOpen);
	DOREPLIFETIME(ADoor, bDoorLocked);
}

void ADoor::RotateDoor()
{
	if (bDoorOpen)
	{
		FRotator CurrentRotation = this->GetActorRotation();
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, OpenRotation, GetWorld()->GetDeltaSeconds(), 7.0f);
		this->SetActorRotation(NewRotation);
		if (NewRotation.Equals(OpenRotation, 0.05))
		{
			GetWorld()->GetTimerManager().PauseTimer(THDoor);
		}
	}
	else
	{
		FRotator CurrentRotation = this->GetActorRotation();
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, ClosedRotation, GetWorld()->GetDeltaSeconds(), 7.0f);
		this->SetActorRotation(NewRotation);
		if (NewRotation.Equals(ClosedRotation, 0.05))
		{
			GetWorld()->GetTimerManager().PauseTimer(THDoor);
		}
	}
}

void ADoor::OnRep_ToggleDoor()
{
	GetWorld()->GetTimerManager().UnPauseTimer(THDoor);
}

void ADoor::ToggleDoor(class ASurvivalCharacter* Player)
{
	if (Role == ROLE_Authority)
	{
		if (bDoorLocked)
		{
			if (UInventory* PlayersInventory = Player->GetInventoryComponent())
			{
				TArray<APickups*> InventoryItems = PlayersInventory->GetInventoryItems();
				for (APickups* Item : InventoryItems)
				{
					if (Item)
					{
						if (ADoorKey* Key = Cast<ADoorKey>(Item))
						{
							if (this == Key->GetLinkedDoor())
							{
								bDoorLocked = false;
								bDoorOpen = !bDoorOpen;
								OnRep_ToggleDoor();
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			bDoorOpen = !bDoorOpen;
			OnRep_ToggleDoor();
		}
	}
}
