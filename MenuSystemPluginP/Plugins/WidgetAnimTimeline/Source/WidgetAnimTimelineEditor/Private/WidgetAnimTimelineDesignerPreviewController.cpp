#include "WidgetAnimTimelineDesignerPreviewController.h"

#include "Animation/WidgetAnimationHandle.h"
#include "Animation/WidgetAnimationState.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "PropertyHandle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"

namespace WidgetAnimTimelineDesignerPreviewController
{
	static constexpr float FallbackEntryDuration = 0.5f;
	static constexpr float EndPadding = 0.2f;
	static TSharedPtr<FWidgetAnimTimelineDesignerPreviewController> ActiveController;
}

bool FWidgetAnimTimelineDesignerPreviewController::Play(TSharedPtr<IPropertyHandle> PhaseHandle)
{
	FWidgetAnimTimelinePhase RootPhase;
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint(PhaseHandle);
	if (WidgetBlueprint == nullptr || !ReadRootPhase(PhaseHandle, RootPhase))
	{
		return false;
	}

	StopActive();
	WidgetAnimTimelineDesignerPreviewController::ActiveController = MakeShareable(new FWidgetAnimTimelineDesignerPreviewController(WidgetBlueprint, RootPhase));
	if (!WidgetAnimTimelineDesignerPreviewController::ActiveController->Start())
	{
		WidgetAnimTimelineDesignerPreviewController::ActiveController.Reset();
		return false;
	}

	return true;
}

void FWidgetAnimTimelineDesignerPreviewController::Stop(TSharedPtr<IPropertyHandle> PhaseHandle)
{
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint(PhaseHandle);
	if (WidgetAnimTimelineDesignerPreviewController::ActiveController.IsValid()
		&& (WidgetBlueprint == nullptr || WidgetAnimTimelineDesignerPreviewController::ActiveController->WidgetBlueprint.Get() == WidgetBlueprint))
	{
		StopActive();
	}
}

void FWidgetAnimTimelineDesignerPreviewController::StopActive()
{
	if (WidgetAnimTimelineDesignerPreviewController::ActiveController.IsValid())
	{
		WidgetAnimTimelineDesignerPreviewController::ActiveController->StopInternal(true);
		WidgetAnimTimelineDesignerPreviewController::ActiveController.Reset();
	}
}

UWidgetBlueprint* FWidgetAnimTimelineDesignerPreviewController::GetWidgetBlueprint(TSharedPtr<IPropertyHandle> PhaseHandle)
{
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

bool FWidgetAnimTimelineDesignerPreviewController::ReadRootPhase(TSharedPtr<IPropertyHandle> PhaseHandle, FWidgetAnimTimelinePhase& OutPhase)
{
	if (!PhaseHandle.IsValid())
	{
		return false;
	}

	TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
	TSharedPtr<IPropertyHandle> EntriesHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, Entries));
	if (!PhaseNameHandle.IsValid() || !EntriesHandle.IsValid())
	{
		return false;
	}

	PhaseNameHandle->GetValue(OutPhase.PhaseName);
	OutPhase.Entries.Reset();

	uint32 EntryCount = 0;
	EntriesHandle->GetNumChildren(EntryCount);
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
	{
		TSharedPtr<IPropertyHandle> EntryHandle = EntriesHandle->GetChildHandle(EntryIndex);
		if (!EntryHandle.IsValid())
		{
			continue;
		}

		FWidgetAnimTimelineEntry Entry;
		uint8 TypeValue = 0;
		uint8 InterruptModeValue = 0;

		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName)))
		{
			Handle->GetValue(Entry.TargetWidgetName);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType)))
		{
			Handle->GetValue(TypeValue);
			Entry.EntryType = static_cast<EWidgetAnimTimelineEntryType>(TypeValue);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName)))
		{
			Handle->GetValue(Entry.AnimationName);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName)))
		{
			Handle->GetValue(Entry.ChildPhaseName);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime)))
		{
			Handle->GetValue(Entry.StartTime);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate)))
		{
			Handle->GetValue(Entry.PlaybackRate);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, NumLoopsToPlay)))
		{
			Handle->GetValue(Entry.NumLoopsToPlay);
		}
		if (TSharedPtr<IPropertyHandle> Handle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, InterruptMode)))
		{
			Handle->GetValue(InterruptModeValue);
			Entry.InterruptMode = static_cast<EWidgetAnimTimelineInterruptMode>(InterruptModeValue);
		}

		OutPhase.Entries.Add(Entry);
	}

	return true;
}

