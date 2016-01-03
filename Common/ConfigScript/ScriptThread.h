#ifndef __ScriptThread_H__
#define __ScriptThread_H__

#include "Type.h"
#include "Queue.h"
#include "Thread.h"
#include "CommonScriptApi.h"

extern char* g_ServerType;
extern int g_Argc;
extern char** g_Argv;

class ScriptThread : public Thread
{
public:
  enum
  {
    MAX_SCRIPT_NUM = 1024,
    MAX_RESULT_NUM = 1024,
    MAX_LINE_SIZE = 4 * 1024,
  };
private:
  struct TextLine
  {
    char m_Line[MAX_LINE_SIZE];
    TextLine(void)
    {
      m_Line[0] = '\0';
    }
  };
  typedef DequeT<TextLine, MAX_SCRIPT_NUM> Scripts;
  typedef DequeT<TextLine, MAX_RESULT_NUM> Results;
public:
  virtual void stop(void);
  virtual void run(void);
public:
  int PushLine(const char* p);
  int PopLine(char* p, int len);
  int PushResult(const char* p);
  int PopResult(char* p, int len);
  void MarkWaitingQuit(void){ m_IsWaitingQuit = TRUE; }
  int IsWaitingQuit(void)const{ return m_IsWaitingQuit; }
  void StopAfterTheLine(void){ m_IsContinue = FALSE; }
  int CurSrc(void) const { return m_CurSrc; }
  void CurSrc(int val) { m_CurSrc = val; }
public:
  ScriptThread(void) :m_IsWaitingQuit(FALSE), m_IsContinue(TRUE), m_RunFlag(TRUE)
  {}
private:
  int	m_IsWaitingQuit;

  int	m_IsContinue;
  int	m_RunFlag;

  int   m_CurSrc;

  MyLock	m_ScriptLock;
  Scripts	m_Scripts;
  MyLock m_ResultLock;
  Results m_Results;
};

extern ScriptThread* g_pScriptThread;

class ArgvApi : public ExpressionApi
{
  enum
  {
    MAX_STRING_SIZE = 1024,
  };
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      if (1 == num && pParams[0].IsInt()) {
        int ix = pParams[0].GetInt();
        if (ix >= 0 && ix < m_Argc) {
          pRetValue->SetWeakRefConstString(m_Argv[ix]);
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  ArgvApi(Interpreter& interpreter, int argc, char** argv) :ExpressionApi(interpreter), m_Argc(argc), m_Argv(argv){}
private:
  int m_Argc;
  char** m_Argv;
};

#endif //__ScriptThread_H__