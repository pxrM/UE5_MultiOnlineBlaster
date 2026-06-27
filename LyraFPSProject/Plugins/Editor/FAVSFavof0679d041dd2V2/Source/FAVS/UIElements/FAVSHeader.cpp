// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSHeader.h"
#include "FAVSDraggableHandle.h"
#include "FAVSSegmentDragDropOp.h"
#include "FAVSSettings.h"
#include "FAVSStyle.h"
#include "SlateOptMacros.h"
#include "Runtime/AppFramework/Public/Widgets/Colors/SColorPicker.h"

#define LOCTEXT_NAMESPACE "FAVS"

class UFAVSSettings;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FAVSHeader::Construct(const FArguments& InArgs)
{
	SegmentID =  InArgs._SegmentID;
	Parent = InArgs._Parent;

	FAVSSegmentData* SegmentData = GetSegmentData();
	PickedColor = SegmentData->HeaderColor;
	const FText Header = SegmentData->SegmentName;
	const FName FontName = "EmbossedText";
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FontName);
	TitleTextFont.Size = GetDefault<UFAVSSettings>()->HeaderFontSize;
	
	ChildSlot
	[

		SNew(SOverlay)

	   
	   + SOverlay::Slot()
	   [
		   SNew(SHorizontalBox)
	        
		   
		   + SHorizontalBox::Slot()
		   .AutoWidth()
		   [
			   SNew(SImage)
			   .Image(FAVSStyle::Get().GetBrush("SegmentBackground_Left"))
		   ]

		  
		   + SHorizontalBox::Slot()
		   .FillWidth(1)
		   [
			   SNew(SImage)
			   .Image(FAVSStyle::Get().GetBrush("SegmentBackground_Center"))
		   ]

		   
		   + SHorizontalBox::Slot()
		   .AutoWidth()
		   [
			   SNew(SImage)
			   .Image(FAVSStyle::Get().GetBrush("SegmentBackground_Right"))
		   ]
	   ]
			
	   
		+ SOverlay::Slot()
		[
			
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			 .AutoWidth().VAlign(VAlign_Center).Padding(7,0,0,0)
			[
			    SNew(FAVSDraggableHandle)
			    .OnMouseButtonDown(this, &FAVSHeader::OnDragHandleMouseDown)
			    .OnDragDetected(this, &FAVSHeader::OnDragDetected)
			    .Cursor(EMouseCursor::GrabHand)
			]
			
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(7,0,0,0)
			[
				SAssignNew(ToggleButton, SButton)
				 .ButtonStyle(GetCurrentStyle())
				 .ButtonColorAndOpacity(FSlateColor(PickedColor))
				 .OnClicked(this, &FAVSHeader::ToggleCollapse)
			]

			+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(7,0,0,0)
			[
				 SAssignNew(HeaderText, SEditableText)
				.Text(Header)
				.Justification(ETextJustify::Left)
				.OnTextCommitted(this, &FAVSHeader::OnHeaderChanged)
				.HintText(NSLOCTEXT("FAVS", "DefaultHeaderHint", "Enter header name"))
				.MinDesiredWidth(100.0f)
				.Font(TitleTextFont)
				.ColorAndOpacity(FSlateColor(PickedColor))
			]

			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,7,0)
			[
				SNew(SButton)
				.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("RemoveBtn"))
				.OnClicked(this, &FAVSHeader::RemoveSegment)
			]
		]
	];
	
	FAVSEvents::OnSettingsChanged.AddSP(this, &FAVSHeader::HandleFontSizeChanged);
	
}
void FAVSHeader::OnHeaderChanged(const FText& NewText, ETextCommit::Type CommitType)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	if (SegmentData && !NewText.EqualTo(SegmentData->SegmentName))
	{
		SegmentData->SegmentName = NewText;
		GetParent()->GetParent()->SaveAsset();
	}
}

FAVSSegment* FAVSHeader::GetParent() const
{
	return Parent;
}

FAVSSegmentData* FAVSHeader::GetSegmentData()
{
	return Parent->GetSegmentData();
}

FReply FAVSHeader::OnDragHandleMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply FAVSHeader::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedRef<FAVSSegmentDragDropOp> DragDropOp = MakeShared<FAVSSegmentDragDropOp>();
	FAVSSegmentData* SegmentData = GetSegmentData();
	DragDropOp->SegmentID = SegmentID;
	DragDropOp->SegmentName = SegmentData->SegmentName;
	DragDropOp->Init();

	return FReply::Handled().BeginDragDrop(DragDropOp);
}



