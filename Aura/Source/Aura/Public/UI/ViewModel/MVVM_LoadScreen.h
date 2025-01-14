// P

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

class UMVVM_LoadSlot;

/**
 * https://dev.epicgames.com/documentation/zh-cn/unreal-engine/umg-viewmodel-for-unreal-engine
 *
 * 初始化视图模型的几种方法:
 *		Create Instance：该小部件会自动创建它自己的 Viewmodel 实例。
 *		Manual：该小部件以Viewmodel为null进行初始化，您需要手动创建一个实例并分配它。
 *		Global Viewmodel Collection：指的是可供项目中的任何小部件使用的全局可用的视图模型。需要全局视图模型标识符。
 *		Property Path：在初始化时，执行一个const函数来查找 Viewmodel。Viewmodel属性路径使用以句点分隔的成员名称。
 *						例如：GetPlayerController.Vehicle.ViewModel。属性路径始终相对于小部件。
 */
UCLASS()
class AURA_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	void InitializeLoadSlots();
	
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(const int32 Index) const;

	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnterName);

	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);

	void LoadData();
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

private:
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;
};
