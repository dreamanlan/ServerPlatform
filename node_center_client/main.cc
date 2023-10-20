#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kfifo.h"
#include "CenterClient.h"

#ifdef _WIN32
//#include "windows.h"
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include <uv.h>
#include <napi.h>

#pragma pack(push,1)
struct CoreMessageHead {
  int size;
  unsigned int seq;
  uint64_t src;
};
#pragma pack(pop)

bool g_IsRun = true;
static uv_thread_t g_ThreadHandle = 0;
static const int c_MaxPacketSize = 1024 * 1024;
static char g_PacketBuffer[c_MaxPacketSize];
static const int c_MessageQueueSize = 512 * 1024 * 1024;
static const int c_MessageCountPerTick = 1024;
static kfifo<char> g_MessageQueue(c_MessageQueueSize);

int StartCenterClient(int argc, const char * const * argv);
void WaitForCenterClientExit(void);

static inline void MySleep(unsigned int ms)
{
#ifdef WIN32
  SleepEx(ms, 0);
#else
  usleep(ms * 1000);
#endif
}

void PushCoreMessage(unsigned int seq, uint64_t src, const void* pData, int len) {
  if(pData) {
    unsigned int msgLen = sizeof(CoreMessageHead) + len;
    CoreMessageHead head;
    head.size = msgLen;
    head.seq=seq;
    head.src=src;

    while (g_MessageQueue.space()<msgLen){
      MySleep(10);
    }
    g_MessageQueue.put(reinterpret_cast<const char*>(&head),sizeof(CoreMessageHead));
    g_MessageQueue.put(reinterpret_cast<const char*>(pData), len);
  }
}

void CallJsCallback(const Napi::Env& env, const Napi::Object& obj, const Napi::Function& func, unsigned int seq, int src, const char* pMsg) {  
  func.Call(obj, {Napi::Number::New(env, seq), Napi::Number::New(env, src), Napi::String::New(env, pMsg)});
}

void ProcessCoreMessage(const Napi::Env& env, const Napi::Object& obj, const Napi::Function& func) {
  unsigned int headSize = sizeof(CoreMessageHead);
  for (int ct = 0; ct < c_MessageCountPerTick; ++ct) {
    if (g_MessageQueue.length()>headSize){      
      const char* startPtr = 0;
      unsigned int sizeToWrapAround = 0;
      const char* bufferPtr = g_MessageQueue.in_place_get(startPtr, sizeToWrapAround);

      if (sizeToWrapAround < headSize) {
        CoreMessageHead head;
        memcpy(reinterpret_cast<char*>(&head), startPtr, sizeToWrapAround);
        unsigned int sizeFromBuffer = headSize - sizeToWrapAround;
        memcpy(reinterpret_cast<char*>(&head) + sizeToWrapAround, bufferPtr, sizeFromBuffer);
        if (static_cast<unsigned int>(head.size) <= g_MessageQueue.length()){
          CallJsCallback(env, obj, func, head.seq, head.src, bufferPtr+sizeFromBuffer);
          g_MessageQueue.in_place_get_advance(head.size);
        } else {
          break;
        }
      } else {
        const CoreMessageHead* pHead = reinterpret_cast<const CoreMessageHead*>(startPtr);
        if (static_cast<unsigned int>(pHead->size) <= g_MessageQueue.length()){
          if (static_cast<unsigned int>(pHead->size) <= sizeToWrapAround){
            CallJsCallback(env, obj, func, pHead->seq, pHead->src, startPtr + headSize);
          } else {
            if (pHead->size <= c_MaxPacketSize){
              memcpy(g_PacketBuffer, startPtr + headSize, sizeToWrapAround - headSize);
              unsigned int sizeFromBuffer = pHead->size - sizeToWrapAround;
              memcpy(g_PacketBuffer + sizeToWrapAround - headSize, bufferPtr, sizeFromBuffer);
              CallJsCallback(env, obj, func, pHead->seq, pHead->src, g_PacketBuffer);
            } else {
              printf("error: too much packet size %u\n", pHead->size);
            }
          }
          g_MessageQueue.in_place_get_advance(pHead->size);
        } else {
          break;
        }
      }
    } else {
      break;
    }
  }
}

