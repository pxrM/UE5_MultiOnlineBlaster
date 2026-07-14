// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataAsset.h"
#include "UITypewriterText.generated.h"

// class UAkAudioEvent;
class URichTextBlock;
class URichTextBox;
class UTextBlock;


// 打字机控件当前播放状态。
UENUM(BlueprintType)
enum class ETypewriterPlayState : uint8
{
	// 未播放或已复位。
	Idle,

	// 正在逐个显示当前段落的可见 Token。
	Playing,

	// 当前段落已经完整显示，并已显示段落结束提示符。
	ParagraphFinished,

	// 所有段落均已播放完成。
	Completed
};


// RichText 解析后的内部 Token 类型；打字计数只统计可见 Token，不统计样式标签。
UENUM(BlueprintType)
enum class ETypewriterRichTextTokenType : uint8
{
	// 普通可见文字或标点。
	VisibleText,

	// 开启样式标签，例如 <Red>。
	OpenTag,

	// 关闭样式标签，例如 </>。
	CloseTag,

	// 自闭合标签，例如 <Icon id="Warning"/>。
	SelfClosingTag,

	// 空格或 Tab。
	Whitespace,

	// 换行。
	NewLine
};


// 语音时间线关键帧：在指定时间点应该显示到多少个可见 Token。
USTRUCT(BlueprintType)
struct FTypewriterTimelineKey
{
	GENERATED_BODY()

	// 段落开始后的时间，单位为秒。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "0.0"))
	float Time = 0.0f;

	// 当前时间点已经显示的可见 Token 数量；不是原始 FString 下标。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "0"))
	int32 VisibleTokenCount = 0;

	// 更适合配置的文本锚点；不为空时会显示到第 AnchorOccurrence 次出现的该纯文本结尾。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	FString VisibleTextAnchor;

	// 当 VisibleTextAnchor 在段落纯文本中重复出现时，指定匹配第几次出现。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "1"))
	int32 AnchorOccurrence = 1;
};


// 单个段落的打字机配置。
USTRUCT(BlueprintType)
struct FTypewriterParagraphConfig
{
	GENERATED_BODY()

	// 段落原始 RichText 内容；普通文本也可以直接填写。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (MultiLine = "true"))
	FText RichText;

	// 段落开始播放时触发的 Wwise 音效事件。
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	// TObjectPtr<UAkAudioEvent> AudioEvent = nullptr;

	// 没有配置 TimelineKeys 时，当前段落完整显示所需时间，单位为秒。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "0.01"))
	float Duration = 0.75f;

	// 用于精确匹配语音节奏的时间线关键帧；为空时使用 Duration 匀速显示完整段落。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	TArray<FTypewriterTimelineKey> TimelineKeys;

	// 段落完整显示后追加的结尾提示符，支持 RichText，例如 <Hint>/</> 或 <NextIcon/>。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	FText EndMarkerRichText = FText::FromString(TEXT(""));

	// Inline Widget 或图片标签是否占用一个可见 Token。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bInlineWidgetConsumesVisibleToken = true;

	// 空格和 Tab 是否占用一个可见 Token。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bWhitespaceConsumesVisibleToken = true;

	// 换行是否占用一个可见 Token。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bNewLineConsumesVisibleToken = true;

	// 开启自动换段时，当前段落完成后等待多久进入下一段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter", meta = (ClampMin = "0.0"))
	float AutoAdvanceDelay = 0.0f;
};


// 一组完整打字机内容配置，由多个段落组成。
USTRUCT(BlueprintType)
struct FTypewriterContentConfig
{
	GENERATED_BODY()

	// 按播放顺序排列的段落列表。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	TArray<FTypewriterParagraphConfig> Paragraphs;

	// 点击跳过当前段落时，是否让当前段落音效继续播放。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bContinueAudioWhenSkip = true;

	// 段落完成后是否自动进入下一段；关闭时需要外部调用 AdvanceToNextParagraph 或 HandleConfirmInput。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bAutoAdvanceWhenParagraphFinished = false;
};


// RichText 解析后的内部 Token 数据。
USTRUCT()
struct FTypewriterRichTextToken
{
	GENERATED_BODY()

	// Token 类型。
	ETypewriterRichTextTokenType Type = ETypewriterRichTextTokenType::VisibleText;

	// 原始文本片段，包含完整标签或单个字符。
	FString RawText;

	// 标签名，用于调试和后续扩展；普通文字为空。
	FString TagName;

	// 可见 Token 序号；不消耗显示进度的 Token 为 INDEX_NONE。
	int32 VisibleTokenIndex = INDEX_NONE;

	// 是否消耗一个可见 Token 计数。
	bool bConsumesVisibleToken = false;
};


