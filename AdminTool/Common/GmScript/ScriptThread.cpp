#include "ScriptThread.h"

int CreateStreamPacketObj(FunctionScript::Value& retValue, Interpreter& interpreter, SimpleClient& client, BinaryStream& stream)
{
  int ret = FALSE;
  StreamPacketObj* pObj = new StreamPacketObj(interpreter, client, &stream);
  if (pObj)
  {
    retValue.SetExpression(pObj);
    ret = TRUE;
  }
  return ret;
}

class InteractScriptSource : public IScriptSource
{
public:
  void ClearBuffer(void)
  {
    memset(m_Line[m_CurLine], 0, sizeof(m_Line[m_CurLine]));
  }
  const char* GetBackBuffer(void)const
  {
    return m_Line[1 - m_CurLine];
  }
  void ResetTime(void){ m_Time = 0; }
  unsigned int GetTime(void)const{ return m_Time; }
public:
  InteractScriptSource(ScriptThread& thread, int& runFlag) :m_pScriptThread(&thread), m_pRunFlag(&runFlag), m_Time(0)
  {
    memset(m_Line, 0, sizeof(m_Line));
    m_CurLine = 1;
  }
protected:
  virtual int Load(void)
  {
    unsigned int t1 = MyTimeGetTime();
    int ret = TRUE;
    if (NULL != m_pScriptThread)
    {
      char* p = m_Line[1 - m_CurLine];
      while (FALSE == m_pScriptThread->PopLine(p, ScriptThread::MAX_LINE_SIZE + 1))
      {
        if (NULL != m_pRunFlag && FALSE == *m_pRunFlag)
        {
          ret = FALSE;
          break;
        }
        if (TRUE == m_pScriptThread->IsWaitingQuit())
        {
          m_pScriptThread->StopAfterTheLine();
          ret = FALSE;
          break;
        }
        MySleep(10);
      }
      if (TRUE == ret)
      {
        if (strcmp(p, ".") == 0 ||
          strcmp(p, "enabledebuginfo") == 0 ||
          strcmp(p, "disabledebuginfo") == 0)
        {
          ret = FALSE;
        } else
        {
          ClearBuffer();
          m_CurLine = 1 - m_CurLine;
          ret = TRUE;
        }
      }
    } else
    {
      ret = FALSE;
    }
    unsigned int t2 = MyTimeGetTime();
    m_Time += t2 - t1;
    return ret;
  }
  virtual const char* GetBuffer(void)const
  {
    return m_Line[m_CurLine];
  }
private:
  char m_Line[2][ScriptThread::MAX_LINE_SIZE];
  int m_CurLine;
  ScriptThread* m_pScriptThread;
  int* m_pRunFlag;
private:
  unsigned int m_Time;
};

