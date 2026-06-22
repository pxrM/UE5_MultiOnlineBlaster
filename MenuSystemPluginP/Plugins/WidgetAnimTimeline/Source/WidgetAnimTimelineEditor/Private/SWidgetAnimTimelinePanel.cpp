#include "SWidgetAnimTimelinePanel.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyHandle.h"
#include "Rendering/DrawElements.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"
#include "WidgetAnimTimelineDesignerPreviewController.h"
#include "WidgetAnimTimelineSequence.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

namespace WidgetAnimTimelinePanelConstants
{
	static constexpr float MinPixelsPerSecond = 60.0f;
	static constexpr float MaxPixelsPerSecond = 420.0f;
	static constexpr float BlockMinWidth = 38.0f;
	static constexpr float MajorGridAlpha = 0.34f;
	static constexpr float MinorGridAlpha = 0.11f;

	static bool IsTimeOnStep(float Time, float Step)
	{
		return FMath::IsNearlyZero(FMath::Fmod(Time + KINDA_SMALL_NUMBER, Step), 0.001f);
	}

	static FString FormatRulerTime(float Time)
	{
		if (FMath::IsNearlyEqual(Time, FMath::RoundToFloat(Time), 0.001f))
		{
			return FString::Printf(TEXT("%ds"), FMath::RoundToInt(Time));
		}

		FString Text = FString::Printf(TEXT("%.2fs"), Time);
		Text.ReplaceInline(TEXT("0s"), TEXT("s"));
		return Text;
	}
}

void SWidgetAnimTimelinePanel::Construct(const FArguments& InArgs)
{
	PhaseHandle = InArgs._PhaseHandle;
	SourceWidgetBlueprint = InArgs._WidgetBlueprint;
	PhaseIndex = InArgs._PhaseIndex;
	EntryTypeOptions =
	{
		MakeShared<EWidgetAnimTimelineEntryType>(EWidgetAnimTimelineEntryType::DirectAnimation),
		MakeShared<EWidgetAnimTimelineEntryType>(EWidgetAnimTimelineEntryType::ChildSequencePhase)
	};
	RefreshPhaseOptions();
	RefreshEntries();

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBox)
			.MinDesiredHeight(260.0f)
			.MinDesiredWidth(720.0f)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(8.0f, 6.0f, 0.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Phase")))
					.ColorAndOpacity(FLinearColor(0.72f, 0.72f, 0.72f, 1.0f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SComboBox<TSharedPtr<int32>>)
					.ContentPadding(FMargin(6.0f, 2.0f))
					.OptionsSource(&PhaseOptions)
					.InitiallySelectedItem(SelectedPhaseOption)
					.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakePhaseOptionWidget)
					.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnPhaseSelectionChanged)
					[
						SNew(STextBlock)
						.Text(this, &SWidgetAnimTimelinePanel::GetSelectedPhaseText)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(128.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SWidgetAnimTimelinePanel::GetSelectedPhaseNameEditText)
						.IsEnabled(this, &SWidgetAnimTimelinePanel::CanEditPhases)
						.OnTextCommitted(this, &SWidgetAnimTimelinePanel::OnSelectedPhaseNameCommitted)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("New")))
					.IsEnabled(this, &SWidgetAnimTimelinePanel::CanEditPhases)
					.OnClicked(this, &SWidgetAnimTimelinePanel::AddPhase)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Duplicate")))
					.IsEnabled(this, &SWidgetAnimTimelinePanel::CanDuplicatePhase)
					.OnClicked(this, &SWidgetAnimTimelinePanel::DuplicatePhase)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Delete Phase")))
					.IsEnabled(this, &SWidgetAnimTimelinePanel::CanDeletePhase)
					.OnClicked(this, &SWidgetAnimTimelinePanel::DeletePhase)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(12.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("AutoPlay")))
					.ColorAndOpacity(FLinearColor(0.72f, 0.72f, 0.72f, 1.0f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(120.0f)
					[
						SNew(SComboBox<TSharedPtr<FName>>)
						.ContentPadding(FMargin(6.0f, 2.0f))
						.OptionsSource(&AutoPlayOptions)
						.OnComboBoxOpening(this, &SWidgetAnimTimelinePanel::OnAutoPlayOptionsOpening)
						.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakeAutoPlayOptionWidget)
						.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnAutoPlayChanged)
						[
							SNew(STextBlock)
							.Text(this, &SWidgetAnimTimelinePanel::GetSelectedAutoPlayText)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.MinWidth(140.0f)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(FText::FromString(TEXT("Preview In Designer")))
					.OnClicked(this, &SWidgetAnimTimelinePanel::PlayDesignerPreview)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SWidgetAnimTimelinePanel::GetBlueprintCompileStatusText)
					.ColorAndOpacity(this, &SWidgetAnimTimelinePanel::GetBlueprintCompileStatusColor)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Add Direct")))
					.OnClicked(this, &SWidgetAnimTimelinePanel::AddDirectAnimationEntry)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Add Child")))
					.OnClicked(this, &SWidgetAnimTimelinePanel::AddChildSequenceEntry)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Delete Entry")))
					.IsEnabled(this, &SWidgetAnimTimelinePanel::CanDeleteSelectedEntry)
					.OnClicked(this, &SWidgetAnimTimelinePanel::DeleteSelectedEntry)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Fit")))
					.OnClicked(this, &SWidgetAnimTimelinePanel::FitTimelineToContent)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(12.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Drag: edit StartTime    Right-drag: pan    MouseWheel: zoom    Shift: 0.01s snap")))
					.ColorAndOpacity(FLinearColor(0.52f, 0.52f, 0.52f, 1.0f))
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(0.0f, 104.0f, 8.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(280.0f)
			.Visibility(this, &SWidgetAnimTimelinePanel::GetEntryInspectorVisibility)
			[
				SNew(SBorder)
				.Padding(10.0f)
				.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(STextBlock)
						.Text(this, &SWidgetAnimTimelinePanel::GetSelectedEntryTitleText)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("EntryType")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SComboBox<TSharedPtr<EWidgetAnimTimelineEntryType>>)
						.OptionsSource(&EntryTypeOptions)
						.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakeEntryTypeOptionWidget)
						.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnSelectedEntryTypeChanged)
						[
							SNew(STextBlock)
							.Text(this, &SWidgetAnimTimelinePanel::GetSelectedEntryTypeText)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("TargetWidgetName")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SComboBox<TSharedPtr<FName>>)
						.OptionsSource(&TargetOptions)
						.OnComboBoxOpening(this, &SWidgetAnimTimelinePanel::OnTargetOptionsOpening)
						.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakeNameOptionWidget)
						.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnSelectedEntryTargetChanged)
						[
							SNew(STextBlock)
							.Text(this, &SWidgetAnimTimelinePanel::GetSelectedEntryTargetText)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("AnimationName")))
						.Visibility(this, &SWidgetAnimTimelinePanel::GetAnimationFieldVisibility)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SComboBox<TSharedPtr<FName>>)
						.Visibility(this, &SWidgetAnimTimelinePanel::GetAnimationFieldVisibility)
						.OptionsSource(&AnimationOptions)
						.OnComboBoxOpening(this, &SWidgetAnimTimelinePanel::OnAnimationOptionsOpening)
						.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakeNameOptionWidget)
						.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnSelectedEntryAnimationChanged)
						[
							SNew(STextBlock)
							.Text(this, &SWidgetAnimTimelinePanel::GetSelectedEntryAnimationText)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("ChildPhaseName")))
						.Visibility(this, &SWidgetAnimTimelinePanel::GetChildPhaseFieldVisibility)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SComboBox<TSharedPtr<FName>>)
						.Visibility(this, &SWidgetAnimTimelinePanel::GetChildPhaseFieldVisibility)
						.OptionsSource(&ChildPhaseOptions)
						.OnComboBoxOpening(this, &SWidgetAnimTimelinePanel::OnChildPhaseOptionsOpening)
						.OnGenerateWidget(this, &SWidgetAnimTimelinePanel::MakeNameOptionWidget)
						.OnSelectionChanged(this, &SWidgetAnimTimelinePanel::OnSelectedEntryChildPhaseChanged)
						[
							SNew(STextBlock)
							.Text(this, &SWidgetAnimTimelinePanel::GetSelectedEntryChildPhaseText)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SNumericEntryBox<float>)
						.LabelVAlign(VAlign_Center)
						.Label()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("StartTime")))
						]
						.MinValue(0.0f)
						.MinSliderValue(0.0f)
						.Value(this, &SWidgetAnimTimelinePanel::GetSelectedEntryStartTime)
						.OnValueCommitted(this, &SWidgetAnimTimelinePanel::OnSelectedEntryStartTimeCommitted)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SNumericEntryBox<float>)
						.LabelVAlign(VAlign_Center)
						.Label()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("PlaybackRate")))
						]
						.MinValue(0.001f)
						.MinSliderValue(0.1f)
						.Value(this, &SWidgetAnimTimelinePanel::GetSelectedEntryPlaybackRate)
						.OnValueCommitted(this, &SWidgetAnimTimelinePanel::OnSelectedEntryPlaybackRateCommitted)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SNumericEntryBox<int32>)
						.LabelVAlign(VAlign_Center)
						.Label()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Loops (0=Inf)")))
						]
						.MinValue(0)
						.MinSliderValue(0)
						.Value(this, &SWidgetAnimTimelinePanel::GetSelectedEntryNumLoops)
						.OnValueCommitted(this, &SWidgetAnimTimelinePanel::OnSelectedEntryNumLoopsCommitted)
					]
				]
			]
		]
	];
}

