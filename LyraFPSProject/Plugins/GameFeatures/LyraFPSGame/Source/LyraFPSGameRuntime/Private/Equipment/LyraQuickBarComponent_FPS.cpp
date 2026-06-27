// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/LyraQuickBarComponent_FPS.h"

#include "Equipment/LyraEquipmentManagerComponent_FPS.h"

ULyraQuickBarComponent_FPS::ULyraQuickBarComponent_FPS(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

ULyraEquipmentManagerComponent* ULyraQuickBarComponent_FPS::FindEquipmentManager() const
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		return Pawn->FindComponentByClass<ULyraEquipmentManagerComponent_FPS>();
	}
	return Super::FindEquipmentManager();
	
	
}
