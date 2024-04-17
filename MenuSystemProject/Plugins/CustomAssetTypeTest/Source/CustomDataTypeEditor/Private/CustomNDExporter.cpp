// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDExporter.h"
#include "CustomNormalDistribution.h"

UCustomNDExporter::UCustomNDExporter()
{
	// ָ���˸õ�����֧�ֵ����� UCustomNormalDistribution
	SupportedClass = UCustomNormalDistribution::StaticClass();
	// ��������ѡ��ʽ�������������ĵ�����֧�ֶ��ָ�ʽ�������������ָ����ѡ��ʽ��������
	PreferredFormatIndex = 0;
	// ����˵�����ʽ���ļ���չ��
	FormatExtension.Add(TEXT("cnd"));
	// ������ʽ�������������������ʾ���û������У��԰����û�ѡ���ʵ��ĵ�����ʽ��
	FormatDescription.Add(TEXT("Custom Normal Distribution"));
	// ��ʾ�õ����������ı���ʽ�������ݡ�����ζ�ŵ��������ݽ����ı���ʽ���棬�����Ƕ�������ʽ��
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

	// �������
	Ar.Log(TEXT("[MySection]\r\n"));
	Ar.Logf(TEXT("Mean=%f\r\n"), Data->Mean);
	Ar.Logf(TEXT("StandardDeviation=%f"), Data->StandardDeviation);

	return true;
}
