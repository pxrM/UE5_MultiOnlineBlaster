// P


#include "AbilitySystem/Abilities/AuraFireBlast.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(
		TEXT(
			"<Title>火焰爆炸</>\n\n"
			// 细节
			"<Small>等级：</> <Level>%i</>\n"
			"<Small>技能冷却：</> <Cooldown>%.1f</>\n"
			"<Small>蓝量消耗：</> <ManaCost>%.1f</>\n\n" //%.1f会四舍五入到小数点后一位
			// 技能描述
			"<Default>向四面八方发射 %i 颗火球，每颗火球会在返回时发生爆炸，并造成</> <Damage>%i</> <Default>点径向火焰伤害，并有一定几率触发燃烧。</>"),
		Level, ManaCost, Cooldown, NumFireBalls, Damage);
}

FString UAuraFireBlast::GetNextDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(
		TEXT(
			"<Title>NEXT LEVEL: </>\n\n"
			// 细节
			"<Small>等级：</> <Level>%i</>\n"
			"<Small>技能冷却：</> <Cooldown>%.1f</>\n"
			"<Small>蓝量消耗：</> <ManaCost>%.1f</>\n\n" //%.1f会四舍五入到小数点后一位
			// 技能描述
			"<Default>向四面八方发射 %i 颗火球，每颗火球会在返回时发生爆炸，并造成</> <Damage>%i</> <Default>点径向火焰伤害，并有一定几率触发燃烧。</>"),
		Level, ManaCost, Cooldown, FMath::Min(Level, NumFireBalls), Damage);
}

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	for(const FRotator& Rotator:Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());

		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass, SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefault();
		FireBalls.Add(FireBall);
		FireBall->FinishSpawning(SpawnTransform);
	}
	
	return FireBalls;
}
