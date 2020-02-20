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

	TArray<class AMagazineBase*> ExtraMagazines;
	class AMagazineBase* CurrentMagazine;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult);

public:
	void SetupWeapon(FName WeaponName);
	FHitResult Fire();
	FHitResult Fire(FHitResult ClientHitResult);
};
