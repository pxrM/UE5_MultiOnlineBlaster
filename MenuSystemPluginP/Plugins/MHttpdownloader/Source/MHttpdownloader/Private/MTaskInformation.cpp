// Fill out your copyright notice in the Description page of Project Settings.


#include "MTaskInformation.h"
#include "JsonObjectConverter.h"


bool FMTaskInformation::SerializeToJsonString(FString& OutJsonString) const
{
	return FJsonObjectConverter::UStructToJsonObjectString(this->StaticStruct(), this, OutJsonString, 0, 0);
}

bool FMTaskInformation::DeserializeFromJsonString(const FString& InJsonString)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(InJsonString, this, 0, 0);
}
