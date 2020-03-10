// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "GrenadeBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVAL_API AGrenadeBase : public APickups
{
	GENERATED_BODY()
public:
	AGrenadeBase();
	
protected:
	UPROPERTY(EditAnywhere)
		float FuseTime;

	UPROPERTY(EditAnywhere)
		float ThrowSpeed;

	UPROPERTY(EditAnywhere)
		class USphereComponent* ExplosionSphere;

	UPROPERTY(EditAnywhere)
		class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere)
		class UParticleSystemComponent* ExplosionParticle;

	UPROPERTY(EditAnywhere)
		class UProjectileMovementComponent* ProjectileMovement;

	class ULineTrace* LineTrace;

	FTimerHandle ExplosionHandle;
	FTimerHandle DestroyHandle;

	UPROPERTY(ReplicatedUsing = OnRep_Explode)
		bool bIsExploded;

	UFUNCTION()
		void OnRep_Explode();

protected:
	void ExplodeGrenade();
	void DestroyGrenade();
	bool ValidActorHit(class ASurvivalCharacter* Player);
	virtual void BeginPlay() override;

public:
	void Throw(FVector ThrowLocation, FRotator ControlRotation);
};
