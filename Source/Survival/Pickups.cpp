// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups.h"
#include "SurvivalCharacter.h"
#include "PlayerStatComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Texture2D.h"

// Sets default values
APickups::APickups()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	Icon = CreateDefaultSubobject<UTexture2D>("IconTexture");

	RootComponent = MeshComp;
	bReplicates = true;
	bReplicateMovement = true;

	IncreaseAmount = 30.0f;
	ObjectPickedUp = false;
}

void APickups::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickups, ObjectPickedUp);
}

// Called when the game starts or when spawned
void APickups::BeginPlay()
{
	Super::BeginPlay();

}

void APickups::UseItem(ASurvivalCharacter* Player)
{
	if (Role == ROLE_Authority && PickupType != EPickupType::EKey && PickupType != EPickupType::EGreneade)
	{
		if (PickupType == EPickupType::EFood)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING HUNGER"));
			Player->PlayerStatComp->AddHunger(IncreaseAmount);
		}
		else if (PickupType == EPickupType::EWater)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING THIRST"));
			Player->PlayerStatComp->AddThirst(IncreaseAmount);
		}
		else if (PickupType == EPickupType::EHealth)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING Health"));
			Player->PlayerStatComp->AddHealth(IncreaseAmount);
		}
		Destroy();
	}
}

void APickups::OnRep_PickedUp()
{
	this->MeshComp->SetHiddenInGame(ObjectPickedUp);
	this->SetActorEnableCollision(!ObjectPickedUp);
}

void APickups::InInventory(bool In)
{
	if (Role == ROLE_Authority)
	{
		ObjectPickedUp = In;
		OnRep_PickedUp();
	}
}

UTexture2D* APickups::GetItemIcon()
{
	return Icon;
}
