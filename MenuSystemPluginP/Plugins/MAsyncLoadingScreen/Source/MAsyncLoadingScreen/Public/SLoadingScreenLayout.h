// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Loading screen�Ļ�������Slate�����ڴ������ؽ��沼�֡�
 */
class MASYNCLOADINGSCREEN_API SLoadingScreenLayout : public SCompoundWidget
{
public:
	/// <summary>
	/// ���ֺŴ�С�ӵ㣨point����λת��ΪSlate����е����ص�λ��ȷ���ڲ�ͬϵͳ����ʾ���ı���Сһ��
	/// ��Slate����У�ʹ��������ת������ʹ���ı��ڲ�ͬDPI�������и��õ���ʾЧ����
	/// </summary>
	/// <param name="PointSize"></param>
	/// <returns></returns>
	static float PointSizeToSlateUnits(float PointSize);

protected:
	/// <summary>
	/// ��ȡ��ǰ Loading Screen �� DPI ���ű���ֵ
	/// </summary>
	/// <returns></returns>
	float GetDPIScale() const;
};
