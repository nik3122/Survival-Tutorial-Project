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

	class UChatComponent* ChatComponent;

	class UInventory* Inventory;

	TSubclassOf<class UUserWidget> InventoryWidgetClass;
	class UUserWidget* InventoryWidget;

	TSubclassOf<class UUserWidget> DoorWidgetClass;
	class UUserWidget* DoorWidget;

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
		bool IsPlayerAiming();

	UPROPERTY(Replicated)
		float PlayerPitch;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetPlayerPitch(float Pitch);
	bool Server_SetPlayerPitch_Validate(float Pitch);
	void Server_SetPlayerPitch_Implementation(float Pitch);

	UFUNCTION(BlueprintPure)
		float GetPlayerPitch();

	bool DoubleClicked;

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

	void OpenDoorWidget();

	UFUNCTION(BlueprintCallable)
		void LockDoor(bool Lock);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_LockDoor(bool Lock, class ADoor* Door);
	bool Server_LockDoor_Validate(bool Lock, class ADoor* Door);
	void Server_LockDoor_Implementation(bool Lock, class ADoor* Door);

	class ADoor* InteractingDoor;

	void DoubleInteract();
	void CheckDoubleInteract();
	void SingleInteract();
	FTimerHandle THDoubleInteract;

	void Interact(bool WasDoubleClick);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Interact(FVector End);
	bool Server_Interact_Validate(FVector End);
	void Server_Interact_Implementation(FVector End);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_InventoryClose();
	bool Server_InventoryClose_Validate();
	void Server_InventoryClose_Implementation();

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Attack(FHitResult HitResult);
	bool Server_Attack_Validate(FHitResult HitResult);
	void Server_Attack_Implementation(FHitResult HitResult);

	void Reload();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Reload();
	bool Server_Reload_Validate();
	void Server_Reload_Implementation();

	void Throw();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Throw(FVector ClientHeadLocation, FRotator ControlRotation);
	bool Server_Throw_Validate(FVector ClientHeadLocation, FRotator ControlRotation);
	void Server_Throw_Implementation(FVector ClientHeadLocation, FRotator ControlRotation);

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
		class UChatComponent* GetChatComponent();

	UFUNCTION(BlueprintCallable)
		class AStorageContainer* GetOpenedContainer();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

