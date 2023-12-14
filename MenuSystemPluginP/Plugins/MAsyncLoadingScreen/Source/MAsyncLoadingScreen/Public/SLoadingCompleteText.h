// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FLoadingCompleteTextSettings;

/**
 *
 */
class MASYNCLOADINGSCREEN_API SLoadingCompleteText : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingCompleteText)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const FLoadingCompleteTextSettings& CompleteTextSettings);

public:
	/// <summary>
	/// ��ȡ��������ı��Ŀɼ���
	/// </summary>
	/// <returns></returns>
	EVisibility GetLoadingCompleteTextVisibility() const;

	/// <summary>
	/// �޸Ķ����ı���͸���ȣ��Ӷ�ʵ���ı���˸��Ч��
	/// </summary>
	/// <param name="InCurrentTime"> ��ǰʱ�� </param>
	/// <param name="InDeltaTime"> ʱ������ </param>
	/// <returns></returns>
	EActiveTimerReturnType AnimateText(double InCurrentTime, float InDeltaTime);

	/// <summary>
	/// ��ȡ��������ı�����ɫ
	/// </summary>
	/// <returns></returns>
	FORCEINLINE FSlateColor GetLoadingCompleteTextColor() const { return CompleteTextColor; };

private:
	/// <summary>
	/// ���سɹ��ı���ɫ
	/// </summary>
	FLinearColor CompleteTextColor = FLinearColor::White;

	/// <summary>
	/// �Ƿ���Ҫ��ת����
	/// </summary>
	bool bCompleteTextReverseAnim = false;

	/// <summary>
	/// �ı������ٶ�
	/// </summary>
	float CompleteTextAnimationSpeed = 1.0f;

	/// <summary>
	/// �Ƿ�ע���˼�ʱ��
	/// </summary>
	bool bIsActiveTimerRegsitered = false;
};
