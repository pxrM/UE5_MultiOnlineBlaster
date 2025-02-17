// P

/*
 *	explicit AMapEntrance(const FObjectInitializer& ObjectInitializer);
 *	
 *  explicit 是 C++ 中的一个关键字，用于修饰构造函数。它的作用是禁止隐式类型转换，确保构造函数只能通过显式调用，而不能被隐式调用。
 *  为什么需要 explicit？
 *		在 C++ 中，构造函数可以用于隐式类型转换。
 *		例如，如果有一个构造函数 A(int)，那么可以将一个整数隐式转换为类 A 的对象。
 *		这种隐式转换可能会导致代码的可读性降低，甚至引发错误。为了防止这种情况，可以使用 explicit 关键字来禁止隐式转换。
 *	示例
 *	假设没有 explicit：
 *		class AMapEntrance{
 *		public:
 *			AMapEntrance(const FObjectInitializer& ObjectInitializer);
 *			};
 *			AMapEntrance MyEntrance = FObjectInitializer();  // 隐式调用构造函数
 *	如果加上 explicit：
 *		class AMapEntrance{
 *		public:
 *			explicit AMapEntrance(const FObjectInitializer& ObjectInitializer);
 *		};
 *		AMapEntrance MyEntrance = FObjectInitializer();  // 编译错误，需要显式调用
 *		AMapEntrance MyEntrance(FObjectInitializer());  // 正确
 */

#pragma once

#include "CoreMinimal.h"
#include "Checkpoints/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()

public:
	explicit AMapEntrance(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp,
								 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	/* start IHighlightInterface */
	// 传送点不需要激活高亮效果，覆写它，不让它执行高亮函数
	virtual void HighlightActor_Implementation() override;
	/* end IHighlightInterface */
	
	/* start ISaveInterface */
	virtual void LoadActor_Implementation() override;
	/* end ISaveInterface */

public:
	// 需要传送到的目标地图
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	// 传送到目标地图后的出生点标签
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;
};
