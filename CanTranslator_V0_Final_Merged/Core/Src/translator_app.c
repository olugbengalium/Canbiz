#include "translator_app.h"
#include "can_bus.h"
#include "can_trace.h"

extern void MainLoop_Poll(void);

bool Translator_AppInit(void)
{
    TraceLog_Init();
    return CanBus_Init();
}

void Translator_AppPoll(void)
{
    MainLoop_Poll();
}