void SWidgetAnimTimelinePanel::RefreshEntries()
{
	Entries.Reset();
	LaneNames.Reset();

	TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle();
	if (!EntriesHandle.IsValid())
	{
		FWidgetAnimTimelinePhase* Phase = GetTimelinePhase();
		if (Phase == nullptr)
		{
			return;
		}

		for (int32 EntryIndex = 0; EntryIndex < Phase->Entries.Num(); ++EntryIndex)
		{
			const FWidgetAnimTimelineEntry& Entry = Phase->Entries[EntryIndex];

			FEntryViewModel ViewModel;
			ViewModel.EntryIndex = EntryIndex;
			ViewModel.LaneName = Entry.TargetWidgetName.IsNone() ? TEXT("Self") : Entry.TargetWidgetName.ToString();
			ViewModel.StartTime = Entry.StartTime;
			ViewModel.Duration = GetEntryDuration(Entry.TargetWidgetName, Entry.EntryType, Entry.AnimationName, Entry.PlaybackRate, Entry.NumLoopsToPlay);
			ViewModel.ValidationError = BuildEntryValidationError(Entry.TargetWidgetName, Entry.EntryType, Entry.AnimationName, Entry.ChildPhaseName);
			if (Entry.EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
			{
				ViewModel.Label = Entry.AnimationName.IsNone() ? TEXT("Animation") : Entry.AnimationName.ToString();
				if (Entry.NumLoopsToPlay == 0)
				{
					ViewModel.Label += TEXT(" (Infinite)");
				}
				ViewModel.Color = FLinearColor(0.12f, 0.48f, 0.95f, 1.0f);
			}
			else
			{
				ViewModel.Label = Entry.ChildPhaseName.IsNone() ? TEXT("Child Phase") : FString::Printf(TEXT("Phase: %s"), *Entry.ChildPhaseName.ToString());
				ViewModel.Color = FLinearColor(0.22f, 0.7f, 0.36f, 1.0f);
			}

			Entries.Add(ViewModel);
			LaneNames.AddUnique(ViewModel.LaneName);
		}

		return;
	}

	uint32 EntryCount = 0;
	EntriesHandle->GetNumChildren(EntryCount);

	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
	{
		TSharedPtr<IPropertyHandle> EntryHandle = EntriesHandle->GetChildHandle(EntryIndex);
		if (!EntryHandle.IsValid())
		{
			continue;
		}

		TSharedPtr<IPropertyHandle> TargetHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName));
		TSharedPtr<IPropertyHandle> TypeHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType));
		TSharedPtr<IPropertyHandle> AnimationHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName));
		TSharedPtr<IPropertyHandle> ChildPhaseHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName));
		TSharedPtr<IPropertyHandle> StartHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime));
		TSharedPtr<IPropertyHandle> RateHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate));

		FName TargetName = NAME_None;
		FName AnimationName = NAME_None;
		FName ChildPhaseName = NAME_None;
		uint8 TypeValue = 0;
		float StartTime = 0.0f;
		float PlaybackRate = 1.0f;

		if (TargetHandle.IsValid())
		{
			TargetHandle->GetValue(TargetName);
		}
		if (TypeHandle.IsValid())
		{
			TypeHandle->GetValue(TypeValue);
		}
		if (AnimationHandle.IsValid())
		{
			AnimationHandle->GetValue(AnimationName);
		}
		if (ChildPhaseHandle.IsValid())
		{
			ChildPhaseHandle->GetValue(ChildPhaseName);
		}
		if (StartHandle.IsValid())
		{
			StartHandle->GetValue(StartTime);
		}
		if (RateHandle.IsValid())
		{
			RateHandle->GetValue(PlaybackRate);
		}
		int32 NumLoopsToPlay = 1;
		if (TSharedPtr<IPropertyHandle> LoopsHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay)))
		{
			LoopsHandle->GetValue(NumLoopsToPlay);
		}

		FEntryViewModel ViewModel;
		ViewModel.EntryIndex = static_cast<int32>(EntryIndex);
		ViewModel.LaneName = TargetName.IsNone() ? TEXT("Self") : TargetName.ToString();
		ViewModel.StartTime = StartTime;

		const EWidgetAnimTimelineEntryType EntryType = static_cast<EWidgetAnimTimelineEntryType>(TypeValue);
		ViewModel.Duration = GetEntryDuration(TargetName, EntryType, AnimationName, PlaybackRate, NumLoopsToPlay);
		ViewModel.ValidationError = BuildEntryValidationError(TargetName, EntryType, AnimationName, ChildPhaseName);
		if (EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
		{
			ViewModel.Label = AnimationName.IsNone() ? TEXT("Animation") : AnimationName.ToString();
			if (NumLoopsToPlay == 0)
			{
				ViewModel.Label += TEXT(" (Infinite)");
			}
			ViewModel.Color = FLinearColor(0.12f, 0.48f, 0.95f, 1.0f);
		}
		else
		{
			ViewModel.Label = ChildPhaseName.IsNone() ? TEXT("Child Phase") : FString::Printf(TEXT("Phase: %s"), *ChildPhaseName.ToString());
			ViewModel.Color = FLinearColor(0.22f, 0.7f, 0.36f, 1.0f);
		}

		Entries.Add(ViewModel);
		LaneNames.AddUnique(ViewModel.LaneName);
	}
}

void SWidgetAnimTimelinePanel::RefreshPhaseOptions()
{
	PhaseOptions.Reset();
	SelectedPhaseOption.Reset();

	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < Config->Phases.Num(); ++Index)
	{
		TSharedPtr<int32> Option = MakeShared<int32>(Index);
		PhaseOptions.Add(Option);
		if (Index == PhaseIndex)
		{
			SelectedPhaseOption = Option;
		}
	}

	if (!SelectedPhaseOption.IsValid() && PhaseOptions.Num() > 0)
	{
		SelectedPhaseOption = PhaseOptions[0];
		PhaseIndex = *SelectedPhaseOption;
	}

	RefreshAutoPlayOptions();
}

void SWidgetAnimTimelinePanel::RefreshAutoPlayOptions()
{
	AutoPlayOptions.Reset();
	AutoPlayOptions.Add(MakeShared<FName>(NAME_None));

	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr)
	{
		return;
	}

	TSet<FName> AddedNames;
	AddedNames.Add(NAME_None);
	for (const FWidgetAnimTimelinePhase& Phase : Config->Phases)
	{
		if (Phase.PhaseName.IsNone() || AddedNames.Contains(Phase.PhaseName))
		{
			continue;
		}

		AddedNames.Add(Phase.PhaseName);
		AutoPlayOptions.Add(MakeShared<FName>(Phase.PhaseName));
	}
}

int32 SWidgetAnimTimelinePanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 CurrentLayer = LayerId;

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SmallFont = FAppStyle::GetFontStyle(TEXT("SmallFont"));
	const float Duration = GetTimelineDuration();
	const int32 LaneCount = LaneNames.Num();
	const float TimelineWidth = GetTimelineWidth(AllottedGeometry);
	LastTimelineWidth = TimelineWidth;
	const float TimelineRight = HeaderWidth + TimelineWidth;
	const float LaneAreaBottom = FMath::Max(Size.Y - 8.0f, TimelineTop + LaneCount * LaneHeight);

	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 1, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, Size.Y), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.018f, 0.018f, 0.018f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 2, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, TimelineTop), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.028f, 0.028f, 0.03f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 3, AllottedGeometry.ToPaintGeometry(FVector2f(HeaderWidth, Size.Y), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.032f, 0.032f, 0.034f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 4, AllottedGeometry.ToPaintGeometry(FVector2f(TimelineWidth, RulerHeight), FSlateLayoutTransform(FVector2f(HeaderWidth, TimelineTop - RulerHeight))), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.055f, 0.055f, 0.06f, 1.0f));

	const float MinorStep = PixelsPerSecond >= 180.0f ? 0.1f : 0.25f;
	// Labels become denser after zooming so sub-second edits still show concrete ruler time.
	const float LabelStep = PixelsPerSecond >= 300.0f ? 0.1f : PixelsPerSecond >= 180.0f ? 0.25f : PixelsPerSecond >= 100.0f ? 0.5f : 1.0f;
	const float VisibleEndTime = ViewStartTime + (TimelineWidth / FMath::Max(PixelsPerSecond, KINDA_SMALL_NUMBER));
	const float FirstTickTime = FMath::Max(0.0f, FMath::GridSnap(ViewStartTime, MinorStep) - MinorStep);
	for (float Time = FirstTickTime; Time <= FMath::Min(Duration, VisibleEndTime + MinorStep) + KINDA_SMALL_NUMBER; Time += MinorStep)
	{
		const bool bIsMajor = FMath::IsNearlyZero(FMath::Fmod(Time, 1.0f), 0.001f);
		const bool bIsHalf = !bIsMajor && FMath::IsNearlyZero(FMath::Fmod(Time, 0.5f), 0.001f);
		const bool bDrawLabel = WidgetAnimTimelinePanelConstants::IsTimeOnStep(Time, LabelStep);
		if (!bIsMajor && !bIsHalf && PixelsPerSecond < 180.0f)
		{
			continue;
		}

		const float X = TimeToX(Time, AllottedGeometry);
		if (X < HeaderWidth || X > TimelineRight)
		{
			continue;
		}

		const float TickHeight = bIsMajor ? 11.0f : 6.0f;
		const FLinearColor LineColor = bIsMajor ? FLinearColor(0.45f, 0.45f, 0.45f, WidgetAnimTimelinePanelConstants::MajorGridAlpha) : FLinearColor(0.38f, 0.38f, 0.38f, WidgetAnimTimelinePanelConstants::MinorGridAlpha);
		const TArray<FVector2D> GridLine = { FVector2D(X, TimelineTop - RulerHeight + TickHeight), FVector2D(X, LaneAreaBottom) };
		FSlateDrawElement::MakeLines(OutDrawElements, CurrentLayer + 6, AllottedGeometry.ToPaintGeometry(), GridLine, ESlateDrawEffect::None, LineColor, true, bIsMajor ? 1.0f : 0.5f);

		const TArray<FVector2D> RulerTick = { FVector2D(X, TimelineTop - TickHeight), FVector2D(X, TimelineTop) };
		FSlateDrawElement::MakeLines(OutDrawElements, CurrentLayer + 7, AllottedGeometry.ToPaintGeometry(), RulerTick, ESlateDrawEffect::None, FLinearColor(0.64f, 0.64f, 0.64f, bIsMajor ? 0.78f : 0.28f), true, 1.0f);

		if (bDrawLabel)
		{
			const FLinearColor LabelColor = bIsMajor ? FLinearColor(0.78f, 0.78f, 0.78f, 1.0f) : FLinearColor(0.62f, 0.62f, 0.62f, 0.88f);
			FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 8, AllottedGeometry.ToPaintGeometry(FVector2f(48.0f, 16.0f), FSlateLayoutTransform(FVector2f(X + 4.0f, TimelineTop - RulerHeight + 2.0f))), FText::FromString(WidgetAnimTimelinePanelConstants::FormatRulerTime(Time)), SmallFont, ESlateDrawEffect::None, LabelColor);
		}
	}

	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		const float Y = TimelineTop + LaneIndex * LaneHeight;
		const FLinearColor LaneColor = LaneIndex % 2 == 0 ? FLinearColor(0.042f, 0.042f, 0.044f, 1.0f) : FLinearColor(0.06f, 0.06f, 0.064f, 1.0f);
		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 9, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, LaneHeight - 1.0f), FSlateLayoutTransform(FVector2f(0.0f, Y))), WhiteBrush, ESlateDrawEffect::None, LaneColor);
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 10, AllottedGeometry.ToPaintGeometry(FVector2f(HeaderWidth - 16.0f, 18.0f), FSlateLayoutTransform(FVector2f(10.0f, Y + 9.0f))), FText::FromString(LaneNames[LaneIndex]), SmallFont, ESlateDrawEffect::None, FLinearColor(0.82f, 0.82f, 0.82f, 1.0f));
	}

	for (const FEntryViewModel& Entry : Entries)
	{
		const int32 LaneIndex = GetLaneIndex(Entry.LaneName);
		if (LaneIndex == INDEX_NONE)
		{
			continue;
		}

		const float Y = TimelineTop + LaneIndex * LaneHeight;
		const float X = TimeToX(Entry.StartTime, AllottedGeometry);
		const float Width = FMath::Max(WidgetAnimTimelinePanelConstants::BlockMinWidth, Entry.Duration * PixelsPerSecond);
		const bool bSelected = Entry.EntryIndex == SelectedEntryIndex;
		const bool bActive = bSelected || Entry.EntryIndex == DraggingEntryIndex || Entry.EntryIndex == HoveredEntryIndex;
		const bool bHasValidationError = !Entry.ValidationError.IsEmpty();
		const FLinearColor ErrorColor(1.0f, 0.14f, 0.08f, 1.0f);
		const FLinearColor BlockColor = bHasValidationError ? FLinearColor(0.42f, 0.06f, 0.04f, bActive ? 1.0f : 0.92f) : (bActive ? Entry.Color.CopyWithNewOpacity(1.0f) : Entry.Color.CopyWithNewOpacity(0.82f));
		const FLinearColor OutlineColor = bHasValidationError ? ErrorColor : (bSelected ? FLinearColor(1.0f, 0.84f, 0.22f, 1.0f) : (bActive ? FLinearColor(1.0f, 1.0f, 1.0f, 0.62f) : Entry.Color.CopyWithNewOpacity(0.36f)));

		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 12, AllottedGeometry.ToPaintGeometry(FVector2f(Width, LaneHeight - 10.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, BlockColor);
		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(Width, bSelected ? 2.0f : 1.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, OutlineColor);
		if (bHasValidationError)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(Width, 1.0f), FSlateLayoutTransform(FVector2f(X, Y + LaneHeight - 6.0f))), WhiteBrush, ESlateDrawEffect::None, OutlineColor);
			FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(4.0f, LaneHeight - 10.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, ErrorColor);
		}
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 14, AllottedGeometry.ToPaintGeometry(FVector2f(Width - 10.0f, 18.0f), FSlateLayoutTransform(FVector2f(X + 7.0f, Y + 7.0f))), FText::FromString(Entry.Label), SmallFont, ESlateDrawEffect::None, FLinearColor::White);
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 14, AllottedGeometry.ToPaintGeometry(FVector2f(Width - 10.0f, 14.0f), FSlateLayoutTransform(FVector2f(X + 7.0f, Y + 21.0f))), FText::FromString(bHasValidationError ? Entry.ValidationError : FormatTime(Entry.StartTime)), SmallFont, ESlateDrawEffect::None, bHasValidationError ? FLinearColor(1.0f, 0.82f, 0.76f, 1.0f) : FLinearColor(0.82f, 0.9f, 1.0f, 0.82f));
	}

	const int32 ChildLayer = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, CurrentLayer + 15, InWidgetStyle, bParentEnabled);
	return FMath::Max(CurrentLayer + 15, ChildLayer);
}

FWidgetAnimTimelineConfig* SWidgetAnimTimelinePanel::GetTimelineConfig() const
{
	if (!SourceWidgetBlueprint.IsValid())
	{
		return nullptr;
	}

	UClass* OwnerClass = ResolveOwnerWidgetClass();
	if (OwnerClass == nullptr)
	{
		return nullptr;
	}

	UObject* DefaultObject = OwnerClass->GetDefaultObject();
	if (DefaultObject == nullptr)
	{
		return nullptr;
	}

	for (TFieldIterator<FStructProperty> It(OwnerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->Struct == FWidgetAnimTimelineConfig::StaticStruct())
		{
			return It->ContainerPtrToValuePtr<FWidgetAnimTimelineConfig>(DefaultObject);
		}
	}

	return nullptr;
}

FWidgetAnimTimelinePhase* SWidgetAnimTimelinePanel::GetTimelinePhase() const
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return nullptr;
	}

	return &Config->Phases[PhaseIndex];
}

bool SWidgetAnimTimelinePanel::CanEditPhases() const
{
	return GetTimelineConfig() != nullptr;
}

FName SWidgetAnimTimelinePanel::MakeUniquePhaseName(const FString& BaseName) const
{
	FString CleanBaseName = BaseName.TrimStartAndEnd();
	if (CleanBaseName.IsEmpty())
	{
		CleanBaseName = TEXT("Phase");
	}
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr)
	{
		return FName(*CleanBaseName);
	}

	TSet<FName> ExistingNames;
	for (const FWidgetAnimTimelinePhase& Phase : Config->Phases)
	{
		if (!Phase.PhaseName.IsNone())
		{
			ExistingNames.Add(Phase.PhaseName);
		}
	}

	FName CandidateName(*CleanBaseName);
	for (int32 Suffix = 1; ExistingNames.Contains(CandidateName); ++Suffix)
	{
		CandidateName = FName(*FString::Printf(TEXT("%s_%d"), *CleanBaseName, Suffix));
	}

	return CandidateName;
}

void SWidgetAnimTimelinePanel::CommitConfigChange(const FText& TransactionText, TFunctionRef<void(FWidgetAnimTimelineConfig&)> ChangeConfig)
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr)
	{
		return;
	}

	FScopedTransaction Transaction(TransactionText);
	if (UClass* OwnerClass = ResolveOwnerWidgetClass())
	{
		if (UObject* DefaultObject = OwnerClass->GetDefaultObject())
		{
			DefaultObject->Modify();
		}
	}

	if (UWidgetBlueprint* Blueprint = GetWidgetBlueprint())
	{
		Blueprint->Modify();
	}

	ChangeConfig(*Config);

	MarkWidgetBlueprintModified();
}

void SWidgetAnimTimelinePanel::MarkWidgetBlueprintModified() const
{
	UWidgetBlueprint* Blueprint = GetWidgetBlueprint();
	if (Blueprint == nullptr)
	{
		return;
	}

	Blueprint->Modify();
	Blueprint->MarkPackageDirty();
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

UWidgetBlueprint* SWidgetAnimTimelinePanel::GetWidgetBlueprint() const
{
	if (SourceWidgetBlueprint.IsValid())
	{
		return SourceWidgetBlueprint.Get();
	}

	if (!PhaseHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PhaseHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		for (UObject* Object = OuterObject; Object != nullptr; Object = Object->GetOuter())
		{
			if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object))
			{
				return WidgetBlueprint;
			}

			if (UClass* Class = Cast<UClass>(Object))
			{
				if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Class->ClassGeneratedBy))
				{
					return WidgetBlueprint;
				}
			}

			if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object->GetClass()->ClassGeneratedBy))
			{
				return WidgetBlueprint;
			}
		}
	}

	return nullptr;
}

UClass* SWidgetAnimTimelinePanel::ResolveOwnerWidgetClass() const
{
	if (UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		return WidgetBlueprint->GeneratedClass;
	}

	if (!PhaseHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PhaseHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		for (UObject* Object = OuterObject; Object != nullptr; Object = Object->GetOuter())
		{
			if (UClass* Class = Cast<UClass>(Object))
			{
				return Class;
			}

			UClass* ObjectClass = Object->GetClass();
			if (ObjectClass != nullptr && ObjectClass->ClassGeneratedBy != nullptr)
			{
				return ObjectClass;
			}
		}
	}

	return nullptr;
}

