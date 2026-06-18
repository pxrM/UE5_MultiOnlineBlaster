#include "WidgetAnimTimelineSequence.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "TimerManager.h"

void UWidgetAnimTimelinePlayer::Initialize(UUserWidget* InOwnerWidget, const FWidgetAnimTimelineConfig& InConfig)
{
	OwnerWidget = InOwnerWidget;
	Config = InConfig;
}

bool UWidgetAnimTimelinePlayer::PlayPhase(FName PhaseName)
{
	const FWidgetAnimTimelinePhase* Phase = FindPhase(PhaseName);
	if (Phase == nullptr || OwnerWidget == nullptr)
	{
		return false;
	}

	Stop();
	for (const FWidgetAnimTimelineEntry& Entry : Phase->Entries)
	{
		ScheduleEntry(Entry);
	}

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

	for (const FActiveTimelineAnimation& ActiveAnimation : ActiveAnimations)
	{
		UUserWidget* TargetWidget = ActiveAnimation.TargetWidget.Get();
		UWidgetAnimation* Animation = ActiveAnimation.Animation.Get();
		if (TargetWidget != nullptr && Animation != nullptr)
		{
			ApplyInterruptMode(TargetWidget, Animation, ActiveAnimation.InterruptMode);
		}
	}
	ActiveAnimations.Reset();
}

void UWidgetAnimTimelinePlayer::ScheduleEntry(const FWidgetAnimTimelineEntry& Entry)
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
		ExecuteEntry(Entry);
		return;
	}

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::ExecuteEntry, Entry), Entry.StartTime, false);
	ActiveTimers.Add(TimerHandle);
}

void UWidgetAnimTimelinePlayer::ExecuteEntry(FWidgetAnimTimelineEntry Entry)
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
				if (ChildPlayer->PlayPhase(Entry.ChildPhaseName))
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

	TargetWidget->PlayAnimation(Animation, 0.0f, Entry.NumLoopsToPlay, EUMGSequencePlayMode::Forward, Entry.PlaybackRate);
	TrackActiveAnimation(TargetWidget, Animation, Entry.InterruptMode);
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

void UWidgetAnimTimelinePlayer::TrackActiveAnimation(UUserWidget* TargetWidget, UWidgetAnimation* Animation, EWidgetAnimTimelineInterruptMode InterruptMode)
{
	if (TargetWidget == nullptr || Animation == nullptr)
	{
		return;
	}

	ActiveAnimations.RemoveAll([TargetWidget, Animation](const FActiveTimelineAnimation& ActiveAnimation)
	{
		return ActiveAnimation.TargetWidget.Get() == TargetWidget && ActiveAnimation.Animation.Get() == Animation;
	});

	FActiveTimelineAnimation ActiveAnimation;
	ActiveAnimation.TargetWidget = TargetWidget;
	ActiveAnimation.Animation = Animation;
	ActiveAnimation.InterruptMode = InterruptMode;
	ActiveAnimations.Add(ActiveAnimation);
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
