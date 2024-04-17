// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDExporter.h"
#include "CustomNormalDistribution.h"

UCustomNDExporter::UCustomNDExporter()
{
	// 指定了该导出器支持的类是 UCustomNormalDistribution
	SupportedClass = UCustomNormalDistribution::StaticClass();
	// 设置了首选格式的索引。如果你的导出器支持多种格式，你可以在这里指定首选格式的索引。
	PreferredFormatIndex = 0;
	// 添加了导出格式的文件扩展名
	FormatExtension.Add(TEXT("cnd"));
	// 导出格式的描述。这个描述将显示在用户界面中，以帮助用户选择适当的导出格式。
	FormatDescription.Add(TEXT("Custom Normal Distribution"));
	// 表示该导出器将以文本格式导出数据。这意味着导出的数据将以文本形式保存，而不是二进制形式。
	bText = true;
}

bool UCustomNDExporter::SupportsObject(UObject* Object) const
{
	return (SupportedClass && Object->IsA(SupportedClass));
}

bool UCustomNDExporter::ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, uint32 PortFlags)
{
	UCustomNormalDistribution* Data = Cast<UCustomNormalDistribution>(Object);
	if (!Data)
	{
		return false;
	}

	// 输出内容
	Ar.Log(TEXT("[MySection]\r\n"));
	Ar.Logf(TEXT("Mean=%f\r\n"), Data->Mean);
	Ar.Logf(TEXT("StandardDeviation=%f"), Data->StandardDeviation);

	return true;
}