FWidgetAnimTimelineDesignerPreviewController::FWidgetAnimTimelineDesignerPreviewController(UWidgetBlueprint* InWidgetBlueprint, const FWidgetAnimTimelinePhase& InRootPhase)
	: WidgetBlueprint(InWidgetBlueprint)
	, RootPhase(InRootPhase)
{
}

bool FWidgetAnimTimelineDesignerPreviewController::Start()
{
	if (WidgetBlueprint.Get() == nullptr || GEditor == nullptr)
	{
		return false;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	FWidgetBlueprintEditor* WidgetEditor = AssetEditorSubsystem != nullptr
		? static_cast<FWidgetBlueprintEditor*>(AssetEditorSubsystem->FindEditorForAsset(WidgetBlueprint.Get(), true))
		: nullptr;
	if (WidgetEditor == nullptr)
	{
		return false;
	}

	bPreviousIsSimulating = WidgetEditor->IsSimulating();
	bCapturedSimulationState = true;
	WidgetEditor->SetIsSimulating(true);
	WidgetEditor->SetCurrentMode(FWidgetBlueprintApplicationModes::DesignerMode);
	WidgetEditor->RefreshPreview();

	UUserWidget* PreviewWidget = WidgetEditor->GetPreview();
	if (PreviewWidget == nullptr)
	{
		return false;
	}

	PendingEntries.Reset();
	CurrentTime = 0.0f;
	MaxScheduledEndTime = 1.0f;
	TSet<FString> PhaseStack;
	SchedulePhaseEntries(PreviewWidget, RootPhase, 0.0f, TArray<FName>(), PhaseStack);
	PendingEntries.StableSort([](const FPendingEntry& Left, const FPendingEntry& Right)
	{
		return Left.AbsoluteStartTime < Right.AbsoluteStartTime;
	});
	PrimeAllDirectAnimationsToStart();

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(AsShared(), &FWidgetAnimTimelineDesignerPreviewController::Tick));
	return true;
}

bool FWidgetAnimTimelineDesignerPreviewController::Tick(float DeltaTime)
{
	if (bStopping)
	{
		return false;
	}

	UUserWidget* PreviewWidget = GetPreviewWidget();
	if (PreviewWidget == nullptr)
	{
		StopInternal(false);
		WidgetAnimTimelineDesignerPreviewController::ActiveController.Reset();
		return false;
	}

	CurrentTime += DeltaTime;
	bool bHasPendingEntry = false;
	for (int32 EntryIndex = 0; EntryIndex < PendingEntries.Num(); ++EntryIndex)
	{
		FPendingEntry& PendingEntry = PendingEntries[EntryIndex];
		if (PendingEntry.bFinished)
		{
			continue;
		}

		bHasPendingEntry = true;
		if (!PendingEntry.bStarted && CurrentTime >= PendingEntry.AbsoluteStartTime)
		{
			StartEntry(PendingEntry);
		}

		if (PendingEntry.bStarted && !PendingEntry.bFinished && PendingEntry.Entry.EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
		{
			TickDirectAnimation(PendingEntry, DeltaTime);
		}
	}

	if (!bHasPendingEntry && CurrentTime >= MaxScheduledEndTime + WidgetAnimTimelineDesignerPreviewController::EndPadding)
	{
		StopInternal(true);
		WidgetAnimTimelineDesignerPreviewController::ActiveController.Reset();
		return false;
	}

	return true;
}

void FWidgetAnimTimelineDesignerPreviewController::StopInternal(bool bRestorePreview)
{
	if (bStopping)
	{
		return;
	}

	bStopping = true;
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	if (UUserWidget* PreviewWidget = GetPreviewWidget())
	{
		StopAllWidgetAnimations(PreviewWidget);
	}

	if (bRestorePreview && WidgetBlueprint.Get() != nullptr && GEditor != nullptr)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			if (FWidgetBlueprintEditor* WidgetEditor = static_cast<FWidgetBlueprintEditor*>(AssetEditorSubsystem->FindEditorForAsset(WidgetBlueprint.Get(), false)))
			{
				if (bCapturedSimulationState)
				{
					WidgetEditor->SetIsSimulating(bPreviousIsSimulating);
				}
				WidgetEditor->RefreshPreview();
			}
		}
	}
}

