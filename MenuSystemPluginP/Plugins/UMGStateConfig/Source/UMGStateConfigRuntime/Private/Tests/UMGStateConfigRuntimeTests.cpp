#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "UMGStateConfigPropertyRuntimeLibrary.h"
#include "UMGStateConfigUserWidgetExtension.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUMGStateConfigPropertyValueEqualityTest, "TikiStar.UMGStateConfig.PropertyValueEquality", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUMGStateConfigPropertyValueEqualityTest::RunTest(const FString& Parameters)
{
	FUMGStateConfigPropertyValue A;
	A.SerializedPropertyPath = TEXT("RenderOpacity");
	A.SerializedPropertyValue = TEXT("1.0");

	FUMGStateConfigPropertyValue B = A;
	TestTrue(TEXT("Same serialized property values are equal"), FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType::SerializedProperty, A, B));

	B.SerializedPropertyValue = TEXT("0.5");
	TestFalse(TEXT("Different serialized property values are not equal"), FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType::SerializedProperty, A, B));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUMGStateConfigSerializedPropertyWhitelistTest, "TikiStar.UMGStateConfig.SerializedPropertyWhitelist", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUMGStateConfigSerializedPropertyWhitelistTest::RunTest(const FString& Parameters)
{
	UImage* Image = NewObject<UImage>();
	UTextBlock* TextBlock = NewObject<UTextBlock>();

	TestTrue(TEXT("Image brush resource is allowed"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(Image, TEXT("Brush.ResourceObject")));

	TestTrue(TEXT("Image color is allowed"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(Image, TEXT("ColorAndOpacity")));
	TestTrue(TEXT("Text content is allowed"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(TextBlock, TEXT("Text")));
	TestTrue(TEXT("Text font size is allowed"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(TextBlock, TEXT("Font.Size")));
	TestTrue(TEXT("Generic visibility is allowed"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(Image, TEXT("Visibility")));
	TestFalse(TEXT("Unsupported random property is rejected"), FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(Image, TEXT("DesignerFlags")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUMGStateConfigChangeKeyHashTest, "TikiStar.UMGStateConfig.ChangeKeyHash", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUMGStateConfigChangeKeyHashTest::RunTest(const FString& Parameters)
{
	FUMGStateConfigChangeKey A{ TEXT("Image_Icon"), EUMGStateConfigPropertyType::SerializedProperty, TEXT("Brush.ResourceObject") };
	FUMGStateConfigChangeKey B{ TEXT("Image_Icon"), EUMGStateConfigPropertyType::SerializedProperty, TEXT("Brush.ResourceObject") };
	FUMGStateConfigChangeKey C{ TEXT("Image_Icon"), EUMGStateConfigPropertyType::SerializedProperty, TEXT("Brush.TintColor") };

	TestTrue(TEXT("Same change keys are equal"), A == B);
	TestEqual(TEXT("Same change keys have the same hash"), GetTypeHash(A), GetTypeHash(B));
	TestFalse(TEXT("Different property paths are not equal"), A == C);
	return true;
}

#endif