const FButtonStyle* FAVSHeader::GetCurrentStyle()
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	return SegmentData->IsCollapsed ? &FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("ButtonZwiniety") : &FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("ButtonRozwiniety");
}

FReply FAVSHeader::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		
		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			BuildContextMenu(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);
		return FReply::Handled();
	}

	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

void FAVSHeader::HandleFontSizeChanged()
{
	FSlateFontInfo Font = HeaderText->GetFont();
	Font.Size = GetDefault<UFAVSSettings>()->HeaderFontSize;
	HeaderText->SetFont(Font);
}

TSharedRef<SWidget> FAVSHeader::BuildContextMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	bool IsArchived =  GetParent()->GetParent()->IsArchived(SegmentID);

	const FText ChangeColorText = LOCTEXT("ChangeColor","Change color");
	const FText ChangeColorTooltip = LOCTEXT("ChangeColorTooltip","Change color for Header text");
	const FText ArchiveText = LOCTEXT("Archive","Archive");
	const FText ArchiveTextTooltip = LOCTEXT("ArchiveTooltip","Archive this segment");
	const FText UnrchiveText = LOCTEXT("Unarchive","Unarchive");
	const FText UnarchiveTextTooltip = LOCTEXT("UnarchiveTooltip","Unarchive this segment"); 
	
	MenuBuilder.AddMenuEntry(
				ChangeColorText,
				ChangeColorTooltip,
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &FAVSHeader::PickHeaderColor))
	);

	if(IsArchived)
	{
		MenuBuilder.AddMenuEntry(
		UnrchiveText,
			UnarchiveTextTooltip,
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FAVSHeader::OnUnarchiveClicked))
		);
	}else
	{
		MenuBuilder.AddMenuEntry(
		ArchiveText,
			ArchiveTextTooltip,
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FAVSHeader::OnArchiveClicked))
		);
	}
	return MenuBuilder.MakeWidget();
}

void FAVSHeader::OnArchiveClicked()
{
	GetParent()->GetParent()->ArchiveSegment(SegmentID);
}

void FAVSHeader::PickHeaderColor()
{
	FColorPickerArgs PickerArgs;
	PickerArgs.bUseAlpha = false;
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &FAVSHeader::OnColorPicked);
	PickerArgs.OnColorPickerCancelled = FOnLinearColorValueChanged::CreateSP(this, &FAVSHeader::OnColorNotPicked);
	PickerArgs.OnColorPickerWindowClosed = FOnWindowClosed::CreateSP(this, &FAVSHeader::OnColorPickerClosed);
	OpenColorPicker(PickerArgs);
	
}

void FAVSHeader::OnColorPicked(const FLinearColor _PickedColor)
{
	PickedColor = _PickedColor;
	if (HeaderText.IsValid())
	{
		HeaderText->SetColorAndOpacity(PickedColor);
	}
	if (ToggleButton.IsValid())
	{
		ToggleButton->SetBorderBackgroundColor(PickedColor);
	}
		
}

void FAVSHeader::OnColorNotPicked(FLinearColor _PickedColor)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	if (HeaderText.IsValid())
	{
		HeaderText->SetColorAndOpacity(SegmentData->HeaderColor);  //przywroc zapisany kolor
	}
	if (ToggleButton.IsValid())
	{
		ToggleButton->SetBorderBackgroundColor(SegmentData->HeaderColor);
	}
}

void FAVSHeader::OnColorPickerClosed(const TSharedRef<SWindow>& Window)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	SegmentData->HeaderColor = PickedColor;
	GetParent()->GetParent()->SaveAsset();
}

void FAVSHeader::OnUnarchiveClicked()
{
	GetParent()->GetParent()->UnarchiveSegment(SegmentID);
}

FReply FAVSHeader::ToggleCollapse()
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	if(SegmentData->IsCollapsed)
	{
		GetParent()->SetListVisibility(true);
	}
	else
	{
		GetParent()->SetListVisibility(false);
	}
	
	SetToggleBtnStyle();
	
	GetParent()->GetParent()->SaveAsset();
	return FReply::Handled();
}

void FAVSHeader::SetToggleBtnStyle()
{
	const FButtonStyle* NewStyle = GetCurrentStyle();
	if (ToggleButton.IsValid())
	{
		ToggleButton->SetButtonStyle(NewStyle);
	}
}

FReply FAVSHeader::RemoveSegment()
{
	GetParent()->GetParent()->RemoveSegment(SegmentID);
	return FReply::Handled();
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE