// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Loading screen的基础主题Slate，用于创建加载界面布局。
 */
class MASYNCLOADINGSCREEN_API SLoadingScreenLayout : public SCompoundWidget
{
public:
	/// <summary>
	/// 将字号大小从点（point）单位转换为Slate框架中的像素单位，确保在不同系统上显示的文本大小一致
	/// 在Slate框架中，使用这样的转换可以使得文本在不同DPI设置下有更好的显示效果。
	/// </summary>
	/// <param name="PointSize"></param>
	/// <returns></returns>
	static float PointSizeToSlateUnits(float PointSize);

protected:
	/// <summary>
	/// 获取当前 Loading Screen 的 DPI 缩放比例值
	/// </summary>
	/// <returns></returns>
	float GetDPIScale() const;
};
