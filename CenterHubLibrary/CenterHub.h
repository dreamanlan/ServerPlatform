#ifndef CenterClient_H
#define CenterClient_H

#include <stdint.h>

typedef void(*HandleNameHandleChangedPtr)(int worldId, int addOrUpdate, const char* name, int handle);
typedef void(*HandleMessagePtr)(int worldId, unsigned int seq, int src, int dest, const void* msg, int len);
typedef void(*HandleMessageResultPtr)(int worldId, unsigned int seq, int src, int dest, int result);
typedef void(*HandleCommandPtr)(int worldId, int src, int dest, const char* msg);
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
  DLLIMPORT int TargetHandle(int worldId, const char* name);
  DLLIMPORT int TargetName(int worldId, int handle, char* buf, int len);
  DLLIMPORT int SendByHandle(int worldId, int dest, const void* data, int len);
  DLLIMPORT int SendByName(int worldId, const char* name, const void* data, int len);
  DLLIMPORT int SendCommandByHandle(int worldId, int dest, const char* command);
  DLLIMPORT int SendCommandByName(int worldId, const char* name, const char* command);
  DLLIMPORT int IsRun(void);
  DLLIMPORT void Quit(void);
  DLLIMPORT void SetCenterLogHandler(LogHandlerPtr logHandler);
  DLLIMPORT void Init(const char* serverType, int argc, char* argv[], HandleNameHandleChangedPtr nameHandleCallback, HandleMessagePtr msgCallback, HandleMessageResultPtr msgResultCallback, HandleCommandPtr cmdCallback);
  DLLIMPORT void Tick(void);
  DLLIMPORT int IsConnected(int worldId);
  DLLIMPORT void Disconnect(int worldId);
  DLLIMPORT void Release(void);
}

#endif //CenterClient_H