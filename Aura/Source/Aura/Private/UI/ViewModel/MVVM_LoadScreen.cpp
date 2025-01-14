// P


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName( FString("LoadSlot_0"));
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName( FString("LoadSlot_1"));
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName( FString("LoadSlot_2"));
	
	LoadSlots.Add(0, LoadSlot_0);
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlots.Add(2, LoadSlot_2);
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(const int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnterName)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	LoadSlots[Slot]->SetPlayerName(EnterName); 	// 修改mvvm上的角色名称数据
	LoadSlots[Slot]->SlotStatus = Taken;		// 修改界面为加载加载状态

	AuraGameMode->SaveSlotData(LoadSlots[Slot], Slot);

	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
}

void UMVVM_LoadScreen::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	for(const TTuple<int32, UMVVM_LoadSlot*> Slot : LoadSlots)
	{
		ULoadScreenSaveGame* SaveGameObj = AuraGameMode->GetSaveSlotData(Slot.Value->GetLoadSlotName(), Slot.Key);
		// 获取存档数据
		const FString PlayerName = SaveGameObj->PlayerName;
		const TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveGameObj->SaveSlotStatus;
		// 设置存档视图模型的数据
		Slot.Value->SetPlayerName(PlayerName);
		Slot.Value->SlotStatus = SaveSlotStatus;
		// 调用模型初始化
		Slot.Value->InitializeSlot();
	}
}
 