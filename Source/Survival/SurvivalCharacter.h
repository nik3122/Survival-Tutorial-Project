// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivalCharacter.generated.h"

UCLASS(config=Game)
class ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ASurvivalCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	class ULineTrace* LineTraceComp;

	class UInventory* Inventory;

	TSubclassOf<class UUserWidget> InventoryWidgetClass;
	class UUserWidget* InventoryWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OpenCloseInventory)
		class AStorageContainer* OpenedContainer;

	bool bIsSprinting;

	FTimerHandle SprintingHandle;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AWeaponBase> WeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponIteracted)
		class AWeaponBase* Weapon;

	void SetIsAiming();
	void SetIsNotAiming();

	UPROPERTY(ReplicatedUsing = OnRep_SetAiming)
		bool bIsAiming;

	UFUNCTION()
		void OnRep_SetAiming();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Aim(bool Aiming);
	bool Server_Aim_Validate(bool Aiming);
	void Server_Aim_Implementation(bool Aiming);

	UFUNCTION(BlueprintPure)
		FString ReturnPlayerStats();

protected:
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void StartSprinting();
	void StopSprinting();

	void StartCrouch();
	void StopCrouch();

	void HandleSprinting();

	void AttemptJump();

	UFUNCTION()
		void OnRep_WeaponIteracted();

	UFUNCTION(BlueprintCallable)
		bool GetPlayerHasWeapon();

	UFUNCTION()
		void OnRep_OpenCloseInventory();

	void OpenCloseInventory();

	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Interact();
	bool Server_Interact_Validate();
	void Server_Interact_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_InventoryClose();
	bool Server_InventoryClose_Validate();
	void Server_InventoryClose_Implementation();

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Attack(FHitResult HitResult);
	bool Server_Attack_Validate(FHitResult HitResult);
	void Server_Attack_Implementation(FHitResult HitResult);

	void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void MultiDie();
	bool MultiDie_Validate();
	void MultiDie_Implementation();

	FTimerHandle DestroyHandle;
	void CallDestroy();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

public:
	class UPlayerStatComponent* PlayerStatComp;

public:
	float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		class UInventory* GetInventoryComponent();

	UFUNCTION(BlueprintCallable)
		class AStorageContainer* GetOpenedContainer();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

