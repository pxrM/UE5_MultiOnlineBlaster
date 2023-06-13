// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * �������״̬
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
		�����׳���
			�� C++ �У����һ��ָ��û�б���ʼ������ô����Ĭ��ֵ��һ��δ֪�������ַ����ˣ���ʹ��һ��δ��ʼ����ָ��ʱ��������޷�Ԥ�����Ϊ�ͽ����
			�ڴ����У�Character ����û����ʽ��ʼ���������ֵ��δ֪�ġ����ֱ�Ӷ�����п�ָ���жϣ��� if (Character == nullptr)����ô�п��ܻ�õ�����Ľ����
		����취��
			1.class ABlasterCharacter* Character = nullptr;  ��ʼ��Ϊ��ָ��
			2.UPROPERTY()  ue�Զ���ʼ��Ϊ��ָ��
				UPROPERTY����������һ�����Ա����Ϊue�����ԣ�������������ñ��������л����༭�ͱ��浽���̵Ȳ�����
				������Ҫ�����Ƿ���ع�����Ϸ�����״̬�����ݣ��Լ��ṩ���ӻ���������޸ĺ͵��ԡ�
				������ʱue�ͻ��Զ���ʼ�� Character ��������Ϊ������ڴ�ռ䡣��ˣ��ں����Ĵ�����ֱ�Ӷ� Character ���п�ָ���жϣ������赣��δ��ʼ�������⡣
	*/

	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 Defeats;

};
