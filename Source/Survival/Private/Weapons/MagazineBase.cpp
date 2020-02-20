// Fill out your copyright notice in the Description page of Project Settings.


#include "MagazineBase.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMagazineBase::AMagazineBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComp;
	bReplicates = true;
	CurrentMagazineAmmo = 0;
	MagazineInUse = false;
}

// Called when the game starts or when spawned
void AMagazineBase::BeginPlay()
{
	Super::BeginPlay();
	SetupMagazine(FName("AR-15Mag"), false);
}

void AMagazineBase::OnRep_MagazineInUse()
{
	this->MeshComp->SetHiddenInGame(MagazineInUse);
	this->SetActorEnableCollision(!MagazineInUse);
}

void AMagazineBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagazineBase, CurrentMagazineAmmo);
	DOREPLIFETIME(AMagazineBase, MagazineInUse);
}

void AMagazineBase::Pickup()
{
	if (Role == ROLE_Authority)
	{
		MagazineInUse = true;
		OnRep_MagazineInUse();
	}
}

void AMagazineBase::SetupMagazine(FName MagazineName, bool IsForWeapon)
{
	if (MagazineDataTable)
	{
		static const FString PString = "DefaultPString";
		MagazineData = MagazineDataTable->FindRow<FMagazineData>(MagazineName, PString, true);
		if (MagazineData)
		{
			MeshComp->SetStaticMesh(MagazineData->MagazineMesh);
			CurrentMagazineAmmo = MagazineData->MagazineCapacity;
		}
	}
	this->MeshComp->SetHiddenInGame(IsForWeapon);
	this->SetActorEnableCollision(!IsForWeapon);
}

int AMagazineBase::CurrentAmmo()
{
	return CurrentMagazineAmmo;
}

void AMagazineBase::Fire()
{
	--CurrentMagazineAmmo;
}