// RichText 解析与重建工具：负责把原始 RichText 转成可见 Token，并在任意进度下输出合法 RichText。
class UIFRAMEWORKWIDGETS_API FTypewriterRichTextParser
{
public:
	// 解析段落文本，并根据段落配置决定空格、换行、Inline Widget 是否参与打字计数。
	void Parse(const FText& SourceText, const FTypewriterParagraphConfig& Config);

	// 返回当前段落总共需要显示的可见 Token 数。
	int32 GetTotalVisibleTokenCount() const;

	// 将 TimelineKeys 中的文本锚点解析为可见 Token 数，无法解析时保留原始 VisibleTokenCount。
	TArray<FTypewriterTimelineKey> ResolveTimelineKeys(const TArray<FTypewriterTimelineKey>& TimelineKeys) const;

	// 构建指定可见 Token 数量对应的 RichText，并追加段落结尾提示符。
	FText BuildVisibleText(int32 VisibleTokenCount, const FText& EndMarkerRichText) const;

	// 构建指定可见 Token 数量对应的 RichText，不追加结尾提示符。
	FText BuildVisibleText(int32 VisibleTokenCount) const;

private:
	static bool IsWhitespace(TCHAR Character);
	static bool IsTagToken(const FString& Source, int32 StartIndex, int32& OutEndIndex, FString& OutRawTag);
	// 提取 Rich 标签
	static FString ExtractTagName(const FString& RawTag, bool& bOutIsClosingTag, bool& bOutIsSelfClosingTag);
	void AddToken(ETypewriterRichTextTokenType Type, const FString& RawText, const FString& TagName, bool bConsumesVisibleToken);
	int32 ResolveVisibleTokenCountByAnchor(const FTypewriterTimelineKey& TimelineKey) const;
	// 根据当前应该显示的可见 Token 数量，重新生成一段“合法的 RichText 字符串”。
	FString BuildVisibleString(int32 VisibleTokenCount) const;

private:
	// 解析后的 Token 序列。
	TArray<FTypewriterRichTextToken> Tokens;

	// 参与打字进度的 Token 总数。
	int32 TotalVisibleTokenCount = 0;

	// 去掉 RichText 标签后的纯文本，用于配置型文本锚点查找。
	FString PlainVisibleText;
};


// 打字速度解析工具：优先使用 TimelineKeys，否则使用默认匀速。
class UIFRAMEWORKWIDGETS_API FTypewriterTimingResolver
{
public:
	/*
	 * 根据段落播放时间计算当前应该显示到多少个可见 Token。
	 * @param ElapsedTime	当前段落已经播放了多少秒
	 * @param TotalVisibleTokenCount	当前段落总共有多少个可见 Token
	 */
	static int32 ResolveVisibleTokenCount(const FTypewriterParagraphConfig& ParagraphConfig, const TArray<FTypewriterTimelineKey>& ResolvedTimelineKeys, float ElapsedTime, int32 TotalVisibleTokenCount);
};


// 打字机内容配置资产
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UTypewriterContentDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 多组打字机内容配置；Key 用于区分不同对白、提示或演出文本。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Typewriter")
	TMap<FName, FTypewriterContentConfig> Contents;

	// 尝试按配置名读取一组打字机内容。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	bool GetContent(FName ContentName, FTypewriterContentConfig& OutContent) const;
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTypewriterSimpleEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTypewriterParagraphEvent, int32, ParagraphIndex);


