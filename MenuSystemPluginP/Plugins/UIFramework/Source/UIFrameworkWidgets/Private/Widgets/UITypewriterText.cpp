// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UITypewriterText.h"

// #include "AkAudioDevice.h"
// #include "AkAudioEvent.h"
// #include "AkGameplayStatics.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogTypewriterTextWidget, Log, All);

namespace TypewriterTextWidgetDebug
{
	const TCHAR* LexToString(ETypewriterRichTextTokenType TokenType)
	{
		switch (TokenType)
		{
		case ETypewriterRichTextTokenType::VisibleText:
			return TEXT("VisibleText");
		case ETypewriterRichTextTokenType::OpenTag:
			return TEXT("OpenTag");
		case ETypewriterRichTextTokenType::CloseTag:
			return TEXT("CloseTag");
		case ETypewriterRichTextTokenType::SelfClosingTag:
			return TEXT("SelfClosingTag");
		case ETypewriterRichTextTokenType::Whitespace:
			return TEXT("Whitespace");
		case ETypewriterRichTextTokenType::NewLine:
			return TEXT("NewLine");
		default:
			return TEXT("Unknown");
		}
	}
}

void FTypewriterRichTextParser::Parse(const FText& SourceText, const FTypewriterParagraphConfig& Config)
{
	Tokens.Reset();
	TotalVisibleTokenCount = 0;
	PlainVisibleText.Reset();

	const FString Source = SourceText.ToString();
	for (int32 Index = 0; Index < Source.Len();)
	{
		int32 TagEndIndex = INDEX_NONE;
		FString RawTag;
		if (IsTagToken(Source, Index, TagEndIndex, RawTag))
		{
			bool bIsClosingTag = false;
			bool bIsSelfClosingTag = false;
			const FString TagName = ExtractTagName(RawTag, bIsClosingTag, bIsSelfClosingTag);
			if (bIsSelfClosingTag)
			{
				AddToken(ETypewriterRichTextTokenType::SelfClosingTag, RawTag, TagName, Config.bInlineWidgetConsumesVisibleToken);
			}
			else if (bIsClosingTag)
			{
				AddToken(ETypewriterRichTextTokenType::CloseTag, RawTag, TagName, false);
			}
			else
			{
				AddToken(ETypewriterRichTextTokenType::OpenTag, RawTag, TagName, false);
			}

			Index = TagEndIndex + 1;
			continue;
		}

		const TCHAR Character = Source[Index];
		FString CharacterString;
		CharacterString.AppendChar(Character);

		if (Character == TEXT('\r'))
		{
			++Index;
			continue;
		}
		if (Character == TEXT('\n'))
		{
			AddToken(ETypewriterRichTextTokenType::NewLine, CharacterString, FString(), Config.bNewLineConsumesVisibleToken);
		}
		else if (IsWhitespace(Character))
		{
			AddToken(ETypewriterRichTextTokenType::Whitespace, CharacterString, FString(), Config.bWhitespaceConsumesVisibleToken);
		}
		else
		{
			AddToken(ETypewriterRichTextTokenType::VisibleText, CharacterString, FString(), true);
		}
		++Index;
	}

	UE_LOG(LogTypewriterTextWidget, Log, TEXT("Parse RichText Source=[%s], Tokens=%d, VisibleTokens=%d"), *Source, Tokens.Num(), TotalVisibleTokenCount);
	for (int32 TokenIndex = 0; TokenIndex < Tokens.Num(); ++TokenIndex)
	{
		const FTypewriterRichTextToken& Token = Tokens[TokenIndex];
		UE_LOG(LogTypewriterTextWidget, Verbose, TEXT("  Token[%d] Type=%s Raw=[%s] TagName=[%s] VisibleIndex=%d Consumes=%d"),
			TokenIndex,
			TypewriterTextWidgetDebug::LexToString(Token.Type),
			*Token.RawText,
			*Token.TagName,
			Token.VisibleTokenIndex,
			Token.bConsumesVisibleToken ? 1 : 0);
	}
	
	// todo 每段 Parse 后缓存 BuildVisibleString 结果
}