UClass* SWidgetAnimTimelinePanel::ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, FName TargetWidgetName) const
{
	if (TargetWidgetName.IsNone())
	{
		return ResolveOwnerWidgetClass();
	}

	if (WidgetBlueprint != nullptr && WidgetBlueprint->WidgetTree != nullptr)
	{
		UWidget* TargetWidget = WidgetBlueprint->WidgetTree->FindWidget(TargetWidgetName);
		if (TargetWidget != nullptr && TargetWidget->GetClass()->IsChildOf(UUserWidget::StaticClass()))
		{
			return TargetWidget->GetClass();
		}
	}

	if (UClass* OwnerClass = ResolveOwnerWidgetClass())
	{
		for (TFieldIterator<FObjectProperty> It(OwnerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (It->GetFName() == TargetWidgetName && It->PropertyClass != nullptr && It->PropertyClass->IsChildOf(UUserWidget::StaticClass()))
			{
				return It->PropertyClass;
			}
		}
	}

	return nullptr;
}

FName SWidgetAnimTimelinePanel::GetCurrentPhaseName() const
{
	if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		return Phase->PhaseName;
	}

	if (!PhaseHandle.IsValid())
	{
		return NAME_None;
	}

	TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
	FName PhaseName = NAME_None;
	if (PhaseNameHandle.IsValid())
	{
		PhaseNameHandle->GetValue(PhaseName);
	}
	return PhaseName;
}

bool SWidgetAnimTimelinePanel::HasAnimation(UClass* TargetClass, FName AnimationName) const
{
	if (TargetClass == nullptr || AnimationName.IsNone())
	{
		return false;
	}

	for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		FString PropertyName = It->GetName();
		PropertyName.RemoveFromEnd(TEXT("_INST"));
		if (PropertyName == AnimationName.ToString())
		{
			return true;
		}
	}

	return false;
}

bool SWidgetAnimTimelinePanel::HasChildPhase(UClass* TargetClass, FName TargetWidgetName, FName ChildPhaseName) const
{
	if (TargetClass == nullptr || ChildPhaseName.IsNone())
	{
		return false;
	}

	if (TargetWidgetName.IsNone() && ChildPhaseName == GetCurrentPhaseName())
	{
		return false;
	}

	for (TFieldIterator<FStructProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->Struct != FWidgetAnimTimelineConfig::StaticStruct())
		{
			continue;
		}

		UObject* DefaultObject = TargetClass->GetDefaultObject();
		if (DefaultObject == nullptr)
		{
			continue;
		}

		const FWidgetAnimTimelineConfig* Config = It->ContainerPtrToValuePtr<FWidgetAnimTimelineConfig>(DefaultObject);
		if (Config == nullptr)
		{
			continue;
		}

		for (const FWidgetAnimTimelinePhase& Phase : Config->Phases)
		{
			if (Phase.PhaseName == ChildPhaseName)
			{
				return true;
			}
		}
	}

	return false;
}

float SWidgetAnimTimelinePanel::GetEntryDuration(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, float PlaybackRate, int32 NumLoopsToPlay) const
{
	static constexpr float FallbackDuration = 0.5f;
	const float SafePlaybackRate = FMath::Max(PlaybackRate, KINDA_SMALL_NUMBER);
	const int32 PreviewLoopCount = NumLoopsToPlay == 0 ? 1 : FMath::Max(NumLoopsToPlay, 1);
	if (EntryType != EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		return FallbackDuration * PreviewLoopCount / SafePlaybackRate;
	}

	UClass* TargetClass = ResolveTargetWidgetClass(GetWidgetBlueprint(), TargetWidgetName);
	if (TargetClass == nullptr || AnimationName.IsNone())
	{
		return FallbackDuration * PreviewLoopCount / SafePlaybackRate;
	}

	UObject* DefaultObject = TargetClass->GetDefaultObject();
	if (DefaultObject == nullptr)
	{
		return FallbackDuration * PreviewLoopCount / SafePlaybackRate;
	}

	for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		FString PropertyName = It->GetName();
		PropertyName.RemoveFromEnd(TEXT("_INST"));
		if (PropertyName != AnimationName.ToString())
		{
			continue;
		}

		if (const UWidgetAnimation* Animation = Cast<UWidgetAnimation>(It->GetObjectPropertyValue_InContainer(DefaultObject)))
		{
			return FMath::Max(Animation->GetEndTime(), FallbackDuration) * PreviewLoopCount / SafePlaybackRate;
		}
	}

	return FallbackDuration * PreviewLoopCount / SafePlaybackRate;
}

FString SWidgetAnimTimelinePanel::BuildEntryValidationError(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, FName ChildPhaseName) const
{
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	UClass* TargetClass = ResolveTargetWidgetClass(WidgetBlueprint, TargetWidgetName);
	if (!TargetWidgetName.IsNone() && TargetClass == nullptr)
	{
		return FString::Printf(TEXT("Missing TargetWidgetName: %s"), *TargetWidgetName.ToString());
	}

	if (EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		if (AnimationName.IsNone())
		{
			return TEXT("AnimationName is not set");
		}
		if (!HasAnimation(TargetClass, AnimationName))
		{
			return FString::Printf(TEXT("Missing AnimationName: %s"), *AnimationName.ToString());
		}
		return FString();
	}

	if (ChildPhaseName.IsNone())
	{
		return TEXT("ChildPhaseName is not set");
	}
	if (!HasChildPhase(TargetClass, TargetWidgetName, ChildPhaseName))
	{
		return FString::Printf(TEXT("Missing ChildPhaseName: %s"), *ChildPhaseName.ToString());
	}

	return FString();
}

FReply SWidgetAnimTimelinePanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	if ((MouseEvent.GetEffectingButton() == EKeys::RightMouseButton || MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
		&& IsInTimelineArea(MyGeometry, LocalPosition))
	{
		bPanningTimeline = true;
		PanMouseStartX = LocalPosition.X;
		PanViewStartTime = ViewStartTime;
		return FReply::Handled()
			.CaptureMouse(SharedThis(this))
			.SetUserFocus(SharedThis(this), EFocusCause::Mouse);
	}

	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	const int32 HitEntryIndex = HitTestEntry(MyGeometry, LocalPosition);
	if (HitEntryIndex == INDEX_NONE)
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
		SelectedEntryIndex = INDEX_NONE;
		Invalidate(EInvalidateWidgetReason::Paint);
		return FReply::Unhandled();
	}

	const FEntryViewModel* Entry = Entries.FindByPredicate([HitEntryIndex](const FEntryViewModel& Item)
	{
		return Item.EntryIndex == HitEntryIndex;
	});

	if (Entry == nullptr)
	{
		return FReply::Unhandled();
	}

	if (SelectedEntryIndex != HitEntryIndex)
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
	}
	SelectedEntryIndex = HitEntryIndex;
	DraggingEntryIndex = HitEntryIndex;
	DragEntryStartTime = Entry->StartTime;
	DragMouseStartTime = XToTime(LocalPosition.X, MyGeometry);
	DragTransaction = MakeUnique<FScopedTransaction>(FText::FromString(TEXT("Move Widget Anim Timeline Entry")));
	return FReply::Handled()
		.CaptureMouse(SharedThis(this))
		.SetUserFocus(SharedThis(this), EFocusCause::Mouse);
}

FReply SWidgetAnimTimelinePanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bPanningTimeline)
	{
		bPanningTimeline = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	if (DraggingEntryIndex == INDEX_NONE)
	{
		return FReply::Unhandled();
	}

	DraggingEntryIndex = INDEX_NONE;
	DragTransaction.Reset();
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SWidgetAnimTimelinePanel::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bPanningTimeline && HasMouseCapture())
	{
		const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		ViewStartTime = PanViewStartTime - (LocalPosition.X - PanMouseStartX) / FMath::Max(PixelsPerSecond, KINDA_SMALL_NUMBER);
		ClampViewStartTime(GetTimelineWidth(MyGeometry));
		Invalidate(EInvalidateWidgetReason::Paint);
		return FReply::Handled();
	}

	if (DraggingEntryIndex == INDEX_NONE || !HasMouseCapture())
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float CurrentMouseTime = XToTime(LocalPosition.X, MyGeometry);
	const float NewStartTime = SnapTime(FMath::Max(0.0f, DragEntryStartTime + CurrentMouseTime - DragMouseStartTime), MouseEvent.IsShiftDown());

	// Dragging commits directly to the StartTime property, so Details panel and serialized data stay in sync.
	CommitEntryStartTime(DraggingEntryIndex, NewStartTime);
	RefreshEntries();
	return FReply::Handled();
}

FReply SWidgetAnimTimelinePanel::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float AnchorTime = XToTime(LocalPosition.X, MyGeometry);
	const float AnchorScreenOffset = LocalPosition.X - HeaderWidth;
	const float ZoomFactor = MouseEvent.GetWheelDelta() > 0.0f ? 1.12f : 1.0f / 1.12f;
	PixelsPerSecond = FMath::Clamp(PixelsPerSecond * ZoomFactor, WidgetAnimTimelinePanelConstants::MinPixelsPerSecond, WidgetAnimTimelinePanelConstants::MaxPixelsPerSecond);
	ViewStartTime = AnchorTime - AnchorScreenOffset / FMath::Max(PixelsPerSecond, KINDA_SMALL_NUMBER);
	ClampViewStartTime(GetTimelineWidth(MyGeometry));
	Invalidate(EInvalidateWidgetReason::Paint);
	return FReply::Handled();
}

FReply SWidgetAnimTimelinePanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Delete && CanDeleteSelectedEntry())
	{
		DeleteEntry(SelectedEntryIndex);
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FCursorReply SWidgetAnimTimelinePanel::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition());
	HoveredEntryIndex = HitTestEntry(MyGeometry, LocalPosition);
	if (bPanningTimeline)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	return HoveredEntryIndex != INDEX_NONE ? FCursorReply::Cursor(EMouseCursor::GrabHand) : FCursorReply::Unhandled();
}

bool SWidgetAnimTimelinePanel::SupportsKeyboardFocus() const
{
	return true;
}

void SWidgetAnimTimelinePanel::CommitEntryStartTime(int32 EntryIndex, float NewStartTime)
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (!EntryHandle.IsValid())
	{
		if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
		{
			if (Phase->Entries.IsValidIndex(EntryIndex))
			{
				if (UClass* OwnerClass = ResolveOwnerWidgetClass())
				{
					if (UObject* DefaultObject = OwnerClass->GetDefaultObject())
					{
						DefaultObject->Modify();
					}
				}
				Phase->Entries[EntryIndex].StartTime = NewStartTime;
				MarkWidgetBlueprintModified();
			}
		}
		return;
	}

	TSharedPtr<IPropertyHandle> StartHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime));
	if (StartHandle.IsValid())
	{
		StartHandle->SetValue(NewStartTime);
	}
}

bool SWidgetAnimTimelinePanel::GetEntrySnapshot(int32 EntryIndex, FWidgetAnimTimelineEntry& OutEntry) const
{
	if (const FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		if (Phase->Entries.IsValidIndex(EntryIndex))
		{
			OutEntry = Phase->Entries[EntryIndex];
			return true;
		}
	}

	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (!EntryHandle.IsValid())
	{
		return false;
	}

	uint8 TypeValue = 0;
	uint8 InterruptModeValue = 0;
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName)))
	{
		Handle->GetValue(OutEntry.TargetWidgetName);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType)))
	{
		Handle->GetValue(TypeValue);
		OutEntry.EntryType = static_cast<EWidgetAnimTimelineEntryType>(TypeValue);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName)))
	{
		Handle->GetValue(OutEntry.AnimationName);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName)))
	{
		Handle->GetValue(OutEntry.ChildPhaseName);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime)))
	{
		Handle->GetValue(OutEntry.StartTime);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate)))
	{
		Handle->GetValue(OutEntry.PlaybackRate);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay)))
	{
		Handle->GetValue(OutEntry.NumLoopsToPlay);
	}
	if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, InterruptMode)))
	{
		Handle->GetValue(InterruptModeValue);
		OutEntry.InterruptMode = static_cast<EWidgetAnimTimelineInterruptMode>(InterruptModeValue);
	}

	return true;
}

void SWidgetAnimTimelinePanel::CommitEntryChange(int32 EntryIndex, const FText& TransactionText, TFunctionRef<void(FWidgetAnimTimelineEntry&)> ChangeEntry)
{
	if (EntryIndex == INDEX_NONE)
	{
		return;
	}

	FScopedTransaction Transaction(TransactionText);
	if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		if (!Phase->Entries.IsValidIndex(EntryIndex))
		{
			return;
		}

		if (UClass* OwnerClass = ResolveOwnerWidgetClass())
		{
			if (UObject* DefaultObject = OwnerClass->GetDefaultObject())
			{
				DefaultObject->Modify();
			}
		}

		if (UWidgetBlueprint* Blueprint = GetWidgetBlueprint())
		{
			Blueprint->Modify();
		}

		ChangeEntry(Phase->Entries[EntryIndex]);
		MarkWidgetBlueprintModified();
	}
}

void SWidgetAnimTimelinePanel::CommitEntryName(int32 EntryIndex, FName PropertyName, FName NewValue, const FText& TransactionText)
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (EntryHandle.IsValid())
	{
		FScopedTransaction Transaction(TransactionText);
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(PropertyName))
		{
			Handle->SetValue(NewValue);
		}
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::Paint);
		return;
	}

	CommitEntryChange(EntryIndex, TransactionText, [PropertyName, NewValue](FWidgetAnimTimelineEntry& Entry)
	{
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName))
		{
			Entry.TargetWidgetName = NewValue;
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName))
		{
			Entry.AnimationName = NewValue;
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName))
		{
			Entry.ChildPhaseName = NewValue;
		}
	});
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::CommitEntryFloat(int32 EntryIndex, FName PropertyName, float NewValue, const FText& TransactionText)
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (EntryHandle.IsValid())
	{
		FScopedTransaction Transaction(TransactionText);
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(PropertyName))
		{
			Handle->SetValue(NewValue);
		}
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::Paint);
		return;
	}

	CommitEntryChange(EntryIndex, TransactionText, [PropertyName, NewValue](FWidgetAnimTimelineEntry& Entry)
	{
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime))
		{
			Entry.StartTime = NewValue;
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate))
		{
			Entry.PlaybackRate = NewValue;
		}
	});
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::CommitEntryInt(int32 EntryIndex, FName PropertyName, int32 NewValue, const FText& TransactionText)
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (EntryHandle.IsValid())
	{
		FScopedTransaction Transaction(TransactionText);
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(PropertyName))
		{
			Handle->SetValue(NewValue);
		}
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::Paint);
		return;
	}

	CommitEntryChange(EntryIndex, TransactionText, [PropertyName, NewValue](FWidgetAnimTimelineEntry& Entry)
	{
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay))
		{
			Entry.NumLoopsToPlay = NewValue;
		}
	});
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::CommitEntryType(int32 EntryIndex, EWidgetAnimTimelineEntryType NewValue, const FText& TransactionText)
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (EntryHandle.IsValid())
	{
		FScopedTransaction Transaction(TransactionText);
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType)))
		{
			Handle->SetValue(static_cast<uint8>(NewValue));
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName)))
		{
			Handle->SetValue(NewValue == EWidgetAnimTimelineEntryType::DirectAnimation ? NAME_None : NAME_None);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName)))
		{
			Handle->SetValue(NAME_None);
		}
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::Paint);
		return;
	}

	CommitEntryChange(EntryIndex, TransactionText, [NewValue](FWidgetAnimTimelineEntry& Entry)
	{
		Entry.EntryType = NewValue;
		Entry.AnimationName = NAME_None;
		Entry.ChildPhaseName = NAME_None;
	});
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::AddEntry(EWidgetAnimTimelineEntryType EntryType)
{
	FWidgetAnimTimelineEntry NewEntry;
	NewEntry.EntryType = EntryType;
	NewEntry.StartTime = SelectedEntryIndex == INDEX_NONE ? 0.0f : GetTimelineDuration() - 0.5f;
	NewEntry.StartTime = FMath::Max(0.0f, SnapTime(NewEntry.StartTime, false));

	FScopedTransaction Transaction(FText::FromString(TEXT("Add Widget Anim Timeline Entry")));
	if (TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle())
	{
		if (TSharedPtr<IPropertyHandleArray> EntriesArray = EntriesHandle->AsArray())
		{
			uint32 EntryCount = 0;
			EntriesHandle->GetNumChildren(EntryCount);
			EntriesArray->AddItem();
			SelectedEntryIndex = static_cast<int32>(EntryCount);

			if (TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(SelectedEntryIndex))
			{
				if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType)))
				{
					Handle->SetValue(static_cast<uint8>(NewEntry.EntryType));
				}
				if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime)))
				{
					Handle->SetValue(NewEntry.StartTime);
				}
				if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate)))
				{
					Handle->SetValue(NewEntry.PlaybackRate);
				}
				if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay)))
				{
					Handle->SetValue(NewEntry.NumLoopsToPlay);
				}
				if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, InterruptMode)))
				{
					Handle->SetValue(static_cast<uint8>(NewEntry.InterruptMode));
				}
			}
		}
	}
	else if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		if (UClass* OwnerClass = ResolveOwnerWidgetClass())
		{
			if (UObject* DefaultObject = OwnerClass->GetDefaultObject())
			{
				DefaultObject->Modify();
			}
		}

		if (UWidgetBlueprint* Blueprint = GetWidgetBlueprint())
		{
			Blueprint->Modify();
		}

		SelectedEntryIndex = Phase->Entries.Add(NewEntry);
		MarkWidgetBlueprintModified();
	}

	RefreshEntries();
	RefreshAnimationOptions();
	RefreshChildPhaseOptions();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::DeleteEntry(int32 EntryIndex)
{
	if (EntryIndex == INDEX_NONE)
	{
		return;
	}

	FScopedTransaction Transaction(FText::FromString(TEXT("Delete Widget Anim Timeline Entry")));
	if (TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle())
	{
		if (TSharedPtr<IPropertyHandleArray> EntriesArray = EntriesHandle->AsArray())
		{
			EntriesArray->DeleteItem(EntryIndex);
		}
	}
	else if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		if (!Phase->Entries.IsValidIndex(EntryIndex))
		{
			return;
		}

		if (UClass* OwnerClass = ResolveOwnerWidgetClass())
		{
			if (UObject* DefaultObject = OwnerClass->GetDefaultObject())
			{
				DefaultObject->Modify();
			}
		}

		if (UWidgetBlueprint* Blueprint = GetWidgetBlueprint())
		{
			Blueprint->Modify();
		}

		Phase->Entries.RemoveAt(EntryIndex);
		MarkWidgetBlueprintModified();
	}

	SelectedEntryIndex = INDEX_NONE;
	DraggingEntryIndex = INDEX_NONE;
	HoveredEntryIndex = INDEX_NONE;
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