class SendMessageT2MTApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    void SendMessageMT2T(int, const char*);
    if (0 == pParams || 0 == m_Interpreter)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (1 == num && pParams[0].IsString() && 0 != pRetValue) {
      const char* pMsg = pParams[0].GetString();
      if (pMsg) {
        SendMessageMT2T(-1, pMsg);
        pRetValue->SetInt(1);
      } else {
        pRetValue->SetInt(0);
      }
    }
    else if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && 0 != pRetValue) {
      int index = pParams[0].GetInt();
      const char* pMsg = pParams[1].GetString();
      if (pMsg) {
        SendMessageMT2T(index, pMsg);
        pRetValue->SetInt(1);
      } else {
        pRetValue->SetInt(0);
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit SendMessageT2MTApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class PushMessageMT2TApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams || 0 == m_Interpreter)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (1 == num && pParams[0].IsString() && 0 != pRetValue) {
      const char* pMsg = pParams[0].GetString();
      if (pMsg && m_pScriptThread) {
        SharedMessageQueue::MessageType msg;
        tsnprintf(msg.m_Content, sizeof(msg.m_Content), "%s", pMsg);
        m_pScriptThread->GetSharedMessageQueue().Push(msg);
        pRetValue->SetInt(1);
      } else {
        pRetValue->SetInt(0);
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit PushMessageMT2TApi(Interpreter& interpreter, ScriptThread& scriptThread) :ExpressionApi(interpreter), m_pScriptThread(&scriptThread){}
private:
  ScriptThread* m_pScriptThread;
};

class PopMessageMT2TApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams || 0 == m_Interpreter)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (0 != pRetValue) {
      if (m_pScriptThread) {
        SharedMessageQueue::MessageType msg = m_pScriptThread->GetSharedMessageQueue().Pop();
        char buf[513];
        tsnprintf(buf, sizeof(buf), "%s", msg.m_Content);
        if (buf[0] != 0) {
          pRetValue->AllocString(buf);
        } else {
          pRetValue->SetWeakRefConstString("");
        }
      } else {
        pRetValue->SetWeakRefConstString("");
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit PopMessageMT2TApi(Interpreter& interpreter, ScriptThread& scriptThread) :ExpressionApi(interpreter), m_pScriptThread(&scriptThread){}
private:
  ScriptThread* m_pScriptThread;
};

class GetScriptThreadIndexApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (0 != pRetValue && NULL != m_pScriptThread)
    {
      pRetValue->SetInt(m_pScriptThread->GetIndex());
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  GetScriptThreadIndexApi(Interpreter& interpreter, ScriptThread& scriptThread) :ExpressionApi(interpreter), m_pScriptThread(&scriptThread){}
private:
  ScriptThread* m_pScriptThread;
};

class ExitScriptThreadApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams || 0 == m_Interpreter)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (0 != pRetValue)
    {
      if (m_pScriptThread)
      {
        m_pScriptThread->MarkWaitingQuit();
        m_pScriptThread->StopAfterTheLine();
        pRetValue->SetInt(1);
      } else
      {
        pRetValue->SetInt(0);
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  ExitScriptThreadApi(Interpreter& interpreter, ScriptThread& scriptThread) :ExpressionApi(interpreter), m_pScriptThread(&scriptThread){}
private:
  ScriptThread* m_pScriptThread;
};

void ScriptThread::stop(void)
{
  m_IsContinue = FALSE;
  m_RunFlag = FALSE;
}

void ScriptThread::run(void)
{
  Reset();
  InterpreterOptions options;
  options.SetExpressionPoolSize(4 * 1024);
  options.SetMaxPredefinedValueNum(256);
  options.SetMaxFunctionDimensionNum(8);
  options.SetMaxInnerFunctionApiNum(256);
  options.SetMaxInnerStatementApiNum(32);
  options.SetMaxLocalNum(256);
  options.SetMaxProgramSize(4 * 1024);
  options.SetMaxStatementApiNum(4);
  options.SetMaxStatementNum(1024);
  options.SetStackValuePoolSize(4 * 1024);
  options.SetStringBufferSize(32 * 1024);
  options.SetValuePoolSize(4 * 1024);
  SourceCodeScript script(options);
  Interpreter& interpreter = script.GetInterpreter();
  interpreter.SetExternRunFlagRef(m_RunFlag);

  SharedStringMapObj mapObj(interpreter);
  interpreter.RegisterPredefinedValue("stringMap", FunctionScript::Value(&mapObj));

  SleepApi sleepApi(interpreter);
  interpreter.RegisterPredefinedValue("sleep", FunctionScript::Value(&sleepApi));

  TestPrintfApi testPrintfApi(interpreter);
  interpreter.RegisterPredefinedValue("sprintf", FunctionScript::Value(&testPrintfApi));
  ArgvApi argvApi(interpreter);
  interpreter.RegisterPredefinedValue("argv", FunctionScript::Value(&argvApi));
  interpreter.RegisterPredefinedValue("argc", FunctionScript::Value(g_Argc));

  WriteConsoleApi writeConsoleApi(interpreter);
  interpreter.RegisterPredefinedValue("writeConsole", FunctionScript::Value(&writeConsoleApi));
  GetLogFileIdApi getLogFileIdApi(interpreter);
  interpreter.RegisterPredefinedValue("getLogFileId", FunctionScript::Value(&getLogFileIdApi));
  GetTimeStringApi getTimeStringApi(interpreter);
  interpreter.RegisterPredefinedValue("getTimeString", FunctionScript::Value(&getTimeStringApi));
  GetMillisecondsApi getMillisecondsApi(interpreter);
  interpreter.RegisterPredefinedValue("getMilliseconds", FunctionScript::Value(&getMillisecondsApi));
  ReadStringApi readStringApi(interpreter);
  interpreter.RegisterPredefinedValue("readString", FunctionScript::Value(&readStringApi));
  WriteStringApi writeStringApi(interpreter);
  interpreter.RegisterPredefinedValue("writeString", FunctionScript::Value(&writeStringApi));
  CreateIniReaderApi createIniReaderApi(interpreter);
  interpreter.RegisterPredefinedValue("createIniReader", FunctionScript::Value(&createIniReaderApi));
  CreateTxtTableApi createTxtTableApi(interpreter);
  interpreter.RegisterPredefinedValue("createTxtTable", FunctionScript::Value(&createTxtTableApi));
  CreateConfigTableApi createConfigTableApi(interpreter);
  interpreter.RegisterPredefinedValue("createConfigTable",FunctionScript::Value(&createConfigTableApi));
  CreateXmlVisitorApi createXmlVisitorApi(interpreter);
  interpreter.RegisterPredefinedValue("createXmlVisitor", FunctionScript::Value(&createXmlVisitorApi));

  SimpleClientObj clientObj(interpreter);
  SendMessageT2MTApi sendMessageApi(interpreter);
  PushMessageMT2TApi pushMessageApi(interpreter, *this);
  PopMessageMT2TApi popMessageApi(interpreter, *this);
  GetScriptThreadIndexApi getScriptThreadIndexApi(interpreter, *this);
  ExitScriptThreadApi exitScriptThreadApi(interpreter, *this);

  interpreter.RegisterPredefinedValue("client", FunctionScript::Value(&clientObj));
  interpreter.RegisterPredefinedValue("sendMessage", FunctionScript::Value(&sendMessageApi));
  interpreter.RegisterPredefinedValue("pushMessage", FunctionScript::Value(&pushMessageApi));
  interpreter.RegisterPredefinedValue("popMessage", FunctionScript::Value(&popMessageApi));
  interpreter.RegisterPredefinedValue("getThreadIndex", FunctionScript::Value(&getScriptThreadIndexApi));
  interpreter.RegisterPredefinedValue("exit", FunctionScript::Value(&exitScriptThreadApi));

  InteractScriptSource source(*this, m_RunFlag);
  for (; m_IsContinue;)
  {
    source.ClearBuffer();
    source.ResetTime();
    unsigned int t1 = MyTimeGetTime();
    script.Parse(source);
    t1 += source.GetTime();
    unsigned int t2 = MyTimeGetTime();
    if (interpreter.HasError())
    {
      for (int ix = 0; ix < interpreter.GetErrorNum(); ++ix)
      {
        printf("[%X]:%s\n", (unsigned int)getTID(), interpreter.GetErrorInfo(ix));
      }
      interpreter.Reset();
      printf("[%X]:Interpreter has already reset !\n", (unsigned int)getTID());
    } else
    {
      if (interpreter.GetStatementNum() >= options.GetMaxProgramSize() - 10 ||
        interpreter.GetValueNum() >= options.GetValuePoolSize() - 10 ||
        interpreter.GetStackValueNum() != options.GetStackValuePoolSize() && interpreter.GetStackValueNum() >= options.GetStackValuePoolSize() - 10 ||
        interpreter.GetSyntaxComponentNum() >= options.GetExpressionPoolSize() - 10 ||
        int(interpreter.GetUnusedStringPtrRef() - interpreter.GetStringBuffer()) >= options.GetStringBufferSize() - 1024)
      {
        printf("[%X Statistic] statements:%u value:%u stack:%u syntax:%u string:%u\n", (unsigned int)getTID(), interpreter.GetStatementNum(), interpreter.GetValueNum(), interpreter.GetStackValueNum(), interpreter.GetSyntaxComponentNum(), (unsigned int)(interpreter.GetUnusedStringPtrRef() - interpreter.GetStringBuffer()));
      }
      Value val;
      interpreter.Execute(&val);
    }
    unsigned int t3 = MyTimeGetTime();
    //printf("[%X]:[Time] parse:%u interpret:%u\n",(unsigned int)getTID(),t2-t1,t3-t2);
    const char* line = source.GetBackBuffer();
    if (NULL != line)
    {
      if (strcmp(line, ".") == 0)
      {
        interpreter.Reset();
        m_RunFlag = TRUE;
        printf("[%X]:Process command: reset\n", (unsigned int)getTID());
      } else if (strcmp(line, "enabledebuginfo") == 0)
      {
        interpreter.EnableDebugInfo();
      } else if (strcmp(line, "disabledebuginfo") == 0)
      {
        interpreter.DisableDebugInfo();
      }
    }
    MySleep(100);
  }
}

int ScriptThread::PushLine(const char* p)
{
  AutoLock_T lock(m_Lock);
  if (m_Scripts.Full())
    return FALSE;
  else
  {
    if (strcmp(p, ".") == 0)
    {
      m_RunFlag = FALSE;
    }
    ScriptLine line;
    tsnprintf(line.m_Line, MAX_LINE_SIZE + 1, "%s", p);
    m_Scripts.PushBack(line);
    return TRUE;
  }
}

int ScriptThread::PopLine(char* p, int space)
{
  AutoLock_T lock(m_Lock);
  if (m_Scripts.Empty())
    return FALSE;
  else
  {
    const ScriptLine& line = m_Scripts.Front();
    tsnprintf(p, space, "%s", line.m_Line);
    m_Scripts.PopFront();
    return TRUE;
  }
}