int32 FTypewriterRichTextParser::GetTotalVisibleTokenCount() const
{
	return TotalVisibleTokenCount;
}

TArray<FTypewriterTimelineKey> FTypewriterRichTextParser::ResolveTimelineKeys(const TArray<FTypewriterTimelineKey>& TimelineKeys) const
{
	TArray<FTypewriterTimelineKey> ResolvedKeys = TimelineKeys;
	for (FTypewriterTimelineKey& TimelineKey : ResolvedKeys)
	{
		const int32 AnchorVisibleTokenCount = ResolveVisibleTokenCountByAnchor(TimelineKey);
		if (AnchorVisibleTokenCount != INDEX_NONE)
		{
			TimelineKey.VisibleTokenCount = AnchorVisibleTokenCount;
		}
	}
	return ResolvedKeys;
}

FText FTypewriterRichTextParser::BuildVisibleText(int32 VisibleTokenCount, const FText& EndMarkerRichText) const
{
	FString Result = BuildVisibleString(VisibleTokenCount);
	if (!EndMarkerRichText.IsEmpty())
	{
		Result += EndMarkerRichText.ToString();
	}
	UE_LOG(LogTypewriterTextWidget, Verbose, TEXT("BuildVisibleText Visible=%d EndMarker=[%s] Result=[%s]"), VisibleTokenCount, *EndMarkerRichText.ToString(), *Result);
	return FText::FromString(Result);
}

FText FTypewriterRichTextParser::BuildVisibleText(int32 VisibleTokenCount) const
{
	const FString Result = BuildVisibleString(VisibleTokenCount);
	UE_LOG(LogTypewriterTextWidget, Verbose, TEXT("BuildVisibleText Visible=%d Result=[%s]"), VisibleTokenCount, *Result);
	return FText::FromString(Result);
}

bool FTypewriterRichTextParser::IsWhitespace(TCHAR Character)
{
	return Character == TEXT(' ') || Character == TEXT('\t');
}

