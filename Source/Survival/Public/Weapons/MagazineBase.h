// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "MagazineBase.generated.h"

USTRUCT(BlueprintType)
struct FMagazineData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		class UStaticMesh* MagazineMesh;

	UPROPERTY(EditAnywhere)
		FString MagazineName;

	UPROPERTY(EditAnywhere)
		TArray<FString> CompatibleWeapons;

	UPROPERTY(EditAnywhere)
		int MagazineCapacity;
};

UCLASS()
class SURVIVAL_API AMagazineBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazineBase();

protected:
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(Replicated)
		int CurrentMagazineAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_MagazineInUse)
		bool MagazineInUse;

	UPROPERTY(EditAnywhere)
		UDataTable* MagazineDataTable;

	FMagazineData* MagazineData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnRep_MagazineInUse();

public:
	void Pickup();
	void SetupMagazine(FName MagazineName, bool IsForWeapon);
	int CurrentAmmo();
	void Fire();
};
