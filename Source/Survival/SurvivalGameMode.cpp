// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SurvivalGameMode.h"
#include "SurvivalCharacter.h"
#include "SpawnPoint.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Public/EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

ASurvivalGameMode::ASurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	DefaultSpawnLocation = FVector(-400.0f, 50.0f, 300.0f);
	RespawnTime = 3.0f;
}

void ASurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	UClass* SpawnPointClass = ASpawnPoint::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(), SpawnPointClass); Actor; ++Actor)
	{
		SpawnPoints.Add(Cast<ASpawnPoint>(*Actor));
	}
}

void ASurvivalGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (AController* Controller = Cast<AController>(NewPlayer))
	{
		Spawn(Controller);
	}
}

ASpawnPoint* ASurvivalGameMode::GetSpawnPoint()
{
	for (int32 i = 0; i < SpawnPoints.Num(); ++i)
	{
		int32 Slot = FMath::RandRange(0, SpawnPoints.Num() - 1);
		if (SpawnPoints[Slot])
			return SpawnPoints[Slot];
	}
	return nullptr;
}

void ASurvivalGameMode::Spawn(AController* Controller)
{
	if (ASpawnPoint* SpawnPoint = GetSpawnPoint())
	{
		FVector Location = SpawnPoint->GetActorLocation();
		FRotator Rotation = SpawnPoint->GetActorRotation();
		if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation))
		{
			Controller->Possess(Pawn);
		}
	}
	else
	{
		FVector Location = DefaultSpawnLocation;
		FRotator Rotation = FRotator::ZeroRotator;
		if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation))
		{
			Controller->Possess(Pawn);
		}
	}
}

void ASurvivalGameMode::Respawn(AController* Controller)
{
	if (Controller)
	{
		if (Role == ROLE_Authority)
		{
			FTimerDelegate RespawnDele;
			RespawnDele.BindUFunction(this, FName("Spawn"), Controller);
			FTimerHandle RespawnHandle;
			GetWorld()->GetTimerManager().SetTimer(RespawnHandle, RespawnDele, RespawnTime, false);
		}
	}
}