int SendCoreMessageByHandle(uint64_t handle, const char* pMsg) {
  int ret=SendByHandle(handle,pMsg,strlen(pMsg)+1);
  return ret;
}

int SendCoreMessageByName(const char* pName, const char* pMsg) {
  int ret=SendByName(pName,pMsg,strlen(pMsg)+1);
  return ret;
}

Napi::Value JsCoreQueryServiceName(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==1 && args[0].IsBigInt()) {
    bool lossless = false;
    uint64_t handle = args[0].As<Napi::BigInt>().Uint64Value(&lossless);
    const int c_max_buf = 1024;
    char namebuf[c_max_buf+1];
    TargetName(handle,namebuf,c_max_buf);
    return Napi::String::New(env, namebuf);
  }
  return env.Null();
}

Napi::Value JsCoreQueryServiceHandle(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==1 && args[0].IsString()) {
    std::string name = args[0].As<Napi::String>().Utf8Value();    
    if(name.c_str()) {
      uint64_t handle = TargetHandle(name.c_str());
      return Napi::Number::New(env, handle);
    }
  }
  return env.Null();
}

Napi::Value JsCoreGetServiceName(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  const int c_max_buf = 1024;
  char namebuf[c_max_buf+1]={0};
  GetConfig("name", namebuf, c_max_buf);
  return Napi::String::New(env, namebuf);
}

Napi::Value JsSendCoreMessageByHandle(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==2 && args[0].IsBigInt() && args[1].IsString()) {
    bool lossless = false;
    uint64_t handle = args[0].As<Napi::BigInt>().Uint64Value(&lossless);
    std::string msg = args[1].As<Napi::String>().Utf8Value();
    if(msg.c_str()) {
      unsigned int r = SendCoreMessageByHandle(handle, msg.c_str());
      return Napi::Number::New(env, r);
    }
  }
  return env.Null();
}

Napi::Value JsSendCoreMessageByName(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==2 && args[0].IsString() && args[1].IsString()) {
    std::string name = args[0].As<Napi::String>().Utf8Value();
    std::string msg = args[1].As<Napi::String>().Utf8Value();
    if(name.c_str() && msg.c_str()) {
      unsigned int r=SendCoreMessageByName(name.c_str(), msg.c_str());
      return Napi::Number::New(env, r);
    }
  }
  return env.Null();
}

Napi::Value JsCoreQueryConfig(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==1 && args[0].IsString()) {
    std::string key = args[0].As<Napi::String>().Utf8Value();
    if(key.c_str()) {
      const int c_max_buf = 1024;
      char cfgbuf[c_max_buf+1];
      int r = GetConfig(key.c_str(),cfgbuf,c_max_buf);
      if(r)
        return Napi::String::New(env, cfgbuf);
      else
        return Napi::String::New(env, "");
    }
  }
  return env.Null();
}

Napi::Value JsSendCoreCommandByHandle(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==2 && args[0].IsBigInt() && args[1].IsString()) {
    bool lossless = false;
    uint64_t handle = args[0].As<Napi::BigInt>().Uint64Value(&lossless);
    std::string msg = args[1].As<Napi::String>().Utf8Value();
    if(msg.c_str()) {
      unsigned int r=SendCommandByHandle(handle, msg.c_str());
      return Napi::Number::New(env, r);
    }
  }
  return env.Null();
}

Napi::Value JsSendCoreCommandByName(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==2 && args[0].IsString() && args[1].IsString()) {
    std::string name = args[0].As<Napi::String>().Utf8Value();
    std::string msg = args[1].As<Napi::String>().Utf8Value();
    if(name.c_str() && msg.c_str()) {
      unsigned int r=SendCommandByName(name.c_str(),msg.c_str());
      return Napi::Number::New(env, r);
    }
  }
  return env.Null();
}

