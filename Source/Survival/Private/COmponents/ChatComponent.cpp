
#include "Public/Components/ChatComponent.h"
#include "SurvivalCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UChatComponent::UChatComponent()
{

}


// Called when the game starts
void UChatComponent::BeginPlay()
{
	Super::BeginPlay();

}

bool UChatComponent::Server_SendMessage_Validate(const FString& Message)
{
	return true;
}

void UChatComponent::Server_SendMessage_Implementation(const FString& Message)
{
	TArray<AActor*> SurvivalActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivalCharacter::StaticClass(), SurvivalActors);

	for (AActor* Actor : SurvivalActors)
	{
		if (Actor)
		{
			if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(Actor))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Player: %s     Message: %s"), *Player->GetName(), *Message);
				Player->GetChatComponent()->Client_ReceiveMessage(Message);
			}
		}
	}
}

void UChatComponent::Client_ReceiveMessage_Implementation(const FString& Message)
{
	if (Message.Len() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player: %s     Message: %s"), *GetOwner()->GetName(), *Message);
		ChatMessages.Add(Message);
		ChatReceived.Broadcast();
	}
}

void UChatComponent::SendMessage(FString Message)
{//do checking in cpp or bp
	Server_SendMessage(Message);
}

TArray<FString> UChatComponent::GetChatMessages()
{
	return ChatMessages;
}