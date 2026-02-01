#include "UMGStateControllerDetails.h"

#include "UMGStateController.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorFontGlyphs.h"

#define LOCTEXT_NAMESPACE "UMGStateControllerDetails"

TSharedRef<IDetailCustomization> FUMGStateControllerDetails::MakeInstance()
{
	return MakeShareable(new FUMGStateControllerDetails);
}

void FUMGStateControllerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
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
	
	CustomCategory.AddCustomRow(LOCTEXT("AddCategory", "Add Category"))
	[
		SNew(SButton)
		.Text(LOCTEXT("AddCategory", "+ Add New State Category"))
		.OnClicked_Lambda([CategoriesHandle]()
		{
			// 操作 PropertyHandle 来添加元素
			// 这里需要通过 IPropertyHandleArray 接口操作，稍显复杂，略写
			return FReply::Handled();
		})
	];
}

void FUMGStateControllerDetails::DrawCategoryUI(TSharedPtr<IPropertyHandle> CategoryHandle, IDetailLayoutBuilder& DetailBuilder)
{
	// 获取分类名和状态数组的 Handle
	TSharedPtr<IPropertyHandle> EnumNameHandle = CategoryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateCategory, EnumName));
	TSharedPtr<IPropertyHandle> StateHandle = CategoryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateCategory, States));
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
	StateHandle->GetNumChildren(NumStates);
	for (uint32 StateIdx = 0; StateIdx < NumStates; StateIdx++)
	{
		TSharedPtr<IPropertyHandle> StateItemHandel = StateHandle->GetChildHandle(StateIdx);
		TSharedPtr<IPropertyHandle> NameHandel = StateItemHandel->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateGroup, StateName));
		TSharedPtr<IPropertyHandle> RecordModeHandle = StateItemHandel->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIStateGroup, bRecordMode));

		// 获取当前的 StateName 字符串（用于显示预览状态）
		FString CurrentStateName;
		NameHandel->GetValue(CurrentStateName);

		// 检查当前是否正在录制
		bool bIsRecording = false;
		RecordModeHandle->GetValue(bIsRecording);
	}
}
#undef LOCTEXT_NAMESPACE