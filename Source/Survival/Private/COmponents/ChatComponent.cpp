
#include "Public/Components/ChatComponent.h"
#include "SurvivalCharacter.h"
#include "Public/Player/SurvivalPlayerState.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

// Sets default values for this component's properties
UChatComponent::UChatComponent()
{
	MinimumTimeBetweenMessages = 4;
	Time = FDateTime::Now();
}


// Called when the game starts
void UChatComponent::BeginPlay()
{
	Super::BeginPlay();

}

bool UChatComponent::ValidWaitTimeForMessage()
{
	int PreviousSecond = Time.GetSecond();
	int CurrentSecond = FDateTime::Now().GetSecond();
	if (CurrentSecond > (PreviousSecond + MinimumTimeBetweenMessages))
	{
		Time = FDateTime::Now();
		return true;
	}
	else if (CurrentSecond < PreviousSecond)
	{
		if ((CurrentSecond + PreviousSecond) > PreviousSecond + MinimumTimeBetweenMessages)
		{
			Time = FDateTime::Now();
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;//time between messages to short
	}
}

bool UChatComponent::Server_SendMessage_Validate(const FText& Message)
{
	return true;
}

void UChatComponent::Server_SendMessage_Implementation(const FText& Message)
{
	if (!ValidWaitTimeForMessage())
		return;

	FString MessageStr = Message.ToString();
	if (MessageStr.Len() <= 0)
		return;
	FText SafeMessage = FText();

	TArray<AActor*> SurvivalActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivalCharacter::StaticClass(), SurvivalActors);

	for (AActor* Actor : SurvivalActors)
	{
		if (Actor)
		{
			if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(Actor))
			{
				if (ASurvivalPlayerState* PState = Cast<ASurvivalPlayerState>(Player->GetPlayerState()))
				{
					FString MessageToSend = PState->GetPlayerName() + ": " + MessageStr;
					
					if (MessageToSend.Len() > 108)
					{
						MessageToSend = MessageToSend.Mid(0, 108);
						SafeMessage = FText::FromString(MessageToSend);
					}
					else
					{
						SafeMessage = FText::FromString(MessageToSend);
					}
					Player->GetChatComponent()->Client_ReceiveMessage(SafeMessage);
				}				
			}
		}
	}
}

void UChatComponent::Client_ReceiveMessage_Implementation(const FText& Message)
{
	if (Message.ToString().Len() > 0)
	{
		ChatReceived.Broadcast(Message);
	}
}

void UChatComponent::SendMessage(FText Message)
{
	if (!ValidWaitTimeForMessage())
		return;

	FString MessageStr = Message.ToString();
	if (MessageStr.Len() <= 0)
		return;

	if (MessageStr.Len() > 108)
	{
		MessageStr = MessageStr.Mid(0, 108);
		Message = FText::FromString(MessageStr);
	}

	Server_SendMessage(Message);
}

FText UChatComponent::GetSafeMessage(FText Message)
{
	if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(GetOwner()))
	{
		if (ASurvivalPlayerState* PState = Cast<ASurvivalPlayerState>(Player->GetPlayerState()))
		{
			FString MessageStr = Message.ToString();
			FString MessageToSend = PState->GetPlayerName() + ": " + MessageStr;
			if (MessageToSend.Len() > 108)
			{
				int Difference = MessageToSend.Len() - 108;
				MessageToSend = MessageStr.Mid(0, MessageStr.Len() - Difference);
				return FText::FromString(MessageToSend);
			}
		}
	}

	return Message;
}
