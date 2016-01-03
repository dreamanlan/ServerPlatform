#include "ByteCodeScript.h"
#include "ByteCode.h"

//--------------------------------------------------------------------------------------
class ActionForByteCodeScript : public RuntimeBuilderT < ActionForByteCodeScript >
{
  static const INT s_c_MaxStringSize = 4 * 1024;
  typedef RuntimeBuilderT<ActionForByteCodeScript> BaseType;
public:
  inline INT setLastToken(const CHAR* token)
  {
    INT size = 0;
    if (NULL != mDataFile && NULL != token) {
      size = (INT)strlen(token);
      if (size > 0) {
        CHAR* pStr = mDataFile->AllocString(size);
        if (NULL != pStr) {
          for (INT i = 0; i < size; ++i) {
            pStr[i] = RuntimeBuilderData::Decode(token[i], __SERVERVERSION__);
          }
          pStr[size] = 0;
        }
      }
    }
    return size;
  }
  inline CHAR* getLastToken(void) const{ return m_LastToken; }
  inline void setLastLineNumber(INT number){ m_LastNumber = number; }
  inline INT getLastLineNumber(void) const{ return m_LastNumber; }
  inline void setCanFinish(BOOL val){}
  inline INT pushToken(INT type, const CHAR* val)
  {
    INT size = 0;
    if (NULL != mDataFile && NULL != val) {
      size = (INT)strlen(val);
      if (size > 0) {
        CHAR* pStr = mDataFile->AllocString(size);
        if (NULL != pStr) {
          for (INT i = 0; i < size; ++i) {
            pStr[i] = RuntimeBuilderData::Decode(val[i], __SERVERVERSION__);
          }
          pStr[size] = 0;
          mData.push(RuntimeBuilderData::TokenInfo(pStr, type));
        }
      }
    }
    return size;
  }
public:
  ActionForByteCodeScript(ScriptableDataFile& dataFile) :BaseType(dataFile), m_LastToken(NULL), m_LastNumber(0)
  {
    setEnvironmentObjRef(*this);
  }
private:
  CHAR*	m_LastToken;
  INT		m_LastNumber;
};
//--------------------------------------------------------------------------------------

namespace ScriptableData
{
  void ParseBinary(const CHAR* buf, INT size, ScriptableDataFile& file)
  {
    ActionForByteCodeScript action(file);
    for (INT ix = 0; ix < size;) {
      UCHAR c = static_cast<UCHAR>(RuntimeBuilderData::Decode(buf[ix], __SERVERVERSION__));
      ix += sizeof(c);
      switch (c) {
      case BYTE_CODE_SET_LAST_TOKEN:
      {
        const CHAR* pStr = buf + ix;
        INT size = action.setLastToken(pStr);
        ix += size + 1;
      }
        break;
      case BYTE_CODE_SET_LAST_LINE_NUMBER:
      {
        INT lineNumber = *reinterpret_cast<const INT*>(buf + ix);
        for (INT byteIndex = 0; byteIndex < sizeof(lineNumber); ++byteIndex) {
          reinterpret_cast<CHAR*>(&lineNumber)[byteIndex] = RuntimeBuilderData::Decode(reinterpret_cast<CHAR*>(&lineNumber)[byteIndex], __SERVERVERSION__);
        }
        ix += sizeof(lineNumber);
        action.setLastLineNumber(lineNumber);
      }
        break;
      case BYTE_CODE_PUSH_TOKEN:
      {
        UCHAR type = static_cast<UCHAR>(RuntimeBuilderData::Decode(*(buf + ix), __SERVERVERSION__));
        ix += sizeof(type);
        const CHAR* pStr = buf + ix;
        INT size = action.pushToken(type, pStr);
        ix += size + 1;
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
  }
}