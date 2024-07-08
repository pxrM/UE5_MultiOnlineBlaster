// P

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * 使用自定义Asset Manager需要配置：
 *		Config\DefaultEngine.ini  =》 [/Script/Engine.Engine]  =》 AssetManagerClassName=/Script/Aura.AuraAssetManager
 *		
 * Asset Manager:
 *		全局单例，类似subsystem
 *		在编辑器或游戏中，只有一个
 *		利用Asset Registry对资源分类和查询
 *		保持全局资产加载状态，即哪些在内存中，哪些不在
 *		整合了一些独立的系统，如cooking，async loading，asset registry
 *		管理primary assets和asset bundles
 *	Asset Registry:
 *		为编辑器模式中的ContentBrowser提供支持
 *		储存资产信息，即使资产没加载到内存
 *		启动编辑器时，从磁盘刷新资产
 *		每个资产都有key value键值对，方便查询
 *		为Asset Manager提供数据
 *	Asset Manager Component:
 *		Primary asset：最顶层的资产，如map或level
 *			Primary Asset Type: 资产类型
 *			Primary Asset Id：资产Id，唯一的
 *			Primary Asset Rules：cooking，chunking，management rules
 *		Asset Manager Settings：明确如何加载和使用Primary Asset
 *		Secondary Asset：次要资产，随Primary asset一起加载卸载，如mesh，texture
 *
 *	Asset:
 *		是在Content Browser中看到的那些物件。贴图，BP，音频和地图等都属于Asset.
 *	Asset Registry:
 *		资源注册表，其中存储了每个特定的asset的有用的信息。这些信息会在asset被储存的时候进行更新。
 *	Streamable Managers:
 *		负责进行读取物件并将其放在内存中。
 *	Primary Assets:
 *		是在游戏中可以进行手动载入/释放的东西。包括地图文件以及一些游戏相关的物件，例如character classs或者inventory items.
 *	Secondary Assets:
 *		是其他的那些Assets了，例如贴图和声音等。这一类型的assets是根据Primary Assets来自动进行载入的。
 *	Asset Bundle:
 *		是一个Asset的列表，用于将一堆Asset在runtime的时候载入。
 *
 *	资源管理系统将所有资源分为两类：主资源 和 次资源。资源管理器通过主资源的 主资源 ID 即可直接对其进行操作，调用 GetPrimaryAssetId 即可获得此 ID。
 *	为将特定 UObject 类构成的资源指定为主资源，覆盖 GetPrimaryAssetId 即可返回一个有效的 FPrimaryAssetId 结构。次资源不由资源管理器直接处理，但其被主资源引用或使用后引擎便会自动进行加载。
 *	默认只有 UWorld 资源（关卡）为主资源；所有其他资源均为次资源。为将次资源设为主资源，必须覆盖其类的 GetPrimaryAssetId 函数，返回一个有效的 FPrimaryAssetId 结构。
 *		将次资源设置为主资源示例：
 *			.h
 *			//定义资源类型
 *			static const FPrimaryAssetType CharacterType = TEXT("Character");
 *			//重写方法以获取主资产ID，GetPrimaryAssetId是在UObject就声明的抽象函数，所以不用担心父类没有这个抽象方法。
 *			virtual FPrimaryAssetId GetPrimaryAssetId() const override;
 *			.cpp
 *			FPrimaryAssetId AMyCharacter::GetPrimaryAssetId() const
 *			{
 *				//AssetType是FPrimaryAssetType类型的变量
 *				return  FPrimaryAssetId(CharacterType,GetFName());
 *			}
 *		注册资源：
 *			引擎里面注册：
 *				打开ProjectSettings，然后找到AssetManager，在设置面板里面可以看到PrimaryAssetTypestoScan，这个变量就是对AssetType进行注册的地方了
 *			代码里面注册：
 *				virtual int32 ScanPathsForPrimaryAssets(FPrimaryAssetType PrimaryAssetType, const TArray<FString>& Paths,
 *					UClass* BaseClass, bool bHasBlueprintClasses, bool bIsEditorOnly = false, bool bForceSynchronousScan = true);
 *				virtual int32 ScanPathForPrimaryAssets(FPrimaryAssetType PrimaryAssetType, const FString& Path,
 *					UClass* BaseClass, bool bHasBlueprintClasses, bool bIsEditorOnly = false, bool bForceSynchronousScan = true);
 *			
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UAuraAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
