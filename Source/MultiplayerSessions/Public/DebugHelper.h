// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class MULTIPLAYERSESSIONS_API DebugHelper
{
public:
	UFUNCTION()
	static void PrintToLog(FString Message = TEXT(""), FColor Color = FColor::Red, float Duration = 5.f);
};