void FWidgetAnimTimelineDesignerPreviewController::SchedulePhaseEntries(UUserWidget* CurrentWidget, const FWidgetAnimTimelinePhase& Phase, float PhaseBaseTime, const TArray<FName>& ContextPath, TSet<FString>& PhaseStack)
{
	if (CurrentWidget == nullptr)
	{
		return;
	}

	const FString StackKey = MakePhaseStackKey(CurrentWidget, Phase.PhaseName);
	if (PhaseStack.Contains(StackKey))
	{
		return;
	}

	PhaseStack.Add(StackKey);
	for (const FWidgetAnimTimelineEntry& Entry : Phase.Entries)
	{
		if (Entry.EntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase)
		{
			UUserWidget* TargetWidget = ResolveTargetWidget(CurrentWidget, Entry.TargetWidgetName);
			FWidgetAnimTimelinePhase ChildPhase;
			if (TargetWidget != nullptr && ReadPhaseFromWidget(TargetWidget, Entry.ChildPhaseName, ChildPhase))
			{
				TArray<FName> ChildContextPath = ContextPath;
				if (!Entry.TargetWidgetName.IsNone())
				{
					ChildContextPath.Add(Entry.TargetWidgetName);
				}
				SchedulePhaseEntries(TargetWidget, ChildPhase, PhaseBaseTime + Entry.StartTime, ChildContextPath, PhaseStack);
			}
			continue;
		}

		FPendingEntry PendingEntry;
		PendingEntry.Entry = Entry;
		PendingEntry.ContextPath = ContextPath;
		PendingEntry.AbsoluteStartTime = PhaseBaseTime + Entry.StartTime;
		PendingEntries.Add(PendingEntry);
		MaxScheduledEndTime = FMath::Max(MaxScheduledEndTime, PendingEntry.AbsoluteStartTime + GetEntryDuration(CurrentWidget, Entry));
	}
	PhaseStack.Remove(StackKey);
}

void FWidgetAnimTimelineDesignerPreviewController::PrimeAllDirectAnimationsToStart()
{
	UUserWidget* RootWidget = GetPreviewWidget();
	if (RootWidget == nullptr)
	{
		return;
	}

	TSet<FString> PrimedAnimations;
	UUserWidget* FlushAnchor = nullptr;
	for (FPendingEntry& PendingEntry : PendingEntries)
	{
		if (PendingEntry.Entry.EntryType != EWidgetAnimTimelineEntryType::DirectAnimation)
		{
			continue;
		}

		UUserWidget* ContextWidget = ResolveContextWidget(RootWidget, PendingEntry.ContextPath);
		UUserWidget* TargetWidget = ResolveTargetWidget(ContextWidget, PendingEntry.Entry.TargetWidgetName);
		UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, PendingEntry.Entry.AnimationName);
		if (TargetWidget == nullptr || Animation == nullptr)
		{
			continue;
		}

		const FString AnimationKey = FString::Printf(TEXT("%s:%s"), *GetPathNameSafe(TargetWidget), *GetPathNameSafe(Animation));
		if (PrimedAnimations.Contains(AnimationKey))
		{
			continue;
		}
		PrimedAnimations.Add(AnimationKey);

		PrimeDirectAnimation(TargetWidget, PendingEntry.Entry);
		FWidgetAnimationHandle PreviewHandle = TargetWidget->PlayAnimation(Animation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
		if (FWidgetAnimationState* State = PreviewHandle.GetAnimationState())
		{
			State->SetCurrentTime(0.0f);
			State->Tick(0.0f);
		}
		FlushAnchor = TargetWidget;
	}

	if (FlushAnchor != nullptr)
	{
		FlushAnchor->FlushAnimations();
	}

	for (FPendingEntry& PendingEntry : PendingEntries)
	{
		if (PendingEntry.Entry.EntryType != EWidgetAnimTimelineEntryType::DirectAnimation)
		{
			continue;
		}

		UUserWidget* ContextWidget = ResolveContextWidget(RootWidget, PendingEntry.ContextPath);
		UUserWidget* TargetWidget = ResolveTargetWidget(ContextWidget, PendingEntry.Entry.TargetWidgetName);
		UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, PendingEntry.Entry.AnimationName);
		if (TargetWidget != nullptr && Animation != nullptr)
		{
			TargetWidget->StopAnimation(Animation);
		}
	}
}

