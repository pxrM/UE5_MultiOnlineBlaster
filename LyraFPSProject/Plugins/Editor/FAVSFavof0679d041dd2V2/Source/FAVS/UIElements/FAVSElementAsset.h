// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once
#include "FAVSElement.h"

class FAVS_API FAVSElementAsset : public FAVSElement
{
public:
	virtual TSharedPtr<SButton> AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	virtual void AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	virtual FReply OnEyeClicked() override;
	virtual FReply OpenAsset() override;
	virtual FReply CloseAsset() override;
	FReply ShowInCB();
};
