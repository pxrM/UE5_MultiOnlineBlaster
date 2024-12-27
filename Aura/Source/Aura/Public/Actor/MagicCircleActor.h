// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicCircleActor.generated.h"

/**
 * 场景贴花
 */
UCLASS()
class AURA_API AMagicCircleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicCircleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	/**
	 * UDecalComponent 是一种用于渲染贴花 (Decal) 的组件。
	 * 贴花是游戏中常用的一种视觉效果，通常用于添加细节，比如血迹、污渍、涂鸦、弹孔等，这些效果通常会被投射到场景中的表面，而不需要对这些表面进行修改。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDecalComponent> MagicCircleDecalCmp;
};
