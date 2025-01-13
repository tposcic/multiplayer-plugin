// Copyright: Toni Poscic

#include "DebugHelper.h"

void DebugHelper::PrintToLog(FString Message, FColor Color, float Duration)
{
    if(GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message, true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
    }
}
