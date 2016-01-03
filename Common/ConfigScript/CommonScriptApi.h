#ifndef __CommonScriptApi_H__
#define __CommonScriptApi_H__

#include "tinyxml.h"
#include <time.h>
#include "Thread.h"
#include "SourceCodeScript.h"

class SleepApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    static const int s_MaxSleepTime = 60000;
    if (1 != num || 0 == pParams)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (0 != pRetValue && pParams[0].IsInt()) {
      int time = pParams[0].GetInt();
      if (time >= 0 && time <= s_MaxSleepTime) {
        MySleep(time);
        pRetValue->SetInt(1);
      } else {
        MySleep(0);
        pRetValue->SetInt(0);
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit SleepApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class TestPrintfApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (3 != num || 0 == pParams)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (0 != pRetValue && pParams[0].IsString() && pParams[1].IsString() && pParams[2].IsString()) {
      const char* pFmt = pParams[0].GetString();
      const char* pVal = pParams[1].GetString();
      const char* pType = pParams[2].GetString();

      if (0 != pFmt && 0 != pVal && 0 != pType) {
        static const int s_c_TempLen = 1025;
        char temp[s_c_TempLen] = { "" };
        if (strcmp(pType, "int") == 0) {
          int val = atoi(pVal);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "uint") == 0) {
          unsigned int val = (unsigned int)atoi(pVal);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "xint") == 0) {
          int val = 0;
          sscanf(pVal, "%x", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "xuint") == 0) {
          unsigned int val = 0;
          sscanf(pVal, "%x", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "longlong") == 0) {
          long long val = 0;
          sscanf(pVal, "%lld", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "ulonglong") == 0) {
          unsigned long long val = 0;
          sscanf(pVal, "%llu", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "xlonglong") == 0) {
          long long val = 0;
          sscanf(pVal, "%llx", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "xulonglong") == 0) {
          unsigned long long val = 0;
          sscanf(pVal, "%llx", &val);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "float") == 0) {
          float val = (float)atof(pVal);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "double") == 0) {
          double val = atof(pVal);
          tsnprintf(temp, s_c_TempLen, pFmt, val);
          pRetValue->AllocString(temp);
        } else if (strcmp(pType, "string") == 0) {
          tsnprintf(temp, s_c_TempLen, pFmt, pVal);
          pRetValue->AllocString(temp);
        }
      } else {
        pRetValue->SetWeakRefString("");
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit TestPrintfApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class WriteConsoleApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams)
      return EXECUTE_RESULT_NORMAL;
    else {
      /*for(INT ix=0;ix<num;++ix)
      {
      if(pParams[ix].IsIndex() || pParams[ix].IsArgIndex() || pParams[ix].IsLocalIndex())
      {
      printf("[%d:%s]\n",ix,m_Interpreter->GetValueName(pParams[ix].GetType(),pParams[ix].GetInt()));
      }
      }*/
      ReplaceVariableWithValue(pParams, num);
      for (int ix = 0; ix < num; ++ix) {
        char buf[MAX_NUMBER_STRING_SIZE];
        const char* pBuf = pParams[ix].ToString(buf, MAX_NUMBER_STRING_SIZE);
        if (0 != pBuf) {
          printf("%s", pBuf);
        }
      }
      return EXECUTE_RESULT_NORMAL;
    }
  }
public:
  explicit WriteConsoleApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class GetMillisecondsApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      unsigned int t = MyTimeGetTime();
      pRetValue->SetInt(t);
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit GetMillisecondsApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

static inline void Trim(char*& p)
{
  if (NULL != p) {
    int size = strlen(p);
    char* pEnd = p + size - 1;
    while (*p == ' ' || *p == '\t') {
      ++p;
    }
    while (pEnd >= p && (*pEnd == ' ' || *pEnd == '\t' || *pEnd == '\r' || *pEnd == '\n')) {
      *pEnd = 0;
      --pEnd;
    }
  }
}

static inline int GetLogFileId(char* pStr, int size)
{
  int ret = FALSE;
  if (NULL != pStr) {
    time_t t;
    memset(&t, 0, sizeof(t));
    time(&t);
    struct tm* pTM = localtime(&t);
    if (NULL != pTM) {
      unsigned int pid = 0, tid = 0;
#if defined(__WINDOWS__)
      pid = (unsigned int)GetCurrentProcessId();
      tid = (unsigned int)GetCurrentThreadId();
#else
      pid = (unsigned int)getpid();
      tid = (unsigned int)pthread_self();
#endif //defined(__WINDOWS__)
      tsnprintf(pStr, size, "%.4d-%.2d-%.2d_%d-%u-%u", (pTM->tm_year + 1900), (pTM->tm_mon + 1), pTM->tm_mday, pTM->tm_hour, pid, tid);
      ret = TRUE;
    }
  }
  return ret;
}

