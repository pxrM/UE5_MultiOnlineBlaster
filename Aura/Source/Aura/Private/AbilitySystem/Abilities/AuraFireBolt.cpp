// P


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectileActor.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(
			TEXT(
				"<Title>FIRE BOLT</>\n\n"
				"<Small>Level: </>"
				"<Level>%d</>\n"
				"<Small>ManaCost: </>"
				"<ManaCost>%.1f</>\n\n"
				"<Small>Cooldown: </>"
				"<Cooldown>%.1f</>\n\n"
				"<Default>Launches a bolt of fire, exploding on impact and dealing: </>"
				"<Damage>%d</>"
				"<Default>fire damage with a chance to burn</>"),
			Level, ManaCost, Cooldown, Damage);
	}
	else
	{
		return FString::Printf(
			TEXT(
				"<Title>FIRE BOLT</>\n\n"
				"<Small>Level: </>"
				"<Level>%d</>\n"
				"<Small>ManaCost: </>"
				"<ManaCost>%.1f</>\n\n"
				"<Small>Cooldown: </>"
				"<Cooldown>%.1f</>\n\n"
				"<Default>Launches %d bolts of fire, exploding on impact and dealing: </>"
				"<Damage>%d</>"
				"<Default>fire damage with a chance to burn</>"),
			Level, ManaCost, Cooldown, FMath::Min(Level, NumProjectiles), Damage);
	}
}

FString UAuraFireBolt::GetNextDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(
		TEXT(
			"<Title>NEXT LEVEL: </>\n\n"
			"<Small>Level: </>"
			"<Level>%d</>\n"
			"<Small>ManaCost: </>"
			"<ManaCost>%.1f</>\n\n"
			"<Small>Cooldown: </>"
			"<Cooldown>%.1f</>\n\n"
			"<Default>Launches %d bolts of fire, exploding on impact and dealing: </>"
			"<Damage>%d</>"
			"<Default>fire damage with a chance to burn</>"),
		Level, ManaCost, Cooldown, FMath::Min(Level, NumProjectiles), Damage);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
                                     bool bOverridePitch, float OverridePitch, AActor* HomingTarget)
{
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
	{
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(), SocketTag);
		// 从 SocketLocation 看过去朝向 ProjectileTargetLocation 的旋转角度。
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = OverridePitch;
		}
		//NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
		// 返回的是与 Rotation 旋转角度对应的单位向量，返回一个朝着该旋转角度的方向的单位向量，也就是这个旋转角度的“前进方向”。
		const FVector Forward = Rotation.Vector();

		TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlyRotatedRotators(Forward, FVector::UpVector, ProjectilesSpread, NumProjectiles);
		for(FRotator& Rot : Rotators)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(Rot.Quaternion());

			AAuraProjectileActor* ProjectileActor = GetWorld()->SpawnActorDeferred<AAuraProjectileActor>(
				ProjectileClass,
				SpawnTransform,
				GetOwningActorFromActorInfo(),
				Cast<APawn>(GetOwningActorFromActorInfo()),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			ProjectileActor->DamageEffectParams = MakeDamageEffectParamsFromClassDefault();
			ProjectileActor->FinishSpawning(SpawnTransform);
		}

		/*
		if (NumProjectiles > 1)
		{
			// 计算到最右侧的角度向量，通过将 Forward 向量绕 FVector::UpVector（即世界的上方向，Z轴）旋转 SpawnSpread / 2.f 度得到的。
			const FVector RightOfSpread = Forward.RotateAngleAxis(ProjectilesSpread / 2.f, FVector::UpVector);
			// 计算到最左侧的角度向量
			const FVector LeftOfSpread = Forward.RotateAngleAxis(-ProjectilesSpread / 2.f, FVector::UpVector);
			const float DeltaSpread = ProjectilesSpread / (NumProjectiles - 1);
			for (int32 i = 0; i < NumProjectiles; i++)
			{
				const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
				UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(),
				                                     SocketLocation + FVector(0, 0, 5),
				                                     SocketLocation + FVector(0, 0, 5) + Direction * 75.f,
				                                     1, FLinearColor::Red, 60, 1);
			}
			UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation,
									 SocketLocation + Forward * 100.f, 1, FLinearColor::White, 60, 1);
			UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation,
												 SocketLocation + LeftOfSpread * 100.f, 1, FLinearColor::Gray, 60, 1);
			UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation,
											 SocketLocation + RightOfSpread * 100.f, 1, FLinearColor::Gray, 60, 1);
		}*/
	}
}
