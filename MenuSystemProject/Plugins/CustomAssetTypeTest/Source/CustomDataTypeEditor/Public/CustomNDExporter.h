// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Exporters/Exporter.h"
#include "CustomNDExporter.generated.h"

/**
 * 导出自定义资产
 */
UCLASS()
class CUSTOMDATATYPEEDITOR_API UCustomNDExporter : public UExporter
{
	GENERATED_BODY()

public:
	UCustomNDExporter();

	virtual bool SupportsObject(UObject* Object) const override;
	virtual bool ExportText(const class FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, uint32 PortFlags/* =0 */) override;
	
};
