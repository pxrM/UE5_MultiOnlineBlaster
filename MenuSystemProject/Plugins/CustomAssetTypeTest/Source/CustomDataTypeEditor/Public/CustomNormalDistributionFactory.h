// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CustomNormalDistributionFactory.generated.h"

/**
 * UFactory 是一个工厂类，用于创建和导入新对象。
 * 它是一个抽象类，不能直接实例化，它的子类必须实现 FactoryCreateNew 函数，以便在 ContentBrowser 中创建资产。
 */
UCLASS()
class CUSTOMDATATYPEEDITOR_API UCustomNormalDistributionFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UCustomNormalDistributionFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
