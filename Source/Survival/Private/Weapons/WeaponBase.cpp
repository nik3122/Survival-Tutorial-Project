// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Weapons/WeaponBase.h"
#include "SurvivalCharacter.h"
#include "LineTrace.h"

#include "Components/SkeletalMeshComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComponent");
	RootComponent = MeshComp;

	LineTraceComp = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");

	DefaultWeaponName = FName("");
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultWeaponName != "")
		SetupWeapon(DefaultWeaponName);
}

void AWeaponBase::SetupWeapon(FName WeaponName)
{
	if (WeaponDataTable)
	{
		static const FString PString = FString("AR-15DT");
		WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, PString, true);
		if (WeaponData)
		{
			MeshComp->SetSkeletalMesh(WeaponData->WeaponMesh);
		}
	}
}

FHitResult AWeaponBase::Fire()
{
	if (Role < ROLE_Authority)
	{
		if (WeaponData && WeaponData->FireAnimation)
		{
			MeshComp->PlayAnimation(WeaponData->FireAnimation, false);
		}

		FVector StartLocation = MeshComp->GetSocketLocation(FName("s_muzzle"));
		FRotator Rotation = MeshComp->GetSocketRotation(FName("s_muzzle"));
		FVector EndLocation = StartLocation + Rotation.Vector() * 3500.0f;

		FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLocation, EndLocation, true);

		return HitResult;
	}
	else
	{
		return FHitResult();
	}
}

bool AWeaponBase::IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult)
{
	if (ServerHitResult.GetActor() == nullptr)
	{
		return false;
	}

	float ClientStart = ClientHitResult.TraceStart.Size();
	float ClientEnd = ClientHitResult.TraceEnd.Size();
	float ServerStart = ServerHitResult.TraceStart.Size();
	float ServerEnd = ServerHitResult.TraceEnd.Size();

	if (ClientStart >= ServerStart - 15.0f && ClientStart <= ServerStart + 15.0f && ClientEnd >= ServerEnd - 15.0f && ClientEnd <= ServerEnd + 15.0f)
	{
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("INVALID HIT START: %f"), ClientStart - ServerStart);
		UE_LOG(LogTemp, Warning, TEXT("INVALID HIT END: %f"), ClientEnd - ServerEnd);
		return false;
	}
}

FHitResult AWeaponBase::Fire(FHitResult ClientHitResult)
{
	if (Role == ROLE_Authority)
	{
		FVector StartLocation = MeshComp->GetSocketLocation(FName("s_muzzle"));

		if (AActor* HitActor = ClientHitResult.GetActor())
		{
			FRotator Rotation = MeshComp->GetSocketRotation(FName("s_muzzle"));
			FVector EndLocation = StartLocation + Rotation.Vector() * 3500.0f;

			FHitResult ServerHitResult = LineTraceComp->LineTraceSingle(StartLocation, EndLocation, true);

			if (IsValidShot(ClientHitResult, ServerHitResult))
			{
				if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(HitActor))
				{
					float TestDamage = 20.0f;
					Player->TakeDamage(TestDamage, FDamageEvent(), nullptr, GetOwner());
				}
			}
			else//play hit object effects
			{

			}
		}
		else//nothing hit
		{

		}
	}
	return FHitResult();
}
