// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Weapons/WeaponBase.h"
#include "SurvivalCharacter.h"
#include "LineTrace.h"
#include "Public/Weapons/MagazineBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComponent");
	RootComponent = MeshComp;

	LineTraceComp = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");

	DefaultWeaponName = FName("");
	CurrentMagazine = nullptr;
	DefaultMagazinesSpawned = false;
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, ExtraMagazines);
	DOREPLIFETIME(AWeaponBase, CurrentMagazine);
	DOREPLIFETIME(AWeaponBase, DefaultMagazinesSpawned);
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
			if (Role == ROLE_Authority)
			{
				if (WeaponData->MagazineRowNames.Num() > 0 && WeaponData->BPMagazineClass)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.bNoFail = true;
					SpawnParams.Owner = this;
					if (AMagazineBase* Magazine = GetWorld()->SpawnActor<AMagazineBase>(WeaponData->BPMagazineClass, this->GetActorLocation(), this->GetActorRotation(), SpawnParams))
					{
						CurrentMagazine = Magazine;

						int32 RandomNum = FMath::RandRange(1, 3);

						for (uint8 i = 0; i < RandomNum; ++i)
						{
							if (AMagazineBase* Magazine = GetWorld()->SpawnActor<AMagazineBase>(WeaponData->BPMagazineClass, this->GetActorLocation(), this->GetActorRotation(), SpawnParams))
							{
								ExtraMagazines.Add(Magazine);
							}
						}
						OnRep_MagazinesSpawned();
						FTimerHandle THandle;
						GetWorld()->GetTimerManager().SetTimer(THandle, this, &AWeaponBase::MagsSpawned, 1.5f, false);
					}
				}
			}
		}
	}
}

void AWeaponBase::OnRep_MagazinesSpawned()
{
	if (WeaponData->MagazineRowNames.Num() > 0)
	{
		TArray<FName> MagazineRowName = WeaponData->MagazineRowNames;
		if (CurrentMagazine != nullptr)
		{
			CurrentMagazine->SetupMagazine(WeaponData->MagazineRowNames[0], true);
			UE_LOG(LogTemp, Warning, TEXT("SETTING UP CURRENTMAGAZINE ONREP"));
		}

		for (AMagazineBase* Magazine : ExtraMagazines)
		{
			if (Magazine != nullptr)
			{
				Magazine->SetupMagazine(WeaponData->MagazineRowNames[0], true);
				UE_LOG(LogTemp, Warning, TEXT("SETTING UP MAGAZINE ONREP"));
			}
		}
	}
}

void AWeaponBase::MagsSpawned()
{
	DefaultMagazinesSpawned = true;
}

bool AWeaponBase::CanReloadWeapon()
{
	return (ExtraMagazines.Num() > 0);
}

void AWeaponBase::ReloadWeapon()
{
	if (Role == ROLE_Authority)
	{
		AMagazineBase* MagInWeapon = CurrentMagazine;
		if (MagInWeapon)
		{
			if (ExtraMagazines.Num() == 1)
			{
				if (ExtraMagazines[0])
				{
					if (MagInWeapon->CurrentAmmo() > 0)
						ExtraMagazines.Add(MagInWeapon);
					else
						MagInWeapon->Destroy();

					CurrentMagazine = ExtraMagazines[0];
					ExtraMagazines.RemoveAt(0, 1, true);
				}
			}
			else if (ExtraMagazines.Num() > 1)
			{
				if (ExtraMagazines[0] != nullptr)
				{
					AMagazineBase* FullestMagazine = ExtraMagazines[0];
					uint8 MagCount = 0;
					uint8 FullestMagIndex = 0;
					for (AMagazineBase* Magazine : ExtraMagazines)
					{
						if (Magazine != nullptr)
						{
							if (Magazine->CurrentAmmo() > FullestMagazine->CurrentAmmo())
							{
								FullestMagazine = Magazine;
								FullestMagIndex = MagCount;
							}
						}
						++MagCount;
					}
					if (MagInWeapon->CurrentAmmo() > 0)
						ExtraMagazines.Add(MagInWeapon);
					else
						MagInWeapon->Destroy();

					CurrentMagazine = FullestMagazine;
					ExtraMagazines.RemoveAt(FullestMagIndex, 1, true);
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Current Magazine Ammo: %d"), CurrentMagazine->CurrentAmmo());
			uint8 MagIndex = 0;
			for (AMagazineBase* Magazine : ExtraMagazines)
			{
				if (Magazine != nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Extra Magazine %d Ammo: %d"), (MagIndex + 1), Magazine->CurrentAmmo());
				}
				++MagIndex;
			}
		}
	}
}

FHitResult AWeaponBase::Fire()
{
	if (Role < ROLE_Authority && CurrentMagazine)
	{
		if (CurrentMagazine->CurrentAmmo() > 0)
		{
			if (WeaponData && WeaponData->FireAnimation)
			{
				MeshComp->PlayAnimation(WeaponData->FireAnimation, false);
			}

			FVector StartLocation = MeshComp->GetSocketLocation(FName("s_muzzle"));
			FRotator Rotation = MeshComp->GetSocketRotation(FName("s_muzzle"));
			FVector EndLocation = StartLocation + Rotation.Vector() * 3500.0f;

			FHitResult HitResult = LineTraceComp->LineTraceSingle(StartLocation, EndLocation, true);

			CurrentMagazine->Fire();
			UE_LOG(LogTemp, Warning, TEXT("Current Ammo: %d"), CurrentMagazine->CurrentAmmo());
			return HitResult;
		}
		else
			return FHitResult();
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
	if (Role == ROLE_Authority && CurrentMagazine)
	{
		if (CurrentMagazine->CurrentAmmo() > 0)
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
			CurrentMagazine->Fire();
		}
		else
			return FHitResult();
	}
	return FHitResult();
}

bool AWeaponBase::IsCompatibleMagazine(AMagazineBase* Magazine)
{
	if (Magazine != nullptr)
	{
		TArray<FString> WeaponList = Magazine->GetCompatibleWeapons();
		for (FString WeaponName : WeaponList)
		{
			if (WeaponData->WeaponName == WeaponName)
				return true;
		}
	}
	return false;
}

void AWeaponBase::AddMagazine(AMagazineBase* Magazine)
{
	if (Role == ROLE_Authority)
	{
		if (Magazine != nullptr)
		{
			ExtraMagazines.Add(Magazine);
			Magazine->SetOwner(this);
			Magazine->Pickup();
		}
	}
}
