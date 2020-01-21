// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

UCLASS()
class SURVIVAL_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
		FRotator OpenRotation;

	FRotator ClosedRotation;

	UPROPERTY(ReplicatedUsing = OnRep_ToggleDoor)
		bool bDoorOpen;

	UPROPERTY(EditAnywhere, Replicated)
		bool bDoorLocked;

	FTimerHandle THDoor;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnRep_ToggleDoor();

	void RotateDoor();

public:	
	void ToggleDoor(class ASurvivalCharacter* Player);

};
