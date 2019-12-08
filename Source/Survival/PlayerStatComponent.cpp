// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatComponent.h"
#include "SurvivalCharacter.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UPlayerStatComponent::UPlayerStatComponent()
{
	Hunger = 50.0f;
	HungerDecrementValue = 0.5f;
	Thirst = 50.0f;
	ThirstDecrementValue = 0.5f;
	Health = 50.0f;

	Stamina = 100.0f;
}

// Called when the game starts
void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerStatComponent::HandleHungerAndThirst, 3.0f, true);
		GetWorld()->GetTimerManager().SetTimer(StaminaHandle, this, &UPlayerStatComponent::RegenerateStamina, 1.0f, true);//rengerates stamina
	}
}

void UPlayerStatComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Hunger, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Thirst, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerStatComponent, Stamina, COND_OwnerOnly);
}

void UPlayerStatComponent::HandleHungerAndThirst()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(HungerDecrementValue);
		LowerThirst(ThirstDecrementValue);
	}
}

void UPlayerStatComponent::LowerHunger(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHunger(Value);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		Hunger -= Value;
		if (Hunger < 0)
		{
			if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(GetOwner()))
			{
				Character->TakeDamage(Hunger * -1, FDamageEvent(), Character->GetController(), Character);
			}
		}
	}
}

void UPlayerStatComponent::LowerThirst(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerThirst(Value);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		Thirst -= Value;
		if (Thirst < 0)
		{
			if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(GetOwner()))
			{
				Character->TakeDamage(Thirst * -1, FDamageEvent(), Character->GetController(), Character);
			}
		}
	}
}

void UPlayerStatComponent::LowerHealth(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHealth(Value);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		Health -= Value;
		if (Health < 0.0f)
			Health = 0.0f;
	}
}

void UPlayerStatComponent::LowerStamina(float Value)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerStamina(Value);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		if (Stamina - Value < 0.0f)
			Stamina = 0.0f;
		else
			Stamina -= Value;
	}
}

bool UPlayerStatComponent::ServerLowerHunger_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerHunger_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(Value);
	}
}

bool UPlayerStatComponent::ServerLowerThirst_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerThirst_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerThirst(Value);
	}
}

bool UPlayerStatComponent::ServerLowerHealth_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerHealth_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHealth(Value);
	}
}

bool UPlayerStatComponent::ServerLowerStamina_Validate(float Value)
{
	return true;
}

void UPlayerStatComponent::ServerLowerStamina_Implementation(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerStamina(Value);
	}
}

void UPlayerStatComponent::RegenerateStamina()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Stamina >= 100.0f)
			Stamina = 100.0f;
		else
			++Stamina;
	}
}

float UPlayerStatComponent::GetHunger()
{
	return Hunger;
}

float UPlayerStatComponent::GetThirst()
{
	return Thirst;
}

float UPlayerStatComponent::GetHealth()
{
	return Health;
}

float UPlayerStatComponent::GetStamina()
{
	return Stamina;
}

void UPlayerStatComponent::ControlSprintingTimer(bool IsSprinting)//make it pause timer on the server
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerControlSprintingTimer(IsSprinting);
		return;
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		if (IsSprinting)
		{
			GetWorld()->GetTimerManager().PauseTimer(StaminaHandle);
		}
		else
		{
			GetWorld()->GetTimerManager().UnPauseTimer(StaminaHandle);
		}
	}
}


bool UPlayerStatComponent::ServerControlSprintingTimer_Validate(bool IsSprinting)
{
	return true;
}

void UPlayerStatComponent::ServerControlSprintingTimer_Implementation(bool IsSprinting)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ControlSprintingTimer(IsSprinting);
	}
}

void UPlayerStatComponent::AddHunger(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Hunger + Value > 100.0f)
			Hunger = 100.0f;
		else
			Hunger += Value;
	}
}

void UPlayerStatComponent::AddThirst(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Thirst + Value > 100.0f)
			Thirst = 100.0f;
		else
			Thirst += Value;
	}
}

void UPlayerStatComponent::AddHealth(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Health + Value > 100.0f)
			Health = 100.0f;
		else
			Health += Value;
	}
}