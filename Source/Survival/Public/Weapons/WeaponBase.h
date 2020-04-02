// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "WeaponBase.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		class USkeletalMesh* WeaponMesh;

	UPROPERTY(EditAnywhere)
		FString WeaponName;

	UPROPERTY(EditAnywhere)
		TArray<FName> MagazineRowNames;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AMagazineBase> BPMagazineClass;

	UPROPERTY(EditAnywhere)
		class UAnimationAsset* FireAnimation;
};

UCLASS()
class SURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	UPROPERTY(EditAnywhere)
		class USkeletalMeshComponent* MeshComp;

	class ULineTrace* LineTraceComp;

	UPROPERTY(EditAnywhere)
		class UDataTable* WeaponDataTable;

	FWeaponData* WeaponData;

	UPROPERTY(EditAnywhere)
		FName DefaultWeaponName;

	UPROPERTY(Replicated)
		TArray<class AMagazineBase*> ExtraMagazines;
	UPROPERTY(Replicated)
		class AMagazineBase* CurrentMagazine;

	UPROPERTY(ReplicatedUsing = OnRep_MagazinesSpawned)
		bool DefaultMagazinesSpawned;

	UFUNCTION()
		void OnRep_MagazinesSpawned();
	void MagsSpawned();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult);

public:
	void SetupWeapon(FName WeaponName);
	bool CanReloadWeapon();
	void ReloadWeapon();
	FHitResult Fire(FVector ImpactPoint);
	FHitResult Fire(FHitResult ClientHitResult);
	bool IsCompatibleMagazine(class AMagazineBase* Magazine);
	void AddMagazine(class AMagazineBase* Magazine);
};
