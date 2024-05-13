// P


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

AAuraPlayerState::AAuraPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// Minimal 	Multiplayer,AI-Controlled 		Effect不复制，Cues和Tags会被复制到所有client
	// Mixed	Multiplayer,Player-Controlled	Effect只复制到自己的client，Cues和Tags会被复制到所有client
	// Full		SinglePlayer(单人游戏)			GAS数据都会被复制到所有client
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
 
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	// 设置复制更新频率。
	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
  