Napi::Value JsCoreStart(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int argc = (int)info.Length();
  const char * * argv = new const char *[argc];
  for (int i = 0; i < argc; ++i) {
    auto arg = info[i];
    if (arg.IsString()) {
      std::string str = arg.As<Napi::String>().Utf8Value();
      argv[i] = str.c_str();
    }
  }
  int r = StartCenterClient(argc, argv);
  delete[] argv;
  return Napi::Number::New(env, r);
}

Napi::Value JsCoreProcessMessage(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if(args.Length()==2 && args[0].IsObject() && args[1].IsFunction()){
  	auto obj = args[0].As<Napi::Object>();
  	auto func = args[1].As<Napi::Function>();
  	ProcessCoreMessage(env, obj, func);
  }
  return env.Null();
}

Napi::Value JsCoreQuit(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  g_IsRun = false;
  WaitForCenterClientExit();
  return env.Null();
}

void HandleNameHandleChanged(int addOrUpdate, const char* name, uint64_t handle)
{
  if(0!=name) {
    printf("name handle changed:%d %s->%lld\n", addOrUpdate, name, handle);
    const int c_max_buf = 1024;
    char cmdbuf[c_max_buf+1];
    sprintf(cmdbuf,"AddOrUpdate %d %s %lld",addOrUpdate,name,handle);
    PushCoreMessage(0xffffffff, 0, cmdbuf, strlen(cmdbuf) + 1);  
  }
}

void HandleMessage(unsigned int seq, uint64_t src, uint64_t dest, const void* msg, int len)
{
  if(0!=msg) {
    PushCoreMessage(seq, src, reinterpret_cast<const char*>(msg), len);
  }
}

void HandleMessageResult(unsigned int seq, uint64_t src, uint64_t dest, int result)
{
}

void HandleCommand(uint64_t src, uint64_t dest, const char* msg)
{
  if(0!=msg) {
    printf("command:%s\n", msg);
    PushCoreMessage(0xffffffff, src, msg, strlen(msg) + 1);
  }
}

void CenterClientTheadProcess(void* p)
{
  while (g_IsRun){
    Tick();
    MySleep(10);
  }
}

int StartCenterClient(int argc, const char * const * argv)
{
  Init("nodejs", argc, argv, HandleNameHandleChanged, HandleMessage, HandleMessageResult, HandleCommand);
  uv_thread_create(&g_ThreadHandle, CenterClientTheadProcess, 0);
  return 0;
}

void WaitForCenterClientExit(void)
{
  if (g_ThreadHandle) {
    g_IsRun = false;
    uv_thread_join(&g_ThreadHandle);
    g_ThreadHandle = NULL;
    Release();
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "core_send_message_by_handle"), Napi::Function::New(env, JsSendCoreMessageByHandle));
  exports.Set(Napi::String::New(env, "core_send_message_by_name"), Napi::Function::New(env, JsSendCoreMessageByName));
  exports.Set(Napi::String::New(env, "core_service_name"), Napi::Function::New(env, JsCoreGetServiceName));
  exports.Set(Napi::String::New(env, "core_query_service_name"), Napi::Function::New(env, JsCoreQueryServiceName));
  exports.Set(Napi::String::New(env, "core_query_service_handle"), Napi::Function::New(env, JsCoreQueryServiceHandle));
  exports.Set(Napi::String::New(env, "core_query_config"), Napi::Function::New(env, JsCoreQueryConfig));
  exports.Set(Napi::String::New(env, "core_send_command_by_handle"), Napi::Function::New(env, JsSendCoreCommandByHandle));
  exports.Set(Napi::String::New(env, "core_send_command_by_name"), Napi::Function::New(env, JsSendCoreCommandByName));

  exports.Set(Napi::String::New(env, "core_start"), Napi::Function::New(env, JsCoreStart));
  exports.Set(Napi::String::New(env, "core_process_message"), Napi::Function::New(env, JsCoreProcessMessage));
  exports.Set(Napi::String::New(env, "core_quit"), Napi::Function::New(env, JsCoreQuit));
  return exports;
}

NODE_API_MODULE(node_center_client, Init)
