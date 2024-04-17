// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "CustomNDImportFactory.generated.h"

/**
 * 导入为自定义资产类型
 * 添加一个可导入的后缀名 .cnd，并在 FactoryCanImport() 中进行判断。
 * 假设数据的文件格式与 .ini 文件类似，它具有一些Section，每个Section下有名称不同的参数，且Section之间可能具有同名参数，就像这样：
 *	[MySection]
 *  Mean=0.75
 *  StandardDeviation=0.2
 * 
 *  [OtherSection]
 *  Mean=0.52
 *  Area=0.88
 * 这样的好处是，由于使用了与 .ini 文件同样的格式，可以使用 ConfigCacheIni.h 中的现有的功能来读取文件。当然也可以替换为任意的自定义规则。
 */
UCLASS()
class CUSTOMDATATYPEEDITOR_API UCustomNDImportFactory : public UFactory/*创建新资源的工厂类*/, public FReimportHandler/*处理资源重新导入的结构体*/
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
