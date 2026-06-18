#pragma once

#include "Animation/WidgetAnimationHandle.h"
#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "WidgetAnimTimelineSequence.h"

class IPropertyHandle;
class UUserWidget;
class UWidgetAnimation;
class UWidgetBlueprint;

class FWidgetAnimTimelineDesignerPreviewController : public TSharedFromThis<FWidgetAnimTimelineDesignerPreviewController>
{
public:
	static bool Play(TSharedPtr<IPropertyHandle> PhaseHandle);
	static bool Play(UWidgetBlueprint* WidgetBlueprint, int32 PhaseIndex);
	static void Stop(TSharedPtr<IPropertyHandle> PhaseHandle);
	static void StopActive();

private:
	struct FPendingEntry
	{
		FWidgetAnimTimelineEntry Entry;
		TArray<FName> ContextPath;
		float AbsoluteStartTime = 0.0f;
		FWidgetAnimationHandle AnimHandle;
		bool bStarted = false;
		bool bFinished = false;
	};

	static UWidgetBlueprint* GetWidgetBlueprint(TSharedPtr<IPropertyHandle> PhaseHandle);
	static bool ReadRootPhase(TSharedPtr<IPropertyHandle> PhaseHandle, FWidgetAnimTimelinePhase& OutPhase);
	static bool ReadRootPhase(UWidgetBlueprint* WidgetBlueprint, int32 PhaseIndex, FWidgetAnimTimelinePhase& OutPhase);

	FWidgetAnimTimelineDesignerPreviewController(UWidgetBlueprint* InWidgetBlueprint, const FWidgetAnimTimelinePhase& InRootPhase);

	bool Start();
	bool Tick(float DeltaTime);
	void StopInternal(bool bRestorePreview);
	void SchedulePhaseEntries(UUserWidget* CurrentWidget, const FWidgetAnimTimelinePhase& Phase, float PhaseBaseTime, const TArray<FName>& ContextPath, TSet<FString>& PhaseStack);
	void PrimeAllDirectAnimationsToStart();
	void StartEntry(FPendingEntry& PendingEntry);
	void TickDirectAnimation(FPendingEntry& PendingEntry, float DeltaTime);
	void PrimeDirectAnimation(UUserWidget* TargetWidget, const FWidgetAnimTimelineEntry& Entry);
	void StopAllWidgetAnimations(UUserWidget* Widget) const;
	bool ReadPhaseFromWidget(UUserWidget* Widget, FName PhaseName, FWidgetAnimTimelinePhase& OutPhase) const;
	float GetEntryDuration(UUserWidget* ContextWidget, const FWidgetAnimTimelineEntry& Entry) const;
	UUserWidget* GetPreviewWidget() const;
	UUserWidget* ResolveContextWidget(UUserWidget* RootWidget, const TArray<FName>& ContextPath) const;
	UUserWidget* ResolveTargetWidget(UUserWidget* ContextWidget, FName TargetWidgetName) const;
	UWidgetAnimation* ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName) const;
	FString MakePhaseStackKey(UUserWidget* Widget, FName PhaseName) const;

	TWeakObjectPtr<UWidgetBlueprint> WidgetBlueprint;
	FWidgetAnimTimelinePhase RootPhase;
	TArray<FPendingEntry> PendingEntries;
	FTSTicker::FDelegateHandle TickHandle;
	float CurrentTime = 0.0f;
	float MaxScheduledEndTime = 1.0f;
	bool bPreviousIsSimulating = false;
	bool bCapturedSimulationState = false;
	bool bStopping = false;
};
