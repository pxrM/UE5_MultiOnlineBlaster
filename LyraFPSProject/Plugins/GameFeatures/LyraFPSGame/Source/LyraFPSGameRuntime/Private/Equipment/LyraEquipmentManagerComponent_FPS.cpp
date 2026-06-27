// Fill out your copyright notice in the Description page of Project Settings.

#include "Equipment/LyraEquipmentManagerComponent_FPS.h"

#include "Equipment/LyraEquipmentDefinition_FPS.h"


ULyraEquipmentManagerComponent_FPS::ULyraEquipmentManagerComponent_FPS(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

ULyraEquipmentDefinition_FPS* ULyraEquipmentManagerComponent_FPS::GetDefinitionByInstance(
	ULyraEquipmentInstance* Instance)
{
	for (FLyraAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Instance == Entry.Instance)
		{
			return Cast<ULyraEquipmentDefinition_FPS>(Entry.EquipmentDefinition.GetDefaultObject());
		}
	}
	return nullptr;
}