void FWidgetAnimTimelineDesignerPreviewController::StartEntry(FPendingEntry& PendingEntry)
{
	UUserWidget* RootWidget = GetPreviewWidget();
	UUserWidget* ContextWidget = ResolveContextWidget(RootWidget, PendingEntry.ContextPath);
	UUserWidget* TargetWidget = ResolveTargetWidget(ContextWidget, PendingEntry.Entry.TargetWidgetName);
	if (TargetWidget == nullptr)
	{
		PendingEntry.bFinished = true;
		return;
	}

	PendingEntry.bStarted = true;
	if (PendingEntry.Entry.EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		PrimeDirectAnimation(TargetWidget, PendingEntry.Entry);
		UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, PendingEntry.Entry.AnimationName);
		if (Animation == nullptr)
		{
			PendingEntry.bFinished = true;
			return;
		}

		// PlayAnimation() 后，UE内部会把 animation state 标记为刚开始播放，部分初始化/事件/求值不是马上完整落到 Slate/Widget 属性上。
		// DesignerPreview 下正常的 UUMGSequenceTickManager 不会像运行时那样自动 tick/flush，因为 IsDesignTime() 会跳过很多 animation tick。
		PendingEntry.AnimHandle = TargetWidget->PlayAnimation(
			Animation, 0.0f,
			PendingEntry.Entry.NumLoopsToPlay == 0 ? 1 : PendingEntry.Entry.NumLoopsToPlay,
			EUMGSequencePlayMode::Forward,
			FMath::Max(PendingEntry.Entry.PlaybackRate, KINDA_SMALL_NUMBER),
			false);
		// 所以手动调用 FWidgetAnimationState::Tick(DeltaTime) 前后，需要 FlushAnimations()：
		// 播放后 flush：清掉 begin-play 阶段的内部状态，否则后续 State->Tick() 可能不推进。
		// tick 后 flush：把动画求值结果真正应用到 Widget 属性和 Slate 显示上。
		TargetWidget->FlushAnimations();     // 让动画进入可 tick 状态
		return;
	}

	FWidgetAnimTimelinePhase ChildPhase;
	if (!ReadPhaseFromWidget(TargetWidget, PendingEntry.Entry.ChildPhaseName, ChildPhase))
	{
		PendingEntry.bFinished = true;
		return;
	}

	TArray<FName> ChildContextPath = PendingEntry.ContextPath;
	if (!PendingEntry.Entry.TargetWidgetName.IsNone())
	{
		ChildContextPath.Add(PendingEntry.Entry.TargetWidgetName);
	}

	PendingEntry.bFinished = true;
	TSet<FString> PhaseStack;
	SchedulePhaseEntries(TargetWidget, ChildPhase, PendingEntry.AbsoluteStartTime, ChildContextPath, PhaseStack);
}

void FWidgetAnimTimelineDesignerPreviewController::TickDirectAnimation(FPendingEntry& PendingEntry, float DeltaTime)
{
	UUserWidget* RootWidget = GetPreviewWidget();
	UUserWidget* ContextWidget = ResolveContextWidget(RootWidget, PendingEntry.ContextPath);
	UUserWidget* TargetWidget = ResolveTargetWidget(ContextWidget, PendingEntry.Entry.TargetWidgetName);
	if (TargetWidget == nullptr)
	{
		PendingEntry.bFinished = true;
		return;
	}

	FWidgetAnimationState* State = PendingEntry.AnimHandle.GetAnimationState();
	if (State == nullptr)
	{
		PendingEntry.bFinished = true;
		return;
	}

	if (State->GetPlaybackStatus() == EMovieScenePlayerStatus::Playing)
	{
		State->Tick(DeltaTime);	// 手动推进动画
		TargetWidget->FlushAnimations();	// 把推进后的结果刷新到预览界面
		return;
	}

	PendingEntry.bFinished = true;
	if (UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, PendingEntry.Entry.AnimationName))
	{
		TargetWidget->StopAnimation(Animation);
	}
}

void FWidgetAnimTimelineDesignerPreviewController::PrimeDirectAnimation(UUserWidget* TargetWidget, const FWidgetAnimTimelineEntry& Entry)
{
	UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, Entry.AnimationName);
	if (Animation == nullptr)
	{
		return;
	}

	if (Entry.InterruptMode == EWidgetAnimTimelineInterruptMode::StopActiveAnimations)
	{
		TargetWidget->StopAnimation(Animation);
	}
}

void FWidgetAnimTimelineDesignerPreviewController::StopAllWidgetAnimations(UUserWidget* Widget) const
{
	if (Widget == nullptr)
	{
		return;
	}

	Widget->StopAllAnimations();
	if (Widget->WidgetTree == nullptr)
	{
		return;
	}

	Widget->WidgetTree->ForEachWidget([this](UWidget* ChildWidget)
	{
		if (UUserWidget* ChildUserWidget = Cast<UUserWidget>(ChildWidget))
		{
			StopAllWidgetAnimations(ChildUserWidget);
		}
	});
}

