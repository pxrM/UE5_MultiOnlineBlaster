// Copyright (c) 2025 Moon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "CSVMarkerPlugin.generated.h"

/**
 * 
 */
UCLASS()
class CSVLOGGER_API UCSVMarkerPlugin : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    /**
     * Initializes the CSV logger.
     *
     * Optionally accepts a custom file name (for example, "Log.csv").
     * If no name is provided, a new file will be created automatically
     * using the current timestamp as its name (for example, "2025-11-13_16-32-22.csv").
     *
     * @param FileName  Optional custom name for the CSV file.
     */
    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void InitCSVLogger(const FString& FileName);

    /**
     * Logs the current transform of the specified Actor to the active CSV file.
     *
     * Each time this function is called, a new entry containing the Actor's
     * location and rotation will be appended to the log.
     *
     * @param Actor  The Actor whose transform should be recorded.
     */
    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void LogActorTransform(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void LogTransform(const FTransform& Transform,bool bAutoSave=false);
    
    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void LogLoc(const FVector& Loc,bool bAutoSave=false);
    
    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void LogRot(const FRotator& Rot,bool bAutoSave=false);
    /**
     * Saves the current log data to the CSV file.
     *
     * This function can be called manually at any time. It may be used to
     * save all pending data once at the end of a session, or called frequently
     * to write each new record immediately after it is added.
     */
    UFUNCTION(BlueprintCallable, Category = "CSV Logger Plugin")
    static void SaveCSV();

public:
    static FString CSVFilePath;
    static TArray<FString> Rows;
};
