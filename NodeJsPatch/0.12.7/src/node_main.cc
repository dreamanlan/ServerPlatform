// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "env-inl.h"
#include "node.h"
#include "uv.h"
#include "kfifo.h"
#include "CenterClient.h"

#ifdef _WIN32
#include "windows.h"
#else
#include <unistd.h>
#include <pthread.h>
#endif

using namespace v8;

//----------------------------------------------------------------------------------------------------------------------------------------
//将来自CenterApi的消息数据传给v8
//----------------------------------------------------------------------------------------------------------------------------------------
namespace node {
  extern void CallCoreMessageCallback(unsigned int seq, int src, const char* msg);
}
#pragma pack(push,1)
struct CoreMessageHead {
  int size;
  unsigned int seq;
  int src;
};
#pragma pack(pop)

uv_async_t g_AsyncHandle;
bool g_IsRun = true;
static uv_thread_t g_ThreadHandle = 0;
static const int c_MaxPacketSize = 1024 * 1024;
static char g_PacketBuffer[c_MaxPacketSize];
static const int c_MessageQueueSize = 512 * 1024 * 1024;
static const int c_MessageCountPerTick = 1024;
static kfifo<char> g_MessageQueue(c_MessageQueueSize);

static inline void MySleep(unsigned int ms)
{
#ifdef WIN32
  SleepEx(ms, 0);
#else
  usleep(ms * 1000);
#endif
}

void PushCoreMessage(unsigned int seq, int src, const void* pData, int len) {
  if(pData) {
    unsigned int msgLen = sizeof(CoreMessageHead) + len;
    CoreMessageHead head;
    head.size = msgLen;
    head.seq=seq;
    head.src=src;

    //队列满了，休息。。
    while (g_MessageQueue.space()<msgLen){
      MySleep(10);
    }
    g_MessageQueue.put(reinterpret_cast<const char*>(&head),sizeof(CoreMessageHead));
    g_MessageQueue.put(reinterpret_cast<const char*>(pData), len);
    //通知libuv处理消息
    uv_async_send(&g_AsyncHandle);
  }
}

