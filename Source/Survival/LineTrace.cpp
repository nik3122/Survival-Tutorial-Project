// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTrace.h"
#include "SurvivalCharacter.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "Public/DrawDebugHelpers.h"

// Sets default values for this component's properties
ULineTrace::ULineTrace()
{

}


// Called when the game starts
void ULineTrace::BeginPlay()
{
	Super::BeginPlay();
	
}

FHitResult ULineTrace::LineTraceSingle(FVector Start, FVector End)
{
	FHitResult HitResult;
	FCollisionObjectQueryParams CollisionParams;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	if (AActor* Actor = GetOwner()->GetOwner())
	{
		if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(Actor))
		{
			CollisionQueryParams.AddIgnoredActor(Character);
		}
	}

	if (GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		Start,
		End,
		CollisionParams,
		CollisionQueryParams
	))
	{
		return HitResult;
	}
	else
	{
		return HitResult;
	}
}

FHitResult ULineTrace::LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine)
{
	FHitResult Actor = LineTraceSingle(Start, End);
	if (ShowDebugLine)
	{
		DrawDebugLine(
			GetWorld(),
			Start,
			End,
			FColor::Red,
			false,
			3.0f,
			0,
			5.0f
		);
	}
	return Actor;
}