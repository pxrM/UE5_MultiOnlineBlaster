// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Checkpoint.generated.h"

class USphereComponent;

/**
 * 检查点，在角色接触后，自动保存当前进度。
 */
UCLASS()
class AURA_API ACheckpoint : public APlayerStart
{
	GENERATED_BODY()

public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult);

	/**
	 * 检查点激活后的处理
	 * @param DynamicMaterialInstance 检查点模型的材质实例
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/**
	 * 玩家和检查点碰撞后调用
	 */
	void HandleGlowEffects();

private:
	// 检查点显示的模型
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	// 检查点模型使用的碰撞体
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
