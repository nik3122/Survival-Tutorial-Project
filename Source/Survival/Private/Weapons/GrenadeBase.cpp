// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeBase.h"
#include "SurvivalCharacter.h"
#include "LineTrace.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

AGrenadeBase::AGrenadeBase()
{
	ExplosionSphere = CreateDefaultSubobject<USphereComponent>("ExplosionSphereComponent");
	ExplosionSphere->InitSphereRadius(305.0f);

	ExplosionParticle = CreateDefaultSubobject<UParticleSystemComponent>("ParticleSystemComponent");

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->Velocity = FVector();
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Friction = 0.1f;

	LineTrace = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");

	FuseTime = 5.0f;
	ThrowSpeed = 1500.0f;
	bIsExploded = false;
}

void AGrenadeBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGrenadeBase, bIsExploded);
}

bool AGrenadeBase::ValidActorHit(ASurvivalCharacter* Player)
{
	FHitResult HitResult = LineTrace->LineTraceSingle(this->GetActorLocation(), Player->GetActorLocation(), true);
	if (AActor* Actor = HitResult.GetActor())
	{
		if (Actor == Player)
		{
			UE_LOG(LogTemp, Warning, TEXT("ACTOR IS PLAYER"));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ACTOR IS NOT PLAYER"));
			return false;
		}
	}
	return false;
}

void AGrenadeBase::OnRep_Explode()
{
	if (Role == ROLE_Authority)
	{//for damage checking
		TArray<AActor*> OverlappingActors;
		ExplosionSphere->GetOverlappingActors(OverlappingActors);
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor)
			{
				if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(Actor))
				{
					if (ValidActorHit(Player))
					{
						float Distance = FVector::Distance(this->GetActorLocation(), Player->GetActorLocation());
						float Damage = (200.0f / Distance) * 100.0f;
						Damage = FMath::RoundToFloat(Damage);
						Player->TakeDamage(Damage, FDamageEvent(), nullptr, this);
					}
				}
			}
		}

		this->MeshComp->SetHiddenInGame(true);
		this->SetActorEnableCollision(false);
		GetWorld()->GetTimerManager().UnPauseTimer(DestroyHandle);
	}
	else
	{//for effects on client
		if (ExplosionParticle)
			ExplosionParticle->Activate(false);
		if (ExplosionSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, this->GetActorLocation());

		this->MeshComp->SetHiddenInGame(true);
		this->SetActorEnableCollision(false);
	}
}

void AGrenadeBase::ExplodeGrenade()
{
	if (Role == ROLE_Authority)
	{
		bIsExploded = true;
		OnRep_Explode();
	}
}

void AGrenadeBase::DestroyGrenade()
{
	if (Role == ROLE_Authority)
	{
		Destroy();
	}
}

void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(ExplosionHandle, this, &AGrenadeBase::ExplodeGrenade, FuseTime, false);
		GetWorld()->GetTimerManager().PauseTimer(ExplosionHandle);
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &AGrenadeBase::DestroyGrenade, 5.0f, false);
		GetWorld()->GetTimerManager().PauseTimer(DestroyHandle);
	}
}

void AGrenadeBase::Throw(FVector ThrowLocation, FRotator ControlRotation)
{
	if (Role == ROLE_Authority)
	{
		ObjectPickedUp = false;
		OnRep_PickedUp();

		ThrowLocation = ThrowLocation + ControlRotation.Vector() * 85.0f;
		this->SetActorLocation(ThrowLocation);

		if (ControlRotation.Pitch >= 65.0f && ControlRotation.Pitch < 90.0f)
			ControlRotation.Pitch = 65.0f;
		else if (ControlRotation.Pitch <= 320.0f && ControlRotation.Pitch > 270.0f)
			ControlRotation.Pitch = 320.0f;

		GetWorld()->GetTimerManager().UnPauseTimer(ExplosionHandle);

		ProjectileMovement->Velocity = ControlRotation.Vector() * ThrowSpeed;
		ProjectileMovement->ProjectileGravityScale = 1.0f;
	}
}