// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNormalDistributionFactory.h"
#include "CustomNormalDistribution.h"

UCustomNormalDistributionFactory::UCustomNormalDistributionFactory()
{
	// 指定这个工厂生产的类
	SupportedClass = UCustomNormalDistribution::StaticClass();
	bCreateNew = true;
}

UObject* UCustomNormalDistributionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCustomNormalDistribution>(InParent, InClass, InName, Flags, Context);
}
