// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/DataAsset_ProceduralAnim.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"

#if WITH_EDITOR
void UDataAsset_ProceduralAnim::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	//将PropertyChangedEvent默认值设置为1
	if (PropertyChangedEvent.ChangeType== EPropertyChangeType::ArrayAdd)
	{
		// 获取被修改的属性名称
		FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UDataAsset_ProceduralAnim, TargetOffsetRate))
		{
			if(TargetOffsetRate.end()){
				TargetOffsetRate.end()->Value=1.f;
				MarkPackageDirty();
			}
		}
	}
	
}
#endif