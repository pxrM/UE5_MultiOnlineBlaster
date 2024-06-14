// P

#pragma once

#include "CoreMinimal.h"
#include "AuraInputConfig.h"
#include "EnhancedInputComponent.h"
#include "AuraInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/*
	 * 将输入配置中的动作绑定到特定的功能上
	 *  const UAuraInputConfig* InputConfig: 配置
	 *	UserClass* Object: 需要绑定输入动作的对象
	 *	PressedFuncType PressedFunc: 按下动作触发时调用的函数
	 *	ReleasedFuncType ReleasedFunc: 释放动作触发时调用的函数
	 *	HeldFuncType HeldFunc: 持续按住动作触发时调用的函数
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
	
};


template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UAuraInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);
	check(Object);
	/*
	 * Started：输入动作刚刚开始。例如，按下一个键时会触发这个事件。
	 * Ongoing：输入动作正在进行中。这通常用于持续输入的情况，例如按住一个键。
	 * Canceled：输入动作被取消。例如，在按住一个键的过程中松开。
	 * Completed：输入动作完成。例如，释放按下的键。
	 * Triggered：输入动作触发。这可以是任意时刻的触发，但一般用于持续输入的情况。
	 */
	for(const FAuraInputAction& Action : InputConfig->AbilityInputActions)
	{
		if(Action.InputAction && Action.InputTag.IsValid())
		{
			if(PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}
			if(HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
