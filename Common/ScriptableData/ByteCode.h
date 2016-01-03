#ifndef __ByteCode_H__
#define __ByteCode_H__

#include "ScriptableData.h"
#include "RuntimeBuilderData.h"

namespace ScriptableData
{
  enum SimpleScriptByteCodeEnum
  {
    BYTE_CODE_SET_LAST_TOKEN,
    BYTE_CODE_SET_LAST_LINE_NUMBER,
    BYTE_CODE_PUSH_TOKEN,
    BYTE_CODE_MARK_PERIOD_PARENTHESIS_PARAM,
    BYTE_CODE_MARK_PERIOD_BRACKET_PARAM,
    BYTE_CODE_MARK_PERIOD_BRACE_PARAM,
    BYTE_CODE_SET_MEMBER_ID,
    BYTE_CODE_MARK_PERIOD_PARAM,
    BYTE_CODE_MARK_BRACKET_PARAM,
    BYTE_CODE_BUILD_HIGHORDER_FUNCTION,
    BYTE_CODE_MARK_PARENTHESIS_PARAM,
    BYTE_CODE_SET_EXTERN_SCRIPT,
    BYTE_CODE_MARK_HAVE_STATEMENT,
    BYTE_CODE_MARK_HAVE_EXTERN_SCRIPT,
    BYTE_CODE_SET_FUNCTION_ID,
    BYTE_CODE_BEGIN_FUNCTION,
    BYTE_CODE_END_FUNCTION,
    BYTE_CODE_BEGIN_STATEMENT,
    BYTE_CODE_END_STATEMENT,
    BYTE_CODE_BUILD_OPERATOR,
    BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR,
    BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR,
    BYTE_CODE_NUM
  };

  template<typename RealTypeT>
  class RuntimeBuilderT
  {
  public:
    RuntimeBuilderT(ScriptableDataFile& dataFile) :mThis(NULL), mDataFile(&dataFile)
    {
      CrashAssert(mDataFile);
    }
    inline void    setEnvironmentObjRef(RealTypeT& thisObj)
    {
      mThis = &thisObj;
      CrashAssert(mThis);
    }
  public:
    inline void    beginStatement(void);
    inline void    endStatement(void);
    inline void    buildOperator(void);
    inline void    buildFirstTernaryOperator(void);
    inline void    buildSecondTernaryOperator(void);
    inline void    beginFunction(void);
    inline void    endFunction(void);
    inline void    setFunctionId(void);
    inline void    markHaveStatement(void);
    inline void    markHaveExternScript(void);
    inline void    setExternScript(void);
    inline void    buildHighOrderFunction(void);
    inline void    markParenthesisParam(void);
    inline void    markBracketParam(void);
    inline void    markPeriodParam(void);
    inline void    setMemberId(void);
    inline void    markPeriodParenthesisParam(void);
    inline void    markPeriodBracketParam(void);
    inline void    markPeriodBraceParam(void);
  private:
    inline ISyntaxComponent& simplifyStatement(Statement& data)const;
    inline ISyntaxComponent& simplifyStatement(Function& data)const;
    inline ISyntaxComponent& simplifyStatement(Call& data)const;
    inline void simplifyStatementData(Statement& data)const;
    inline void simplifyFunctionData(Function& data)const;
    inline void simplifyCallData(Call& data)const;
    inline bool	preconditionCheck(void)const
    {
      return NULL != mThis && NULL != mDataFile;
    }
  protected:
    RuntimeBuilderData		mData;
    ScriptableDataFile*		mDataFile;
    RealTypeT*						mThis;
  };

