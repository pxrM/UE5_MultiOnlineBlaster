// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSElement.h"
#include "ContentBrowserModule.h"
#include "FAVSDraggableHandle.h"
#include "FAVSElementDragDropOp.h"
#include "FAVSSettings.h"
#include "FAVSStyle.h"
#include "IContentBrowserSingleton.h"
#include "Styling/SlateIconFinder.h"


class FContentBrowserModule;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION



void FAVSElement::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;
	SegmentID = InArgs._SegmentID;
	Parent = InArgs._Parent;

	const FString DisplayName = Asset.DisplayName;
	AssetName = FName(*DisplayName);

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle("EmbossedText");
	TitleTextFont.Size = GetDefault<UFAVSSettings>()->ElementFontSize;
	
		
	const FSlateBrush* IconBrush = GetIcon();
	const FSlateBrush* PanelBrush = FAppStyle::Get().GetBrush("Brushes.Panel");
	FLinearColor DetailsBackgroundColor = PanelBrush->GetTint(FWidgetStyle());
	
	TopSpacer = SNew(SBox).HeightOverride(0)
	[
		SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.ColorAndOpacity(DetailsBackgroundColor)
	];
	
	BottomSpacer = SNew(SBox).HeightOverride(0)
	[
		SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.ColorAndOpacity(DetailsBackgroundColor)
	];
	
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth().Padding(12,0,0,0).VAlign(VAlign_Center)
		[
			SNew(FAVSDraggableHandle)
			.OnMouseButtonDown(this, &FAVSElement::OnDragHandleMouseDown)
			.OnDragDetected(this, &FAVSElement::OnDragDetected)
			.Cursor(EMouseCursor::GrabHand)
		]
	
		+ SHorizontalBox::Slot()
		.AutoWidth().Padding(15,0,0,0).VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(IconBrush)
			.ColorAndOpacity(FLinearColor(Asset.IconColor) * (Asset.IsValid ? 1.0f : 0.5f))
		]
				
		+ SHorizontalBox::Slot()
		.AutoWidth().FillWidth(1).Padding(7,0,0,0).VAlign(VAlign_Center)
		[
			SAssignNew(ElementText, STextBlock) 
			.Text(FText::FromName(AssetName))
			.Justification(ETextJustify::Left)
			.MinDesiredWidth(200.0f)
			.Font(TitleTextFont)
			.OnDoubleClicked(this, &FAVSElement::OnElementDoubleClicked)
			.IsEnabled(Asset.IsValid)
		];

	OpenAssetBtn = AddOpenAssetBtn(HorizontalBox);
	AddCBBtn(HorizontalBox);
			
	if(!Asset.IsValid){
		HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)
		[
			SNew(SImage).Image(FAVSStyle::Get().GetBrush("WrongAsset"))
		];
		
	}
	
	AddRemoveBtn(HorizontalBox);
		
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			TopSpacer.ToSharedRef()
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ElementBox, SBox)
			.HeightOverride(GetDefault<UFAVSSettings>()->ElementHeight)
			[
				HorizontalBox
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		        
			  + SHorizontalBox::Slot()
			  .AutoWidth()
			  [
				  SNew(SImage)
				  .Image(FAVSStyle::Get().GetBrush("ElementStrip_Left"))
			  ]

			  + SHorizontalBox::Slot()
			  .FillWidth(1)
			  [
				  SNew(SImage)
				  .Image(FAVSStyle::Get().GetBrush("ElementStrip_Center"))
			  ]

			  + SHorizontalBox::Slot()
			  .AutoWidth()
			  [
				  SNew(SImage)
				  .Image(FAVSStyle::Get().GetBrush("ElementStrip_Right"))
			  ]
		]
		
		+ SVerticalBox::Slot().AutoHeight()
	    [
    		BottomSpacer.ToSharedRef()
	    ]
	];
	
	FAVSEvents::OnSettingsChanged.AddSP(this, &FAVSElement::HandleFontSizeChanged);

}

