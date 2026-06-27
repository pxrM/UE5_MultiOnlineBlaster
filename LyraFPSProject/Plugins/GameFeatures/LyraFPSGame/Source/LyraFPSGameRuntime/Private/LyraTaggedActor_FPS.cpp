// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraTaggedActor_FPS.h"


// Sets default values
ALyraTaggedActor_FPS::ALyraTaggedActor_FPS()
{

}

#if WITH_EDITOR
bool ALyraTaggedActor_FPS::CanEditChange(const FProperty* InProperty) const
{
	return Super::Super::CanEditChange(InProperty);
	
}
#endif