static inline int GetTimeString(char* pStr, int size)
{
  int ret = FALSE;
  if (NULL != pStr) {
    time_t t;
    memset(&t, 0, sizeof(t));
    time(&t);
    struct tm* pTM = localtime(&t);
    if (NULL != pTM) {
      tsnprintf(pStr, size, "%.4d-%.2d-%.2d_%.2d-%.2d-%.2d", (pTM->tm_year + 1900), (pTM->tm_mon + 1), pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
      ret = TRUE;
    }
  }
  return ret;
}

template<int SizeV>
struct ConstSizeT
{
  static const int MAX_STRING_SIZE = SizeV;
};

template<typename MaxSizeT>
static inline int ReadString(const char* pFile, int startRow, char* pStr, int space, const MaxSizeT&)
{
  const int MAX_STRING_SIZE = MaxSizeT::MAX_STRING_SIZE;
  int ret = FALSE;
  if (pFile && pStr) {
    FILE* fp = fopen(pFile, "rb");
    if (NULL != fp) {
      for (int ix = 0; ix < startRow && !feof(fp); ++ix) {
        char temp[MAX_STRING_SIZE + 1] = { 0 };
        fgets(temp, MAX_STRING_SIZE, fp);
      }
      if (!feof(fp)) {
        char line[MAX_STRING_SIZE + 1] = { 0 };
        fgets(line, MAX_STRING_SIZE, fp);
        char* p = line;
        Trim(p);
        tsnprintf(pStr, space, "%s", p);
        ret = TRUE;
      }
      fclose(fp);
    }
  }
  return ret;
}

static inline int WriteString(const char* pFile, const char* pStr, const char* pEnd = "\n")
{
  int ret = FALSE;
  if (pFile && pStr && pEnd) {
    FILE* fp = fopen(pFile, "ab");
    if (NULL != fp) {
      fprintf(fp, "%s%s", pStr, pEnd);
      fclose(fp);
      ret = TRUE;
    }
  }
  return ret;
}

class IniReader
{
  enum
  {
    INFO_NUM_DEFAULT = 64,
    INFO_NUM_DELTA = 1024,
  };
public:
  struct IniInfo
  {
    char*		m_pSection;
    char*		m_pKey;
    char*		m_pValue;

    IniInfo(void) :m_pSection(""), m_pKey(""), m_pValue("")
    {}
    static inline IniInfo& GetInvalidRef(void)
    {
      static IniInfo s_Info;
      return s_Info;
    }
  };
public:
  int Load(const char* file)
  {
    int ret = FALSE;
    Unload();
    if (file) {
      FILE* fp = fopen(file, "rb");
      if (NULL != fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        if (size > 0) {
          fseek(fp, 0, SEEK_SET);
          m_pBuffer = new char[size + 1];
          if (NULL != m_pBuffer) {
            size_t realSize = fread(m_pBuffer, 1, size, fp);
            if (realSize == (size_t)size) {
              m_pBuffer[size] = 0;
              int lineNum = INFO_NUM_DEFAULT;
              char* p0 = m_pBuffer;
              if (p0) {
                //统计前面几行的平均长度
                int sum = 0;
                int ct = 0;
                for (; ct < 5; ++ct) {
                  char* p = strchr(p0 + 1, '\n');
                  if (p) {
                    sum += int(p - p0);
                    p0 = p;
                  } else {
                    break;
                  }
                }
                if (ct>0) {
                  lineNum = int(size*ct / sum);
                }
              }
              InitInfoSpace(lineNum);
              ret = Parse();
              if (FALSE == ret)
                Unload();
            } else {
              Unload();
            }
          }
        }
        fclose(fp);
      }
    }
    return ret;
  }
  int Parse(void)
  {
    int ret = FALSE;
    if (NULL != m_pBuffer && NULL != m_pIniInfos) {
      char* p = m_pBuffer;
      if (NULL != p) {
        char* pSection = "";
        for (; p;) {
          if (m_InfoCount >= m_InfoSpace) {
            ResizeInfoSpace();
          }
          if (m_InfoCount < m_InfoSpace) {
            char* p0 = p;
            p = strchr(p0, '\n');
            if (p) {
              *p = 0;
              ++p;
            }
            Trim(p0);
            if (p0[0] == ';' || p0[0] == '#') {
              continue;
            }
            if (p0[0] == '[') {
              char* p1 = p0 + 1;
              char* p2 = strchr(p0, ']');
              if (p2) {
                *p2 = 0;
                ++p2;
                char* p3 = strchr(p2, ';');
                if (p3) {
                  *p3 = 0;
                }
                Trim(p1);
                Trim(p2);
                if (p2[0] == 0) {
                  pSection = p1;
                }
              }
            } else {
              char* p1 = p0;
              char* p2 = strchr(p0, '=');
              if (p2) {
                *p2 = 0;
                ++p2;
                char* p3 = strchr(p2, ';');
                if (p3) {
                  *p3 = 0;
                }
                Trim(p1);
                Trim(p2);
                m_pIniInfos[m_InfoCount].m_pSection = pSection;
                m_pIniInfos[m_InfoCount].m_pKey = p1;
                m_pIniInfos[m_InfoCount].m_pValue = p2;
                ++m_InfoCount;
              }
            }
          } else {
            break;
          }
        }
        ret = TRUE;
      }
    }
    return ret;
  }
  void Unload(void)
  {
    if (NULL != m_pIniInfos) {
      delete[] m_pIniInfos;
      m_pIniInfos = NULL;
    }
    if (NULL != m_pBuffer) {
      delete[] m_pBuffer;
      m_pBuffer = NULL;
    }
    m_InfoCount = 0;
    m_InfoSpace = 0;
  }
  int	GetInfoCount(void)const
  {
    return m_InfoCount;
  }
  const IniInfo& GetInfo(int index)const
  {
    if (NULL != m_pIniInfos) {
      if (index >= 0 && index < m_InfoCount) {
        return m_pIniInfos[index];
      }
    }
    return IniInfo::GetInvalidRef();
  }
public:
  IniReader(void) :m_pBuffer(NULL), m_pIniInfos(NULL), m_InfoCount(0), m_InfoSpace(0)
  {}
  ~IniReader(void)
  {
    Unload();
  }
private:
  void			InitInfoSpace(int size)
  {
    if (NULL == m_pIniInfos) {
      IniInfo* pNew = new IniInfo[size];
      if (NULL != pNew) {
        m_pIniInfos = pNew;
        m_InfoSpace = size;
      }
    }
  }
  void			ResizeInfoSpace(void)
  {
    if (NULL != m_pIniInfos) {
      IniInfo* pNew = new IniInfo[m_InfoSpace + INFO_NUM_DELTA];
      if (NULL != pNew) {
        for (int ix = 0; ix < m_InfoCount; ++ix) {
          pNew[ix] = m_pIniInfos[ix];
        }
        delete[] m_pIniInfos;
        m_pIniInfos = pNew;
        m_InfoSpace += INFO_NUM_DELTA;
      }
    }
  }
private:
  char*			m_pBuffer;
  IniInfo*		m_pIniInfos;
  int				m_InfoCount;
  int				m_InfoSpace;
};

class IniReaderObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_LOAD = 0,
    CUSTOM_MEMBER_INDEX_UNLOAD,
    CUSTOM_MEMBER_INDEX_GETINFOCOUNT,
    CUSTOM_MEMBER_INDEX_GETSECTION,
    CUSTOM_MEMBER_INDEX_GETKEY,
    CUSTOM_MEMBER_INDEX_GETVALUE,
    CUSTOM_MEMBER_INDEX_FINDVALUE,
    CUSTOM_MEMBER_INDEX_NUM,
  };
public:
  int Load(const char* file)
  {
    return m_IniReader.Load(file);
  }
  void Unload(void)
  {
    m_IniReader.Unload();
  }
