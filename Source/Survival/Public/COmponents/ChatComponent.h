// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChatReceived);

UCLASS( ClassGroup=(Blueprintable), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UChatComponent();

protected:
	TArray<FString> ChatMessages;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendMessage(const FString& Message);
	bool Server_SendMessage_Validate(const FString& Message);
	void Server_SendMessage_Implementation(const FString& Message);

	UFUNCTION(Client, Reliable)
		void Client_ReceiveMessage(const FString& Message);
	void Client_ReceiveMessage_Implementation(const FString& Message);

	UPROPERTY(BlueprintAssignable)
		FChatReceived ChatReceived;

public:
	UFUNCTION(BlueprintCallable)
		void SendMessage(FString Message);

	UFUNCTION(BlueprintCallable)
		TArray<FString> GetChatMessages();
};
