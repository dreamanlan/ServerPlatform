#include "ScriptThread.h"
#include "CommonScriptApi.h"

extern int g_IsRun;
ScriptThread* g_pScriptThread = NULL;
const int c_MaxFileBuffer = 1024 * 1024;
static char g_FileBuffer[c_MaxFileBuffer];

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
  InteractScriptSource(int& runFlag) :m_pRunFlag(&runFlag), m_Time(0)
  {
    memset(m_Line, 0, sizeof(m_Line));
    m_CurLine = 1;
  }
protected:
  virtual int Load(void)
  {
    unsigned int t1 = MyTimeGetTime();
    int ret = TRUE;
    if (NULL != g_pScriptThread) {
      char* p = m_Line[1 - m_CurLine];
      //printf(">");
      //gets(p);
      while (FALSE == g_pScriptThread->PopLine(p, ScriptThread::MAX_LINE_SIZE)) {
        if (NULL != m_pRunFlag && FALSE == *m_pRunFlag) {
          ret = FALSE;
          break;
        }
        if (TRUE == g_pScriptThread->IsWaitingQuit()) {
          g_pScriptThread->StopAfterTheLine();
          ret = FALSE;
          break;
        }
        MySleep(100);
      }
      if (TRUE == ret) {
        if (strcmp(p, ".") == 0 ||
          strcmp(p, "enabledebuginfo") == 0 ||
          strcmp(p, "disabledebuginfo") == 0) {
          ret = FALSE;
        } else if (p == strstr(p, "runfile")) {
          const char* pFile = p + 8;
          FILE* fp = fopen(pFile, "r");
          if (NULL != fp) {
            int size = (int)fread(g_FileBuffer, 1, c_MaxFileBuffer, fp);
            char temp[ScriptThread::MAX_LINE_SIZE + 1];
            int pos = 0;
            for (int cix = 0; cix <= size; ++cix) {
              char c = g_FileBuffer[cix];
              if (c && c != '\n') {
                temp[pos] = (c == '\r' ? 0 : c);
                ++pos;
              } else {
                temp[pos] = 0;
                if (temp[0] != 0) {
                  g_pScriptThread->PushLine(temp);
                }
                pos = 0;
              }
            }
            fclose(fp);
          }
        } else {
          ClearBuffer();
          m_CurLine = 1 - m_CurLine;
          ret = TRUE;
        }
      }
    } else {
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
  int* m_pRunFlag;
private:
  unsigned int m_Time;
};

class ReturnCommandToLogicApi : public ExpressionApi
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
      if (1 == num && pParams[0].IsString()) {
        const char* content = pParams[0].GetString();
        if (content) {
          if (NULL != g_pScriptThread) {
            g_pScriptThread->PushResult(content);
          }
          pRetValue->SetInt(1);
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit ReturnCommandToLogicApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class QuitApi : public ExpressionApi
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
      g_IsRun = FALSE;
      pRetValue->SetInt(1);
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit QuitApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

void ScriptThread::stop(void)
{
  m_IsContinue = FALSE;
  m_RunFlag = FALSE;
}

void ScriptThread::run(void)
{
  SourceCodeScript script;
  Interpreter& interpreter = script.GetInterpreter();
  interpreter.SetExternRunFlagRef(m_RunFlag);

  interpreter.RegisterPredefinedValue("serverType", FunctionScript::Value(g_ServerType));
  ArgvApi argvApi(interpreter, g_Argc, g_Argv);
  interpreter.RegisterPredefinedValue("argv", FunctionScript::Value(&argvApi));
  interpreter.RegisterPredefinedValue("argc", FunctionScript::Value(g_Argc));
  SleepApi sleepApi(interpreter);
  interpreter.RegisterPredefinedValue("sleep", FunctionScript::Value(&sleepApi));

  TestPrintfApi testPrintfApi(interpreter);
  interpreter.RegisterPredefinedValue("sprintf", FunctionScript::Value(&testPrintfApi));
  ReturnCommandToLogicApi returnCommandToLogicApi(interpreter);
  interpreter.RegisterPredefinedValue("returnCommandToLogic", FunctionScript::Value(&returnCommandToLogicApi));
  QuitApi quitApi(interpreter);
  interpreter.RegisterPredefinedValue("quit", FunctionScript::Value(&quitApi));

  WriteConsoleApi writeConsoleApi(interpreter);
  interpreter.RegisterPredefinedValue("writeConsole", FunctionScript::Value(&writeConsoleApi));
  GetMillisecondsApi getMillisecondsApi(interpreter);
  interpreter.RegisterPredefinedValue("getMilliseconds", FunctionScript::Value(&getMillisecondsApi));

  StringMapObj strMapObj(interpreter);
  interpreter.RegisterPredefinedValue("stringMap", FunctionScript::Value(&strMapObj));
  GetLogFileIdApi getLogFileIdApi(interpreter);
  interpreter.RegisterPredefinedValue("getLogFileId", FunctionScript::Value(&getLogFileIdApi));
  GetTimeStringApi getTimeStringApi(interpreter);
  interpreter.RegisterPredefinedValue("getTimeString", FunctionScript::Value(&getTimeStringApi));
  ReadStringApi readStringApi(interpreter);
  interpreter.RegisterPredefinedValue("readString", FunctionScript::Value(&readStringApi));
  WriteStringApi writeStringApi(interpreter);
  interpreter.RegisterPredefinedValue("writeString", FunctionScript::Value(&writeStringApi));
  CreateIniReaderApi createIniReaderApi(interpreter);
  interpreter.RegisterPredefinedValue("createIniReader", FunctionScript::Value(&createIniReaderApi));
  CreateTxtTableApi createTxtTableApi(interpreter);
  interpreter.RegisterPredefinedValue("createTxtTable", FunctionScript::Value(&createTxtTableApi));
  CreateConfigTableApi createConfigTableApi(interpreter);
  interpreter.RegisterPredefinedValue("createConfigTable", FunctionScript::Value(&createConfigTableApi));
  CreateXmlVisitorApi createXmlVisitorApi(interpreter);
  interpreter.RegisterPredefinedValue("createXmlVisitor", FunctionScript::Value(&createXmlVisitorApi));

  InteractScriptSource source(m_RunFlag);
  for (; m_IsContinue;) {
    source.ClearBuffer();
    source.ResetTime();
    unsigned int t1 = MyTimeGetTime();
    script.Parse(source);
    t1 += source.GetTime();
    unsigned int t2 = MyTimeGetTime();
    if (interpreter.HasError()) {
      for (int ix = 0; ix < interpreter.GetErrorNum(); ++ix) {
        printf("%s\n", interpreter.GetErrorInfo(ix));
      }
      interpreter.Reset();
      printf("Interpreter has already reset !\n");
    } else {
      Value val;
      interpreter.Execute(&val);
      if (!val.IsInvalid()) {
        if (val.IsString()) {
          printf("%s\n", val.GetString());
        } else {
          char temp[MAX_NUMBER_STRING_SIZE];
          const char* p = val.ToString(temp, MAX_NUMBER_STRING_SIZE);
          if (p && p[0] != '\0') {
            printf("Command Result:%s\n", p);
          } else {
            printf("Command Result Is Empty.\n");
          }
        }
      } else {
        printf("Command No Result.\n");
      }
    }
    unsigned int t3 = MyTimeGetTime();
    printf("[Time] parse:%u interpret:%u\n", t2 - t1, t3 - t2);
    const char* line = source.GetBackBuffer();
    if (NULL != line) {
      if (strcmp(line, ".") == 0) {
        interpreter.Reset();
        m_RunFlag = TRUE;
        printf("Process command: reset\n");
      } else if (strcmp(line, "enabledebuginfo") == 0) {
        interpreter.EnableDebugInfo();
        printf("Process command: enabledebuginfo\n");
      } else if (strcmp(line, "disabledebuginfo") == 0) {
        interpreter.DisableDebugInfo();
        printf("Process command: disabledebuginfo\n");
      }
    }
    MySleep(100);
  }
}

int ScriptThread::PushLine(const char* p)
{
  AutoLock_T lock(m_ScriptLock);
  if (m_Scripts.Full())
    return FALSE;
  else {
    if (strcmp(p, ".") == 0) {
      m_RunFlag = FALSE;
    }
    TextLine line;
    strncpy(line.m_Line, p, MAX_LINE_SIZE);
    m_Scripts.PushBack(line);
    return TRUE;
  }
}

int ScriptThread::PopLine(char* p, int len)
{
  AutoLock_T lock(m_ScriptLock);
  if (m_Scripts.Empty())
    return FALSE;
  else {
    const TextLine& line = m_Scripts.Front();
    strncpy(p, line.m_Line, len);
    m_Scripts.PopFront();
    return TRUE;
  }
}

int ScriptThread::PushResult(const char* p)
{
  AutoLock_T lock(m_ResultLock);
  if (m_Results.Full())
    return FALSE;
  else {
    TextLine line;
    strncpy(line.m_Line, p, MAX_LINE_SIZE);
    m_Results.PushBack(line);
    return TRUE;
  }
}

int ScriptThread::PopResult(char* p, int len)
{
  AutoLock_T lock(m_ResultLock);
  if (m_Results.Empty())
    return FALSE;
  else {
    const TextLine& line = m_Results.Front();
    strncpy(p, line.m_Line, len);
    m_Results.PopFront();
    return TRUE;
  }
}