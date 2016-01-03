// PacketTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ScriptThread.h"
#include "SourceCodeScript.h"

#if defined(__WINDOWS__)
#include <conio.h>
#elif defined(__LINUX__)
#include <curses.h>
#endif

#if defined(_DEBUG) && defined(__WINDOWS__)
#include <crtdbg.h>
#endif

#include <fstream>

class ConsoleScriptSource : public IScriptSource
{
public:
	void ClearBuffer(void)
	{
		memset(m_Line[m_CurLine],0,sizeof(m_Line[m_CurLine]));
	}
	void SetBuffer(const char* pBuf)
	{
		m_pSource=pBuf;
	}
	const char* GetBackBuffer(void)const
	{
		return m_Line[1-m_CurLine];
	}
	void ResetTime(void){m_Time=0;}
	unsigned int GetTime(void)const{return m_Time;}
public:
	ConsoleScriptSource(int& runFlag):m_pRunFlag(&runFlag),m_Time(0),m_pSource(NULL)
	{
		memset(m_Line,0,sizeof(m_Line));
		m_CurLine=1;
	}
protected:
	virtual int Load(void)
	{
		unsigned int t1=MyTimeGetTime();
		int ret=TRUE;
		if(m_pSource)
		{
			ClearBuffer();
			ret=FALSE;
		}
		else
		{
			char* p=m_Line[1-m_CurLine];
			printf(">");
			gets(p);
			if(strcmp(p,".")==0 || 
				strcmp(p,"quit")==0 || strcmp(p,"exit")==0 ||
				strcmp(p,"enabledebuginfo")==0 || 
				strcmp(p,"disabledebuginfo")==0)
			{
				ret=FALSE;
			}
			else
			{
				ClearBuffer();
				m_CurLine=1-m_CurLine;
				ret=TRUE;
			}
		}
		unsigned int t2=MyTimeGetTime();
		m_Time+=t2-t1;
		return ret;
	}
	virtual const char* GetBuffer(void)const
	{
		if(m_pSource)
		{
			const char* pRet=m_pSource;
			m_pSource=NULL;
			return pRet;
		}
		else
		{
			return m_Line[m_CurLine];
		}
	}
private:
	mutable const char* m_pSource;
private:
	char m_Line[2][ScriptThread::MAX_LINE_SIZE];
	int m_CurLine;
	int* m_pRunFlag;
private:
	unsigned int m_Time;
};

int DecryptMemoryWithCyclone(char* pInMemory,int size,char*& pOutMemory,int& outSize,int& haveOutMemory)
{
	int ret=FALSE;
	/*if(Cyclone::IsEncrptMemory(pInMemory,size))
	{		
		outSize = size + 32;
		pOutMemory = new CHAR[outSize];
		if(pOutMemory)
		{
			if(Cyclone::PlatformDecrypMemory("key", pInMemory, size, pOutMemory, outSize))
			{
				haveOutMemory=TRUE;
				ret=TRUE;
			}
			else
			{
				delete[] pOutMemory;
				pOutMemory=NULL;
				outSize=0;
				haveOutMemory=FALSE;
				ret=FALSE;
			}
		}
	}
	else*/
	{
		pOutMemory=NULL;
		outSize=0;
		haveOutMemory=FALSE;
		ret=TRUE;
	}
	return ret;
}

const int g_c_MaxScriptThread=1024;
const int g_c_MaxFileBuffer=1024*1024;

static char g_FileBuffer[g_c_MaxFileBuffer]="";
static ScriptThread* g_pScriptThread[g_c_MaxScriptThread]={0};
static int	g_ScriptThreadNum=0;

class RunFileApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			pRetValue->SetInt(0);
			if(1==num && pParams[0].IsString())
			{
				const char* pFile=pParams[0].GetString();
				if(pFile)
				{
					FILE* fp=fopen(pFile,"r");
					if(NULL!=fp)
					{
						int size=(int)fread(g_FileBuffer,1,g_c_MaxFileBuffer,fp);
						if(size>=0)
						{
							g_FileBuffer[size]=0;
							int outSize=0;
							char* pOutMemory=NULL;
							int haveOutMemory=FALSE;
							int isContinue=DecryptMemoryWithCyclone(g_FileBuffer,size,pOutMemory,outSize,haveOutMemory);
							if(TRUE==isContinue)
							{
								if(TRUE==haveOutMemory && NULL!=pOutMemory && outSize>0)
								{
									tsnprintf(g_FileBuffer,g_c_MaxFileBuffer,"%s",pOutMemory);
									size=outSize;
									g_FileBuffer[size]=0;
									delete[] pOutMemory;
								}
								if(m_pSource)
								{
									m_pSource->SetBuffer(g_FileBuffer);
									pRetValue->SetInt(1);
								}
							}
						}
						fclose(fp);
					}
					else
					{
						printf("Can't open %s !!!\n",pFile);
					}
				}
			}
			else if(2==num && pParams[0].IsInt() && pParams[1].IsString())
			{
				int ix=pParams[0].GetInt();
				const char* pFile=pParams[1].GetString();
				if(ix>=0 && ix<g_ScriptThreadNum && ix<g_c_MaxScriptThread && pFile)
				{
					if(g_pScriptThread[ix])
					{
						FILE* fp=fopen(pFile,"r");
						if(NULL!=fp)
						{
							int size=(int)fread(g_FileBuffer,1,g_c_MaxFileBuffer,fp);
							if(size>=0)
							{
								g_FileBuffer[size]=0;
								int outSize=0;
								char* pOutMemory=NULL;
								int haveOutMemory=FALSE;
								int isContinue=DecryptMemoryWithCyclone(g_FileBuffer,size,pOutMemory,outSize,haveOutMemory);
								if(TRUE==isContinue)
								{
									if(TRUE==haveOutMemory && NULL!=pOutMemory && outSize>0)
									{
										tsnprintf(g_FileBuffer,g_c_MaxFileBuffer,"%s",pOutMemory);
										size=outSize;
										g_FileBuffer[size]=0;
										delete[] pOutMemory;
									}
								}
								char temp[ScriptThread::MAX_LINE_SIZE+1];
								int pos=0;
								for(int cix=0;cix<=size;++cix)
								{
									char c=g_FileBuffer[cix];
									if(c && c!='\n')
									{
										temp[pos]=(c=='\r' ? 0 : c);
										++pos;
									}
									else
									{
										temp[pos]=0;
										if(temp[0]!=0)
										{
											while(FALSE==g_pScriptThread[ix]->PushLine(temp))
												MySleep(10);
										}
										pos=0;
									}
									MySleep(0);
								}
								pRetValue->SetInt(1);
							}
							fclose(fp);
						}
						else
						{
							printf("Can't open %s !!!\n",pFile);
						}
					}
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	RunFileApi(Interpreter& interpreter,ConsoleScriptSource& source):ExpressionApi(interpreter),m_pSource(&source){}
private:
	ConsoleScriptSource* m_pSource;
};

class RunScriptApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			pRetValue->SetInt(0);
			if(1==num && pParams[0].IsString())
			{
				const char* pScript=pParams[0].GetString();
				if(pScript)
				{
					tsnprintf(g_FileBuffer,g_c_MaxFileBuffer,"%s",pScript);
					if(m_pSource)
					{
						m_pSource->SetBuffer(g_FileBuffer);
						pRetValue->SetInt(1);
					}
				}
			}
			else if(2==num && pParams[0].IsInt() && pParams[1].IsString())
			{
				int ix=pParams[0].GetInt();
				const char* pScript=pParams[1].GetString();
				if(ix>=0 && ix<g_ScriptThreadNum && ix<g_c_MaxScriptThread && pScript)
				{
					if(g_pScriptThread[ix])
					{
						int r=g_pScriptThread[ix]->PushLine(pScript);
						pRetValue->SetInt(r);
					}
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	RunScriptApi(Interpreter& interpreter,ConsoleScriptSource& source):ExpressionApi(interpreter),m_pSource(&source){}
private:
	ConsoleScriptSource* m_pSource;
};

class QuitApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			if(m_pRunFlag)
			{
				*m_pRunFlag=FALSE;
				pRetValue->SetInt(1);
			}
			else
			{
				pRetValue->SetInt(0);
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	QuitApi(Interpreter& interpreter,int& runFlag):ExpressionApi(interpreter),m_pRunFlag(&runFlag){}
private:
	int*	m_pRunFlag;
};

class GetScriptThreadNumApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			pRetValue->SetInt(g_ScriptThreadNum);
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit GetScriptThreadNumApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

class StartScriptThreadApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			bool find=false;
			for(int ix=0;ix<g_ScriptThreadNum;++ix)
			{
				if(g_pScriptThread[ix] && g_pScriptThread[ix]->getStatus()==Thread::READY)
				{
					g_pScriptThread[ix]->start();
					pRetValue->SetInt(ix);
					find=true;
				}
			}
			if(!find)
			{
				if(g_ScriptThreadNum<g_c_MaxScriptThread)
				{
					g_pScriptThread[g_ScriptThreadNum]=new ScriptThread(g_ScriptThreadNum);
					if(g_pScriptThread[g_ScriptThreadNum])
					{
						g_pScriptThread[g_ScriptThreadNum]->start();
						pRetValue->SetInt(g_ScriptThreadNum);
						++g_ScriptThreadNum;
					}
					else
					{
						pRetValue->SetInt(-1);
					}
				}
				else
				{
					pRetValue->SetInt(-1);
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit StartScriptThreadApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

class MarkWaitingQuitApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			if(1==num && pParams[0].IsInt())
			{
				int ix=pParams[0].GetInt();
				if(ix>=0 && ix<g_ScriptThreadNum && ix<g_c_MaxScriptThread)
				{
					if(g_pScriptThread[ix])
					{
						g_pScriptThread[ix]->MarkWaitingQuit();
						pRetValue->SetInt(1);
					}
					else
					{
						pRetValue->SetInt(0);
					}
				}
				else
				{
					pRetValue->SetInt(0);
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit MarkWaitingQuitApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

class StopScriptThreadApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			if(1==num && pParams[0].IsInt())
			{
				int ix=pParams[0].GetInt();
				if(ix>=0 && ix<g_ScriptThreadNum && ix<g_c_MaxScriptThread)
				{
					if(g_pScriptThread[ix])
					{
						g_pScriptThread[ix]->stop();
						pRetValue->SetInt(1);
					}
					else
					{
						pRetValue->SetInt(0);
					}
				}
				else
				{
					pRetValue->SetInt(0);
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit StopScriptThreadApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

class IsScriptThreadRunningApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(pParams && pRetValue)
		{
			ReplaceVariableWithValue(pParams,num);
			if(1==num && pParams[0].IsInt())
			{
				int ix=pParams[0].GetInt();
				if(ix>=0 && ix<g_ScriptThreadNum && ix<g_c_MaxScriptThread)
				{
					if(g_pScriptThread[ix])
					{
						if(Thread::RUNNING==g_pScriptThread[ix]->getStatus())
						{
							pRetValue->SetInt(1);
						}
						else
						{
							pRetValue->SetInt(0);
						}
					}
					else
					{
						pRetValue->SetInt(0);
					}
				}
				else
				{
					pRetValue->SetInt(0);
				}
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit IsScriptThreadRunningApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

static SharedMessageQueue* g_pSharedMessageQueueT2M=NULL;
SharedStringMap* g_pMap=NULL;
int g_Argc = 0;
char** g_Argv = NULL;

void PushMessageMT2M(const char* pMsg);
void SendMessageMT2T(int index, const char* pMsg)
{
  if (pMsg){
    if (index >= 0 && index < g_ScriptThreadNum && index < g_c_MaxScriptThread){
      if (g_pScriptThread[index]){
        SharedMessageQueue::MessageType msg;
        tsnprintf(msg.m_Content, sizeof(msg.m_Content), "%s", pMsg);
        g_pScriptThread[index]->GetSharedMessageQueue().Push(msg);
      }
    } else {
      PushMessageMT2M(pMsg);
    }
  }
}

void PushMessageMT2M(const char* pMsg)
{
	if(pMsg && g_pSharedMessageQueueT2M)
	{
		SharedMessageQueue::MessageType msg;
		tsnprintf(msg.m_Content,sizeof(msg.m_Content),"%s",pMsg);
		g_pSharedMessageQueueT2M->Push(msg);
	}
}

void PopMessageMT2M(char* pBuf,int space)
{
	if(g_pSharedMessageQueueT2M)
	{
		SharedMessageQueue::MessageType msg=g_pSharedMessageQueueT2M->Pop();
		tsnprintf(pBuf,space,"%s",msg.m_Content);
	}
}

class SendMessageM2TApi : public ExpressionApi
{
public:
  virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
  {
    if (0 == pParams || 0 == m_Interpreter)
      return EXECUTE_RESULT_NORMAL;
    ReplaceVariableWithValue(pParams, num);
    if (2 == num && pParams[0].IsInt() && pParams[1].IsString() && 0 != pRetValue)
    {
      int index = pParams[0].GetInt();
      const char* pMsg = pParams[1].GetString();
      if (pMsg)
      {
        SendMessageMT2T(index, pMsg);
        pRetValue->SetInt(1);
      } else
      {
        pRetValue->SetInt(0);
      }
    }
    return EXECUTE_RESULT_NORMAL;
  }
public:
  explicit SendMessageM2TApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class PushMessageMT2MApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(0==pParams || 0==m_Interpreter)
			return EXECUTE_RESULT_NORMAL;
		ReplaceVariableWithValue(pParams,num);
		if(1==num && pParams[0].IsString() && 0!=pRetValue)
		{
			const char* pMsg=pParams[0].GetString();
			if(pMsg)
			{
				PushMessageMT2M(pMsg);
				pRetValue->SetInt(1);
			}
			else
			{
				pRetValue->SetInt(0);
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
  explicit PushMessageMT2MApi(Interpreter& interpreter) :ExpressionApi(interpreter){}
};

class PopMessageMT2MApi : public ExpressionApi
{
public:
	virtual ExecuteResultEnum Execute(int paramClass,Value* pParams,int num,Value* pRetValue)
	{
		if(0==pParams || 0==m_Interpreter)
			return EXECUTE_RESULT_NORMAL;
		ReplaceVariableWithValue(pParams,num);
		if(0!=pRetValue)
		{
      void PopMessageMT2M(char*, int);
			char buf[513];
      PopMessageMT2M(buf, 513);
			if(buf[0]!=0)
			{
				pRetValue->AllocString(buf);
			}
			else
			{
				pRetValue->SetWeakRefConstString("");
			}
		}
		return EXECUTE_RESULT_NORMAL;
	}
public:
	explicit PopMessageMT2MApi(Interpreter& interpreter):ExpressionApi(interpreter){}
};

#ifdef WIN32
int _tmain(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#if defined(_DEBUG) && defined(__WINDOWS__)
	int tmpDbgFlag;
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );    
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
#endif
#if defined(__LINUX__)
	signal(SIGPIPE,SIG_IGN);
#endif

	g_pSharedMessageQueueT2M=new SharedMessageQueue();
	g_pMap=new SharedStringMap();
	g_pMap->InitMap(1024);

	int runFlag=TRUE;
	int onlyOnce=FALSE;
	InterpreterOptions options;
	options.SetExpressionPoolSize(4*1024);
	options.SetMaxPredefinedValueNum(256);
	options.SetMaxFunctionDimensionNum(8);
	options.SetMaxInnerFunctionApiNum(256);
	options.SetMaxInnerStatementApiNum(32);
	options.SetMaxLocalNum(256);
	options.SetMaxProgramSize(4*1024);
	options.SetMaxStatementApiNum(4);
	options.SetMaxStatementNum(1024);
	options.SetStackValuePoolSize(4*1024);
	options.SetStringBufferSize(32*1024);
	options.SetValuePoolSize(4*1024);
	SourceCodeScript script(options);
	Interpreter& interpreter=script.GetInterpreter();
	interpreter.SetExternRunFlagRef(runFlag);	
	ConsoleScriptSource source(runFlag);
	
	char tempBuf[64];
	tsnprintf(tempBuf,64,"%d",argc-1);
	g_pMap->Set("argc",tempBuf);
	for(int argIx=1;argIx<argc;++argIx)
	{
		tsnprintf(tempBuf,64,"argv%d",argIx-1);
		g_pMap->Set(tempBuf,argv[argIx]);
	}
  g_Argc = argc-1;
  g_Argv = (argc>1 ? &argv[1] : NULL);

	if(argc>=2)
	{
		runFlag=FALSE;
		const char* pFile=argv[1];
		FILE* fp=fopen(pFile,"r");
		if(NULL!=fp)
		{
			int size=(int)fread(g_FileBuffer,1,g_c_MaxFileBuffer,fp);
			if(size>=0)
			{
				g_FileBuffer[size]=0;
				int outSize=0;
				char* pOutMemory=NULL;
				int haveOutMemory=FALSE;
				int isContinue=DecryptMemoryWithCyclone(g_FileBuffer,size,pOutMemory,outSize,haveOutMemory);
				if(TRUE==isContinue)
				{
					if(TRUE==haveOutMemory && NULL!=pOutMemory && outSize>0)
					{
						tsnprintf(g_FileBuffer,g_c_MaxFileBuffer,"%s",pOutMemory);
						size=outSize;
						g_FileBuffer[size]=0;
						delete[] pOutMemory;
					}
					source.SetBuffer(g_FileBuffer);
					runFlag=TRUE;
					onlyOnce=TRUE;
				}
			}
			fclose(fp);
		}
		else
		{
			printf("Can't open %s !!!\n",pFile);
		}
	}
	else
	{
		printf("[usage]:\n");
		printf("\tadmintool\n");
		printf("\tadmintool file.scp [arg1 arg2 ...]\n");
		printf("Start interaction script ...\n");
	}

	for(int ix=0;ix<g_c_MaxScriptThread;++ix)
	{
		g_pScriptThread[ix]=NULL;
	}
	g_ScriptThreadNum=0;

	SharedStringMapObj mapObj(interpreter);
	interpreter.RegisterPredefinedValue("stringMap",FunctionScript::Value(&mapObj));

	SleepApi sleepApi(interpreter);
	interpreter.RegisterPredefinedValue("sleep", FunctionScript::Value(&sleepApi));

	TestPrintfApi testPrintfApi(interpreter);		
	interpreter.RegisterPredefinedValue("sprintf",FunctionScript::Value(&testPrintfApi));
  ArgvApi argvApi(interpreter);
  interpreter.RegisterPredefinedValue("argv",FunctionScript::Value(&argvApi));
  interpreter.RegisterPredefinedValue("argc",FunctionScript::Value(g_Argc));

	WriteConsoleApi writeConsoleApi(interpreter);		
	interpreter.RegisterPredefinedValue("writeConsole",FunctionScript::Value(&writeConsoleApi));
	GetLogFileIdApi getLogFileIdApi(interpreter);
	interpreter.RegisterPredefinedValue("getLogFileId",FunctionScript::Value(&getLogFileIdApi));
	GetTimeStringApi getTimeStringApi(interpreter);
	interpreter.RegisterPredefinedValue("getTimeString",FunctionScript::Value(&getTimeStringApi));
	GetMillisecondsApi getMillisecondsApi(interpreter);
	interpreter.RegisterPredefinedValue("getMilliseconds",FunctionScript::Value(&getMillisecondsApi));
	ReadStringApi readStringApi(interpreter);
	interpreter.RegisterPredefinedValue("readString",FunctionScript::Value(&readStringApi));
	WriteStringApi writeStringApi(interpreter);
	interpreter.RegisterPredefinedValue("writeString",FunctionScript::Value(&writeStringApi));
	CreateIniReaderApi createIniReaderApi(interpreter);
	interpreter.RegisterPredefinedValue("createIniReader",FunctionScript::Value(&createIniReaderApi));
	CreateTxtTableApi createTxtTableApi(interpreter);
	interpreter.RegisterPredefinedValue("createTxtTable",FunctionScript::Value(&createTxtTableApi));
	CreateConfigTableApi createConfigTableApi(interpreter);
	interpreter.RegisterPredefinedValue("createConfigTable",FunctionScript::Value(&createConfigTableApi));
	CreateXmlVisitorApi createXmlVisitorApi(interpreter);
	interpreter.RegisterPredefinedValue("createXmlVisitor",FunctionScript::Value(&createXmlVisitorApi));

  SimpleClientObj clientObj(interpreter);
	RunFileApi runfileApi(interpreter,source);
	RunScriptApi runscriptApi(interpreter,source);
	QuitApi quitApi(interpreter,runFlag);
	GetScriptThreadNumApi getScriptThreadNumApi(interpreter);
	StartScriptThreadApi startScriptThreadApi(interpreter);
	MarkWaitingQuitApi markWaitingQuitApi(interpreter);
	StopScriptThreadApi stopScriptThreadApi(interpreter);
	IsScriptThreadRunningApi isScriptThreadRunningApi(interpreter);
  SendMessageM2TApi sendMessageApi(interpreter);
	PushMessageMT2MApi pushMessageApi(interpreter);
	PopMessageMT2MApi popMessageApi(interpreter);

  interpreter.RegisterPredefinedValue("client", FunctionScript::Value(&clientObj));
	interpreter.RegisterPredefinedValue("runfile",FunctionScript::Value(&runfileApi));
	interpreter.RegisterPredefinedValue("runscript",FunctionScript::Value(&runscriptApi));
	interpreter.RegisterPredefinedValue("quit",FunctionScript::Value(&quitApi));
	interpreter.RegisterPredefinedValue("getScriptThreadNum",FunctionScript::Value(&getScriptThreadNumApi));
	interpreter.RegisterPredefinedValue("startScriptThread",FunctionScript::Value(&startScriptThreadApi));
	interpreter.RegisterPredefinedValue("markWaitingQuit",FunctionScript::Value(&markWaitingQuitApi));
	interpreter.RegisterPredefinedValue("stopScriptThread",FunctionScript::Value(&stopScriptThreadApi));
  interpreter.RegisterPredefinedValue("isScriptThreadRunning", FunctionScript::Value(&isScriptThreadRunningApi));
  interpreter.RegisterPredefinedValue("sendMessage", FunctionScript::Value(&sendMessageApi));
	interpreter.RegisterPredefinedValue("pushMessage",FunctionScript::Value(&pushMessageApi));
	interpreter.RegisterPredefinedValue("popMessage",FunctionScript::Value(&popMessageApi));

  int lastValue=0;

	for(;TRUE==runFlag;)
	{
		source.ClearBuffer();
		source.ResetTime();
		unsigned int t1=MyTimeGetTime();
		script.Parse(source);
		t1+=source.GetTime();
		unsigned int t2=MyTimeGetTime();
		if(interpreter.HasError())
		{
			for(int ix=0;ix<interpreter.GetErrorNum();++ix)
			{
				printf("%s\n",interpreter.GetErrorInfo(ix));
			}
			interpreter.Reset();
			printf("Interpreter has already reset !\n");
		}
		else
		{
			if(interpreter.GetStatementNum()>=options.GetMaxProgramSize()-10 || 
				interpreter.GetValueNum()>=options.GetValuePoolSize()-10 || 
				interpreter.GetStackValueNum()!=options.GetStackValuePoolSize() && interpreter.GetStackValueNum()>=options.GetStackValuePoolSize()-10 || 
				interpreter.GetSyntaxComponentNum()>=options.GetExpressionPoolSize()-10 || 
				int(interpreter.GetUnusedStringPtrRef()-interpreter.GetStringBuffer())>=options.GetStringBufferSize()-1024)
			{
				printf("[Statistic] statements:%u value:%u stack:%u syntax:%u string:%u\n",interpreter.GetStatementNum(),interpreter.GetValueNum(),interpreter.GetStackValueNum(),interpreter.GetSyntaxComponentNum(),(unsigned int)(interpreter.GetUnusedStringPtrRef()-interpreter.GetStringBuffer()));
			}
			Value val;
			interpreter.Execute(&val);
			if(argc==1)
			{
				if(!val.IsInvalid())
				{
					if(val.IsString())
					{
						printf("%s\n",val.GetString());
					}
					else
					{
						char temp[MAX_NUMBER_STRING_SIZE];
						const char* p=val.ToString(temp,MAX_NUMBER_STRING_SIZE);
						if(p && p[0]!='\0')
						{
							printf("Command Result:%s\n",p);
						}
					}
				}
			}
      else
      {
        if (val.IsInt()){
          lastValue = val.GetInt();
        } else {
          lastValue = 0;
        }
      }
		}
		unsigned int t3=MyTimeGetTime();
		//printf("[Time] parse:%u interpret:%u\n",t2-t1,t3-t2);
		const char* line=source.GetBackBuffer();
		if(NULL!=line)
		{
			if(strcmp(line,".")==0)
			{
				interpreter.Reset();
				runFlag=TRUE;
				if(argc==1)
				{
					printf("Process command: reset\n");
				}
			}
			else if(strcmp(line,"quit")==0 || strcmp(line,"exit")==0)
			{
				runFlag=FALSE;
			}
			else if(strcmp(line,"enabledebuginfo")==0)
			{
				interpreter.EnableDebugInfo();
			}
			else if(strcmp(line,"disabledebuginfo")==0)
			{
				interpreter.DisableDebugInfo();
			}
		}
		MySleep(100);
		if(TRUE==onlyOnce)
			break;
	}

	for(int ix=0;ix<g_ScriptThreadNum;++ix)
	{
		if(g_pScriptThread[ix] && g_pScriptThread[ix]->getStatus()==Thread::RUNNING)
		{
			g_pScriptThread[ix]->stop();
		}
	}

	if(argc==1)
	{
		printf("Wait script thread exiting ...\n");
	}

	while(g_CreateThreadCount!=g_QuitThreadCount)
	{
		MySleep(100);
	}
	
	if(argc==1)
	{
		printf("Now quit ...\n");
	}

	for(int ix=0;ix<g_ScriptThreadNum;++ix)
	{
		if(g_pScriptThread[ix])
		{
			delete g_pScriptThread[ix];
			g_pScriptThread[ix]=NULL;
		}
	}
	if(g_pSharedMessageQueueT2M)
	{
		delete g_pSharedMessageQueueT2M;
		g_pSharedMessageQueueT2M=NULL;
	}
	return lastValue;
}

