// P

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

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
	
};
