// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SurvivalCharacter.h"
#include "PlayerStatComponent.h"
#include "LineTrace.h"
#include "Pickups.h"
#include "Inventory.h"
#include "SurvivalGameMode.h"
#include "StorageContainer.h"
#include "Public/Weapons/WeaponBase.h"
#include "Public/Weapons/MagazineBase.h"
#include "Public/Interactable/Door.h"
#include "Public/Weapons/GrenadeBase.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ASurvivalCharacter

ASurvivalCharacter::ASurvivalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PlayerStatComp = CreateDefaultSubobject<UPlayerStatComponent>("PlayerStatComponent");
	Inventory = CreateDefaultSubobject<UInventory>("InventoryComponent");
	LineTraceComp = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");

	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryRef(TEXT("/Game/BlueprintClasses/InventoryWIdgets/InventoryBase"));

	if (InventoryRef.Class)
	{
		InventoryWidgetClass = InventoryRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> DoorRef(TEXT("/Game/BlueprintClasses/DoorControl"));

	if (DoorRef.Class)
	{
		DoorWidgetClass = DoorRef.Class;
	}

	bIsSprinting = false;
	bIsAiming = false;
	DoubleClicked = false;
	InteractingDoor = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASurvivalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASurvivalCharacter::AttemptJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASurvivalCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouch);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ASurvivalCharacter::SetIsAiming);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ASurvivalCharacter::SetIsNotAiming);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::SingleInteract);
	PlayerInputComponent->BindAction("Interact", IE_DoubleClick, this, &ASurvivalCharacter::DoubleInteract);

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ASurvivalCharacter::OpenCloseInventory);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASurvivalCharacter::Attack);
	PlayerInputComponent->BindAction("ReloadWeapon", IE_Pressed, this, &ASurvivalCharacter::Reload);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASurvivalCharacter::Throw);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASurvivalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASurvivalCharacter::LookUpAtRate);
}

void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	/*FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (WeaponClass)
	{
		FTransform WeaponTransform;
		WeaponTransform.SetLocation(FVector::ZeroVector);
		WeaponTransform.SetRotation(FQuat(FRotator::ZeroRotator));

		Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, WeaponTransform, SpawnParams);
		if (Weapon)
		{
			Weapon->SetupWeapon(FName("AR-15"));
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_hand_r"));
		}
	}*/

	GetWorld()->GetTimerManager().SetTimer(SprintingHandle, this, &ASurvivalCharacter::HandleSprinting, 1.0f, true);
}

void ASurvivalCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASurvivalCharacter, OpenedContainer, COND_OwnerOnly);
	DOREPLIFETIME(ASurvivalCharacter, Weapon);
	DOREPLIFETIME(ASurvivalCharacter, bIsAiming);
}

void ASurvivalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASurvivalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASurvivalCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (!bIsSprinting && !GetCharacterMovement()->IsCrouching())
			Value *= 0.5f;
		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		if (!bIsSprinting && !GetCharacterMovement()->IsCrouching())
			Value *= 0.5f;
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::StartSprinting()
{
	if (PlayerStatComp->GetStamina() > 10.0f)
	{
		bIsSprinting = true;
		PlayerStatComp->ControlSprintingTimer(true);
	}
	else if (PlayerStatComp->GetStamina() <= 0.0f)
	{
		PlayerStatComp->ControlSprintingTimer(false);
	}
}

void ASurvivalCharacter::StopSprinting()
{
	bIsSprinting = false;
	PlayerStatComp->ControlSprintingTimer(false);
}

void ASurvivalCharacter::StartCrouch()
{
	if (!GetCharacterMovement()->IsCrouching() && !GetCharacterMovement()->IsFalling())
	{
		UE_LOG(LogTemp, Warning, TEXT("CROUCHING"));
		GetCharacterMovement()->bWantsToCrouch = true;
	}
}

