// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChatReceived, FText, ChatMessage);

UCLASS( ClassGroup=(Blueprintable), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UChatComponent();

protected:
	int MinimumTimeBetweenMessages;

	FDateTime Time;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendMessage(const FText& Message);
	bool Server_SendMessage_Validate(const FText& Message);
	void Server_SendMessage_Implementation(const FText& Message);

	UFUNCTION(Client, Reliable)
		void Client_ReceiveMessage(const FText& Message);
	void Client_ReceiveMessage_Implementation(const FText& Message);

	bool ValidWaitTimeForMessage();

	UPROPERTY(BlueprintAssignable)
		FChatReceived ChatReceived;

public:
	UFUNCTION(BlueprintCallable)
		void SendMessage(FText Message);

	UFUNCTION(BlueprintCallable)
		FText GetSafeMessage(FText Message);
};