bool FTypewriterRichTextParser::IsTagToken(const FString& Source, int32 StartIndex, int32& OutEndIndex, FString& OutRawTag)
{
	// Source = TEXT("小心 <Icon id=\"Warning\"/> 爆炸");
	// StartIndex = 3;	// 指向 '<'
	// OutEndIndex = 标签结尾 '>' 的位置;
	// OutRawTag = TEXT("<Icon id=\"Warning\"/>");
	
	if (!Source.IsValidIndex(StartIndex) || Source[StartIndex] != TEXT('<'))
	{
		return false;
	}

	const int32 CloseIndex = Source.Find(TEXT(">"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex + 1);
	if (CloseIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 NextOpenIndex = Source.Find(TEXT("<"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex + 1);
	if (NextOpenIndex != INDEX_NONE && NextOpenIndex < CloseIndex)
	{
		return false;
	}

	OutEndIndex = CloseIndex;
	OutRawTag = Source.Mid(StartIndex, CloseIndex - StartIndex + 1);
	return true;
}

FString FTypewriterRichTextParser::ExtractTagName(const FString& RawTag, bool& bOutIsClosingTag, bool& bOutIsSelfClosingTag)
{
	// 复制一份可修改的字符串，RawTag 需要保留原样用于后续 RichText 重建。
	FString Inner = RawTag;
	// 去掉标签外围的尖括号，例如 <Icon id="Warning"/> -> Icon id="Warning"/。
	Inner.RemoveFromStart(TEXT("<"));
	Inner.RemoveFromEnd(TEXT(">"));
	Inner.TrimStartAndEndInline();

	// 关闭标签以 / 开头，例如 </> 或 </Red>。
	bOutIsClosingTag = Inner.StartsWith(TEXT("/"));
	if (bOutIsClosingTag)
	{
		// 去掉关闭标签开头的 /，便于后续统一提取标签名。
		Inner.RightChopInline(1);
		Inner.TrimStartInline();
	}

	// 自闭合标签以 / 结尾，例如 <Icon id="Warning"/>。
	bOutIsSelfClosingTag = Inner.EndsWith(TEXT("/"));	// 自闭合标签
	if (bOutIsSelfClosingTag)
	{
		// 去掉自闭合标签结尾的 /，保留标签名和参数部分。
		Inner.LeftChopInline(1);	// "Icon id=\"Warning\" size=\"Small\""
		Inner.TrimEndInline();
	}

	FString TagName;
	// 标签名是第一个空格前的内容；例如 Icon id="Warning" 会提取出 Icon。
	if (!Inner.Split(TEXT(" "), &TagName, nullptr))
	{
		// 没有参数的标签直接使用完整内容，例如 Red 或空关闭标签 </> 的空字符串。
		TagName = Inner;
	}
	return TagName;
}

void FTypewriterRichTextParser::AddToken(ETypewriterRichTextTokenType Type, const FString& RawText, const FString& TagName, bool bConsumesVisibleToken)
{
	FTypewriterRichTextToken& Token = Tokens.AddDefaulted_GetRef();
	Token.Type = Type;
	Token.RawText = RawText;
	Token.TagName = TagName;
	Token.bConsumesVisibleToken = bConsumesVisibleToken;
	if (bConsumesVisibleToken)
	{
		Token.VisibleTokenIndex = TotalVisibleTokenCount;
		++TotalVisibleTokenCount;
		PlainVisibleText += Type == ETypewriterRichTextTokenType::SelfClosingTag ? TEXT("\xFFFC") : RawText;
	}
}

int32 FTypewriterRichTextParser::ResolveVisibleTokenCountByAnchor(const FTypewriterTimelineKey& TimelineKey) const
{
	if (TimelineKey.VisibleTextAnchor.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 RequiredOccurrence = FMath::Max(TimelineKey.AnchorOccurrence, 1);
	int32 SearchStartIndex = 0;
	int32 FoundIndex = INDEX_NONE;
	for (int32 OccurrenceIndex = 0; OccurrenceIndex < RequiredOccurrence; ++OccurrenceIndex)
	{
		FoundIndex = PlainVisibleText.Find(TimelineKey.VisibleTextAnchor, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchStartIndex);
		if (FoundIndex == INDEX_NONE)
		{
			UE_LOG(LogTypewriterTextWidget, Warning, TEXT("Timeline anchor not found. Anchor=[%s], Occurrence=%d, PlainText=[%s], FallbackVisibleTokenCount=%d"),
				*TimelineKey.VisibleTextAnchor,
				RequiredOccurrence,
				*PlainVisibleText,
				TimelineKey.VisibleTokenCount);
			return INDEX_NONE;
		}

		SearchStartIndex = FoundIndex + TimelineKey.VisibleTextAnchor.Len();
	}

	const int32 ResolvedVisibleTokenCount = FoundIndex + TimelineKey.VisibleTextAnchor.Len();
	UE_LOG(LogTypewriterTextWidget, Log, TEXT("Timeline anchor resolved. Anchor=[%s], Occurrence=%d, VisibleTokenCount=%d, PlainText=[%s]"),
		*TimelineKey.VisibleTextAnchor,
		RequiredOccurrence,
		ResolvedVisibleTokenCount,
		*PlainVisibleText);
	return FMath::Clamp(ResolvedVisibleTokenCount, 0, TotalVisibleTokenCount);
}

FString FTypewriterRichTextParser::BuildVisibleString(int32 VisibleTokenCount) const
{
	/*
	* 举例，原始文本是：<Red>危险</>正在靠近
	*		VisibleTokenCount = 1，它不能直接截成：<Red>危，这是不完整的RichText，<Red> 没有闭合。
	*		函数会输出：<Red>危</>
	*	原文：<Red>危险</>来了
	*		VisibleTokenCount = 0 -> ""
	*		VisibleTokenCount = 1 -> "<Red>危</>"
	*		VisibleTokenCount = 2 -> "<Red>危险</>"
	*		VisibleTokenCount = 3 -> "<Red>危险</>来"
	*		VisibleTokenCount = 4 -> "<Red>危险</>来了"
	*/
	const int32 ClampedVisibleTokenCount = FMath::Clamp(VisibleTokenCount, 0, TotalVisibleTokenCount);
	FString Result;
	// 记录已经输出但还没有闭合的样式标签，提前截断时需要自动补 </>。
	TArray<FString> OpenTagStack;

	for (const FTypewriterRichTextToken& Token : Tokens)
	{
		// 遇到第一个不应显示的可见 Token 就停止；非可见标签不参与计数。
		if (Token.bConsumesVisibleToken && Token.VisibleTokenIndex >= ClampedVisibleTokenCount)
		{
			break;
		}

		switch (Token.Type)
		{
		case ETypewriterRichTextTokenType::OpenTag:
			// 开启样式标签必须写入结果，并压栈等待自然闭合或自动补闭合。
			// 例如显示 <Red>危 时，最终会在函数末尾补成 <Red>危</>。
			Result += Token.RawText;
			OpenTagStack.Push(Token.TagName);
			break;
		case ETypewriterRichTextTokenType::CloseTag:
			// 原文中已经到达关闭标签时，直接写回并从栈中移除对应开启标签。
			Result += Token.RawText;
			if (OpenTagStack.Num() > 0)
			{
				OpenTagStack.Pop(EAllowShrinking::No);
			}
			break;
		case ETypewriterRichTextTokenType::SelfClosingTag:
		case ETypewriterRichTextTokenType::VisibleText:
		case ETypewriterRichTextTokenType::Whitespace:
		case ETypewriterRichTextTokenType::NewLine:
			// 自闭合标签、普通文字、空白和换行都可以直接输出。
			// 是否消耗打字进度已经在 Parse/AddToken 阶段决定。
			Result += Token.RawText;
			break;
		default:
			break;
		}
	}

	// 如果打字进度截断在样式标签内部，需要自动补齐关闭标签，保证 RichText 始终合法。
	for (int32 Index = OpenTagStack.Num() - 1; Index >= 0; --Index)
	{
		Result += TEXT("</>");
	}

	return Result;
}

int32 FTypewriterTimingResolver::ResolveVisibleTokenCount(const FTypewriterParagraphConfig& ParagraphConfig, const TArray<FTypewriterTimelineKey>& ResolvedTimelineKeys, float ElapsedTime, int32 TotalVisibleTokenCount)
{
	if (TotalVisibleTokenCount <= 0)
	{
		return 0;
	}

	TArray<FTypewriterTimelineKey> SortedKeys = ResolvedTimelineKeys;
	SortedKeys.Sort([](const FTypewriterTimelineKey& Left, const FTypewriterTimelineKey& Right)
	{
		return Left.Time < Right.Time;
	});

	// 如果配置了时间线，就进入时间线模式
	if (SortedKeys.Num() > 0)
	{
		if (ElapsedTime <= SortedKeys[0].Time)
		{
			return FMath::Clamp(SortedKeys[0].VisibleTokenCount, 0, TotalVisibleTokenCount);
		}

		for (int32 Index = 0; Index < SortedKeys.Num() - 1; ++Index)
		{
			const FTypewriterTimelineKey& CurrentKey = SortedKeys[Index];
			const FTypewriterTimelineKey& NextKey = SortedKeys[Index + 1];
			if (ElapsedTime <= NextKey.Time)
			{
				const float Duration = FMath::Max(NextKey.Time - CurrentKey.Time, UE_SMALL_NUMBER);
				const float Alpha = FMath::Clamp((ElapsedTime - CurrentKey.Time) / Duration, 0.0f, 1.0f);
				const float VisibleTokenCount = FMath::Lerp(static_cast<float>(CurrentKey.VisibleTokenCount), static_cast<float>(NextKey.VisibleTokenCount), Alpha);
				return FMath::Clamp(FMath::FloorToInt(VisibleTokenCount), 0, TotalVisibleTokenCount);
			}
		}

		return FMath::Clamp(SortedKeys.Last().VisibleTokenCount, 0, TotalVisibleTokenCount);
	}

	// 当前段落播放了总时长的多少比例，就显示全文 Token 的多少比例。
	const float Duration = FMath::Max(ParagraphConfig.Duration, UE_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	return FMath::Clamp(FMath::FloorToInt(static_cast<float>(TotalVisibleTokenCount) * Alpha), 0, TotalVisibleTokenCount);
}

bool UTypewriterContentDataAsset::GetContent(FName ContentName, FTypewriterContentConfig& OutContent) const
{
	if (const FTypewriterContentConfig* FoundContent = Contents.Find(ContentName))
	{
		OutContent = *FoundContent;
		return true;
	}

	OutContent = FTypewriterContentConfig();
	return false;
}

void UUITypewriterText::NativeDestruct()
{
	StopTypewriterTicker();
	StopCurrentAudio();
	Super::NativeDestruct();
}

void UUITypewriterText::StartTypewriterTicker()
{
	if (TypewriterTickerHandle.IsValid())
	{
		return;
	}
	UE_LOG(LogTypewriterTextWidget, Log, TEXT("UUITypewriterText::StartTypewriterTicker"));
	TypewriterTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
		{
			const bool bShouldContinue = TickTypewriter(DeltaTime);
			if (!bShouldContinue)
			{
				TypewriterTickerHandle.Reset();
			}
			return bShouldContinue;
		})
	);
}

void UUITypewriterText::StopTypewriterTicker()
{
	UE_LOG(LogTypewriterTextWidget, Log, TEXT("UUITypewriterText::StopTypewriterTicker"));
	if (TypewriterTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TypewriterTickerHandle);
		TypewriterTickerHandle.Reset();
	}
}

bool UUITypewriterText::TickTypewriter(float InDeltaTime)
{
	if (PlayState == ETypewriterPlayState::Playing)	// 正在播放段落时，按时间推进文字显示数量。
	{
		ParagraphElapsedTime += InDeltaTime;
		const FTypewriterParagraphConfig* ParagraphConfig = GetCurrentParagraphConfig();
		if (!ParagraphConfig)
		{
			FinishTypewriter();
			return NeedsTypewriterTicker();
		}

		const int32 NewVisibleTokenCount = FTypewriterTimingResolver::ResolveVisibleTokenCount(*ParagraphConfig, ResolvedTimelineKeys, ParagraphElapsedTime, TotalVisibleTokenCount);
		if (NewVisibleTokenCount != CurrentVisibleTokenCount)
		{
			CurrentVisibleTokenCount = NewVisibleTokenCount;
			UpdateDisplayText(false);
		}

		if (CurrentVisibleTokenCount >= TotalVisibleTokenCount)
		{
			CompleteCurrentParagraph(false);
		}
	}
	else if (PlayState == ETypewriterPlayState::ParagraphFinished && Content.bAutoAdvanceWhenParagraphFinished) // 当前段落完成后，如果开启自动换段，则等待一段时间后进入下一段。
	{
		ParagraphFinishedElapsedTime += InDeltaTime;
		const FTypewriterParagraphConfig* ParagraphConfig = GetCurrentParagraphConfig();
		if (!ParagraphConfig || ParagraphFinishedElapsedTime >= ParagraphConfig->AutoAdvanceDelay)
		{
			AdvanceToNextParagraph();
		}
	}
	return NeedsTypewriterTicker();
}

bool UUITypewriterText::NeedsTypewriterTicker() const
{
	const bool bNeedTicker = PlayState == ETypewriterPlayState::Playing || 
		(PlayState == ETypewriterPlayState::ParagraphFinished && Content.bAutoAdvanceWhenParagraphFinished);
	//UE_LOG(LogTypewriterTextWidget, Log, TEXT("UUITypewriterText::StopTypewriterTicker %hhd"), bNeedTicker);
	return bNeedTicker;
}

bool UUITypewriterText::SetContentAsset(UTypewriterContentDataAsset* InContentAsset, FName ContentName)
{
	if (!InContentAsset)
	{
		SetContent(FTypewriterContentConfig());
		return false;
	}

	FTypewriterContentConfig FoundContent;
	if (!InContentAsset->GetContent(ContentName, FoundContent))
	{
		SetContent(FTypewriterContentConfig());
		return false;
	}

	SetContent(FoundContent);
	return true;
}

void UUITypewriterText::SetContent(const FTypewriterContentConfig& InContent)
{
	Stop();
	Content = InContent;
	ResetTypewriter();
}

void UUITypewriterText::Start()
{
	StopCurrentAudio();
	OnTypewriterStarted.Broadcast();

	if (Content.Paragraphs.Num() == 0)
	{
		FinishTypewriter();
		return;
	}

	PlayParagraph(0);
}

void UUITypewriterText::Stop()
{
	StopTypewriterTicker();
	StopCurrentAudio();
	PlayState = ETypewriterPlayState::Idle;
	CurrentParagraphIndex = INDEX_NONE;
	CurrentVisibleTokenCount = 0;
	ParagraphElapsedTime = 0.0f;
	ParagraphFinishedElapsedTime = 0.0f;
	TotalVisibleTokenCount = 0;
	ResolvedTimelineKeys.Reset();
}

void UUITypewriterText::ResetTypewriter()
{
	Stop();
	SetDisplayText(FText::GetEmpty());
}

void UUITypewriterText::SkipCurrentParagraph()
{
	if (PlayState != ETypewriterPlayState::Playing)
	{
		return;
	}

	CompleteCurrentParagraph(true);
}

void UUITypewriterText::AdvanceToNextParagraph()
{
	if (PlayState != ETypewriterPlayState::ParagraphFinished)
	{
		return;
	}

	const int32 NextParagraphIndex = CurrentParagraphIndex + 1;
	if (!Content.Paragraphs.IsValidIndex(NextParagraphIndex))
	{
		FinishTypewriter();
		return;
	}

	PlayParagraph(NextParagraphIndex);
}

void UUITypewriterText::HandleConfirmInput()
{
	if (PlayState == ETypewriterPlayState::Playing)
	{
		SkipCurrentParagraph();
	}
	else if (PlayState == ETypewriterPlayState::ParagraphFinished)
	{
		AdvanceToNextParagraph();
	}
	else if (PlayState == ETypewriterPlayState::Completed)
	{
		OnTypewriterCompleted.Broadcast();
	}
}

void UUITypewriterText::PlayParagraph(int32 ParagraphIndex)
{
	if (!Content.Paragraphs.IsValidIndex(ParagraphIndex))
	{
		FinishTypewriter();
		return;
	}

	StopCurrentAudio();
	CurrentParagraphIndex = ParagraphIndex;
	CurrentVisibleTokenCount = 0;
	ParagraphElapsedTime = 0.0f;
	ParagraphFinishedElapsedTime = 0.0f;

	FTypewriterParagraphConfig& ParagraphConfig = Content.Paragraphs[ParagraphIndex];
	CurrentParser.Parse(ParagraphConfig.RichText, ParagraphConfig);
	TotalVisibleTokenCount = CurrentParser.GetTotalVisibleTokenCount();
	ResolvedTimelineKeys = CurrentParser.ResolveTimelineKeys(ParagraphConfig.TimelineKeys);
	PlayState = ETypewriterPlayState::Playing;
	StartTypewriterTicker();

	UpdateDisplayText(false);
	PostCurrentParagraphAudio(ParagraphConfig);
	OnParagraphStarted.Broadcast(CurrentParagraphIndex);

	if (TotalVisibleTokenCount <= 0)
	{
		CompleteCurrentParagraph(false);
	}
}

void UUITypewriterText::CompleteCurrentParagraph(bool bSkipped)
{
	if (PlayState != ETypewriterPlayState::Playing)
	{
		return;
	}

	CurrentVisibleTokenCount = TotalVisibleTokenCount;
	PlayState = ETypewriterPlayState::ParagraphFinished;
	ParagraphFinishedElapsedTime = 0.0f;
	UpdateDisplayText(true);

	if (bSkipped)
	{
		if (!Content.bContinueAudioWhenSkip)
		{
			StopCurrentAudio();
		}
		OnParagraphSkipped.Broadcast(CurrentParagraphIndex);
	}

	OnParagraphCompleted.Broadcast(CurrentParagraphIndex);
}

void UUITypewriterText::FinishTypewriter()
{
	StopCurrentAudio();
	PlayState = ETypewriterPlayState::Completed;
	CurrentParagraphIndex = INDEX_NONE;
	CurrentVisibleTokenCount = 0;
	ParagraphElapsedTime = 0.0f;
	ParagraphFinishedElapsedTime = 0.0f;
	TotalVisibleTokenCount = 0;
	ResolvedTimelineKeys.Reset();
	OnTypewriterCompleted.Broadcast();
}

void UUITypewriterText::UpdateDisplayText(bool bShowEndMarker)
{
	const FTypewriterParagraphConfig* ParagraphConfig = GetCurrentParagraphConfig();
	if (!ParagraphConfig)
	{
		SetDisplayText(FText::GetEmpty());
		return;
	}

	const FText DisplayText = bShowEndMarker
		? CurrentParser.BuildVisibleText(CurrentVisibleTokenCount, ParagraphConfig->EndMarkerRichText)
		: CurrentParser.BuildVisibleText(CurrentVisibleTokenCount);
	UE_LOG(LogTypewriterTextWidget, Verbose, TEXT("UpdateDisplayText Paragraph=%d Visible=%d/%d ShowEndMarker=%d Text=[%s]"),
		CurrentParagraphIndex,
		CurrentVisibleTokenCount,
		TotalVisibleTokenCount,
		bShowEndMarker ? 1 : 0,
		*DisplayText.ToString());
	SetDisplayText(DisplayText);
}

void UUITypewriterText::SetDisplayText(const FText& DisplayText)
{
	UE_LOG(LogTypewriterTextWidget, Verbose, TEXT("SetDisplayText RichTextBox=%d RichTextBlock=%d TextBlock=%d Text=[%s]"),
		0,//TypewriterRichTextBox ? 1 : 0,
		TypewriterRichTextBlock ? 1 : 0,
		TypewriterTextBlock ? 1 : 0,
		*DisplayText.ToString());

	// if (TypewriterRichTextBox)
	// {
	// 	TypewriterRichTextBox->SetText(DisplayText);
	// }
	if (TypewriterRichTextBlock)
	{
		TypewriterRichTextBlock->SetText(DisplayText);
	}
	if (TypewriterTextBlock)
	{
		TypewriterTextBlock->SetText(DisplayText);
	}

	BP_OnDisplayTextUpdated(DisplayText);
}

void UUITypewriterText::PostCurrentParagraphAudio(const FTypewriterParagraphConfig& ParagraphConfig)
{
	// if (!ParagraphConfig.AudioEvent)
	// {
	// 	return;
	// }

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AActor* TargetActor = ResolveAudioTargetActor();
	if (!TargetActor)
	{
		return;
	}

	//CurrentAudioPlayingId = UAkGameplayStatics::PostEvent(ParagraphConfig.AudioEvent, TargetActor, 0, FOnAkPostEventCallback(), true);
}

void UUITypewriterText::StopCurrentAudio()
{
	if (CurrentAudioPlayingId == 0)
	{
		return;
	}

	// if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
	// {
	// 	AkAudioDevice->StopPlayingID(CurrentAudioPlayingId);
	// }
	CurrentAudioPlayingId = 0;
}

AActor* UUITypewriterText::ResolveAudioTargetActor() const
{
	if (AudioTargetActor)
	{
		return AudioTargetActor;
	}

	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			return Pawn;
		}
		return OwningPlayer;
	}

	return nullptr;
}

const FTypewriterParagraphConfig* UUITypewriterText::GetCurrentParagraphConfig() const
{
	return Content.Paragraphs.IsValidIndex(CurrentParagraphIndex) ? &Content.Paragraphs[CurrentParagraphIndex] : nullptr;
}

FTypewriterParagraphConfig* UUITypewriterText::GetCurrentParagraphConfig()
{
	return Content.Paragraphs.IsValidIndex(CurrentParagraphIndex) ? &Content.Paragraphs[CurrentParagraphIndex] : nullptr;
}
