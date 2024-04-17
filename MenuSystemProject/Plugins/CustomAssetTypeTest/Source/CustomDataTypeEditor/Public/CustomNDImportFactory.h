// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "CustomNDImportFactory.generated.h"

/**
 * ����Ϊ�Զ����ʲ�����
 * ���һ���ɵ���ĺ�׺�� .cnd������ FactoryCanImport() �н����жϡ�
 * �������ݵ��ļ���ʽ�� .ini �ļ����ƣ�������һЩSection��ÿ��Section�������Ʋ�ͬ�Ĳ�������Section֮����ܾ���ͬ������������������
 *	[MySection]
 *  Mean=0.75
 *  StandardDeviation=0.2
 * 
 *  [OtherSection]
 *  Mean=0.52
 *  Area=0.88
 * �����ĺô��ǣ�����ʹ������ .ini �ļ�ͬ���ĸ�ʽ������ʹ�� ConfigCacheIni.h �е����еĹ�������ȡ�ļ�����ȻҲ�����滻Ϊ������Զ������
 */
UCLASS()
class CUSTOMDATATYPEEDITOR_API UCustomNDImportFactory : public UFactory/*��������Դ�Ĺ�����*/, public FReimportHandler/*������Դ���µ���Ľṹ��*/
{
	GENERATED_BODY()

public:
	UCustomNDImportFactory();

	virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn) override;
	virtual bool FactoryCanImport(const FString& Filename) override;

	FString GetValueFromFile(const TCHAR*& Buffer, FString SectionName, FString VarName);

	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;

};
