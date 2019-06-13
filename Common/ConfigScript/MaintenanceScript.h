#ifndef MaintenanceScript_H
#define MaintenanceScript_H

#include "Type.h"

void InitConfigState(const char* serverType, int argc, const char * const * argv);
void ReloadConfigState(void);
const char* GetConfig(const char* name);

#endif //MaintenanceScript_H