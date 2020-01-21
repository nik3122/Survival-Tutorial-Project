// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorKey.h"

ADoorKey::ADoorKey()
{
	LinkedDoor = nullptr;
}

ADoor* ADoorKey::GetLinkedDoor()
{
	return LinkedDoor;
}
