#ifndef CenterClient_H
#define CenterClient_H

#include <stdint.h>

typedef void(*HandleNameHandleChangedPtr)(int addOrUpdate, const char* name, int handle);
typedef void(*HandleMessagePtr)(unsigned int seq, int src, int dest, const void* msg, int len);
typedef void(*HandleMessageResultPtr)(unsigned int seq, int src, int dest, int result);
typedef void(*HandleCommandPtr)(int src, int dest, const char* msg); 
typedef void(*LogHandlerPtr)(const char* log, int len);

#ifdef WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT
#endif

extern "C"
{
  DLLIMPORT void ReloadConfigScript(void);
  DLLIMPORT int GetConfig(const char* key, char* buf, int len);
  DLLIMPORT int TargetHandle(const char* name);
  DLLIMPORT int TargetName(int handle, char* buf, int len);
  DLLIMPORT int SendByHandle(int dest, const void* data, int len);
  DLLIMPORT int SendByName(const char* name, const void* data, int len);
  DLLIMPORT int SendCommandByHandle(int dest, const char* command);
  DLLIMPORT int SendCommandByName(const char* name, const char* command);
  DLLIMPORT int IsRun(void);
  DLLIMPORT void Quit(void);
  DLLIMPORT void SetCenterLogHandler(LogHandlerPtr logHandler);
  DLLIMPORT void Init(const char* serverType, int argc, const char * const * argv, HandleNameHandleChangedPtr nameHandleCallback, HandleMessagePtr msgCallback, HandleMessageResultPtr msgResultCallback, HandleCommandPtr cmdCallback);
  DLLIMPORT void Tick(void);
  DLLIMPORT int IsConnected(void);
  DLLIMPORT void Disconnect(void);
  DLLIMPORT void Release(void);
}

#endif //CenterClient_H