void FAVSElement::AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
}

void FAVSElement::AddRemoveBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
	_HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,7,0)
	[
		SNew(SButton)
		.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("RemoveBtn"))
		.OnClicked(this, &FAVSElement::RemoveElement)
	];
}

void FAVSElement::HandleFontSizeChanged()
{
	if(ElementText.IsValid())
	{
		FSlateFontInfo Font = ElementText->GetFont();
		Font.Size = GetDefault<UFAVSSettings>()->ElementFontSize;
		ElementText->SetFont(Font);
	}
	if(ElementBox.IsValid())
	{
		ElementBox->SetHeightOverride(GetDefault<UFAVSSettings>()->ElementHeight);
	}
}

const FSlateBrush* FAVSElement::GetIcon()
{
	const UClass* IconClass = Asset.AssetClass;
	
	FSlateIcon SlateIcon = FSlateIconFinder::FindIconForClass(IconClass);
	const FSlateBrush* IconBrush = SlateIcon.GetIcon();
		
	if (Asset.AssetType == EAssetType::Folder)
	{
		IconBrush = FAppStyle::Get().GetBrush("ContentBrowser.AssetTreeFolderClosed");
	}
	else if (Asset.AssetType == EAssetType::Settings)
	{
		IconBrush = FAppStyle::Get().GetBrush("Icons.Settings");
	}
	else if (IconClass)
	{
		IconBrush = FSlateIconFinder::FindIconForClass(IconClass).GetIcon();
	}

	if (!IconBrush)
	{
		IconBrush = FCoreStyle::Get().GetDefaultBrush();
	}

	return IconBrush;
}

void FAVSElement::SetPaddingWhenDragOver(const int32 TopPadding, const int32 BottomPadding)
{
	TopSpacer->SetHeightOverride(TopPadding);
	BottomSpacer->SetHeightOverride(BottomPadding);	
}

bool FAVSElement::IsLevel() const
{
	return Asset.AssetType == EAssetType::Level;
}

FAVSSegment* FAVSElement::GetParent()
{
	return Parent;
}

FReply FAVSElement::OnElementDoubleClicked(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	OpenAsset();
	return FReply::Handled();
}

void FAVSElement::UpdateButtonStyle(const bool _AssetIsOpened)
{
	if (OpenAssetBtn.IsValid())
	{
		const FButtonStyle* StyleToUse = _AssetIsOpened 
			? &FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenAssetBtn_Opened")
			: &FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenAssetBtn");

		AssetIsOpened = _AssetIsOpened;
		OpenAssetBtn->SetButtonStyle(StyleToUse);
	}
	
}

void FAVSElement::UpdateWidget(const FAVSAssetData& _Asset)
{
	Asset = _Asset;
	AssetName = FName(*_Asset.DisplayName);
	ElementText->SetText(FText::FromName(AssetName));
}

FReply FAVSElement::RemoveElement()
{
	FAVSEvents::OnSettingsChanged.RemoveAll(this);
	
	GetParent()->RemoveElement(Asset.AssetPath);
	GetParent()->GetParent()->SaveAsset();
	return FReply::Handled();
}

FReply FAVSElement::OnDragHandleMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply FAVSElement::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedRef<FAVSElementDragDropOp> DragDropOp = MakeShared<FAVSElementDragDropOp>();
	DragDropOp->Asset = Asset;
	DragDropOp->AssetName = AssetName;
	DragDropOp->SegmentID = SegmentID;
	DragDropOp->SourceElement = SharedThis(this);
	DragDropOp->Init();
	
	GetParent()->GetParent()->SetPendingElement(FPendingElement(SegmentID, Asset));

	GetParent()->RemoveElement(Asset.AssetPath);  //todo tu mialem blad 
	GetParent()->GetParent()->SaveAsset();
	return FReply::Handled().BeginDragDrop(DragDropOp);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
