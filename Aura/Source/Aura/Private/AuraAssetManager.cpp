// P


#include "AuraAssetManager.h"

#include "AuraGameplayTags.h"
#include "AbilitySystemGlobals.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);

	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 初始化添加的原生tag。
	FAuraGameplayTags::InitializeNativeGameplayTags();
	// 从UE4.24开始，就必须要调用该函数来使用TargetData，否则会有ScriptStructCache相关的报错，并且客户端将从服务器断开。
	UAbilitySystemGlobals::Get().InitGlobalData();
}