float SWidgetAnimTimelinePanel::GetTimelineDuration() const
{
	return FMath::Max(3.0f, GetTimelineContentEndTime() + 0.5f);
}

float SWidgetAnimTimelinePanel::GetTimelineContentEndTime() const
{
	float Duration = 0.0f;
	for (const FEntryViewModel& Entry : Entries)
	{
		Duration = FMath::Max(Duration, Entry.StartTime + Entry.Duration);
	}
	return Duration;
}

float SWidgetAnimTimelinePanel::GetTimelineWidth(const FGeometry& Geometry) const
{
	return FMath::Max(0.0f, Geometry.GetLocalSize().X - HeaderWidth - GetInspectorWidth() - 12.0f);
}

float SWidgetAnimTimelinePanel::GetInspectorWidth() const
{
	return SelectedEntryIndex == INDEX_NONE ? 0.0f : InspectorWidth;
}

float SWidgetAnimTimelinePanel::GetMaxViewStartTime(float TimelineWidth) const
{
	const float VisibleDuration = TimelineWidth / FMath::Max(PixelsPerSecond, KINDA_SMALL_NUMBER);
	return FMath::Max(0.0f, GetTimelineDuration() - VisibleDuration);
}

void SWidgetAnimTimelinePanel::ClampViewStartTime(float TimelineWidth)
{
	ViewStartTime = FMath::Clamp(ViewStartTime, 0.0f, GetMaxViewStartTime(TimelineWidth));
}

float SWidgetAnimTimelinePanel::TimeToX(float Time, const FGeometry& Geometry) const
{
	return HeaderWidth + (Time - ViewStartTime) * PixelsPerSecond;
}

float SWidgetAnimTimelinePanel::XToTime(float X, const FGeometry& Geometry) const
{
	return FMath::Max(0.0f, ViewStartTime + (X - HeaderWidth) / PixelsPerSecond);
}

bool SWidgetAnimTimelinePanel::IsInTimelineArea(const FGeometry& Geometry, FVector2D LocalPosition) const
{
	const float TimelineWidth = GetTimelineWidth(Geometry);
	return LocalPosition.X >= HeaderWidth
		&& LocalPosition.X <= HeaderWidth + TimelineWidth
		&& LocalPosition.Y >= TimelineTop - RulerHeight;
}

int32 SWidgetAnimTimelinePanel::HitTestEntry(const FGeometry& Geometry, FVector2D LocalPosition) const
{
	for (const FEntryViewModel& Entry : Entries)
	{
		const int32 LaneIndex = GetLaneIndex(Entry.LaneName);
		if (LaneIndex == INDEX_NONE)
		{
			continue;
		}

		const float Y = TimelineTop + LaneIndex * LaneHeight + 4.0f;
		const float X = TimeToX(Entry.StartTime, Geometry);
		const float Width = FMath::Max(WidgetAnimTimelinePanelConstants::BlockMinWidth, Entry.Duration * PixelsPerSecond);
		const FVector2D Min(X, Y);
		const FVector2D Max(X + Width, Y + LaneHeight - 10.0f);

		if (LocalPosition.X >= Min.X && LocalPosition.X <= Max.X && LocalPosition.Y >= Min.Y && LocalPosition.Y <= Max.Y)
		{
			return Entry.EntryIndex;
		}
	}

	return INDEX_NONE;
}

int32 SWidgetAnimTimelinePanel::GetLaneIndex(const FString& LaneName) const
{
	return LaneNames.IndexOfByKey(LaneName);
}

float SWidgetAnimTimelinePanel::SnapTime(float Time, bool bUseFineSnap) const
{
	return FMath::GridSnap(Time, bUseFineSnap ? 0.01f : SnapInterval);
}

FString SWidgetAnimTimelinePanel::FormatTime(float Time) const
{
	return FString::Printf(TEXT("%.2fs"), Time);
}

TSharedPtr<IPropertyHandle> SWidgetAnimTimelinePanel::GetEntriesHandle() const
{
	return PhaseHandle.IsValid() ? PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, Entries)) : nullptr;
}

TSharedPtr<IPropertyHandle> SWidgetAnimTimelinePanel::GetEntryHandle(int32 EntryIndex) const
{
	TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle();
	return EntriesHandle.IsValid() ? EntriesHandle->GetChildHandle(static_cast<uint32>(EntryIndex)) : nullptr;
}

FText SWidgetAnimTimelinePanel::GetPhaseTitle() const
{
	if (FWidgetAnimTimelinePhase* Phase = GetTimelinePhase())
	{
		return FText::FromString(FString::Printf(TEXT("Phase: %s"), Phase->PhaseName.IsNone() ? TEXT("None") : *Phase->PhaseName.ToString()));
	}

	if (!PhaseHandle.IsValid())
	{
		return FText::FromString(TEXT("Phase: None"));
	}

	TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
	FName PhaseName = NAME_None;
	if (PhaseNameHandle.IsValid())
	{
		PhaseNameHandle->GetValue(PhaseName);
	}

	return FText::FromString(FString::Printf(TEXT("Phase: %s"), PhaseName.IsNone() ? TEXT("None") : *PhaseName.ToString()));
}

FText SWidgetAnimTimelinePanel::GetSelectedPhaseText() const
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return GetPhaseTitle();
	}

	const FName PhaseName = Config->Phases[PhaseIndex].PhaseName;
	return FText::FromString(PhaseName.IsNone() ? TEXT("None") : PhaseName.ToString());
}

FText SWidgetAnimTimelinePanel::GetSelectedPhaseNameEditText() const
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return FText::GetEmpty();
	}

	const FName PhaseName = Config->Phases[PhaseIndex].PhaseName;
	return PhaseName.IsNone() ? FText::GetEmpty() : FText::FromName(PhaseName);
}

TSharedRef<SWidget> SWidgetAnimTimelinePanel::MakePhaseOptionWidget(TSharedPtr<int32> Option) const
{
	FText OptionText = FText::FromString(TEXT("None"));
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Option.IsValid() && Config != nullptr && Config->Phases.IsValidIndex(*Option))
	{
		const FName PhaseName = Config->Phases[*Option].PhaseName;
		OptionText = FText::FromString(PhaseName.IsNone() ? TEXT("None") : PhaseName.ToString());
	}

	return SNew(STextBlock)
		.Text(OptionText);
}

void SWidgetAnimTimelinePanel::OnPhaseSelectionChanged(TSharedPtr<int32> Option, ESelectInfo::Type SelectInfo)
{
	if (!Option.IsValid() || PhaseIndex == *Option)
	{
		return;
	}

	PhaseIndex = *Option;
	SelectedPhaseOption = Option;
	SelectedEntryIndex = INDEX_NONE;
	DraggingEntryIndex = INDEX_NONE;
	HoveredEntryIndex = INDEX_NONE;
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SWidgetAnimTimelinePanel::OnSelectedPhaseNameCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return;
	}

	const FString RequestedNameString = NewText.ToString().TrimStartAndEnd();
	if (RequestedNameString.IsEmpty())
	{
		return;
	}

	const FName OldPhaseName = Config->Phases[PhaseIndex].PhaseName;
	const FName RequestedName(*RequestedNameString);
	if (OldPhaseName == RequestedName)
	{
		return;
	}

	FName NewPhaseName = RequestedName;
	for (int32 Index = 0; Index < Config->Phases.Num(); ++Index)
	{
		if (Index != PhaseIndex && Config->Phases[Index].PhaseName == RequestedName)
		{
			NewPhaseName = MakeUniquePhaseName(RequestedNameString);
			break;
		}
	}

	const int32 EditingPhaseIndex = PhaseIndex;
	CommitConfigChange(FText::FromString(TEXT("Rename Widget Anim Timeline Phase")), [EditingPhaseIndex, OldPhaseName, NewPhaseName](FWidgetAnimTimelineConfig& EditableConfig)
	{
		if (!EditableConfig.Phases.IsValidIndex(EditingPhaseIndex))
		{
			return;
		}

		EditableConfig.Phases[EditingPhaseIndex].PhaseName = NewPhaseName;
		if (EditableConfig.AutoPlayPhaseName == OldPhaseName)
		{
			EditableConfig.AutoPlayPhaseName = NewPhaseName;
		}
	});

	RefreshPhaseOptions();
	RefreshChildPhaseOptions();
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::Paint);
}

FReply SWidgetAnimTimelinePanel::AddPhase()
{
	int32 NewPhaseIndex = INDEX_NONE;
	CommitConfigChange(FText::FromString(TEXT("Add Widget Anim Timeline Phase")), [this, &NewPhaseIndex](FWidgetAnimTimelineConfig& Config)
	{
		FWidgetAnimTimelinePhase NewPhase;
		NewPhase.PhaseName = MakeUniquePhaseName(TEXT("Phase"));
		NewPhaseIndex = Config.Phases.Add(NewPhase);
	});

	if (NewPhaseIndex != INDEX_NONE)
	{
		PhaseIndex = NewPhaseIndex;
		SelectedEntryIndex = INDEX_NONE;
		DraggingEntryIndex = INDEX_NONE;
		HoveredEntryIndex = INDEX_NONE;
		RefreshPhaseOptions();
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}

	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

FReply SWidgetAnimTimelinePanel::DuplicatePhase()
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
	}

	const FWidgetAnimTimelinePhase SourcePhase = Config->Phases[PhaseIndex];
	int32 NewPhaseIndex = INDEX_NONE;
	CommitConfigChange(FText::FromString(TEXT("Duplicate Widget Anim Timeline Phase")), [this, SourcePhase, &NewPhaseIndex](FWidgetAnimTimelineConfig& EditableConfig)
	{
		FWidgetAnimTimelinePhase NewPhase = SourcePhase;
		const FString BaseName = SourcePhase.PhaseName.IsNone() ? TEXT("Phase_Copy") : FString::Printf(TEXT("%s_Copy"), *SourcePhase.PhaseName.ToString());
		NewPhase.PhaseName = MakeUniquePhaseName(BaseName);
		NewPhaseIndex = EditableConfig.Phases.Add(NewPhase);
	});

	if (NewPhaseIndex != INDEX_NONE)
	{
		PhaseIndex = NewPhaseIndex;
		SelectedEntryIndex = INDEX_NONE;
		DraggingEntryIndex = INDEX_NONE;
		HoveredEntryIndex = INDEX_NONE;
		RefreshPhaseOptions();
		RefreshEntries();
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}

	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

