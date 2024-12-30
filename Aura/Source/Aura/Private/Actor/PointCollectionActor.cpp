// P


#include "Actor/PointCollectionActor.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

APointCollectionActor::APointCollectionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePts.Add(Pt_0);
	SetRootComponent(Pt_0);

	CreateSceneComponent(Pt_1, "Pt_1");
	CreateSceneComponent(Pt_2, "Pt_2");
	CreateSceneComponent(Pt_3, "Pt_3");
	CreateSceneComponent(Pt_4, "Pt_4");
	CreateSceneComponent(Pt_5, "Pt_5");
	CreateSceneComponent(Pt_6, "Pt_6");
	CreateSceneComponent(Pt_7, "Pt_7");
	CreateSceneComponent(Pt_8, "Pt_8");
	CreateSceneComponent(Pt_9, "Pt_9");
	CreateSceneComponent(Pt_10, "Pt_10");
}

TArray<USceneComponent*> APointCollectionActor::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,
	float YawOverride)
{
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("需要返回的数量超过了数组数量"));

	TArray<USceneComponent*> ArrayCopy;
	for(USceneComponent* Pt : ImmutablePts)
	{
		if(ArrayCopy.Num() >= NumPoints) return ArrayCopy;

		if(Pt != Pt_0)
		{
			// 获取两个组件的位置差值：ToPoint 是从 Pt_0 到 Pt 的方向和距离的矢量
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			// 绕指定轴旋转该位移向量：ToPoint 向量绕 Z 轴旋转 YawOverride 度
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			// 设置 Pt 组件的新位置：将 Pt 组件的位置根据 Pt_0 的位置和一个指定的旋转（YawOverride）进行调整，达到通过旋转平移 Pt 的效果。
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		// 基于 Pt 当前的位置基础上向上和向下各移动 500 单位的坐标。
		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);
		// 获取在半径 1500 单位内、距离当前 Actor位置一定范围内的所有活跃玩家或其他对象。
		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayerWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		// 忽略这些活跃对象
		QueryParams.AddIgnoredActors(IgnoreActors);
		// 从 RaisedLocation（上方 500 单位）发出一条射线，沿着 Z 轴方向向下发射到 LoweredLocation（下方 500 单位）。
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		// 将 Pt 当前的位置的 X 和 Y 坐标保留，结合碰撞点的 Z 坐标，计算出一个新的 AdjustedLocation，这样目标对象的位置就调整到了碰撞点的 Z 坐标高度。
		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);
		// UKismetMathLibrary::MakeRotFromZ()用来基于一个法线向量生成对应的旋转矩阵，将目标对象的旋转设置为与碰撞表面垂直的方向。
		// 根据碰撞法线调整目标对象的旋转，使其与碰撞表面垂直。
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		ArrayCopy.Add(Pt);
	}
	return ArrayCopy;
}

void APointCollectionActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APointCollectionActor::CreateSceneComponent(TObjectPtr<USceneComponent>& Pt, const FName Name)
{
	Pt = CreateDefaultSubobject<USceneComponent>(Name);
	ImmutablePts.Add(Pt);
	Pt->SetupAttachment(GetRootComponent());
}

