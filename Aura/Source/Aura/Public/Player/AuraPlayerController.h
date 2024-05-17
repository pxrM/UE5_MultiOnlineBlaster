// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	
private:
	void Move(const FInputActionValue& InputActionValue);
	void CursorTrace();

	
private:
	// 输入映射上下文 用于定义一组输入映射规则，将物理输入设备的输入映射到虚拟输入轴或按键
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;
	
	// 移动输入动作：输入动作用于绑定玩家输入（例如键盘按键、鼠标点击、手柄按钮等）到游戏中的特定行为或事件
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	TObjectPtr<IEnemyInterface> LastActor;
	TObjectPtr<IEnemyInterface> CurrActor;
};