FReply SWidgetAnimTimelinePanel::DeletePhase()
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || !Config->Phases.IsValidIndex(PhaseIndex))
	{
		return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
	}

	const int32 DeletedPhaseIndex = PhaseIndex;
	const FName DeletedPhaseName = Config->Phases[DeletedPhaseIndex].PhaseName;
	CommitConfigChange(FText::FromString(TEXT("Delete Widget Anim Timeline Phase")), [DeletedPhaseIndex, DeletedPhaseName](FWidgetAnimTimelineConfig& EditableConfig)
	{
		if (!EditableConfig.Phases.IsValidIndex(DeletedPhaseIndex))
		{
			return;
		}

		EditableConfig.Phases.RemoveAt(DeletedPhaseIndex);
		if (EditableConfig.AutoPlayPhaseName == DeletedPhaseName)
		{
			EditableConfig.AutoPlayPhaseName = NAME_None;
		}
	});

	if (FWidgetAnimTimelineConfig* UpdatedConfig = GetTimelineConfig())
	{
		PhaseIndex = UpdatedConfig->Phases.Num() == 0 ? INDEX_NONE : FMath::Clamp(DeletedPhaseIndex, 0, UpdatedConfig->Phases.Num() - 1);
	}
	else
	{
		PhaseIndex = INDEX_NONE;
	}

	SelectedEntryIndex = INDEX_NONE;
	DraggingEntryIndex = INDEX_NONE;
	HoveredEntryIndex = INDEX_NONE;
	RefreshPhaseOptions();
	RefreshEntries();
	Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

bool SWidgetAnimTimelinePanel::CanDuplicatePhase() const
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	return Config != nullptr && Config->Phases.IsValidIndex(PhaseIndex);
}

bool SWidgetAnimTimelinePanel::CanDeletePhase() const
{
	return CanDuplicatePhase();
}

FText SWidgetAnimTimelinePanel::GetSelectedAutoPlayText() const
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (Config == nullptr || Config->AutoPlayPhaseName.IsNone())
	{
		return FText::FromString(TEXT("None"));
	}

	return FText::FromName(Config->AutoPlayPhaseName);
}

TSharedRef<SWidget> SWidgetAnimTimelinePanel::MakeAutoPlayOptionWidget(TSharedPtr<FName> Option) const
{
	return SNew(STextBlock)
		.Text(GetNameOptionText(Option));
}

void SWidgetAnimTimelinePanel::OnAutoPlayOptionsOpening()
{
	RefreshAutoPlayOptions();
}

void SWidgetAnimTimelinePanel::OnAutoPlayChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo)
{
	FWidgetAnimTimelineConfig* Config = GetTimelineConfig();
	if (!Option.IsValid() || Config == nullptr || Config->AutoPlayPhaseName == *Option)
	{
		return;
	}

	const FName NewAutoPlayPhaseName = *Option;
	CommitConfigChange(FText::FromString(TEXT("Edit Widget Anim Timeline AutoPlay Phase")), [NewAutoPlayPhaseName](FWidgetAnimTimelineConfig& EditableConfig)
	{
		EditableConfig.AutoPlayPhaseName = NewAutoPlayPhaseName;
	});

	RefreshAutoPlayOptions();
	Invalidate(EInvalidateWidgetReason::Paint);
}

FText SWidgetAnimTimelinePanel::GetBlueprintCompileStatusText() const
{
	const UWidgetBlueprint* Blueprint = GetWidgetBlueprint();
	if (Blueprint == nullptr)
	{
		return FText::FromString(TEXT("Status: No Blueprint"));
	}

	switch (Blueprint->Status)
	{
	case BS_Dirty:
		return FText::FromString(TEXT("Status: Needs Compile"));
	case BS_Unknown:
		return FText::FromString(TEXT("Status: Unknown"));
	case BS_Error:
		return FText::FromString(TEXT("Status: Compile Error"));
	case BS_UpToDate:
		return FText::FromString(TEXT("Status: Compiled"));
	case BS_BeingCreated:
		return FText::FromString(TEXT("Status: Creating"));
	case BS_UpToDateWithWarnings:
		return FText::FromString(TEXT("Status: Compiled With Warnings"));
	default:
		return FText::FromString(TEXT("Status: Unknown"));
	}
}

FSlateColor SWidgetAnimTimelinePanel::GetBlueprintCompileStatusColor() const
{
	const UWidgetBlueprint* Blueprint = GetWidgetBlueprint();
	if (Blueprint == nullptr)
	{
		return FSlateColor(FLinearColor(0.52f, 0.52f, 0.52f, 1.0f));
	}

	switch (Blueprint->Status)
	{
	case BS_Dirty:
	case BS_Unknown:
		return FSlateColor(FLinearColor(1.0f, 0.74f, 0.18f, 1.0f));
	case BS_Error:
		return FSlateColor(FLinearColor(1.0f, 0.18f, 0.12f, 1.0f));
	case BS_UpToDate:
		return FSlateColor(FLinearColor(0.26f, 0.82f, 0.36f, 1.0f));
	case BS_BeingCreated:
		return FSlateColor(FLinearColor(0.42f, 0.64f, 1.0f, 1.0f));
	case BS_UpToDateWithWarnings:
		return FSlateColor(FLinearColor(1.0f, 0.55f, 0.12f, 1.0f));
	default:
		return FSlateColor(FLinearColor(0.52f, 0.52f, 0.52f, 1.0f));
	}
}

FReply SWidgetAnimTimelinePanel::PlayDesignerPreview()
{
	if (PhaseHandle.IsValid())
	{
		FWidgetAnimTimelineDesignerPreviewController::Play(PhaseHandle);
		return FReply::Handled();
	}

	FWidgetAnimTimelineDesignerPreviewController::Play(GetWidgetBlueprint(), PhaseIndex);
	return FReply::Handled();
}

FReply SWidgetAnimTimelinePanel::FitTimelineToContent()
{
	const float AvailableWidth = LastTimelineWidth > 0.0f ? LastTimelineWidth : 540.0f;
	const float ContentDuration = FMath::Max(GetTimelineContentEndTime() + 0.5f, 1.0f);
	PixelsPerSecond = FMath::Clamp(
		AvailableWidth / ContentDuration,
		WidgetAnimTimelinePanelConstants::MinPixelsPerSecond,
		WidgetAnimTimelinePanelConstants::MaxPixelsPerSecond);
	ViewStartTime = 0.0f;
	Invalidate(EInvalidateWidgetReason::Paint);
	return FReply::Handled();
}

EVisibility SWidgetAnimTimelinePanel::GetEntryInspectorVisibility() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SWidgetAnimTimelinePanel::GetSelectedEntryTitleText() const
{
	return SelectedEntryIndex == INDEX_NONE
		? FText::FromString(TEXT("Entry"))
		: FText::FromString(FString::Printf(TEXT("Entry [%d]"), SelectedEntryIndex));
}

FText SWidgetAnimTimelinePanel::GetSelectedEntryTypeText() const
{
	FWidgetAnimTimelineEntry Entry;
	if (!GetEntrySnapshot(SelectedEntryIndex, Entry))
	{
		return FText::FromString(TEXT("None"));
	}

	return Entry.EntryType == EWidgetAnimTimelineEntryType::DirectAnimation
		? FText::FromString(TEXT("Direct Animation"))
		: FText::FromString(TEXT("Child Sequence Phase"));
}

FText SWidgetAnimTimelinePanel::GetSelectedEntryTargetText() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) && !Entry.TargetWidgetName.IsNone()
		? FText::FromName(Entry.TargetWidgetName)
		: FText::FromString(TEXT("Self"));
}

FText SWidgetAnimTimelinePanel::GetSelectedEntryAnimationText() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) && !Entry.AnimationName.IsNone()
		? FText::FromName(Entry.AnimationName)
		: FText::FromString(TEXT("None"));
}

FText SWidgetAnimTimelinePanel::GetSelectedEntryChildPhaseText() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) && !Entry.ChildPhaseName.IsNone()
		? FText::FromName(Entry.ChildPhaseName)
		: FText::FromString(TEXT("None"));
}

EVisibility SWidgetAnimTimelinePanel::GetAnimationFieldVisibility() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) && Entry.EntryType == EWidgetAnimTimelineEntryType::DirectAnimation
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

EVisibility SWidgetAnimTimelinePanel::GetChildPhaseFieldVisibility() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) && Entry.EntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

TOptional<float> SWidgetAnimTimelinePanel::GetSelectedEntryStartTime() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) ? TOptional<float>(Entry.StartTime) : TOptional<float>();
}

