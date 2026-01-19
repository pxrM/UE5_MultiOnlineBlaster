// Fill out your copyright notice in the Description page of Project Settings.


#include "PImage.h"

UPImage::UPImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPImage::SetBrush(const FSlateBrush& InBrush)
{
	if (Brushes.IsValidIndex(CurrentIndex))
	{
		Brushes[CurrentIndex] = InBrush;
	}
	else
	{
		Brushes.Add(InBrush);
		CurrentIndex = Brushes.Num() - 1;
	}
	Super::SetBrush(InBrush);
}

void UPImage::SetBrushes(const TArray<FSlateBrush>& NewBrushes)
{
	Brushes = NewBrushes;
	CurrentIndex = FMath::Clamp(CurrentIndex, 0, Brushes.Num() - 1);
	SynchronizeProperties();
	if (MyImage.IsValid())
	{
		MyImage->InvalidateImage();
	}
}

void UPImage::SetCurrentIndex(const int32 NewIndex)
{
	if (Brushes.IsEmpty()) return;
	
	const int32 ClampIndex = FMath::Clamp(NewIndex, 0, Brushes.Num() - 1);
	if (CurrentIndex != ClampIndex)
	{
		CurrentIndex = ClampIndex;
		UpdateImageAppearance();
	}
}

void UPImage::UpdateImageAppearance()
{
	if (Brushes.IsValidIndex(CurrentIndex))
	{
		Super::SetBrush(Brushes[CurrentIndex]);
	}
}

void UPImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (Brushes.Num() > 0)
	{
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, Brushes.Num() - 1);
		UpdateImageAppearance();
	}
}

void UPImage::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPImage, CurrentIndex) || PropertyName == GET_MEMBER_NAME_CHECKED(UPImage, Brushes))
	{
		UpdateImageAppearance();
	}
}
