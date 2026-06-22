#include "WidgetAnimTimelineSequence.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogWidgetAnimTimeline, Log, All);

namespace
{
	float GetSafePlaybackRate(float PlaybackRate)
	{
		return FMath::Max(PlaybackRate, 0.001f);
	}

	int32 GetSafeNumLoopsToPlay(int32 NumLoopsToPlay)
	{
		return FMath::Max(NumLoopsToPlay, 0);
	}
}

void UWidgetAnimTimelinePlayer::Initialize(UUserWidget* InOwnerWidget, const FWidgetAnimTimelineConfig& InConfig)
{
	OwnerWidget = InOwnerWidget;
	Config = InConfig;
}

bool UWidgetAnimTimelinePlayer::PlayPhase(FName PhaseName)
{
	TSet<FString> PhaseStack;
	return PlayPhaseInternal(PhaseName, PhaseStack);
}

bool UWidgetAnimTimelinePlayer::PlayPhaseInternal(FName PhaseName, TSet<FString>& PhaseStack)
{
	const FWidgetAnimTimelinePhase* Phase = FindPhase(PhaseName);
	if (Phase == nullptr || OwnerWidget == nullptr)
	{
		return false;
	}

	const FString PhaseStackKey = MakePhaseStackKey(PhaseName);
	if (PhaseStack.Contains(PhaseStackKey))
	{
		UE_LOG(LogWidgetAnimTimeline, Warning, TEXT("Blocked recursive phase playback: %s"), *PhaseStackKey);
		return false;
	}

	PhaseStack.Add(PhaseStackKey);

	Stop();
	for (const FWidgetAnimTimelineEntry& Entry : Phase->Entries)
	{
		ScheduleEntry(Entry, PhaseStack);
	}

	PhaseStack.Remove(PhaseStackKey);
	return true;
}

void UWidgetAnimTimelinePlayer::Stop()
{
	if (OwnerWidget != nullptr)
	{
		if (UWorld* World = OwnerWidget->GetWorld())
		{
			for (FTimerHandle& TimerHandle : ActiveTimers)
			{
				World->GetTimerManager().ClearTimer(TimerHandle);
			}
		}
	}

	ActiveTimers.Reset();

	for (TWeakObjectPtr<UWidgetAnimTimelinePlayer>& ChildPlayerPtr : ActiveChildPlayers)
	{
		if (UWidgetAnimTimelinePlayer* ChildPlayer = ChildPlayerPtr.Get())
		{
			ChildPlayer->Stop();
		}
	}
	ActiveChildPlayers.Reset();

	for (FActiveTimelineAnimation& ActiveAnimation : ActiveAnimations)
	{
		UUserWidget* TargetWidget = ActiveAnimation.TargetWidget.Get();
		UWidgetAnimation* Animation = ActiveAnimation.Animation.Get();
		if (OwnerWidget != nullptr)
		{
			if (UWorld* World = OwnerWidget->GetWorld())
			{
				World->GetTimerManager().ClearTimer(ActiveAnimation.CleanupTimer);
			}
		}
		if (TargetWidget != nullptr && Animation != nullptr)
		{
			ApplyInterruptMode(TargetWidget, Animation, ActiveAnimation.InterruptMode);
		}
	}
	ActiveAnimations.Reset();
}

void UWidgetAnimTimelinePlayer::ScheduleEntry(const FWidgetAnimTimelineEntry& Entry, const TSet<FString>& PhaseStack)
{
	if (OwnerWidget == nullptr)
	{
		return;
	}

	UWorld* World = OwnerWidget->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (Entry.StartTime <= 0.0f)
	{
		ExecuteEntry(Entry, PhaseStack);
		return;
	}

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::ExecuteEntry, Entry, PhaseStack), Entry.StartTime, false);
	ActiveTimers.Add(TimerHandle);
}