TOptional<float> SWidgetAnimTimelinePanel::GetSelectedEntryPlaybackRate() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) ? TOptional<float>(Entry.PlaybackRate) : TOptional<float>();
}

TOptional<int32> SWidgetAnimTimelinePanel::GetSelectedEntryNumLoops() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry) ? TOptional<int32>(Entry.NumLoopsToPlay) : TOptional<int32>();
}

FText SWidgetAnimTimelinePanel::GetNameOptionText(TSharedPtr<FName> Option) const
{
	if (!Option.IsValid() || Option->IsNone())
	{
		return FText::FromString(TEXT("None"));
	}
	return FText::FromName(*Option);
}

TSharedRef<SWidget> SWidgetAnimTimelinePanel::MakeNameOptionWidget(TSharedPtr<FName> Option) const
{
	return SNew(STextBlock)
		.Text(GetNameOptionText(Option));
}

TSharedRef<SWidget> SWidgetAnimTimelinePanel::MakeEntryTypeOptionWidget(TSharedPtr<EWidgetAnimTimelineEntryType> Option) const
{
	const FText Text = Option.IsValid() && *Option == EWidgetAnimTimelineEntryType::ChildSequencePhase
		? FText::FromString(TEXT("Child Sequence Phase"))
		: FText::FromString(TEXT("Direct Animation"));
	return SNew(STextBlock)
		.Text(Text);
}

void SWidgetAnimTimelinePanel::RefreshTargetOptions()
{
	TargetOptions.Reset();
	TargetOptions.Add(MakeShared<FName>(NAME_None));

	TSet<FName> AddedNames;
	AddedNames.Add(NAME_None);
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	if (WidgetBlueprint != nullptr && WidgetBlueprint->WidgetTree != nullptr)
	{
		WidgetBlueprint->WidgetTree->ForEachWidget([this, &AddedNames](UWidget* Widget)
		{
			if (Widget != nullptr && Widget->GetClass()->IsChildOf(UUserWidget::StaticClass()) && !AddedNames.Contains(Widget->GetFName()))
			{
				AddedNames.Add(Widget->GetFName());
				TargetOptions.Add(MakeShared<FName>(Widget->GetFName()));
			}
		});
	}

	if (UClass* OwnerClass = ResolveOwnerWidgetClass())
	{
		for (TFieldIterator<FObjectProperty> It(OwnerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (It->PropertyClass != nullptr && It->PropertyClass->IsChildOf(UUserWidget::StaticClass()) && !AddedNames.Contains(It->GetFName()))
			{
				AddedNames.Add(It->GetFName());
				TargetOptions.Add(MakeShared<FName>(It->GetFName()));
			}
		}
	}
}

void SWidgetAnimTimelinePanel::RefreshAnimationOptions()
{
	AnimationOptions.Reset();
	AnimationOptions.Add(MakeShared<FName>(NAME_None));

	FWidgetAnimTimelineEntry Entry;
	if (!GetEntrySnapshot(SelectedEntryIndex, Entry))
	{
		return;
	}

	TSet<FName> AddedNames;
	AddedNames.Add(NAME_None);
	if (UClass* TargetClass = ResolveTargetWidgetClass(GetWidgetBlueprint(), Entry.TargetWidgetName))
	{
		for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (It->PropertyClass != UWidgetAnimation::StaticClass())
			{
				continue;
			}

			FString PropertyName = It->GetName();
			PropertyName.RemoveFromEnd(TEXT("_INST"));
			const FName AnimationName(*PropertyName);
			if (!AddedNames.Contains(AnimationName))
			{
				AddedNames.Add(AnimationName);
				AnimationOptions.Add(MakeShared<FName>(AnimationName));
			}
		}
	}
}

void SWidgetAnimTimelinePanel::RefreshChildPhaseOptions()
{
	ChildPhaseOptions.Reset();
	ChildPhaseOptions.Add(MakeShared<FName>(NAME_None));

	FWidgetAnimTimelineEntry Entry;
	if (!GetEntrySnapshot(SelectedEntryIndex, Entry))
	{
		return;
	}

	UClass* TargetClass = ResolveTargetWidgetClass(GetWidgetBlueprint(), Entry.TargetWidgetName);
	if (TargetClass == nullptr)
	{
		return;
	}

	UObject* DefaultObject = TargetClass->GetDefaultObject();
	if (DefaultObject == nullptr)
	{
		return;
	}

	TSet<FName> AddedNames;
	AddedNames.Add(NAME_None);
	for (TFieldIterator<FStructProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->Struct != FWidgetAnimTimelineConfig::StaticStruct())
		{
			continue;
		}

		const FWidgetAnimTimelineConfig* Config = It->ContainerPtrToValuePtr<FWidgetAnimTimelineConfig>(DefaultObject);
		if (Config == nullptr)
		{
			continue;
		}

		for (const FWidgetAnimTimelinePhase& Phase : Config->Phases)
		{
			if (Phase.PhaseName.IsNone() || AddedNames.Contains(Phase.PhaseName))
			{
				continue;
			}
			if (Entry.TargetWidgetName.IsNone() && Phase.PhaseName == GetCurrentPhaseName())
			{
				continue;
			}

			AddedNames.Add(Phase.PhaseName);
			ChildPhaseOptions.Add(MakeShared<FName>(Phase.PhaseName));
		}
	}
}

void SWidgetAnimTimelinePanel::OnTargetOptionsOpening()
{
	RefreshTargetOptions();
}

void SWidgetAnimTimelinePanel::OnAnimationOptionsOpening()
{
	RefreshAnimationOptions();
}

void SWidgetAnimTimelinePanel::OnChildPhaseOptionsOpening()
{
	RefreshChildPhaseOptions();
}

void SWidgetAnimTimelinePanel::OnSelectedEntryTypeChanged(TSharedPtr<EWidgetAnimTimelineEntryType> Option, ESelectInfo::Type SelectInfo)
{
	FWidgetAnimTimelineEntry Entry;
	if (!Option.IsValid() || !GetEntrySnapshot(SelectedEntryIndex, Entry) || Entry.EntryType == *Option)
	{
		return;
	}

	CommitEntryType(SelectedEntryIndex, *Option, FText::FromString(TEXT("Edit Widget Anim Timeline Entry Type")));
	RefreshAnimationOptions();
	RefreshChildPhaseOptions();
}

void SWidgetAnimTimelinePanel::OnSelectedEntryTargetChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo)
{
	FWidgetAnimTimelineEntry Entry;
	if (!Option.IsValid() || !GetEntrySnapshot(SelectedEntryIndex, Entry) || Entry.TargetWidgetName == *Option)
	{
		return;
	}

	CommitEntryName(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName), *Option, FText::FromString(TEXT("Edit Widget Anim Timeline Target")));
	CommitEntryName(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName), NAME_None, FText::FromString(TEXT("Reset Widget Anim Timeline Animation")));
	CommitEntryName(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName), NAME_None, FText::FromString(TEXT("Reset Widget Anim Timeline Child Phase")));
	RefreshAnimationOptions();
	RefreshChildPhaseOptions();
}

void SWidgetAnimTimelinePanel::OnSelectedEntryAnimationChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo)
{
	FWidgetAnimTimelineEntry Entry;
	if (!Option.IsValid() || !GetEntrySnapshot(SelectedEntryIndex, Entry) || Entry.AnimationName == *Option)
	{
		return;
	}

	CommitEntryName(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName), *Option, FText::FromString(TEXT("Edit Widget Anim Timeline Animation")));
}

void SWidgetAnimTimelinePanel::OnSelectedEntryChildPhaseChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo)
{
	FWidgetAnimTimelineEntry Entry;
	if (!Option.IsValid() || !GetEntrySnapshot(SelectedEntryIndex, Entry) || Entry.ChildPhaseName == *Option)
	{
		return;
	}

	CommitEntryName(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName), *Option, FText::FromString(TEXT("Edit Widget Anim Timeline Child Phase")));
}

void SWidgetAnimTimelinePanel::OnSelectedEntryStartTimeCommitted(float NewValue, ETextCommit::Type CommitType)
{
	CommitEntryFloat(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime), FMath::Max(0.0f, NewValue), FText::FromString(TEXT("Edit Widget Anim Timeline Start Time")));
}

void SWidgetAnimTimelinePanel::OnSelectedEntryPlaybackRateCommitted(float NewValue, ETextCommit::Type CommitType)
{
	CommitEntryFloat(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate), FMath::Max(0.001f, NewValue), FText::FromString(TEXT("Edit Widget Anim Timeline Playback Rate")));
}

void SWidgetAnimTimelinePanel::OnSelectedEntryNumLoopsCommitted(int32 NewValue, ETextCommit::Type CommitType)
{
	CommitEntryInt(SelectedEntryIndex, GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay), FMath::Max(0, NewValue), FText::FromString(TEXT("Edit Widget Anim Timeline Loop Count")));
}

FReply SWidgetAnimTimelinePanel::AddDirectAnimationEntry()
{
	AddEntry(EWidgetAnimTimelineEntryType::DirectAnimation);
	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

FReply SWidgetAnimTimelinePanel::AddChildSequenceEntry()
{
	AddEntry(EWidgetAnimTimelineEntryType::ChildSequencePhase);
	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

FReply SWidgetAnimTimelinePanel::DeleteSelectedEntry()
{
	DeleteEntry(SelectedEntryIndex);
	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly);
}

bool SWidgetAnimTimelinePanel::CanDeleteSelectedEntry() const
{
	FWidgetAnimTimelineEntry Entry;
	return GetEntrySnapshot(SelectedEntryIndex, Entry);
}
