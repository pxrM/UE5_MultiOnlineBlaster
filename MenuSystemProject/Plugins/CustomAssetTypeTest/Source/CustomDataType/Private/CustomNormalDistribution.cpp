// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNormalDistribution.h"

UCustomNormalDistribution::UCustomNormalDistribution()
	:Mean(0.5f)
	,StandardDeviation(0.2f)
{
}

float UCustomNormalDistribution::DrawSample()
{
	return std::normal_distribution<>(Mean, StandardDeviation)(RandomNumberGenerator);
}

void UCustomNormalDistribution::LogSample()
{
	UE_LOG(LogTemp, Log, TEXT("%f"), DrawSample())
}