  template<typename RealTypeT>
  class GeneratorT
  {
  public:
    GeneratorT(void) :mThis(NULL)
    {}
    inline void    setEnvironmentObjRef(RealTypeT& thisObj)
    {
      mThis = &thisObj;
      CrashAssert(mThis);
    }
    inline const CHAR* getByteCode(INT& len)const
    {
      return mData.getByteCode(len);
    }
  public:
    inline void setExternScript(void)
    {
      genLastToken();
      genCode(BYTE_CODE_SET_EXTERN_SCRIPT);
    }
    inline void buildOperator(void)
    {
      genLastLineNumber();
      genPush();
      genCode(BYTE_CODE_BUILD_OPERATOR);
    }
    inline void buildFirstTernaryOperator(void)
    {
      genLastLineNumber();
      genPush();
      genCode(BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR);
    }
    inline void buildSecondTernaryOperator(void)
    {
      genPush();
      genCode(BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR);
    }
    //--------------------------------------------------------------------------------------	
    inline void beginStatement(void)
    {
      genLastLineNumber();
      genCode(BYTE_CODE_BEGIN_STATEMENT);
    }
    inline void endStatement(void)
    {
      genCode(BYTE_CODE_END_STATEMENT);
    }
    inline void beginFunction(void)
    {
      genCode(BYTE_CODE_BEGIN_FUNCTION);
    }
    inline void setFunctionId(void)
    {
      genPush();
      genCode(BYTE_CODE_SET_FUNCTION_ID);
    }
    inline void setMemberId(void)
    {
      genPush();
      genCode(BYTE_CODE_SET_MEMBER_ID);
    }
    inline void endFunction(void)
    {
      genCode(BYTE_CODE_END_FUNCTION);
    }
    inline void buildHighOrderFunction(void)
    {
      genCode(BYTE_CODE_BUILD_HIGHORDER_FUNCTION);
    }
    inline void markParenthesisParam(void)
    {
      genCode(BYTE_CODE_MARK_PARENTHESIS_PARAM);
    }
    inline void markBracketParam(void)
    {
      genCode(BYTE_CODE_MARK_BRACKET_PARAM);
    }
    inline void markPeriodParam(void)
    {
      genCode(BYTE_CODE_MARK_PERIOD_PARAM);
    }
    inline void markPeriodParenthesisParam(void)
    {
      genCode(BYTE_CODE_MARK_PERIOD_PARENTHESIS_PARAM);
    }
    inline void markPeriodBracketParam(void)
    {
      genCode(BYTE_CODE_MARK_PERIOD_BRACKET_PARAM);
    }
    inline void markPeriodBraceParam(void)
    {
      genCode(BYTE_CODE_MARK_PERIOD_BRACE_PARAM);
    }
    inline void markHaveStatement(void)
    {
      genCode(BYTE_CODE_MARK_HAVE_STATEMENT);
    }
    inline void markHaveExternScript(void)
    {
      genCode(BYTE_CODE_MARK_HAVE_EXTERN_SCRIPT);
    }
  private:
    inline bool preconditionCheck(void)const
    {
      return NULL != mThis;
    }
    inline void genLastLineNumber(void)
    {
      //这信息就不生成了，省点空间
      //if(!preconditionCheck())return;
      //mData.genByteCode(static_cast<UCHAR>(BYTE_CODE_SET_LAST_LINE_NUMBER));
      //mData.genByteCode(mThis->getLastLineNumber());
    }
    inline void genLastToken(void)
    {
      if (!preconditionCheck())return;
      mData.genByteCode(static_cast<UCHAR>(BYTE_CODE_SET_LAST_TOKEN));
      CHAR* pStr = mThis->getLastToken();
      if (NULL != pStr) {
        mData.genByteCode(pStr);
      } else {
        mData.genByteCode("");
      }
    }
    inline void genPush(void)
    {
      if (!preconditionCheck())return;
      RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
      if (TRUE != tokenInfo.IsValid())return;
      mData.genByteCode(static_cast<UCHAR>(BYTE_CODE_PUSH_TOKEN));
      mData.genByteCode(static_cast<UCHAR>(tokenInfo.mType));
      mData.genByteCode(tokenInfo.mString);
    }
    inline void genCode(INT code)
    {
      if (!preconditionCheck())return;
      mData.genByteCode(static_cast<UCHAR>(code));
    }
  protected:
    RuntimeBuilderData				mData;
    RealTypeT*						mThis;
  };
}

#include "ByteCode.inl"

#endif //__ByteCode_H__