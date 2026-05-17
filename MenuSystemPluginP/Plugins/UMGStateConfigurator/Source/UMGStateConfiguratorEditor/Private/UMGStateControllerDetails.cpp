#include "UMGStateControllerDetails.h"

#include "UMGStateController.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorFontGlyphs.h"

#define LOCTEXT_NAMESPACE "FUMGStateControllerDetails"

TSharedRef<IDetailCustomization> FUMGStateControllerDetails::MakeInstance()
{
	UE_LOG(LogTemp, Log, TEXT("### FUMGStateControllerDetails::MakeInstance"));
	return MakeShareable(new FUMGStateControllerDetails);
}

void FUMGStateControllerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UE_LOG(LogTemp, Log, TEXT("### FUMGStateControllerDetails::CustomizeDetails called!"));
	TSharedRef<IPropertyHandle> CategoriesHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUMGStateController, StateCategories));
	DetailBuilder.HideProperty(CategoriesHandle);
	
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("State Manager", FText::GetEmpty(), ECategoryPriority::Important);

	uint32 NumCategories = 0;
	CategoriesHandle->GetNumChildren(NumCategories);

	for (uint32 Index = 0; Index < NumCategories; Index++)
	{
		TSharedPtr<IPropertyHandle> ElementHandle = CategoriesHandle->GetChildHandle(Index);
		DrawCategoryUI(ElementHandle, DetailBuilder);
	}
	
	CustomCategory.AddCustomRow(LOCTEXT("AddCategory", "+ Add New State Category"))
	[
		SNew(SButton)
		.Text(LOCTEXT("AddCategory", "+ Add New State Category"))
		.OnClicked_Lambda([CategoriesHandle]()
		{
			// 操作 PropertyHandle 来添加元素
			// 这里需要通过 IPropertyHandleArray 接口操作，稍显复杂，略写
			TSharedPtr<IPropertyHandleArray> ArrayHandle = CategoriesHandle->AsArray();
			if (ArrayHandle.IsValid())
			{
				ArrayHandle->AddItem();
			}
			return FReply::Handled();
		})
	];
}

void FUMGStateControllerDetails::DrawCategoryUI(TSharedPtr<IPropertyHandle> CategoryHandle, IDetailLayoutBuilder& DetailBuilder)
{
	// 获取分类名和状态数组的 Handle
	TSharedPtr<IPropertyHandle> EnumNameHandle = CategoryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateCategory, EnumName));
	TSharedPtr<IPropertyHandle> StatesHandle = CategoryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateCategory, States));
	TSharedPtr<IPropertyHandle> ActiveStateHandle = CategoryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateCategory, ActiveStateName));

	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("State Manager");

	// --- 绘制分类标题行 ---
	CustomCategory.AddCustomRow(LOCTEXT("CategoryHeader", "Category Header"))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 10, 0, 5)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				EnumNameHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0)
			[
				SNew(SButton)
				.ContentPadding(2)
				.ToolTipText(LOCTEXT("DelCat", "Delete this Category"))
				[
					SNew(STextBlock)
					.Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Trash) // 垃圾桶图标
				]
			]
		]
	];

	// --- 绘制状态列表 ---
	uint32 NumStates = 0;
	StatesHandle->GetNumChildren(NumStates);
	for (uint32 StateIdx = 0; StateIdx < NumStates; StateIdx++)
	{
		TSharedPtr<IPropertyHandle> StateItemHandle = StatesHandle->GetChildHandle(StateIdx);
		TSharedPtr<IPropertyHandle> NameHandle = StateItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateGroup, StateName));
		TSharedPtr<IPropertyHandle> RecordModeHandle = StateItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateGroup, bRecordMode));

		// 获取当前的 StateName 字符串（用于显示预览状态）
		FString CurrentStateName;
		NameHandle->GetValue(CurrentStateName);

		// 检查当前是否正在录制
		bool bIsRecording = false;
		RecordModeHandle->GetValue(bIsRecording);
		
		CustomCategory.AddCustomRow(LOCTEXT("StateRow", "State Row"))[
			SNew(SHorizontalBox)
			// 1. 状态名称 (可编辑)
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				NameHandle->CreatePropertyValueWidget()
			]
			// 2. 预览按钮 (眼睛图标)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("PreviewTip", "Switch Preview to this State"))
				.OnClicked_Lambda([ActiveStateHandle, CurrentStateName]()
				{
					// 点击设置 ActiveStateName，触发 PostEditChangeChainProperty，从而触发预览
				   ActiveStateHandle->SetValue(CurrentStateName);
				   return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Eye)
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			// 3. 录制按钮 (圆形图标，录制时变红)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("RecordTip", "Toggle Record Mode"))
				.OnClicked_Lambda([RecordModeHandle]()
				{
					// 切换录制状态，这会触发 PostEditChangeChainProperty 写好的互斥逻辑
				   bool bCurrent = false;
				   RecordModeHandle->GetValue(bCurrent);
				   RecordModeHandle->SetValue(!bCurrent);
				   return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Circle)
					// 如果正在录制，显示红色，否则显示灰色
					.ColorAndOpacity(bIsRecording ? FLinearColor::Red : FLinearColor::Gray)
				]
			]
		];
	}

	// --- 添加 "Add New State" 按钮 ---
	CustomCategory.AddCustomRow(LOCTEXT("AddState", "Add State"))
	[
		SNew(SButton)
		.Text(LOCTEXT("AddStateBtn", "+ Add State"))
		.OnClicked_Lambda([StatesHandle]()
		{
			// 通过 IPropertyHandleArray 增加一个元素
			TSharedPtr<IPropertyHandleArray> ArrayHandle = StatesHandle->AsArray();
			if (ArrayHandle.IsValid())
			{
				ArrayHandle->AddItem();
			}
			return FReply::Handled();
		})
	];
	
	// 添加分割线
	CustomCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SSeparator)
		.Orientation(Orient_Horizontal)
	];
}
#undef LOCTEXT_NAMESPACE