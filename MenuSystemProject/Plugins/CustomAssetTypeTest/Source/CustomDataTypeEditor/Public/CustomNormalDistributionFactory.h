// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CustomNormalDistributionFactory.generated.h"

/**
 * UFactory ��һ�������࣬���ڴ����͵����¶���
 * ����һ�������࣬����ֱ��ʵ�����������������ʵ�� FactoryCreateNew �������Ա��� ContentBrowser �д����ʲ���
 */
UCLASS()
class CUSTOMDATATYPEEDITOR_API UCustomNormalDistributionFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UCustomNormalDistributionFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