bool FWidgetAnimTimelineDesignerPreviewController::ReadPhaseFromWidget(UUserWidget* Widget, FName PhaseName, FWidgetAnimTimelinePhase& OutPhase) const
{
	if (Widget == nullptr || PhaseName.IsNone())
	{
		return false;
	}

	for (TFieldIterator<FStructProperty> It(Widget->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->Struct != FWidgetAnimTimelineConfig::StaticStruct())
		{
			continue;
		}

		const FWidgetAnimTimelineConfig* Config = It->ContainerPtrToValuePtr<FWidgetAnimTimelineConfig>(Widget);
		if (Config == nullptr)
		{
			continue;
		}

		if (const FWidgetAnimTimelinePhase* Phase = Config->Phases.FindByPredicate([PhaseName](const FWidgetAnimTimelinePhase& Item)
		{
			return Item.PhaseName == PhaseName;
		}))
		{
			OutPhase = *Phase;
			return true;
		}
	}

	return false;
}

float FWidgetAnimTimelineDesignerPreviewController::GetEntryDuration(UUserWidget* ContextWidget, const FWidgetAnimTimelineEntry& Entry) const
{
	if (Entry.EntryType != EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		return WidgetAnimTimelineDesignerPreviewController::FallbackEntryDuration;
	}

	UUserWidget* TargetWidget = ResolveTargetWidget(ContextWidget, Entry.TargetWidgetName);
	UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, Entry.AnimationName);
	if (Animation == nullptr)
	{
		return WidgetAnimTimelineDesignerPreviewController::FallbackEntryDuration;
	}

	const int32 LoopCount = Entry.NumLoopsToPlay == 0 ? 1 : FMath::Max(Entry.NumLoopsToPlay, 1);
	return Animation->GetEndTime() * LoopCount / FMath::Max(Entry.PlaybackRate, KINDA_SMALL_NUMBER);
}

UUserWidget* FWidgetAnimTimelineDesignerPreviewController::GetPreviewWidget() const
{
	if (WidgetBlueprint.Get() == nullptr || GEditor == nullptr)
	{
		return nullptr;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	FWidgetBlueprintEditor* WidgetEditor = AssetEditorSubsystem != nullptr
		? static_cast<FWidgetBlueprintEditor*>(AssetEditorSubsystem->FindEditorForAsset(WidgetBlueprint.Get(), false))
		: nullptr;
	return WidgetEditor != nullptr ? WidgetEditor->GetPreview() : nullptr;
}

UUserWidget* FWidgetAnimTimelineDesignerPreviewController::ResolveContextWidget(UUserWidget* RootWidget, const TArray<FName>& ContextPath) const
{
	UUserWidget* CurrentWidget = RootWidget;
	for (FName WidgetName : ContextPath)
	{
		CurrentWidget = ResolveTargetWidget(CurrentWidget, WidgetName);
		if (CurrentWidget == nullptr)
		{
			return nullptr;
		}
	}
	return CurrentWidget;
}

UUserWidget* FWidgetAnimTimelineDesignerPreviewController::ResolveTargetWidget(UUserWidget* ContextWidget, FName TargetWidgetName) const
{
	if (ContextWidget == nullptr)
	{
		return nullptr;
	}

	if (TargetWidgetName.IsNone())
	{
		return ContextWidget;
	}

	return ContextWidget->WidgetTree != nullptr ? Cast<UUserWidget>(ContextWidget->WidgetTree->FindWidget(TargetWidgetName)) : nullptr;
}

UWidgetAnimation* FWidgetAnimTimelineDesignerPreviewController::ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName) const
{
	if (TargetWidget == nullptr || AnimationName.IsNone())
	{
		return nullptr;
	}

	for (TFieldIterator<FObjectProperty> It(TargetWidget->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		FString PropertyName = It->GetName();
		PropertyName.RemoveFromEnd(TEXT("_INST"));
		if (PropertyName == AnimationName.ToString())
		{
			return Cast<UWidgetAnimation>(It->GetObjectPropertyValue_InContainer(TargetWidget));
		}
	}

	return nullptr;
}

FString FWidgetAnimTimelineDesignerPreviewController::MakePhaseStackKey(UUserWidget* Widget, FName PhaseName) const
{
	return FString::Printf(TEXT("%s:%s"), Widget != nullptr ? *GetPathNameSafe(Widget->GetClass()) : TEXT("None"), *PhaseName.ToString());
}