// UMG 打字机控件：负责段落状态机、输入跳过、文本显示和段落音效播放。
UCLASS(Abstract, BlueprintType, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUITypewriterText : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeDestruct() override;
	
private:
	FTSTicker::FDelegateHandle TypewriterTickerHandle;
	void StartTypewriterTicker();
	void StopTypewriterTicker();
	bool TickTypewriter(float InDeltaTime);
	bool NeedsTypewriterTicker() const;

public:
	// 使用 DataAsset 中指定名称的内容，并重置当前播放状态；找不到时会清空当前内容。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	bool SetContentAsset(UTypewriterContentDataAsset* InContentAsset, FName ContentName);

	// 直接设置内容配置，并重置当前播放状态。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void SetContent(const FTypewriterContentConfig& InContent);

	// 从第一段开始播放。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void Start();

	// 停止播放并停止当前音效，但不清空显示文本。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void Stop();

	// 停止播放并清空显示文本。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void ResetTypewriter();

	// 立即显示当前段落全部内容，并显示段落结尾提示符。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void SkipCurrentParagraph();

	// 从已完成的当前段落进入下一段；没有下一段时结束整个打字机。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void AdvanceToNextParagraph();

	// 推荐绑定到确认/点击输入：播放中则跳过当前段，段落完成则进入下一段。
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void HandleConfirmInput();

	// 获取当前播放状态。
	UFUNCTION(BlueprintPure, Category = "Typewriter")
	FORCEINLINE ETypewriterPlayState GetPlayState() const { return PlayState; }

	// 获取当前段落索引；未播放或已全部完成时为 INDEX_NONE。
	UFUNCTION(BlueprintPure, Category = "Typewriter")
	FORCEINLINE int32 GetCurrentParagraphIndex() const { return CurrentParagraphIndex; }

public:
	// 整个打字机开始播放时触发。
	UPROPERTY(BlueprintAssignable, Category = "Typewriter")
	FTypewriterSimpleEvent OnTypewriterStarted;

	// 某个段落开始播放时触发。
	UPROPERTY(BlueprintAssignable, Category = "Typewriter")
	FTypewriterParagraphEvent OnParagraphStarted;

	// 某个段落完整显示后触发，跳过导致的完成也会触发。
	UPROPERTY(BlueprintAssignable, Category = "Typewriter")
	FTypewriterParagraphEvent OnParagraphCompleted;

	// 玩家跳过当前段落时触发。
	UPROPERTY(BlueprintAssignable, Category = "Typewriter")
	FTypewriterParagraphEvent OnParagraphSkipped;

	// 所有段落播放完成时触发。
	UPROPERTY(BlueprintAssignable, Category = "Typewriter")
	FTypewriterSimpleEvent OnTypewriterCompleted;

protected:
	// 当前使用的内容配置；可以直接在 Widget 蓝图默认值中配置。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Typewriter")
	FTypewriterContentConfig Content;

	// 当前播放状态。
	UPROPERTY(BlueprintReadOnly, Category = "Typewriter")
	ETypewriterPlayState PlayState = ETypewriterPlayState::Idle;

	// 当前段落索引。
	UPROPERTY(BlueprintReadOnly, Category = "Typewriter")
	int32 CurrentParagraphIndex = INDEX_NONE;

	// 当前已经显示的可见 Token 数量。
	UPROPERTY(BlueprintReadOnly, Category = "Typewriter")
	int32 CurrentVisibleTokenCount = 0;

	// 可选绑定的普通 TextBlock；
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TypewriterTextBlock = nullptr;

	// 可选绑定的 UE RichTextBlock；
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URichTextBlock> TypewriterRichTextBlock = nullptr;

	// 可选绑定的项目自定义 RichTextBox；
	// UPROPERTY(meta = (BindWidgetOptional))
	// TObjectPtr<URichTextBox> TypewriterRichTextBox = nullptr;

	// 音效播放目标 Actor；为空时优先使用 OwningPlayer 的 Pawn，其次使用 OwningPlayer。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	TObjectPtr<AActor> AudioTargetActor = nullptr;

	// 文本更新时的蓝图扩展点；适合接入自定义文本控件。
	UFUNCTION(BlueprintImplementableEvent, Category = "Typewriter")
	void BP_OnDisplayTextUpdated(const FText& DisplayText);

private:
	// 开始播放指定段落，初始化段落计时、RichText 解析结果和段落音效。
	void PlayParagraph(int32 ParagraphIndex);

	// 将当前段落补全到结尾状态；bSkipped 用于区分自然完成和玩家跳过。
	void CompleteCurrentParagraph(bool bSkipped);

	// 结束整个打字机流程，停止音效并广播完成事件。
	void FinishTypewriter();

	// 根据当前可见 Token 数重建显示文本；可选择是否追加段落结尾提示符。
	void UpdateDisplayText(bool bShowEndMarker);

	// 写入绑定的文本控件，并触发蓝图文本更新扩展点。
	void SetDisplayText(const FText& DisplayText);

	// 播放当前段落配置的 Wwise 音效。
	void PostCurrentParagraphAudio(const FTypewriterParagraphConfig& ParagraphConfig);

	// 停止当前段落仍在播放的 Wwise 音效。
	void StopCurrentAudio();

	// 获取音效挂载目标；优先使用显式配置，其次使用 OwningPlayer/Pawn。
	AActor* ResolveAudioTargetActor() const;

	// 获取当前段落配置，只读版本；索引非法时返回 nullptr。
	const FTypewriterParagraphConfig* GetCurrentParagraphConfig() const;

	// 获取当前段落配置，可写版本；索引非法时返回 nullptr。
	FTypewriterParagraphConfig* GetCurrentParagraphConfig();

private:
	// 当前段落的 RichText 解析结果。
	FTypewriterRichTextParser CurrentParser;

	// 当前段落已经播放的时间。
	float ParagraphElapsedTime = 0.0f;

	// 当前段落完成后停留的时间，用于自动换段。
	float ParagraphFinishedElapsedTime = 0.0f;

	// 当前段落可见 Token 总数。
	int32 TotalVisibleTokenCount = 0;

	// 当前段落已将文本锚点转换为 VisibleTokenCount 的时间线关键帧。
	TArray<FTypewriterTimelineKey> ResolvedTimelineKeys;

	// 当前段落 Wwise 播放 ID，用于跳过或销毁时停止音效。
	int32 CurrentAudioPlayingId = 0;
};
