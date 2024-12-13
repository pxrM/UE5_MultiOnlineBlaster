
#include "AuraAbilityTypes.h"


bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	int32 RepBits = 0;
	
	if(Ar.IsSaving()) // 判断当前是否在保存数据
	{
		/*
		 * 保存数据时，如果有对应的配置项数据，那么将对应位的值设置为1
		 */
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}

		/* Add start */
		if(bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		if(bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}

		if(bIsSuccessfulDeBuff)
		{
			RepBits |= 1 << 9;
		}
		if(DeBuffDamage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if(DeBuffDuration > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if(DeBuffFrequency > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if(DeBuffDamageType.IsValid())
		{
			RepBits |= 1 << 13;
		}
		if(!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 14;
		}
		/* Add end */
	}

	// 序列化RepBits
	Ar.SerializeBits(&RepBits, 15);

	/*
	 * 如果对应位的值为1，则将数据存入Ar
	 */
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				// HitResult = TSharedPtr<FHitResult>(new FHitResult());
				HitResult = MakeShared<FHitResult>();
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	/* Add start */
	if(RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	if(RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}

	if(RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDeBuff;
	}
	if(RepBits & (1 << 10))
	{
		Ar << DeBuffDamage;
	}
	if(RepBits & (1 << 11))
	{
		Ar << DeBuffDuration;
	}
	if(RepBits & (1 << 12))
	{
		Ar << DeBuffFrequency;
	}
	if(RepBits & (1 << 13))
	{
		if(Ar.IsLoading())
		{
			if(!DeBuffDamageType.IsValid())
			{
				DeBuffDamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		DeBuffDamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	if(RepBits & (1 << 14))
	{
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}
	/* Add end */

	
	// 如果当前是加载操作（表示正在从网络或存储设备中加载数据到内存中）
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	
	
	bOutSuccess = true;
	
	return true;
}
