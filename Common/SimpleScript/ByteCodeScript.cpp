#include "ByteCodeScript.h"
#include "ByteCode.h"

//--------------------------------------------------------------------------------------
class ActionForByteCodeScript : public RuntimeBuilderT<ActionForByteCodeScript>
{
	static const int s_c_MaxStringSize = 4*1024;
	typedef RuntimeBuilderT<ActionForByteCodeScript> BaseType;
public:
	inline int setLastToken(const char* token)
	{
		int size=0;
		if(NULL!=mInterpreter && NULL!=token)
		{
			size=(int)strlen(token);
			if(size>0)
			{
				char* pStr=mInterpreter->AllocString(size);
				if(NULL!=pStr)
				{
					for(int i=0;i<size;++i)
					{
						pStr[i]=RuntimeBuilderData::Decode(token[i],__SERVERVERSION__);
					}
					pStr[size]=0;
				}
			}
		}
		return size;
	}
	inline char* getLastToken(void) const{return m_LastToken;}
	inline void setLastLineNumber(int number){m_LastNumber=number;}
	inline int getLastLineNumber(void) const{return m_LastNumber;}
	inline void setCanFinish(int val){}
	inline int pushToken(int type,const char* val)
	{
		int size=0;
		if(NULL!=mInterpreter && NULL!=val)
		{
			size=(int)strlen(val);
			if(size>0)
			{
				char* pStr=mInterpreter->AllocString(size);
				if(NULL!=pStr)
				{
					for(int i=0;i<size;++i)
					{
						pStr[i]=RuntimeBuilderData::Decode(val[i],__SERVERVERSION__);
					}
					pStr[size]=0;
					mData.push(RuntimeBuilderData::TokenInfo(pStr,type));
				}
			}
		}
		return size;
	}
	inline void pushToken(int type,int val)
	{
		mData.push(RuntimeBuilderData::TokenInfo(val,type));
	}
	inline void pushToken(float val)
	{
		mData.push(RuntimeBuilderData::TokenInfo(val));
	}
public:
	ActionForByteCodeScript(Interpreter& interpreter):BaseType(interpreter),m_LastToken(NULL),m_LastNumber(0)
	{
		setEnvironmentObjRef(*this);
	}
private:
	char*	m_LastToken;
	int		m_LastNumber;
};
//--------------------------------------------------------------------------------------

namespace FunctionScript
{
	void ByteCodeScript::Parse(const char* buf,int size)
	{
		ActionForByteCodeScript action(m_Interpreter);
		for(int ix=0;ix<size;)
		{
			unsigned char c=static_cast<unsigned char>(RuntimeBuilderData::Decode(buf[ix],__SERVERVERSION__));
			ix+=sizeof(c);
			switch(c)
			{
			case BYTE_CODE_SET_LAST_TOKEN:
				{
					const char* pStr=buf+ix;
					int size=action.setLastToken(pStr);
					ix+=size+1;
				}
				break;
			case BYTE_CODE_SET_LAST_LINE_NUMBER:
				{
					int lineNumber=*reinterpret_cast<const int*>(buf+ix);
					for(int byteIndex=0;byteIndex<sizeof(lineNumber);++byteIndex)
					{
						reinterpret_cast<char*>(&lineNumber)[byteIndex]=RuntimeBuilderData::Decode(reinterpret_cast<char*>(&lineNumber)[byteIndex],__SERVERVERSION__);
					}
					ix+=sizeof(lineNumber);
					action.setLastLineNumber(lineNumber);
				}
				break;
			case BYTE_CODE_PUSH_TOKEN:
				{
					unsigned char type=static_cast<unsigned char>(RuntimeBuilderData::Decode(*(buf+ix),__SERVERVERSION__));
					ix+=sizeof(type);
					switch(type)
					{
					case RuntimeBuilderData::STRING_TOKEN:
					case RuntimeBuilderData::VARIABLE_TOKEN:
						{
							const char* pStr=buf+ix;
							int size=action.pushToken(type,pStr);
							ix+=size+1;
						}
						break;
					case RuntimeBuilderData::INT_TOKEN:
						{
							int val=*reinterpret_cast<const int*>(buf+ix);
							for(int byteIndex=0;byteIndex<sizeof(val);++byteIndex)
							{
								reinterpret_cast<char*>(&val)[byteIndex]=RuntimeBuilderData::Decode(reinterpret_cast<char*>(&val)[byteIndex],__SERVERVERSION__);
							}
							ix+=sizeof(val);
							action.pushToken(type,val);
						}
						break;
					case RuntimeBuilderData::FLOAT_TOKEN:
						{
							float val=*reinterpret_cast<const float*>(buf+ix);
							for(int byteIndex=0;byteIndex<sizeof(val);++byteIndex)
							{
								reinterpret_cast<char*>(&val)[byteIndex]=RuntimeBuilderData::Decode(reinterpret_cast<char*>(&val)[byteIndex],__SERVERVERSION__);
							}
							ix+=sizeof(val);
							action.pushToken(val);
						}
						break;
					}					
				}
				break;
			case BYTE_CODE_MARK_PERIOD_PARENTHESIS_PARAM:
				{
					action.markPeriodParenthesisParam();
				}
				break;
			case BYTE_CODE_MARK_PERIOD_BRACKET_PARAM:
				{
					action.markPeriodBracketParam();
				}
				break;
			case BYTE_CODE_MARK_PERIOD_BRACE_PARAM:
				{
					action.markPeriodBraceParam();
				}
				break;
			case BYTE_CODE_SET_MEMBER_ID:
				{
					action.setMemberId();
				}
				break;
			case BYTE_CODE_MARK_PERIOD_PARAM:
				{
					action.markPeriodParam();
				}
				break;
			case BYTE_CODE_MARK_BRACKET_PARAM:
				{
					action.markBracketParam();
				}
				break;
			case BYTE_CODE_BUILD_HIGHORDER_FUNCTION:
				{
					action.buildHighOrderFunction();
				}
				break;
			case BYTE_CODE_MARK_PARENTHESIS_PARAM:
				{
					action.markParenthesisParam();
				}
				break;
			case BYTE_CODE_SET_EXTERN_SCRIPT:
				{
					action.setExternScript();
				}
				break;
			case BYTE_CODE_MARK_HAVE_STATEMENT:
				{
					action.markHaveStatement();
				}
				break;
			case BYTE_CODE_MARK_HAVE_EXTERN_SCRIPT:
				{
					action.markHaveExternScript();
				}
				break;
			case BYTE_CODE_SET_FUNCTION_ID:
				{
					action.setFunctionId();
				}
				break;
			case BYTE_CODE_BEGIN_FUNCTION:
				{
					action.beginFunction();
				}
				break;
			case BYTE_CODE_END_FUNCTION:
				{
					action.endFunction();
				}
				break;
			case BYTE_CODE_BEGIN_STATEMENT:
				{
					action.beginStatement();
				}
				break;
			case BYTE_CODE_END_STATEMENT:
				{
					action.endStatement();
				}
				break;
			case BYTE_CODE_BUILD_OPERATOR:
				{
					action.buildOperator();
				}
				break;
			case BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR:
				{
					action.buildFirstTernaryOperator();
				}
				break;
			case BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR:
				{
					action.buildSecondTernaryOperator();
				}
				break;
			};
		}

		m_Interpreter.PrepareRuntimeObject();
	}
}