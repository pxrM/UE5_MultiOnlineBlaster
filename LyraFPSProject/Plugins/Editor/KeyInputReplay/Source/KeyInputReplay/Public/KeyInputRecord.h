#pragma once

struct FKeyEvent;
struct FAnalogInputEvent;
struct FPointerEvent;
struct FMotionEvent;

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KeyInputRecord.generated.h"

UINTERFACE(BlueprintType)
class UKeyInputRecord : public UInterface
{
	GENERATED_BODY()
};

class IKeyInputRecord
{
    GENERATED_BODY()
public:

    
    virtual void OnKeyDown(const FKeyEvent& KeyEvent){}
    virtual void OnKeyUp(const FKeyEvent& KeyEvent){}
    virtual void OnAnalogInput(const FAnalogInputEvent& AnalogInputEvent){}
    virtual void OnMouseMove(const FPointerEvent& PointerEvent){}
    virtual void OnMouseButtonDown(const FPointerEvent& PointerEvent){}
    virtual void OnMouseButtonUp(const FPointerEvent& PointerEvent){}
    virtual void OnMouseButtonDoubleClick(const FPointerEvent& PointerEvent){}
    virtual void OnMouseWheelOrGesture(const FPointerEvent& WheelEvent){}
    virtual void OnMotionDetected(const FMotionEvent& MotionEvent){}
};
