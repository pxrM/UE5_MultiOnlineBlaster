// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once
#include "FAVSStructs.h"

DECLARE_MULTICAST_DELEGATE(OnFAVSSettingsChanged);
DECLARE_MULTICAST_DELEGATE_TwoParams(OnSegmentChangedDelegate, const FAVSAssetData& /*AssetData*/, int32 /*SegmentID*/);
class FAVS_API FAVSEvents
{
	
	public:
	static OnSegmentChangedDelegate OnAssetAdded;
	static OnFAVSSettingsChanged OnSettingsChanged;
	static OnFAVSSettingsChanged OnSettingsToggleChanged;
};
