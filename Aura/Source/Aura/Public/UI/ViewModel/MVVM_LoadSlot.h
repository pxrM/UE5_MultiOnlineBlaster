// P

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WodgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeSlot() const;

	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const {return PlayerName;}
	
	void SetLoadSlotName(const FString& InLoadSlotName);
	FString GetLoadSlotName() const {return LoadSlotName;} 

	void SetMapName(const FString& InMapName);
	FString GetMapName() const {return MapName;}

	void SetPlayerLevel(const int32& InPlayerLevel);
	int32 GetPlayerLevel() const {return PlayerLevel;}

public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;
	
	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	UPROPERTY()
	FName PlayerStartTag;

	UPROPERTY()
	FString MapAssetName;

private:
	/*
	 * 字段通知。视图可以和小部件链接
	 * EditAnywhere：表示该属性可以在 任何地方 被编辑，包括蓝图编辑器、属性面板等。
	 * BlueprintReadWrite：表示这个属性可以 读取 和 写入，它可以在蓝图中被访问和修改。
	 * FieldNotify：通常用于通知机制，标志这个变量变更时，某个回调函数会被触发。
	 *		这在处理属性变化时非常有用，特别是你需要执行一些逻辑（如 UI 更新）时。这个标记表明当 PlayerName 发生变化时，会触发相关的通知回调。
	 * Setter：表示为该变量提供了一个 设置器（setter）方法，通常这个方法用来在对该属性进行赋值时执行某些逻辑。
	 * Getter：表示为该变量提供了一个 获取器（getter）方法，允许其他代码访问该属性的值。
	 * meta: 是一个关键字，表示附加的元数据（metadata），用来为 UPROPERTY 提供额外的信息或配置。
	 *		元数据是用于控制属性行为的一种机制，通常用于修改属性的可见性、交互方式、编辑器行为等。
	 *		AllowPrivateAccess="true" 是 meta 内的一个设置，表示允许 私有成员 被访问。
	 *				如果设置为 true，则允许蓝图或其他代码访问和修改这个变量，即使它是私有的。
	 *
	 * 是一个可以在编辑器和蓝图中交互的字符串属性，能够触发通知并通过自定义的方法进行赋值和访问。
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess="true"))
	FString PlayerName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess="true"))
	FString LoadSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess="true"))
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess="true"))
	int32 PlayerLevel;
};
