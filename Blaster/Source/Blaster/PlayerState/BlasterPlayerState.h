// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 储存玩家状态
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

public:
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	UFUNCTION()
		virtual void OnRep_Defeats();

private:
	/*
	*
	* class ABlasterCharacter* Character;
	*
		问题抛出：
			在 C++ 中，如果一个指针没有被初始化，那么它的默认值是一个未知的随机地址。因此，在使用一个未初始化的指针时，会出现无法预测的行为和结果。
			在代码中，Character 变量没有显式初始化，因此其值是未知的。如果直接对其进行空指针判断，即 if (Character == nullptr)，那么有可能会得到错误的结果。
		解决办法：
			1.class ABlasterCharacter* Character = nullptr;  初始化为空指针
			2.UPROPERTY()  ue自动初始化为空指针
				UPROPERTY宏用于声明一个类成员变量为ue的属性，其作用是允许该变量被序列化、编辑和保存到磁盘等操作。
				它的主要作用是方便地管理游戏对象的状态和数据，以及提供可视化界面进行修改和调试。
				在运行时ue就会自动初始化 Character 变量，并为其分配内存空间。因此，在后续的代码中直接对 Character 进行空指针判断，而无需担心未初始化的问题。
	*/

	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 Defeats;

};
