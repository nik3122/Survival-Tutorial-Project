// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalGameMode.generated.h"

UCLASS(minimalapi)
class ASurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivalGameMode();

protected:
	TArray<class ASpawnPoint*> SpawnPoints;
	FVector DefaultSpawnLocation;

	UPROPERTY(EditAnywhere, Category = "S|Resapwn")
		float RespawnTime;

protected:
	class ASpawnPoint* GetSpawnPoint();

	UFUNCTION()
	void Spawn(AController* Controller);

	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	void Respawn(AController* Controller);

public:
	virtual void BeginPlay() override;
};