// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnPoint.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ASpawnPoint::ASpawnPoint()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = MeshComp;
	SetActorHiddenInGame(true);
}

// Called when the game starts or when spawned
void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}