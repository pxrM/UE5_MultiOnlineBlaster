// P

#pragma once

#include "CoreMinimal.h"
#include "Aura/Aura.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/HighlightInterface.h"
#include "Interaction/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;

/**
 * 检查点，在角色接触后，自动保存当前进度。
 */
UCLASS()
class AURA_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp,
								 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 检查点激活后的处理
	 * @param DynamicMaterialInstance 检查点模型的材质实例
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/**
	 * 玩家和检查点碰撞后调用
	 */
	UFUNCTION(BlueprintCallable)
	void HandleGlowEffects();

public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

	/* start IHighlightInterface */
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	/* end IHighlightInterface */

	/* start ISaveInterface */
	virtual bool IsShouldLoadTransform_Implementation() override;
	virtual void LoadActor_Implementation() override;
	/* end ISaveInterface */

private:
	// 检查点显示的模型
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;
	
	// 检查点模型使用的碰撞体
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

	// 点击检查点后，自动移动到检查点位置的组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;

public:
	// SaveGame 作用：标记该变量需要被 序列化（Serialize） 到游戏存档中，支持保存和加载游戏状态。
	// 当前检查点是否已被玩家激活
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	// 修改模型自定义深度的配置
	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

	// 是否需要绑定从重叠事件
	UPROPERTY(EditAnywhere)
	bool bCallOverlapCallback = false;
};