void ASurvivalCharacter::StopCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UE_LOG(LogTemp, Warning, TEXT("UNCROUCHING"));
		GetCharacterMovement()->bWantsToCrouch = false;
	}
}

void ASurvivalCharacter::HandleSprinting()
{
	if (bIsSprinting && this->GetVelocity().Size())
	{
		PlayerStatComp->LowerStamina(2.0f);
		if (PlayerStatComp->GetStamina() <= 0.0f)
			StopSprinting();
	}
}

void ASurvivalCharacter::AttemptJump()
{
	if (PlayerStatComp->GetStamina() > 10.0f && !GetCharacterMovement()->IsFalling())
	{
		Jump();
		PlayerStatComp->LowerStamina(10.0f);
	}
}

void ASurvivalCharacter::OnRep_OpenCloseInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
		InventoryWidget->RemoveFromViewport();

	if (OpenedContainer == nullptr)
	{
		InventoryWidget->RemoveFromViewport();
		if (APlayerController* PController = GetController()->CastToPlayerController())
		{
			PController->bShowMouseCursor = false;
			PController->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
			if (APlayerController* PController = GetController()->CastToPlayerController())
			{
				PController->bShowMouseCursor = true;
				PController->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ASurvivalCharacter::OpenCloseInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromViewport();
		if (APlayerController* PController = GetController()->CastToPlayerController())
		{
			PController->bShowMouseCursor = false;
			PController->SetInputMode(FInputModeGameOnly());
		}
		if (OpenedContainer)
			Server_InventoryClose();
	}
	else
	{
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
			if (APlayerController* PController = GetController()->CastToPlayerController())
			{
				PController->bShowMouseCursor = true;
				PController->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ASurvivalCharacter::OpenDoorWidget()
{
	if (DoorWidget && DoorWidget->IsInViewport())
	{
		DoorWidget->RemoveFromViewport();
		if (APlayerController* PController = GetController()->CastToPlayerController())
		{
			PController->bShowMouseCursor = false;
			PController->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		DoorWidget = CreateWidget<UUserWidget>(GetWorld(), DoorWidgetClass);
		if (DoorWidget)
		{
			DoorWidget->AddToViewport();
			if (APlayerController* PController = GetController()->CastToPlayerController())
			{
				PController->bShowMouseCursor = true;
				PController->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ASurvivalCharacter::LockDoor(bool Lock)
{
	if (InteractingDoor)
	{
		Server_LockDoor(Lock, InteractingDoor);
	}
}

bool ASurvivalCharacter::Server_LockDoor_Validate(bool Lock, ADoor* Door)
{
	return true;
}

void ASurvivalCharacter::Server_LockDoor_Implementation(bool Lock, ADoor* Door)
{
	if (Door)
	{
		Door->LockDoor(Lock, this);
	}
}

void ASurvivalCharacter::DoubleInteract()
{
	DoubleClicked = true;
	Interact(true);
}

void ASurvivalCharacter::CheckDoubleInteract()
{
	if (DoubleClicked)
		DoubleClicked = false;
	else
		Interact(false);
}

void ASurvivalCharacter::SingleInteract()
{
	GetWorld()->GetTimerManager().SetTimer(THDoubleInteract, this, &ASurvivalCharacter::CheckDoubleInteract, 0.2f, false);
}

void ASurvivalCharacter::Interact(bool WasDoubleClick)
{
	FVector Start = GetMesh()->GetBoneLocation(FName("head"));
	FVector End = Start + FollowCamera->GetForwardVector() * 170.0f;
	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);
	if (AActor* Actor = HitResult.GetActor())
	{
		//UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR: %s"), *Actor->GetName());
		if (WasDoubleClick)
		{
			if (ADoor* Door = Cast<ADoor>(Actor))
			{
				InteractingDoor = Door;
				OpenDoorWidget();
			}
		}
		else
		{
			if (Cast<APickups>(Actor))
			{
				Server_Interact();
			}
			else if (Cast<AStorageContainer>(Actor))
			{
				Server_Interact();
			}
			else if (Cast<AWeaponBase>(Actor))
			{
				Server_Interact();
			}
			else if (Cast<ADoor>(Actor))
			{
				Server_Interact();
			}
			else if (AMagazineBase* Magazine = Cast<AMagazineBase>(Actor))
			{
				if (Weapon != nullptr)
				{
					if (Weapon->IsCompatibleMagazine(Magazine))
					{
						UE_LOG(LogTemp, Warning, TEXT("PICKING UP MAGAZINE CLIENT"));
						Server_Interact();
					}
				}
			}
		}
	}
}

bool ASurvivalCharacter::Server_Interact_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_Interact_Implementation()
{
	if (Role == ROLE_Authority)
	{
		FVector Start = GetMesh()->GetBoneLocation(FName("head"));
		FVector End = Start + FollowCamera->GetForwardVector() * 170.0f;
		FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);
		if (AActor* Actor = HitResult.GetActor())
		{
			if (APickups* Pickup = Cast<APickups>(Actor))
			{
				Inventory->AddItem(Pickup);
			}
			else if (AStorageContainer* Container = Cast<AStorageContainer>(Actor))
			{
				if (Container->IsChestOpen() && OpenedContainer == nullptr)
					return;

				bool OpenChest = false;
				if (OpenedContainer)
				{
					OpenedContainer = nullptr;
				}
				else
				{
					OpenedContainer = Container;
					OpenChest = true;
				}
				Container->OpenedChest(OpenChest);
			}
			else if (AWeaponBase* HitWeapon = Cast<AWeaponBase>(Actor))
			{
				UE_LOG(LogTemp, Warning, TEXT("PICKING UP WEAPON SERVER"));
				Weapon = HitWeapon;
				Weapon->SetOwner(this);
				OnRep_WeaponIteracted();
			}
			else if (ADoor* Door = Cast<ADoor>(Actor))
			{
				Door->ToggleDoor(this);
			}
			else if (AMagazineBase* Magazine = Cast<AMagazineBase>(Actor))
			{
				if (Weapon != nullptr)
				{
					if (Weapon->IsCompatibleMagazine(Magazine))
					{
						UE_LOG(LogTemp, Warning, TEXT("PICKING UP MAGAZINE SERVER"));
						Weapon->AddMagazine(Magazine);
					}
				}
			}
		}
	}
}

void ASurvivalCharacter::OnRep_WeaponIteracted()
{
	if (Weapon)
	{
		Weapon->SetActorEnableCollision(false);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_hand_r"));
	}
	else// after drop
	{

	}
}

bool ASurvivalCharacter::GetPlayerHasWeapon()
{
	if (Weapon) return true;
	else return false;
}

bool ASurvivalCharacter::Server_InventoryClose_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_InventoryClose_Implementation()
{
	if (OpenedContainer)
		OpenedContainer->OpenedChest(false);
	OpenedContainer = nullptr;
}

void ASurvivalCharacter::Attack()
{
	if (Weapon)
	{
		Server_Attack(Weapon->Fire());
	}
}

bool ASurvivalCharacter::Server_Attack_Validate(FHitResult HitResult)
{
	return true;
}

void ASurvivalCharacter::Server_Attack_Implementation(FHitResult HitResult)
{
	if (Role == ROLE_Authority && Weapon)
	{
		Weapon->Fire(HitResult);
	}
}

void ASurvivalCharacter::Reload()
{
	if (Weapon)
	{
		if (Weapon->CanReloadWeapon())
		{
			Server_Reload();
		}
	}
}

bool ASurvivalCharacter::Server_Reload_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_Reload_Implementation()
{
	if (Weapon)
	{
		if (Weapon->CanReloadWeapon())
		{
			Weapon->ReloadWeapon();
		}
	}
}

void ASurvivalCharacter::Throw()
{
	if (Inventory)
	{
		for (APickups* Item : Inventory->GetInventoryItems())
		{
			if (Item)
			{
				if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(Item))
				{
					Inventory->RemoveItem(Grenade);
					Server_Throw(this->GetMesh()->GetBoneLocation(FName("head")), this->GetControlRotation());
					return;
				}
			}
		}
	}
}

bool ASurvivalCharacter::Server_Throw_Validate(FVector ClientHeadLocation, FRotator ControlRotation)
{
	return true;
}

void ASurvivalCharacter::Server_Throw_Implementation(FVector ClientHeadLocation, FRotator ControlRotation)
{
	if (Inventory)
	{
		for (APickups* Item : Inventory->GetInventoryItems())
		{
			if (Item)
			{
				if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(Item))
				{
					FVector ServerHeadLocation = this->GetMesh()->GetBoneLocation(FName("head"));
					if (FVector::Distance(ServerHeadLocation, ClientHeadLocation) < 50.0f)//fix later
					{
						Grenade->Throw(ClientHeadLocation, ControlRotation);
						Inventory->RemoveItem(Grenade);
					}
					return;
				}
			}
		}
	}

}

void ASurvivalCharacter::SetIsAiming()
{
	if (Weapon)
	{
		Server_Aim(true);
	}
}

void ASurvivalCharacter::SetIsNotAiming()
{
	if (Weapon)
	{
		Server_Aim(false);
	}
}

void ASurvivalCharacter::OnRep_SetAiming()
{
	bUseControllerRotationYaw = bIsAiming;
}

bool ASurvivalCharacter::Server_Aim_Validate(bool Aiming)
{
	return true;
}

void ASurvivalCharacter::Server_Aim_Implementation(bool Aiming)
{
	bIsAiming = Aiming;
	OnRep_SetAiming();
}

float ASurvivalCharacter::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (Role < ROLE_Authority || PlayerStatComp->GetHealth() <= 0.0f)
		return 0.0f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		PlayerStatComp->LowerHealth(ActualDamage);
		if (PlayerStatComp->GetHealth() <= 0.0f)
		{
			Die();
		}
	}

	return ActualDamage;
}

UInventory* ASurvivalCharacter::GetInventoryComponent()
{
	return Inventory;
}

AStorageContainer* ASurvivalCharacter::GetOpenedContainer()
{
	return OpenedContainer;
}

void ASurvivalCharacter::CallDestroy()
{
	if (Role == ROLE_Authority)
		Destroy();
}

void ASurvivalCharacter::Die()
{
	if (Role == ROLE_Authority)
	{
		if (OpenedContainer)
		{
			OpenedContainer->OpenedChest(false);
			OpenedContainer = nullptr;
		}
		Inventory->DropAllInventory();
		MultiDie();
		AGameModeBase* GM = GetWorld()->GetAuthGameMode();
		if (ASurvivalGameMode* GameMode = Cast<ASurvivalGameMode>(GM))
		{
			GameMode->Respawn(GetController());
		}
		//start our destroy timer to remove actor from world
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &ASurvivalCharacter::CallDestroy, 10.0f, false);
	}
}

bool ASurvivalCharacter::MultiDie_Validate()
{
	return true;
}

void ASurvivalCharacter::MultiDie_Implementation()
{
	//ragdoll
	GetCapsuleComponent()->DestroyComponent();
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetAllBodiesSimulatePhysics(true);
}

FString ASurvivalCharacter::ReturnPlayerStats()
{
	FString RetString = "Hunger: " + FString::SanitizeFloat(PlayerStatComp->GetHunger())
		+ "   Thirst: " + FString::SanitizeFloat(PlayerStatComp->GetThirst())
		+ "   Stamina: " + FString::SanitizeFloat(PlayerStatComp->GetStamina())
		+ "   Health: " + FString::SanitizeFloat(PlayerStatComp->GetHealth());
	return RetString;
}