void UWidgetAnimTimelinePlayer::ExecuteEntry(FWidgetAnimTimelineEntry Entry, TSet<FString> PhaseStack)
{
	UUserWidget* TargetWidget = ResolveTargetWidget(Entry.TargetWidgetName);
	if (TargetWidget == nullptr)
	{
		return;
	}

	if (Entry.EntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase)
	{
		// Child phases are delegated to the target widget's own player to keep nested widget timelines local.
		if (UWidgetAnimTimelineHostWidget* ChildHost = Cast<UWidgetAnimTimelineHostWidget>(TargetWidget))
		{
			if (UWidgetAnimTimelinePlayer* ChildPlayer = ChildHost->GetAnimTimelinePlayer())
			{
				if (ChildPlayer->PlayPhaseInternal(Entry.ChildPhaseName, PhaseStack))
				{
					ActiveChildPlayers.Add(ChildPlayer);
				}
			}
		}
		return;
	}

	UWidgetAnimation* Animation = ResolveAnimation(TargetWidget, Entry.AnimationName);
	if (Animation == nullptr)
	{
		return;
	}

	ApplyInterruptMode(TargetWidget, Animation, Entry.InterruptMode);

	const int32 SafeNumLoopsToPlay = GetSafeNumLoopsToPlay(Entry.NumLoopsToPlay);
	const float SafePlaybackRate = GetSafePlaybackRate(Entry.PlaybackRate);
	TargetWidget->PlayAnimation(Animation, 0.0f, SafeNumLoopsToPlay, EUMGSequencePlayMode::Forward, SafePlaybackRate);
	TrackActiveAnimation(TargetWidget, Animation, Entry, SafePlaybackRate, SafeNumLoopsToPlay);
}

void UWidgetAnimTimelinePlayer::ApplyInterruptMode(UUserWidget* TargetWidget, UWidgetAnimation* Animation, EWidgetAnimTimelineInterruptMode InterruptMode) const
{
	if (TargetWidget == nullptr || Animation == nullptr)
	{
		return;
	}

	switch (InterruptMode)
	{
	case EWidgetAnimTimelineInterruptMode::StopActiveAnimations:
		TargetWidget->StopAnimation(Animation);
		break;
	case EWidgetAnimTimelineInterruptMode::FinishActiveAnimations:
		TargetWidget->SetAnimationCurrentTime(Animation, Animation->GetEndTime());
		TargetWidget->FlushAnimations();
		TargetWidget->StopAnimation(Animation);
		break;
	case EWidgetAnimTimelineInterruptMode::None:
	default:
		break;
	}
}

void UWidgetAnimTimelinePlayer::TrackActiveAnimation(UUserWidget* TargetWidget, UWidgetAnimation* Animation, const FWidgetAnimTimelineEntry& Entry, float PlaybackRate, int32 NumLoopsToPlay)
{
	if (TargetWidget == nullptr || Animation == nullptr)
	{
		return;
	}

	RemoveActiveAnimation(TargetWidget, Animation);

	FActiveTimelineAnimation ActiveAnimation;
	ActiveAnimation.TargetWidget = TargetWidget;
	ActiveAnimation.Animation = Animation;
	ActiveAnimation.InterruptMode = Entry.InterruptMode;

	if (NumLoopsToPlay > 0 && OwnerWidget != nullptr)
	{
		if (UWorld* World = OwnerWidget->GetWorld())
		{
			const float Duration = Animation->GetEndTime() * NumLoopsToPlay / GetSafePlaybackRate(PlaybackRate);
			if (Duration > 0.0f)
			{
				World->GetTimerManager().SetTimer(
					ActiveAnimation.CleanupTimer,
					FTimerDelegate::CreateUObject(
						this,
						&ThisClass::CleanupFinishedAnimation,
						TWeakObjectPtr<UUserWidget>(TargetWidget),
						TWeakObjectPtr<UWidgetAnimation>(Animation)),
					Duration,
					false);
			}
		}
	}

	ActiveAnimations.Add(ActiveAnimation);
}

