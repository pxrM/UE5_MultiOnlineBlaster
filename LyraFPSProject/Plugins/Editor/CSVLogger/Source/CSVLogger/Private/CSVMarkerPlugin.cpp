// Copyright (c) 2025 Moon. All Rights Reserved.


#include "CSVMarkerPlugin.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString UCSVMarkerPlugin::CSVFilePath;
TArray<FString> UCSVMarkerPlugin::Rows;

void UCSVMarkerPlugin::InitCSVLogger(const FString& FileName)
{
    FString ActualFileName = FileName;
    UE_LOG(LogTemp, Error, TEXT("CSVFilePath:%s"), *ActualFileName);
    if (ActualFileName.IsEmpty()) {
        FDateTime Now = FDateTime::Now();
        ActualFileName = FString::Printf(TEXT("log_%04d%02d%02d_%02d%02d%02d.csv"),
            Now.GetYear(), Now.GetMonth(), Now.GetDay(),
            Now.GetHour(), Now.GetMinute(), Now.GetSecond());
    }

    CSVFilePath = FPaths::ProjectDir() / TEXT("Saved/CSVData/") / ActualFileName;
    Rows.Empty();
    Rows.Add(TEXT("Time,PosX,PosY,PosZ,RotPitch,RotYaw,RotRoll")); 
}

void UCSVMarkerPlugin::LogActorTransform(AActor* Actor)
{
    if (!Actor) return;

    FVector Pos = Actor->GetActorLocation();
    FRotator Rot = Actor->GetActorRotation();

    FDateTime Now = FDateTime::Now();
    int32 Millisecond = Now.GetMillisecond();

    FString Timestamp = FString::Printf(TEXT("%04d-%02d-%02d_%02d-%02d-%02d.%03d"),
        Now.GetYear(), Now.GetMonth(), Now.GetDay(),
        Now.GetHour(), Now.GetMinute(), Now.GetSecond(),
        Millisecond);

    FString Line = FString::Printf(TEXT("%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f"),
        *Timestamp, Pos.X, Pos.Y, Pos.Z, Rot.Pitch, Rot.Yaw, Rot.Roll);

    Rows.Add(Line);
}

void UCSVMarkerPlugin::LogTransform(const FTransform& Transform,bool bAutoSave)
{
    FDateTime Now = FDateTime::Now();
    int32 Millisecond = Now.GetMillisecond();

    FString Timestamp = FString::Printf(TEXT("%04d-%02d-%02d_%02d-%02d-%02d.%03d"),
        Now.GetYear(), Now.GetMonth(), Now.GetDay(),
        Now.GetHour(), Now.GetMinute(), Now.GetSecond(),
        Millisecond);

    FString Line = FString::Printf(TEXT("%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f"),
        *Timestamp, Transform.GetLocation().X,  Transform.GetLocation().Y,  Transform.GetLocation().Z,  Transform.Rotator().Pitch, Transform.Rotator().Yaw, Transform.Rotator().Roll);

    Rows.Add(Line);
    if (bAutoSave)
    {
        SaveCSV();
    }
}

void UCSVMarkerPlugin::LogLoc(const FVector& Loc, bool bAutoSave)
{
    FTransform transform;
    transform.SetLocation(Loc);
    LogTransform(transform,bAutoSave);
}

void UCSVMarkerPlugin::LogRot(const FRotator& Rot, bool bAutoSave)
{
    FTransform transform;
    transform.SetRotation(Rot.Quaternion());
    LogTransform(transform,bAutoSave);
}

void UCSVMarkerPlugin::SaveCSV()
{
    if (Rows.Num() == 0) return;

    FFileHelper::SaveStringArrayToFile(Rows, *CSVFilePath);
}