void ProcessCoreMessage(uv_async_t*) {
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
        if (head.size <= g_MessageQueue.length()){
          node::CallCoreMessageCallback(head.seq, head.src, bufferPtr+sizeFromBuffer);
          g_MessageQueue.in_place_get_advance(head.size);
        } else {
          break;
        }
      } else {
        const CoreMessageHead* pHead = reinterpret_cast<const CoreMessageHead*>(startPtr);
        if (pHead->size <= g_MessageQueue.length()){
          if (pHead->size <= sizeToWrapAround){
            node::CallCoreMessageCallback(pHead->seq, pHead->src, startPtr + headSize);
          } else {
            if (pHead->size <= c_MaxPacketSize){
              memcpy(g_PacketBuffer, startPtr + headSize, sizeToWrapAround - headSize);
              unsigned int sizeFromBuffer = pHead->size - sizeToWrapAround;
              memcpy(g_PacketBuffer + sizeToWrapAround - headSize, bufferPtr, sizeFromBuffer);
              node::CallCoreMessageCallback(pHead->seq, pHead->src, g_PacketBuffer);
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
  if (!g_IsRun){
    uv_close(reinterpret_cast<uv_handle_t*>(&g_AsyncHandle), NULL);
    uv_stop(uv_default_loop());
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------
//暴露给v8的Api
//----------------------------------------------------------------------------------------------------------------------------------------
int SendCoreMessageByHandle(int handle, const char* pMsg) {
  int ret=SendByHandle(handle,pMsg,strlen(pMsg)+1);
  return ret;
}

int SendCoreMessageByName(const char* pName, const char* pMsg) {
  int ret=SendByName(pName,pMsg,strlen(pMsg)+1);
  return ret;
}

void JsCoreQueryServiceName(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==1 && args[0]->IsInt32()) {
    int handle = args[0]->Int32Value();
    const int c_max_buf = 1024;
    char namebuf[c_max_buf+1];
    TargetName(handle,namebuf,c_max_buf);
    args.GetReturnValue().Set(String::NewFromUtf8(env->isolate(), namebuf));
    return;
  }
  args.GetReturnValue().SetEmptyString();
}

void JsCoreQueryServiceHandle(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==1 && args[0]->IsString()) {
    v8::String::Utf8Value name(args[0]);    
    if(*name) {
      int handle = TargetHandle(*name);
      args.GetReturnValue().Set(handle);
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsCoreGetServiceName(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  const int c_max_buf = 1024;
  char namebuf[c_max_buf+1]={0};
  GetConfig("name", namebuf, c_max_buf);
  args.GetReturnValue().Set(String::NewFromUtf8(env->isolate(), namebuf));
  return;
}

void JsSendCoreMessageByHandle(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==2 && args[0]->IsUint32() && args[1]->IsString()) {
    unsigned int handle = args[0]->Uint32Value();
    v8::String::Utf8Value msg(args[1]);
    if(*msg) {
      unsigned int r = SendCoreMessageByHandle(handle, *msg);
      args.GetReturnValue().Set(r);
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsSendCoreMessageByName(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==2 && args[0]->IsString() && args[1]->IsString()) {
    v8::String::Utf8Value name(args[0]);
    v8::String::Utf8Value msg(args[1]);
    if(*name && *msg) {
      unsigned int r=SendCoreMessageByName(*name,*msg);
      args.GetReturnValue().Set(r);
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsCoreQueryConfig(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==1 && args[0]->IsString()) {
    v8::String::Utf8Value key(args[0]);
    if(*key) {
      const int c_max_buf = 1024;
      char cfgbuf[c_max_buf+1];
      int r = GetConfig(*key,cfgbuf,c_max_buf);
      if(r)
        args.GetReturnValue().Set(String::NewFromUtf8(env->isolate(), cfgbuf));
      else
        args.GetReturnValue().SetEmptyString();
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsSendCoreCommandByHandle(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==2 && args[0]->IsUint32() && args[1]->IsString()) {
    unsigned int handle = args[0]->Uint32Value();
    v8::String::Utf8Value msg(args[1]);
    if(*msg) {
      unsigned int r=SendCommandByHandle(handle,*msg);
      args.GetReturnValue().Set(r);
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsSendCoreCommandByName(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  if(args.Length()==2 && args[0]->IsString() && args[1]->IsString()) {
    v8::String::Utf8Value name(args[0]);
    v8::String::Utf8Value msg(args[1]);
    if(*name && *msg) {
      unsigned int r=SendCommandByName(*name,*msg);
      args.GetReturnValue().Set(r);
      return;
    }
  }
  args.GetReturnValue().SetUndefined();
}

void JsCoreQuitNode(const FunctionCallbackInfo<Value>& args) {
  node::Environment* env = node::Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());
  g_IsRun = false;
  uv_async_send(&g_AsyncHandle);
  args.GetReturnValue().SetUndefined();
}

void RegisterCoreFunctions(Handle<Object> global) {
  TryCatch tryCatch;

  NODE_SET_METHOD(global,"core_send_message_by_handle",&JsSendCoreMessageByHandle);
  NODE_SET_METHOD(global, "core_send_message_by_name", &JsSendCoreMessageByName);
  NODE_SET_METHOD(global, "core_service_name", &JsCoreGetServiceName);
  NODE_SET_METHOD(global, "core_query_service_name", &JsCoreQueryServiceName);
  NODE_SET_METHOD(global, "core_query_service_handle", &JsCoreQueryServiceHandle);
  NODE_SET_METHOD(global, "core_query_config", &JsCoreQueryConfig);
  NODE_SET_METHOD(global, "core_send_command_by_handle", &JsSendCoreCommandByHandle);
  NODE_SET_METHOD(global, "core_send_command_by_name", &JsSendCoreCommandByName);
  NODE_SET_METHOD(global, "core_quit_nodejs", &JsCoreQuitNode);
}

//----------------------------------------------------------------------------------------------------------------------------------------
//CenterApi相关部分
//----------------------------------------------------------------------------------------------------------------------------------------
void HandleNameHandleChanged(int addOrUpdate, const char* name, int handle)
{
  if(0!=name) {
    printf("name handle changed:%d %s->%d\n", addOrUpdate, name, handle);
    const int c_max_buf = 1024;
    char cmdbuf[c_max_buf+1];
    sprintf(cmdbuf,"AddOrUpdate %d %s %d",addOrUpdate,name,handle);
    PushCoreMessage(0xffffffff, 0, cmdbuf, strlen(cmdbuf) + 1);  
  }
}

void HandleMessage(unsigned int seq, int src, int dest, const void* msg, int len)
{
  if(0!=msg) {
    PushCoreMessage(seq, src, reinterpret_cast<const char*>(msg), len);
  }
}

void HandleMessageResult(unsigned int seq, int src, int dest, int result)
{
}

void HandleCommand(int src, int dest, const char* msg)
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

int StartCenterClient(int argc, char *argv[]) {
  Init("nodejs", argc - 1, argc > 1 ? &argv[1] : NULL, HandleNameHandleChanged, HandleMessage, HandleMessageResult, HandleCommand);
  uv_thread_create(&g_ThreadHandle, CenterClientTheadProcess, 0);
  return 0;
}

void WaitForCenterClientExit(void) {
  if (g_ThreadHandle) {
    g_IsRun = false;
    uv_thread_join(&g_ThreadHandle);
    g_ThreadHandle = NULL;
    Release();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------
//nodejs启动部分
//----------------------------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
int wmain(int argc, wchar_t *wargv[]) {
  // Convert argv to to UTF8
  char** argv = new char*[argc];
  for (int i = 0; i < argc; i++) {
    // Compute the size of the required buffer
    DWORD size = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     wargv[i],
                                     -1,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL);
    if (size == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
    // Do the actual conversion
    argv[i] = new char[size];
    DWORD result = WideCharToMultiByte(CP_UTF8,
                                       0,
                                       wargv[i],
                                       -1,
                                       argv[i],
                                       size,
                                       NULL,
                                       NULL);
    if (result == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
  }
  StartCenterClient(argc,argv);
  // Now that conversion is done, we can finally start.
  return node::Start(argc, argv);
}
#else
// UNIX
int main(int argc, char *argv[]) {
  StartCenterClient(argc,argv);
  return node::Start(argc, argv);
}
#endif