void UWidgetAnimTimelinePlayer::RemoveActiveAnimation(UUserWidget* TargetWidget, UWidgetAnimation* Animation)
{
	if (OwnerWidget == nullptr)
	{
		ActiveAnimations.RemoveAll([TargetWidget, Animation](const FActiveTimelineAnimation& ActiveAnimation)
		{
			return ActiveAnimation.TargetWidget.Get() == TargetWidget && ActiveAnimation.Animation.Get() == Animation;
		});
		return;
	}

	UWorld* World = OwnerWidget->GetWorld();
	for (int32 Index = ActiveAnimations.Num() - 1; Index >= 0; --Index)
	{
		FActiveTimelineAnimation& ActiveAnimation = ActiveAnimations[Index];
		if (ActiveAnimation.TargetWidget.Get() == TargetWidget && ActiveAnimation.Animation.Get() == Animation)
		{
			if (World != nullptr)
			{
				World->GetTimerManager().ClearTimer(ActiveAnimation.CleanupTimer);
			}
			ActiveAnimations.RemoveAtSwap(Index);
		}
	}
}

void UWidgetAnimTimelinePlayer::CleanupFinishedAnimation(TWeakObjectPtr<UUserWidget> TargetWidget, TWeakObjectPtr<UWidgetAnimation> Animation)
{
	RemoveActiveAnimation(TargetWidget.Get(), Animation.Get());
}

UUserWidget* UWidgetAnimTimelinePlayer::ResolveTargetWidget(FName TargetWidgetName) const
{
	if (OwnerWidget == nullptr)
	{
		return nullptr;
	}

	if (TargetWidgetName.IsNone())
	{
		return OwnerWidget;
	}

	if (OwnerWidget->WidgetTree == nullptr)
	{
		return nullptr;
	}

	return Cast<UUserWidget>(OwnerWidget->WidgetTree->FindWidget(TargetWidgetName));
}

UWidgetAnimation* UWidgetAnimTimelinePlayer::ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName) const
{
	if (TargetWidget == nullptr || AnimationName.IsNone())
	{
		return nullptr;
	}

	UClass* WidgetClass = TargetWidget->GetClass();
	for (TFieldIterator<FObjectProperty> It(WidgetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FObjectProperty* ObjectProperty = *It;
		if (ObjectProperty->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		FString PropertyName = ObjectProperty->GetName();
		// Blueprint-generated WidgetAnimation properties commonly carry the _INST suffix.
		PropertyName.RemoveFromEnd(TEXT("_INST"));
		if (PropertyName == AnimationName.ToString())
		{
			return Cast<UWidgetAnimation>(ObjectProperty->GetObjectPropertyValue_InContainer(TargetWidget));
		}
	}

	return nullptr;
}

const FWidgetAnimTimelinePhase* UWidgetAnimTimelinePlayer::FindPhase(FName PhaseName) const
{
	return Config.Phases.FindByPredicate([PhaseName](const FWidgetAnimTimelinePhase& Phase)
	{
		return Phase.PhaseName == PhaseName;
	});
}

FString UWidgetAnimTimelinePlayer::MakePhaseStackKey(FName PhaseName) const
{
	return FString::Printf(TEXT("%s:%s"), *GetPathNameSafe(OwnerWidget), *PhaseName.ToString());
}

UWidgetAnimTimelinePlayer* UWidgetAnimTimelineHostWidget::GetAnimTimelinePlayer() const
{
	return AnimTimelinePlayer;
}

void UWidgetAnimTimelineHostWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	AnimTimelinePlayer = NewObject<UWidgetAnimTimelinePlayer>(this);
	AnimTimelinePlayer->Initialize(this, AnimTimelineConfig);
}

void UWidgetAnimTimelineHostWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Auto-play is intentionally bound to widget construction rather than initialization so Slate is ready.
	if (AnimTimelinePlayer != nullptr && !AnimTimelineConfig.AutoPlayPhaseName.IsNone())
	{
		AnimTimelinePlayer->PlayPhase(AnimTimelineConfig.AutoPlayPhaseName);
	}
}

void UWidgetAnimTimelineHostWidget::NativeDestruct()
{
	if (AnimTimelinePlayer != nullptr)
	{
		AnimTimelinePlayer->Stop();
	}

	Super::NativeDestruct();
}