public:
  IniReaderObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM)
  {}
  virtual ~IniReaderObj(void)
  {}
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "load") == 0)
      return CUSTOM_MEMBER_INDEX_LOAD;
    else if (strcmp(name, "unload") == 0)
      return CUSTOM_MEMBER_INDEX_UNLOAD;
    else if (strcmp(name, "getInfoCount") == 0)
      return CUSTOM_MEMBER_INDEX_GETINFOCOUNT;
    else if (strcmp(name, "getSection") == 0)
      return CUSTOM_MEMBER_INDEX_GETSECTION;
    else if (strcmp(name, "getKey") == 0)
      return CUSTOM_MEMBER_INDEX_GETKEY;
    else if (strcmp(name, "getValue") == 0)
      return CUSTOM_MEMBER_INDEX_GETVALUE;
    else if (strcmp(name, "findValue") == 0)
      return CUSTOM_MEMBER_INDEX_FINDVALUE;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (NULL != pParams) {
      switch (index) {
      case CUSTOM_MEMBER_INDEX_LOAD:
      {
        if (1 == num && pParams[0].IsString() && pParams[0].GetString() && NULL != pRetValue) {
          int ret = m_IniReader.Load(pParams[0].GetString());
          pRetValue->SetInt(ret);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_UNLOAD:
      {
        if (NULL != pRetValue) {
          m_IniReader.Unload();
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETINFOCOUNT:
      {
        if (NULL != pRetValue)
          pRetValue->SetInt(m_IniReader.GetInfoCount());
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETSECTION:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int index = pParams[0].GetInt();
          int ct = m_IniReader.GetInfoCount();
          if (index < ct) {
            const IniReader::IniInfo& info = m_IniReader.GetInfo(index);
            if (info.m_pSection)
              pRetValue->SetWeakRefString(info.m_pSection);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETKEY:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int index = pParams[0].GetInt();
          int ct = m_IniReader.GetInfoCount();
          if (index < ct) {
            const IniReader::IniInfo& info = m_IniReader.GetInfo(index);
            if (info.m_pKey)
              pRetValue->SetWeakRefString(info.m_pKey);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETVALUE:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int index = pParams[0].GetInt();
          int ct = m_IniReader.GetInfoCount();
          if (index < ct) {
            const IniReader::IniInfo& info = m_IniReader.GetInfo(index);
            if (info.m_pValue)
              pRetValue->SetWeakRefString(info.m_pValue);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_FINDVALUE:
      {
        if (2 == num && pParams[0].IsString() && pParams[1].IsString() && NULL != pRetValue) {
          const char* pSection = pParams[0].GetString();
          const char* pKey = pParams[1].GetString();
          if (pSection && pKey) {
            char* p = "";
            int ct = m_IniReader.GetInfoCount();
            for (int ix = 0; ix < ct; ++ix) {
              const IniReader::IniInfo& info = m_IniReader.GetInfo(ix);
              if (info.m_pSection && info.m_pKey && info.m_pValue) {
                if (strcmp(pSection, info.m_pSection) == 0 && strcmp(pKey, info.m_pKey) == 0) {
                  p = info.m_pValue;
                  break;
                }
              }
            }
            pRetValue->SetWeakRefString(p);
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  IniReader	m_IniReader;
};

class TxtTable
{
  enum
  {
    ROW_NUM_DEFAULT = 64,
    ROW_NUM_DELTA = 1024,
  };
  typedef char*		ColumnType;
  typedef ColumnType*	RowType;
public:
  struct ColumnInfo
  {
    int			m_ColIndex;
    const char*	m_ColValue;

    ColumnInfo(void) :m_ColIndex(-1), m_ColValue(NULL)
    {}
  };
public:
  int Find(int startRowIndex, const ColumnInfo* pInfos, int num)const
  {
    int rowIndex = -1;
    if (NULL != pInfos && startRowIndex >= 0 && num >= 0) {
      for (int ix = startRowIndex; ix < m_RowCount; ++ix) {
        bool find = true;
        for (int index = 0; index < num; ++index) {
          int colIndex = pInfos[index].m_ColIndex;
          const char* pVal1 = pInfos[index].m_ColValue;
          if (colIndex >= 0 && colIndex < m_ColumnCount && NULL != pVal1) {
            const char* pVal2 = GetCell(ix, colIndex);
            if (NULL == pVal2 || strcmp(pVal1, pVal2) != 0) {
              find = false;
              break;
            }
          }
        }
        if (find) {
          rowIndex = ix;
          break;
        }
      }
    }
    return rowIndex;
  }
public:
  int Load(const char* file)
  {
    int ret = FALSE;
    Unload();
    if (file) {
      FILE* fp = fopen(file, "rb");
      if (NULL != fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        if (size > 0) {
          fseek(fp, 0, SEEK_SET);
          m_pBuffer = new char[size + 1];
          if (NULL != m_pBuffer) {
            size_t realSize = fread(m_pBuffer, 1, size, fp);
            if (realSize == (size_t)size) {
              m_pBuffer[size] = 0;
              int lineNum = ROW_NUM_DEFAULT;
              char* p0 = strchr(m_pBuffer, '\n');//第一行是字段类型
              if (p0) {
                p0 = strchr(p0 + 1, '\n');//第二行是字段说明
                if (p0) {
                  //统计前面几行的平均长度
                  int sum = 0;
                  int ct = 0;
                  for (; ct < 5; ++ct) {
                    char* p = strchr(p0 + 1, '\n');
                    if (p) {
                      sum += int(p - p0);
                      p0 = p;
                    } else {
                      break;
                    }
                  }
                  if (ct>0) {
                    lineNum = int(size*ct / sum);
                  }
                }
              }
              InitRowSpace(lineNum);
              ret = Parse();
              if (FALSE == ret)
                Unload();
            } else {
              Unload();
            }
          }
        }
        fclose(fp);
      }
    }
    return ret;
  }
  int Parse(void)
  {
    int ret = FALSE;
    if (NULL != m_pBuffer && NULL != m_pRows) {
      const char* c_Seps = "\t\n";
      char* p = m_pBuffer;
      for (m_ColumnCount = 1; NULL != p; ++m_ColumnCount) {
        p = strpbrk(p, c_Seps);
        if (NULL != p) {
          if (*p == '\n')
            break;
          ++p;
        }
      }
      p = m_pBuffer;
      if (NULL != p) {
        for (; p;) {
          if (m_RowCount >= m_RowSpace) {
            ResizeRowSpace();
          }
          if (m_RowCount < m_RowSpace) {
            char* pc = p;
            while (*pc == ' ')
              ++pc;
            if (*pc == ';' || *pc == '#') {
              p = strchr(pc, '\n');
              if (p)
                ++p;
            }
            ColumnType* pColumns = new ColumnType[m_ColumnCount];
            if (NULL != pColumns) {
              memset(pColumns, 0, sizeof(ColumnType)*m_ColumnCount);
              m_pRows[m_RowCount] = pColumns;
              ++m_RowCount;
              int colNum = 1;
              for (; p; ++colNum) {
                if (colNum <= m_ColumnCount) {
                  pColumns[colNum - 1] = p;
                  p = strpbrk(p, c_Seps);
                  if (NULL == p) {
                    break;
                  } else {
                    bool isEnd = false;
                    if (*p == '\n') {
                      isEnd = true;
                    }
                    *p = 0;
                    ++p;
                    if (isEnd) {
                      break;
                    }
                  }
                } else {
                  p = strchr(p, '\n');
                  if (p) {
                    *p = 0;
                    ++p;
                  }
                  break;
                }
              }
              for (int ix = 0; ix < m_ColumnCount; ++ix) {
                Trim(pColumns[ix]);
              }
            }
          } else {
            break;
          }
        }
        ret = TRUE;
      }
    }
    return ret;
  }
  void Unload(void)
  {
    if (NULL != m_pRows) {
      for (int ix = 0; ix < m_RowCount; ++ix) {
        delete[] m_pRows[ix];
      }
      delete[] m_pRows;
      m_pRows = NULL;
    }
    if (NULL != m_pBuffer) {
      delete[] m_pBuffer;
      m_pBuffer = NULL;
    }
    m_RowCount = 0;
    m_ColumnCount = 0;
    m_RowSpace = 0;
  }
  int	GetRowCount(void)const
  {
    return m_RowCount;
  }
  int	GetColumnCount(void)const
  {
    return m_ColumnCount;
  }
  char* GetCell(int rowIndex, int colIndex)const
  {
    char* p = NULL;
    if (NULL != m_pRows) {
      if (rowIndex >= 0 && rowIndex < m_RowCount) {
        RowType row = m_pRows[rowIndex];
        if (NULL != row) {
          if (colIndex >= 0 && colIndex < m_ColumnCount) {
            p = row[colIndex];
          }
        }
      }
    }
    return p;
  }
public:
  TxtTable(void) :m_pBuffer(NULL), m_pRows(NULL), m_RowCount(0), m_ColumnCount(0), m_RowSpace(0)
  {}
  ~TxtTable(void)
  {
    Unload();
  }
private:
  void			InitRowSpace(int size)
  {
    if (NULL == m_pRows) {
      RowType* pNew = new RowType[size];
      if (NULL != pNew) {
        memset(pNew, 0, sizeof(RowType)*size);
        m_pRows = pNew;
        m_RowSpace = size;
      }
    }
  }
  void			ResizeRowSpace(void)
  {
    if (NULL != m_pRows) {
      RowType* pNew = new RowType[m_RowSpace + ROW_NUM_DELTA];
      if (NULL != pNew) {
        memcpy(pNew, m_pRows, sizeof(RowType)*m_RowCount);
        memset(pNew + m_RowCount, 0, sizeof(RowType)*(m_RowSpace + ROW_NUM_DELTA - m_RowCount));
        delete[] m_pRows;//这里只释放行空间，列不释放，列由新行空间接手管理
        m_pRows = pNew;
        m_RowSpace += ROW_NUM_DELTA;
      }
    }
  }
private:
  char*			m_pBuffer;
  RowType*		m_pRows;
  int				m_RowCount;
  int				m_ColumnCount;
  int				m_RowSpace;
};

class TxtTableObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_LOAD = 0,
    CUSTOM_MEMBER_INDEX_UNLOAD,
    CUSTOM_MEMBER_INDEX_GETROWCOUNT,
    CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT,
    CUSTOM_MEMBER_INDEX_GETCELL,
    CUSTOM_MEMBER_INDEX_FINDROW,
    CUSTOM_MEMBER_INDEX_NUM,
  };
public:
  int Load(const char* file)
  {
    return m_TxtTable.Load(file);
  }
  void Unload(void)
  {
    m_TxtTable.Unload();
  }
public:
  TxtTableObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM)
  {}
  virtual ~TxtTableObj(void)
  {}
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "load") == 0)
      return CUSTOM_MEMBER_INDEX_LOAD;
    else if (strcmp(name, "unload") == 0)
      return CUSTOM_MEMBER_INDEX_UNLOAD;
    else if (strcmp(name, "getRowCount") == 0)
      return CUSTOM_MEMBER_INDEX_GETROWCOUNT;
    else if (strcmp(name, "getColumnCount") == 0)
      return CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT;
    else if (strcmp(name, "getCell") == 0)
      return CUSTOM_MEMBER_INDEX_GETCELL;
    else if (strcmp(name, "findRow") == 0)
      return CUSTOM_MEMBER_INDEX_FINDROW;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (NULL != pParams) {
      switch (index) {
      case CUSTOM_MEMBER_INDEX_LOAD:
      {
        if (1 == num && pParams[0].IsString() && pParams[0].GetString() && NULL != pRetValue) {
          int ret = m_TxtTable.Load(pParams[0].GetString());
          pRetValue->SetInt(ret);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_UNLOAD:
      {
        if (NULL != pRetValue) {
          m_TxtTable.Unload();
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETROWCOUNT:
      {
        if (NULL != pRetValue)
          pRetValue->SetInt(m_TxtTable.GetRowCount());
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT:
      {
        if (NULL != pRetValue)
          pRetValue->SetInt(m_TxtTable.GetColumnCount());
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETCELL:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          int colIndex = pParams[1].GetInt();
          char* p = m_TxtTable.GetCell(rowIndex, colIndex);
          if (p)
            pRetValue->SetWeakRefString(p);
          else
            pRetValue->SetWeakRefString("");
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_FINDROW:
      {
        if (num >= 1 && pParams[0].IsInt() && NULL != pRetValue) {
          const int c_MaxCondNum = FunctionScript::MAX_FUNCTION_PARAM_NUM / 2 - 1;
          TxtTable::ColumnInfo infos[c_MaxCondNum];

          int startRowIndex = pParams[0].GetInt();
          int condNum = (num - 1) / 2;
          if (condNum > c_MaxCondNum)
            condNum = c_MaxCondNum;
          for (int i = 0; i < condNum; ++i) {
            int paramIndex = 1 + i * 2;
            if (pParams[paramIndex].IsInt() && pParams[paramIndex + 1].IsString()) {
              infos[i].m_ColIndex = pParams[paramIndex].GetInt();
              infos[i].m_ColValue = pParams[paramIndex + 1].GetString();
            }
          }
          int r = m_TxtTable.Find(startRowIndex, infos, condNum);
          pRetValue->SetInt(r);
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  TxtTable	m_TxtTable;
};

class ConfigTableObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_LOAD = 0,
    CUSTOM_MEMBER_INDEX_UNLOAD,
    CUSTOM_MEMBER_INDEX_GETROWCOUNT,
    CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT,
    CUSTOM_MEMBER_INDEX_GETIP0,
    CUSTOM_MEMBER_INDEX_GETIP1,
    CUSTOM_MEMBER_INDEX_GETSERVERNAME,
    CUSTOM_MEMBER_INDEX_GETSERVERID,
    CUSTOM_MEMBER_INDEX_GETZONEID,
    CUSTOM_MEMBER_INDEX_GETBIGWORLDID,
    CUSTOM_MEMBER_INDEX_GETWORLDID,
    CUSTOM_MEMBER_INDEX_GETKEYVALUENUM,
    CUSTOM_MEMBER_INDEX_GETKEY,
    CUSTOM_MEMBER_INDEX_GETVALUE,
    CUSTOM_MEMBER_INDEX_GETVALUEBYKEY,
    CUSTOM_MEMBER_INDEX_FINDBYIP0,
    CUSTOM_MEMBER_INDEX_FINDBYIP1,
    CUSTOM_MEMBER_INDEX_FINDBYID,
    CUSTOM_MEMBER_INDEX_NUM,
  };
  enum
  {
    COL_INDEX_IP0 = 0,
    COL_INDEX_IP1,
    COL_INDEX_SERVERNAME,
    COL_INDEX_SERVERID,
    COL_INDEX_ZONEID,
    COL_INDEX_BIGWORLDID,
    COL_INDEX_WORLDID,
    COL_INDEX_NUM,
    MAX_KEY_VALUE_NUM = 32,
    MAX_COLUMN_NUM = MAX_KEY_VALUE_NUM * 2 + COL_INDEX_NUM,
  };
  struct KeyValue
  {
    char* m_Key;
    char* m_Value;

    KeyValue(void) :m_Key(""), m_Value("")
    {}
  };
  struct ConfigLine
  {
    char* m_IP0;
    char* m_IP1;
    char* m_ServerName;
    int			m_ServerID;
    int			m_ZoneID;
    int			m_BigworldID;
    int			m_WorldID;
    KeyValue	m_KeyValue[MAX_KEY_VALUE_NUM];
    int			m_KeyValueNum;

    ConfigLine(void) :m_IP0(""), m_IP1(""), m_ServerName(""), m_ServerID(-1), m_ZoneID(-1), m_WorldID(-1), m_KeyValueNum(0)
    {}
  };
public:
  int Load(const char* file)
  {
    Unload();
    int ret = m_TxtTable.Load(file);
    if (TRUE == ret) {
      m_RowCount = m_TxtTable.GetRowCount() - 2;
      m_ColumnCount = m_TxtTable.GetColumnCount();
      m_pLines = new ConfigLine[m_RowCount];
      m_ColumnCount = COL_INDEX_NUM + (m_ColumnCount - COL_INDEX_NUM) / 2 * 2;
      if (NULL != m_pLines && m_ColumnCount >= COL_INDEX_NUM && m_ColumnCount <= MAX_COLUMN_NUM) {
        for (int ix = 0; ix < m_RowCount; ++ix) {
          char* p = m_TxtTable.GetCell(ix + 2, COL_INDEX_IP0);
          if (p)
            m_pLines[ix].m_IP0 = p;
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_IP1);
          if (p)
            m_pLines[ix].m_IP1 = p;
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_SERVERNAME);
          if (p)
            m_pLines[ix].m_ServerName = p;
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_SERVERID);
          if (p)
            m_pLines[ix].m_ServerID = atoi(p);
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_ZONEID);
          if (p)
            m_pLines[ix].m_ZoneID = atoi(p);
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_BIGWORLDID);
          if (p)
            m_pLines[ix].m_BigworldID = atoi(p);
          p = m_TxtTable.GetCell(ix + 2, COL_INDEX_WORLDID);
          if (p)
            m_pLines[ix].m_WorldID = atoi(p);
          m_pLines[ix].m_KeyValueNum = (m_ColumnCount - COL_INDEX_NUM) / 2;
          for (int kvIndex = 0; kvIndex < m_pLines[ix].m_KeyValueNum; ++kvIndex) {
            p = m_TxtTable.GetCell(ix + 2, COL_INDEX_NUM + kvIndex * 2);
            if (p)
              m_pLines[ix].m_KeyValue[kvIndex].m_Key = p;
            p = m_TxtTable.GetCell(ix + 2, COL_INDEX_NUM + kvIndex * 2 + 1);
            if (p)
              m_pLines[ix].m_KeyValue[kvIndex].m_Value = p;
          }
        }
      }
    }
    return ret;
  }
  void Unload(void)
  {
    m_TxtTable.Unload();
    if (NULL != m_pLines) {
      delete[] m_pLines;
      m_pLines = NULL;
    }
    m_RowCount = 0;
    m_ColumnCount = 0;
  }
public:
  ConfigTableObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM), m_pLines(NULL), m_RowCount(0), m_ColumnCount(0)
  {}
  virtual ~ConfigTableObj(void)
  {
    Unload();
  }
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "load") == 0)
      return CUSTOM_MEMBER_INDEX_LOAD;
    else if (strcmp(name, "unload") == 0)
      return CUSTOM_MEMBER_INDEX_UNLOAD;
    else if (strcmp(name, "getRowCount") == 0)
      return CUSTOM_MEMBER_INDEX_GETROWCOUNT;
    else if (strcmp(name, "getColumnCount") == 0)
      return CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT;
    else if (strcmp(name, "getIP0") == 0)
      return CUSTOM_MEMBER_INDEX_GETIP0;
    else if (strcmp(name, "getIP1") == 0)
      return CUSTOM_MEMBER_INDEX_GETIP1;
    else if (strcmp(name, "getServerName") == 0)
      return CUSTOM_MEMBER_INDEX_GETSERVERNAME;
    else if (strcmp(name, "getServerID") == 0)
      return CUSTOM_MEMBER_INDEX_GETSERVERID;
    else if (strcmp(name, "getZoneID") == 0)
      return CUSTOM_MEMBER_INDEX_GETZONEID;
    else if (strcmp(name, "getBigworldID") == 0)
      return CUSTOM_MEMBER_INDEX_GETBIGWORLDID;
    else if (strcmp(name, "getWorldID") == 0)
      return CUSTOM_MEMBER_INDEX_GETWORLDID;
    else if (strcmp(name, "getKeyValueNum") == 0)
      return CUSTOM_MEMBER_INDEX_GETKEYVALUENUM;
    else if (strcmp(name, "getKey") == 0)
      return CUSTOM_MEMBER_INDEX_GETKEY;
    else if (strcmp(name, "getValue") == 0)
      return CUSTOM_MEMBER_INDEX_GETVALUE;
    else if (strcmp(name, "getValueByKey") == 0)
      return CUSTOM_MEMBER_INDEX_GETVALUEBYKEY;
    else if (strcmp(name, "findByIP0") == 0)
      return CUSTOM_MEMBER_INDEX_FINDBYIP0;
    else if (strcmp(name, "findByIP1") == 0)
      return CUSTOM_MEMBER_INDEX_FINDBYIP1;
    else if (strcmp(name, "findByID") == 0)
      return CUSTOM_MEMBER_INDEX_FINDBYID;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (NULL != pParams) {
      switch (index) {
      case CUSTOM_MEMBER_INDEX_LOAD:
      {
        if (1 == num && pParams[0].IsString() && pParams[0].GetString() && NULL != pRetValue) {
          int ret = Load(pParams[0].GetString());
          pRetValue->SetInt(ret);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_UNLOAD:
      {
        if (NULL != pRetValue) {
          Unload();
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETROWCOUNT:
      {
        if (NULL != pRetValue)
          pRetValue->SetInt(m_RowCount);
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETCOLUMNCOUNT:
      {
        if (NULL != pRetValue)
          pRetValue->SetInt(m_ColumnCount);
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETIP0:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            char* p = m_pLines[rowIndex].m_IP0;
            if (p)
              pRetValue->SetWeakRefString(p);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETIP1:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            char* p = m_pLines[rowIndex].m_IP1;
            if (p)
              pRetValue->SetWeakRefString(p);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETSERVERNAME:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            char* p = m_pLines[rowIndex].m_ServerName;
            if (p)
              pRetValue->SetWeakRefString(p);
            else
              pRetValue->SetWeakRefString("");
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETSERVERID:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int id = m_pLines[rowIndex].m_ServerID;
            pRetValue->SetInt(id);
          } else {
            pRetValue->SetInt(-1);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETZONEID:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int id = m_pLines[rowIndex].m_ZoneID;
            pRetValue->SetInt(id);
          } else {
            pRetValue->SetInt(-1);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETBIGWORLDID:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int id = m_pLines[rowIndex].m_BigworldID;
            pRetValue->SetInt(id);
          } else {
            pRetValue->SetInt(-1);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETWORLDID:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int id = m_pLines[rowIndex].m_WorldID;
            pRetValue->SetInt(id);
          } else {
            pRetValue->SetInt(-1);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETKEYVALUENUM:
      {
        if (1 == num && pParams[0].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int id = m_pLines[rowIndex].m_KeyValueNum;
            pRetValue->SetInt(id);
          } else {
            pRetValue->SetInt(-1);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETKEY:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          int kvIndex = pParams[1].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int num = m_pLines[rowIndex].m_KeyValueNum;
            if (kvIndex >= 0 && kvIndex < num) {
              char* p = m_pLines[rowIndex].m_KeyValue[kvIndex].m_Key;
              if (NULL != p)
                pRetValue->SetWeakRefString(p);
              else
                pRetValue->SetWeakRefString("");
            } else {
              pRetValue->SetWeakRefString("");
            }
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETVALUE:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsInt() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          int kvIndex = pParams[1].GetInt();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines) {
            int num = m_pLines[rowIndex].m_KeyValueNum;
            if (kvIndex >= 0 && kvIndex < num) {
              char* p = m_pLines[rowIndex].m_KeyValue[kvIndex].m_Value;
              if (NULL != p)
                pRetValue->SetWeakRefString(p);
              else
                pRetValue->SetWeakRefString("");
            } else {
              pRetValue->SetWeakRefString("");
            }
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETVALUEBYKEY:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && NULL != pRetValue) {
          int rowIndex = pParams[0].GetInt();
          const char* p = pParams[1].GetString();
          if (rowIndex >= 0 && rowIndex < m_RowCount && NULL != m_pLines && p) {
            int find = FALSE;
            int num = m_pLines[rowIndex].m_KeyValueNum;
            for (int ix = 0; ix < num; ++ix) {
              char* pKey = m_pLines[rowIndex].m_KeyValue[ix].m_Key;
              char* pValue = m_pLines[rowIndex].m_KeyValue[ix].m_Value;
              if (NULL != pKey && NULL != pValue && strcmp(pKey, p) == 0) {
                pRetValue->SetWeakRefString(pValue);
                find = TRUE;
                break;
              }
            }
            if (FALSE == find) {
              pRetValue->SetWeakRefString("");
            }
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_FINDBYIP0:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && NULL != pRetValue) {
          int startRowIndex = pParams[0].GetInt();
          const char* pIP = pParams[1].GetString();
          int r = FindByIP0(startRowIndex, pIP);
          pRetValue->SetInt(r);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_FINDBYIP1:
      {
        if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && NULL != pRetValue) {
          int startRowIndex = pParams[0].GetInt();
          const char* pIP = pParams[1].GetString();
          int r = FindByIP1(startRowIndex, pIP);
          pRetValue->SetInt(r);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_FINDBYID:
      {
        if (5 == num && pParams[0].IsInt() && pParams[1].IsInt() && pParams[2].IsInt() && pParams[3].IsInt() && pParams[4].IsInt() && NULL != pRetValue) {
          int startRowIndex = pParams[0].GetInt();
          int zoneID = pParams[1].GetInt();
          int bigworldID = pParams[2].GetInt();
          int worldID = pParams[3].GetInt();
          int serverID = pParams[4].GetInt();
          int r = FindByID(startRowIndex, zoneID, bigworldID, worldID, serverID);
          pRetValue->SetInt(r);
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  int FindByIP0(int startRowIndex, const char* pIP)const
  {
    int rowIndex = -1;
    if (NULL != m_pLines && NULL != pIP && startRowIndex >= 0) {
      for (int ix = startRowIndex; ix < m_RowCount; ++ix) {
        const ConfigLine& line = m_pLines[ix];
        if (strcmp(line.m_IP0, pIP) == 0) {
          rowIndex = ix;
          break;
        }
      }
    }
    return rowIndex;
  }
  int FindByIP1(int startRowIndex, const char* pIP)const
  {
    int rowIndex = -1;
    if (NULL != m_pLines && NULL != pIP && startRowIndex >= 0) {
      for (int ix = startRowIndex; ix < m_RowCount; ++ix) {
        const ConfigLine& line = m_pLines[ix];
        if (strcmp(line.m_IP1, pIP) == 0) {
          rowIndex = ix;
          break;
        }
      }
    }
    return rowIndex;
  }
  int FindByID(int startRowIndex, int zoneID, int bigworldID, int worldID, int serverID)const
  {
    int rowIndex = -1;
    if (NULL != m_pLines && startRowIndex >= 0) {
      for (int ix = startRowIndex; ix < m_RowCount; ++ix) {
        const ConfigLine& line = m_pLines[ix];
        if ((-1 == zoneID || line.m_ZoneID == zoneID) &&
          (-1 == bigworldID || line.m_BigworldID == bigworldID) &&
          (-1 == worldID || line.m_WorldID == worldID) &&
          (-1 == serverID || line.m_ServerID == serverID)) {
          rowIndex = ix;
          break;
        }
      }
    }
    return rowIndex;
  }
private:
  TxtTable	m_TxtTable;
  ConfigLine*	m_pLines;
  int			m_RowCount;
  int			m_ColumnCount;
};

class StringMapObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_INITMAP = 0,
    CUSTOM_MEMBER_INDEX_GETNUM,
    CUSTOM_MEMBER_INDEX_FIND,
    CUSTOM_MEMBER_INDEX_INSERT,
    CUSTOM_MEMBER_INDEX_ERASE,
    CUSTOM_MEMBER_INDEX_CLEARMAP,
    CUSTOM_MEMBER_INDEX_LISTMAP,
    CUSTOM_MEMBER_INDEX_NUM,
  };
  typedef StringKeyT<128> StringType;
  typedef HashtableT<StringType, char*, StringType> StringMap;
public:
  void InitMap(int maxNum)
  {
    if (m_StringMap.IsInited()) {
      ClearMap();
    }
    m_StringMap.InitTable(maxNum);
  }
  int Set(const char* key, char* val)
  {
    int ret = FALSE;
    if (0 != key && 0 != val) {
      size_t size = strlen(val);
      char* p = new char[size + 1];
      if (p) {
        memcpy(p, val, size);
        p[size] = 0;
        char* pOld = Get(key);
        if (pOld) {
          Erase(key);
        }
        ret = m_StringMap.Add(key, p);
      }
    }
    return ret;
  }
  char* Get(const char* key)const
  {
    return m_StringMap.Get(key);
  }
  void Erase(const char* key)
  {
    char* pOld = Get(key);
    if (pOld) {
      delete[] pOld;
    }
    m_StringMap.Remove(key);
  }
  int GetNum(void)const
  {
    return m_StringMap.GetNum();
  }
  void ClearMap(void)
  {
    StringMap::Iterator it = m_StringMap.First();
    for (; FALSE == it.IsNull(); ++it) {
      char* p = it->GetValue();
      if (p) {
        delete[] p;
      }
    }
    m_StringMap.CleanUp();
  }
  void ListMap(const char* file)const
  {
    FILE* fp = fopen(file, "ab");
    StringMap::Iterator it = m_StringMap.First();
    for (; FALSE == it.IsNull(); ++it) {
      const char* key = it->GetKey().GetString();
      char* p = it->GetValue();
      printf("%s -> %s\n", key, p);
      if (fp) {
        fprintf(fp, "%s -> %s\n", key, p);
      }
    }
    if (fp) {
      fclose(fp);
    }
  }
public:
  StringMapObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM)
  {}
  virtual ~StringMapObj(void)
  {
    ClearMap();
  }
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "initMap") == 0)
      return CUSTOM_MEMBER_INDEX_INITMAP;
    else if (strcmp(name, "getNum") == 0)
      return CUSTOM_MEMBER_INDEX_GETNUM;
    else if (strcmp(name, "find") == 0)
      return CUSTOM_MEMBER_INDEX_FIND;
    else if (strcmp(name, "insert") == 0)
      return CUSTOM_MEMBER_INDEX_INSERT;
    else if (strcmp(name, "erase") == 0)
      return CUSTOM_MEMBER_INDEX_ERASE;
    else if (strcmp(name, "clearMap") == 0)
      return CUSTOM_MEMBER_INDEX_CLEARMAP;
    else if (strcmp(name, "listMap") == 0)
      return CUSTOM_MEMBER_INDEX_LISTMAP;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (pParams && pRetValue) {
      switch (index) {
      case CUSTOM_MEMBER_INDEX_INITMAP:
      {
        if (1 == num && pParams[0].IsInt()) {
          int val = pParams[0].GetInt();
          InitMap(val);
          pRetValue->SetInt(1);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_GETNUM:
      {
        int val = GetNum();
        pRetValue->SetInt(val);
      }
        break;
      case CUSTOM_MEMBER_INDEX_FIND:
      {
        if (1 == num && pParams[0].IsString()) {
          const char* pKey = pParams[0].GetString();
          if (NULL != pKey) {
            char* p = Get(pKey);
            pRetValue->SetWeakRefString(p);
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_INSERT:
      {
        if (2 == num && pParams[0].IsString() && pParams[1].IsString()) {
          const char* pKey = pParams[0].GetString();
          char* pValue = pParams[1].GetString();
          if (NULL != pKey && NULL != pValue) {
            int r = Set(pKey, pValue);
            pRetValue->SetInt(r);
          } else {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ERASE:
      {
        if (1 == num && pParams[0].IsString()) {
          const char* pKey = pParams[0].GetString();
          if (NULL != pKey) {
            Erase(pKey);
            pRetValue->SetInt(1);
          } else {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_CLEARMAP:
      {
        ClearMap();
        pRetValue->SetInt(1);
      }
        break;
      case CUSTOM_MEMBER_INDEX_LISTMAP:
      {
        if (1 == num && pParams[0].IsString()) {
          const char* pFile = pParams[0].GetString();
          if (NULL != pFile) {
            ListMap(pFile);
            pRetValue->SetInt(1);
          } else {
            pRetValue->SetInt(0);
          }
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  StringMap m_StringMap;
};

class GetLogFileIdApi : public ExpressionApi
{
  enum
  {
    MAX_STRING_SIZE = 1024,
  };
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      char temp[MAX_STRING_SIZE + 1];
      if (TRUE == GetLogFileId(temp, MAX_STRING_SIZE + 1)) {
        pRetValue->AllocString(temp);
      } else {
        pRetValue->SetWeakRefString("");
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit GetLogFileIdApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class GetTimeStringApi : public ExpressionApi
{
  enum
  {
    MAX_STRING_SIZE = 1024,
  };
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      char temp[MAX_STRING_SIZE + 1];
      if (TRUE == GetTimeString(temp, MAX_STRING_SIZE + 1)) {
        pRetValue->AllocString(temp);
      } else {
        pRetValue->SetWeakRefString("");
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit GetTimeStringApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class ReadStringApi : public ExpressionApi
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
      if (2 == num && pParams[0].IsString() && pParams[1].IsInt()) {
        const char* file = pParams[0].GetString();
        int startRow = pParams[1].GetInt();
        if (file) {
          char line[MAX_STRING_SIZE + 1] = { 0 };
          if (TRUE == ReadString(file, startRow, line, MAX_STRING_SIZE + 1, ConstSizeT<MAX_STRING_SIZE>())) {
            pRetValue->AllocString(line);
          } else {
            pRetValue->SetWeakRefString("");
          }
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit ReadStringApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
private:
};

class WriteStringApi : public ExpressionApi
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
      if ((2 == num || 3 == num) && pParams[0].IsString() && pParams[1].IsString()) {
        const char* file = pParams[0].GetString();
        const char* pStr = pParams[1].GetString();
        const char* pEnd = "\n";
        if (3 == num && pParams[2].IsString())
          pEnd = pParams[2].GetString();
        if (file && pStr) {
          if (TRUE == WriteString(file, pStr, pEnd)) {
            pRetValue->SetInt(1);
          } else {
            pRetValue->SetInt(0);
          }
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit WriteStringApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class CreateIniReaderApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      IniReaderObj* pObj = new IniReaderObj(*m_Interpreter);
      if (pObj) {
        m_Interpreter->AddRuntimeComponent(pObj);
        pRetValue->SetExpression(pObj);

        if (1 == num && pParams[0].IsString()) {
          pObj->Load(pParams[0].GetString());
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit CreateIniReaderApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class CreateTxtTableApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      TxtTableObj* pObj = new TxtTableObj(*m_Interpreter);
      if (pObj) {
        m_Interpreter->AddRuntimeComponent(pObj);
        pRetValue->SetExpression(pObj);

        if (1 == num && pParams[0].IsString()) {
          pObj->Load(pParams[0].GetString());
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit CreateTxtTableApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class CreateConfigTableApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      ConfigTableObj* pObj = new ConfigTableObj(*m_Interpreter);
      if (pObj) {
        m_Interpreter->AddRuntimeComponent(pObj);
        pRetValue->SetExpression(pObj);

        if (1 == num && pParams[0].IsString()) {
          pObj->Load(pParams[0].GetString());
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit CreateConfigTableApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class MyXmlVisitor : public TiXmlVisitor
{
public:
  virtual bool VisitEnter(const TiXmlDocument& doc)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      Value params[] = { Value(doc.Value()) };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onBeginDocument"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 1, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool VisitExit(const TiXmlDocument& doc)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      Value params[] = { Value(doc.Value()) };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onEndDocument"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 1, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const char* name = element.Value();
      const TiXmlNode* pNode = element.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      const char* pText = element.GetText();
      Value text;
      if (pText)
        text.SetWeakRefConstString(pText);
      else
        text.SetWeakRefConstString("");
      {
        Value params[] = { Value(name), text, parent };
        Value retVal;
        m_pInterpreter->CallMember(*m_pScpObj, Value("onBeginElement"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 3, &retVal);
        if (retVal.IsInt())
          ret = retVal.GetInt();
      }
      const TiXmlAttribute* pAttr = firstAttribute;
      while (NULL != pAttr) {
        Value params[] = { Value(name), Value(pAttr->Name()), Value(pAttr->Value()), text, parent };
        Value retVal;
        m_pInterpreter->CallMember(*m_pScpObj, Value("onAttribute"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 5, &retVal);
        pAttr = pAttr->Next();
      }
    }
    return (TRUE == ret);
  }
  virtual bool VisitExit(const TiXmlElement& element)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const TiXmlNode* pNode = element.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      Value params[] = { Value(element.Value()), parent };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onEndElement"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 2, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool Visit(const TiXmlDeclaration& declaration)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const TiXmlNode* pNode = declaration.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      Value params[] = { Value(declaration.Value()), parent };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onDeclaration"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 2, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool Visit(const TiXmlText& text)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const TiXmlNode* pNode = text.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      Value params[] = { Value(text.Value()), parent };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onText"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 2, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool Visit(const TiXmlComment& comment)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const TiXmlNode* pNode = comment.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      Value params[] = { Value(comment.Value()), parent };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onComment"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 2, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
  virtual bool Visit(const TiXmlUnknown& unknown)
  {
    int ret = TRUE;
    if (0 != m_pInterpreter && 0 != m_pScpObj) {
      const TiXmlNode* pNode = unknown.Parent();
      Value parent;
      if (pNode)
        parent.SetWeakRefConstString(pNode->Value());
      else
        parent.SetWeakRefConstString("");
      Value params[] = { Value(unknown.Value()), parent };
      Value retVal;
      m_pInterpreter->CallMember(*m_pScpObj, Value("onUnknown"), FALSE, Function::PARAM_CLASS_PARENTHESIS, params, 2, &retVal);
      if (retVal.IsInt())
        ret = retVal.GetInt();
    }
    return (TRUE == ret);
  }
public:
  int Load(const char* file)
  {
    return (m_XmlDoc.LoadFile(file) ? TRUE : FALSE);
  }
  int Accept(FunctionScript::ExpressionApi* pScpObj)
  {
    m_pScpObj = pScpObj;
    return (m_XmlDoc.Accept(this) ? TRUE : FALSE);
  }
  int Accept(FunctionScript::ExpressionApi* pScpObj, const char** ppName, int num)
  {
    int ct = 0;
    const TiXmlElement* pElement = m_XmlDoc.RootElement();
    if (pElement) {
      m_pScpObj = pScpObj;
      ct = AcceptElements(pElement, ppName, num);
    }
    return ct;
  }
public:
  MyXmlVisitor(FunctionScript::Interpreter& interpreter) :m_pInterpreter(&interpreter), m_pScpObj(NULL)
  {}
  virtual ~MyXmlVisitor(void)
  {
    m_pScpObj = NULL;
  }
private:
  int AcceptElements(const TiXmlElement* pElement, const char** pNames, int num)
  {
    int count = 0;
    if (pElement) {
      if (num > 0) {
        if (pNames) {
          const char* pName = pNames[0];
          if (NULL == pName || 0 == pName[0] || strcmp(pName, "//") == 0) {
            if (2 <= num) {
              const char* pTarget = pNames[1];
              if (pTarget && pTarget[0]) {
                if (strcmp(pTarget, "*") == 0) {
                  for (const TiXmlElement* p = pElement->FirstChildElement(); p; p = p->NextSiblingElement()) {
                    count += AcceptElements(p, pNames + 2, num - 2);
                  }
                  for (const TiXmlElement* p = pElement->FirstChildElement(); p; p = p->NextSiblingElement()) {
                    count += AcceptElements(p, pNames, num);
                  }
                } else {
                  for (const TiXmlElement* p = pElement->FirstChildElement(); p; p = p->NextSiblingElement()) {
                    const char* pTmpName = p->Value();
                    if (pTmpName && strcmp(pTmpName, pTarget) == 0) {
                      count += AcceptElements(p, pNames + 2, num - 2);
                    } else {
                      count += AcceptElements(p, pNames, num);
                    }
                  }
                }
              } else {
                count += AcceptElements(pElement, pNames + 1, num - 1);
              }
            }
          } else if (strcmp(pName, "*") == 0) {
            for (const TiXmlElement* p = pElement->FirstChildElement(); p; p = p->NextSiblingElement()) {
              count += AcceptElements(p, pNames + 1, num - 1);
            }
          } else {
            for (const TiXmlElement* p = pElement->FirstChildElement(pName); p; p = p->NextSiblingElement(pName)) {
              count += AcceptElements(p, pNames + 1, num - 1);
            }
          }
        }
      } else {
        pElement->Accept(this);
        ++count;
      }
    }
    return count;
  }
private:
  TiXmlDocument m_XmlDoc;
  FunctionScript::Interpreter* m_pInterpreter;
  FunctionScript::ExpressionApi* m_pScpObj;
};

class MyXmlVisitorObj : public ObjectBase
{
  enum
  {
    CUSTOM_MEMBER_INDEX_LOAD = 0,
    CUSTOM_MEMBER_INDEX_ACCEPT,
    CUSTOM_MEMBER_INDEX_NUM,
  };
public:
  int Load(const char* file)
  {
    int ret = FALSE;
    if (file) {
      ret = m_Visitor.Load(file);
    }
    return ret;
  }
  int Accept(ExpressionApi* p)
  {
    int ret = FALSE;
    if (p) {
      ret = m_Visitor.Accept(p);
    }
    return ret;
  }
  int Accept(ExpressionApi* p, const char** pNames, int num)
  {
    int ct = 0;
    if (p) {
      ct = m_Visitor.Accept(p, pNames, num);
    }
    return ct;
  }
public:
  MyXmlVisitorObj(Interpreter& interpreter) :ObjectBase(interpreter, CUSTOM_MEMBER_INDEX_NUM), m_Visitor(interpreter)
  {}
  virtual ~MyXmlVisitorObj(void)
  {}
protected:
  virtual int	 GetCustomInnerMemberIndex(const char* name)const
  {
    if (NULL == name)
      return -1;
    else if (strcmp(name, "load") == 0)
      return CUSTOM_MEMBER_INDEX_LOAD;
    else if (strcmp(name, "accept") == 0)
      return CUSTOM_MEMBER_INDEX_ACCEPT;
    else
      return -1;
  }
  virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (pParams && pRetValue) {
      switch (index) {
      case CUSTOM_MEMBER_INDEX_LOAD:
      {
        if (1 == num && pParams[0].IsString()) {
          const char* file = pParams[0].GetString();
          int r = Load(file);
          pRetValue->SetInt(r);
        }
      }
        break;
      case CUSTOM_MEMBER_INDEX_ACCEPT:
      {
        if (1 <= num && pParams[0].IsExpression()) {
          ExpressionApi* p = pParams[0].GetExpression();
          if (1 == num) {
            int r = Accept(p);
            pRetValue->SetInt(r);
          } else {
            const char* pNames[MAX_FUNCTION_PARAM_NUM - 1];
            memset(pNames, 0, sizeof(pNames));
            for (int ix = 1; ix < num; ++ix) {
              if (pParams[ix].IsString())
                pNames[ix - 1] = pParams[ix].GetString();
              else
                pNames[ix - 1] = "*";
            }
            int ct = Accept(p, pNames, num - 1);
            pRetValue->SetInt(ct);
          }
        }
      }
        break;
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
private:
  MyXmlVisitor m_Visitor;
};

class CreateXmlVisitorApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (m_Interpreter && pParams && pRetValue) {
      ReplaceVariableWithValue(pParams, num);
      MyXmlVisitorObj* pObj = new MyXmlVisitorObj(*m_Interpreter);
      if (pObj) {
        m_Interpreter->AddRuntimeComponent(pObj);
        pRetValue->SetExpression(pObj);

        if (1 == num && pParams[0].IsString()) {
          pObj->Load(pParams[0].GetString());
        }
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit CreateXmlVisitorApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

#endif //__CommonScriptApi_H__