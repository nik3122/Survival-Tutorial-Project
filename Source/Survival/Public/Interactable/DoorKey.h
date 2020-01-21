// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "DoorKey.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVAL_API ADoorKey : public APickups
{
	GENERATED_BODY()
public:
    ADoorKey();

protected:
    UPROPERTY(EditAnywhere)
        class ADoor* LinkedDoor;

public:
    class ADoor* GetLinkedDoor();
};
