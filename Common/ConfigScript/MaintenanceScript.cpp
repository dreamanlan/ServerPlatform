#include "MaintenanceScript.h"
#include "ScriptThread.h"

char* g_ServerType = NULL;
int g_Argc = 0;
char** g_Argv = NULL;

class MaintenanceScript
{
  static const int MAX_LOG_FILE_PATH = 255;
  static const int MAX_ARGUMENT_NUM = 1024;
public:
  void Reload(void)
  {
    AutoLock_T lock(m_Lock);

    char logFilePath[MAX_LOG_FILE_PATH + 1];
    char fileID[256] = { 0 };
    GetLogFileId(fileID, (int)sizeof(fileID));
    tsnprintf(logFilePath, sizeof(logFilePath), "./log/config_%s_%s.log", m_ServerType, fileID);
    char logMsg[1025] = { 0 };

    m_StrMapObj.ClearMap();

    FunctionScript::Interpreter& interpreter = m_Script.GetInterpreter();
    interpreter.Reset();

    char scpFilePath[MAX_LOG_FILE_PATH + 1];
    tsnprintf(scpFilePath, sizeof(scpFilePath), "./config/config.scp");
    FILE* fp = fopen(scpFilePath, "rb");
    if (NULL != fp) {
      fseek(fp, 0, SEEK_END);
      long size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      if (size >= 0) {
        char* p = new char[size + 1];
        if (NULL != p) {
          fread(p, 1, size, fp);
          fclose(fp);
          p[size] = 0;
          int scpRet = FALSE;

          m_Script.Parse(p);
          if (interpreter.HasError()) {
            for (int ix = 0; ix < interpreter.GetErrorNum(); ++ix) {
              tsnprintf(logMsg, sizeof(logMsg), "%s\n", interpreter.GetErrorInfo(ix));
              printf("%s", logMsg);
              WriteString(logFilePath, logMsg);
            }
            interpreter.Reset();
          }
          Value val;
          interpreter.Execute(&val);
          if (!val.IsInvalid() && val.IsInt()) {
            scpRet = val.GetInt();
          }
          interpreter.Reset();
          delete[] p;
          p = NULL;
          {
            tsnprintf(logMsg, sizeof(logMsg), "config.scp return %d.", scpRet);
            printf("%s", logMsg);
            WriteString(logFilePath, logMsg);
            if (!scpRet) {
              printf("Config.scp execute failed !!! Please read ./log/config_*.log !!!\n");
            }
          }
        } else {
          fclose(fp);
          printf("Can't alloc memory for read ./config/config.scp size !!!\n");
        }
      } else {
        fclose(fp);
        printf("Can't get ./config/config.scp size !!!\n");
      }
    } else {
      printf("Can't open ./config/config.scp !!!\n");
    }
  }
  const char* GetString(const char* key) const
  {
    if (NULL != key) {
      AutoLock_T lock(m_Lock);
      return m_StrMapObj.Get(key);
    }
    return NULL;
  }
  const StringMapObj& GetStringMap(void) const { return m_StrMapObj; }
  StringMapObj& GetStringMap(void){ return m_StrMapObj; }
  MyLock& GetLock(void){ return m_Lock; }
public:
  MaintenanceScript(const char* pServerType, int argc, char* argv[]) :
    m_Script(),
    m_SleepApi(m_Script.GetInterpreter()),
    m_TestPrintfApi(m_Script.GetInterpreter()),
    m_WriteConsoleApi(m_Script.GetInterpreter()),
    m_GetMillisecondsApi(m_Script.GetInterpreter()),
    m_StrMapObj(m_Script.GetInterpreter()),
    m_GetLogFileIdApi(m_Script.GetInterpreter()),
    m_GetTimeStringApi(m_Script.GetInterpreter()),
    m_ReadStringApi(m_Script.GetInterpreter()),
    m_WriteStringApi(m_Script.GetInterpreter()),
    m_CreateIniReaderApi(m_Script.GetInterpreter()),
    m_CreateTxtTableApi(m_Script.GetInterpreter()),
    m_CreateConfigTableApi(m_Script.GetInterpreter()),
    m_CreateXmlVisitorApi(m_Script.GetInterpreter()),
    m_Argc(argc < MAX_ARGUMENT_NUM ? argc : MAX_ARGUMENT_NUM),
    m_ArgvApi(m_Script.GetInterpreter(), m_Argc, m_Argv)
  {
    FunctionScript::Interpreter& interpreter = m_Script.GetInterpreter();
    tsnprintf(m_ServerType, sizeof(m_ServerType), "%s", pServerType);

    interpreter.RegisterPredefinedValue("sleep", FunctionScript::Value(&m_SleepApi));

    interpreter.RegisterPredefinedValue("sprintf", FunctionScript::Value(&m_TestPrintfApi));
    interpreter.RegisterPredefinedValue("writeConsole", FunctionScript::Value(&m_WriteConsoleApi));
    interpreter.RegisterPredefinedValue("getMilliseconds", FunctionScript::Value(&m_GetMillisecondsApi));

    interpreter.RegisterPredefinedValue("stringMap", FunctionScript::Value(&m_StrMapObj));
    interpreter.RegisterPredefinedValue("getLogFileId", FunctionScript::Value(&m_GetLogFileIdApi));
    interpreter.RegisterPredefinedValue("getTimeString", FunctionScript::Value(&m_GetTimeStringApi));
    interpreter.RegisterPredefinedValue("readString", FunctionScript::Value(&m_ReadStringApi));
    interpreter.RegisterPredefinedValue("writeString", FunctionScript::Value(&m_WriteStringApi));
    interpreter.RegisterPredefinedValue("createIniReader", FunctionScript::Value(&m_CreateIniReaderApi));
    interpreter.RegisterPredefinedValue("createTxtTable", FunctionScript::Value(&m_CreateTxtTableApi));
    interpreter.RegisterPredefinedValue("createConfigTable", FunctionScript::Value(&m_CreateConfigTableApi));
    interpreter.RegisterPredefinedValue("createXmlVisitor", FunctionScript::Value(&m_CreateXmlVisitorApi));
    interpreter.RegisterPredefinedValue("serverType", FunctionScript::Value(m_ServerType));
    interpreter.RegisterPredefinedValue("argc", FunctionScript::Value(m_Argc));
    for (int i = 0; i < m_Argc; ++i) {
      int len = strlen(argv[i]);
      m_Argv[i] = new char[len + 1];
      memcpy(m_Argv[i], argv[i], len + 1);
    }
    g_ServerType = m_ServerType;
    g_Argc = m_Argc;
    g_Argv = m_Argv;
    interpreter.RegisterPredefinedValue("argv", FunctionScript::Value(&m_ArgvApi));
  }
  ~MaintenanceScript(void)
  {
    for (int i = 0; i < m_Argc; ++i) {
      delete[] m_Argv[i];
    }
  }
private:
  SourceCodeScript  m_Script;
  char              m_ServerType[MAX_LOG_FILE_PATH];
  int               m_Argc;
  char*             m_Argv[MAX_ARGUMENT_NUM];
private:
  SleepApi 			m_SleepApi;
  TestPrintfApi     m_TestPrintfApi;
  WriteConsoleApi   m_WriteConsoleApi;
  GetMillisecondsApi m_GetMillisecondsApi;
  StringMapObj      m_StrMapObj;
  GetLogFileIdApi   m_GetLogFileIdApi;
  GetTimeStringApi  m_GetTimeStringApi;
  ReadStringApi     m_ReadStringApi;
  WriteStringApi    m_WriteStringApi;
  CreateIniReaderApi  m_CreateIniReaderApi;
  CreateTxtTableApi   m_CreateTxtTableApi;
  CreateConfigTableApi  m_CreateConfigTableApi;
  CreateXmlVisitorApi m_CreateXmlVisitorApi;
  ArgvApi             m_ArgvApi;

  mutable MyLock      m_Lock;
};

static MaintenanceScript* s_pMaintenanceScript = NULL;

void InitConfigState(const char* pServerType, int argc, char* argv[])
{
  if (NULL == s_pMaintenanceScript) {
    s_pMaintenanceScript = new MaintenanceScript(pServerType, argc, argv);
    s_pMaintenanceScript->Reload();
  }
}

void ReloadConfigState(void)
{
  if (NULL != s_pMaintenanceScript) {
    s_pMaintenanceScript->Reload();
  }
}

const char* GetConfig(const char* name)
{
  const char* pRet = NULL;
  if (NULL != s_pMaintenanceScript) {
    pRet = s_pMaintenanceScript->GetString(name);
  }
  return pRet;
}