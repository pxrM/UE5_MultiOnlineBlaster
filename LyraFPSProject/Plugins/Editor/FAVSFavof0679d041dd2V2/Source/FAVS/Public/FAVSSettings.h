// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSEvents.h"
#include "FAVSSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings)
class FAVS_API UFAVSSettings : public UObject
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, config, Category=Settings, meta=(ClampMin="8", ClampMax="22"))
	int32 HeaderFontSize = 12;

	UPROPERTY(EditAnywhere, config, Category=Settings, meta=(ClampMin="8", ClampMax="22"))
	int32 ElementFontSize = 10;

	UPROPERTY(EditAnywhere, config, Category=Settings, meta=(ClampMin="8", ClampMax="50"))
	int32 ElementHeight = 30;
	
	UPROPERTY(EditAnywhere, config, Category=Settings)
	FLinearColor IconColor = FLinearColor(.036f, .5f, 0, 1.0f);

	UPROPERTY(EditAnywhere, config, Category=Settings)
	bool DisplaySettings = true;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		FName PropertyName = PropertyChangedEvent.GetPropertyName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UFAVSSettings, DisplaySettings))
		{
			FAVSEvents::OnSettingsToggleChanged.Broadcast();
		}
		else FAVSEvents::OnSettingsChanged.Broadcast();
	}

	static void SetSettingsForSegment(bool NewToogle) 
	{
		UFAVSSettings* Settings = GetMutableDefault<UFAVSSettings>();
		if (Settings && Settings->DisplaySettings != NewToogle)
		{
			Settings->DisplaySettings = NewToogle;
			Settings->SaveConfig(); 
		}
	}
			
#endif
};
