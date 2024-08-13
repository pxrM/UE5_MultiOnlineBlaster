// P


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	// 获取召唤师的朝向
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	// 获取召唤师的位置
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	// 将召唤的角度范围进行分段，在每段里面生成一个召唤物
	const float DeltaSpread = SpawnSpread / NumMinions;
	
	// 计算到最右侧的角度向量，通过将 Forward 向量绕 FVector::UpVector（即世界的上方向，Z轴）旋转 SpawnSpread / 2.f 度得到的。
	const FVector RightOfSpread = Forward.RotateAngleAxis(SpawnSpread / 2.f, FVector::UpVector);
	// 计算到最左侧的角度向量
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);

	// 在每个分段上面获取位置
	TArray<FVector> SpawnLocations;
	for (int32 i = 0; i < NumMinions; i++)
	{
		// 获取每段的方向
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
		// 根据方向在向量上随机一个位置
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
		// 确保生成位置位于地面。
		// 进行线性碰撞检测，这条线沿着 Z 轴（上下方向）移动了 400 单位，检查是否与任何物体发生碰撞。
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f),
		                                     ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		if(Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}

		SpawnLocations.Add(ChosenSpawnLocation);
		
		// debug生成位置
		// DrawDebugSphere(GetWorld(), ChosenSpawnLocation, 3.f, 12, FColor::Cyan, false, 3.f);
		// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location,
		//                                      Location + Direction * MaxSpawnDistance, 4.f, FLinearColor::Green, 3.f);
		// DrawDebugSphere(GetWorld(), Location + Direction * MinSpawnDistance, 3.f, 12, FColor::Red, false, 3.f);
		// DrawDebugSphere(GetWorld(), Location + Direction * MaxSpawnDistance, 3.f, 12, FColor::Red, false, 3.f);
	}

	// debug角度范围
	// DrawDebugSphere(GetWorld(), Location + RightOfSpread * MinSpawnDistance, 10.f, 12, FColor::Red, false, 3.f);
	// DrawDebugSphere(GetWorld(), Location + RightOfSpread * MaxSpawnDistance, 10.f, 12, FColor::Red, false, 3.f);
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location,
	//                                      Location + RightOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Green, 3.f);
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location,
	//                                      Location + LeftOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Gray, 3.f);
	// DrawDebugSphere(GetWorld(), Location + LeftOfSpread * MinSpawnDistance, 10.f, 12, FColor::Red, false, 3.f);
	// DrawDebugSphere(GetWorld(), Location + LeftOfSpread * MaxSpawnDistance, 10.f, 12, FColor::Red, false, 3.f);

	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}
