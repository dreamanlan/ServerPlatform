#ifndef __ByteCode_H__
#define __ByteCode_H__

#include "calc.h"
#include "RuntimeBuilderData.h"

namespace FunctionScript
{
    enum SimpleScriptByteCodeEnum
    {
        BYTE_CODE_SET_LAST_TOKEN,
        BYTE_CODE_SET_LAST_LINE_NUMBER,
        BYTE_CODE_PUSH_TOKEN,
        BYTE_CODE_MARK_PERIOD_PARAM,
        BYTE_CODE_MARK_BRACKET_PARAM,
        BYTE_CODE_BUILD_HIGHORDER_FUNCTION,
        BYTE_CODE_MARK_PARENTHESIS_PARAM,
        BYTE_CODE_SET_EXTERN_SCRIPT,
        BYTE_CODE_MARK_STATEMENT,
        BYTE_CODE_MARK_EXTERN_SCRIPT,
        BYTE_CODE_MARK_PARENTHESIS_COLON_PARAM,
        BYTE_CODE_MARK_BRACKET_COLON_PARAM,
        BYTE_CODE_ANGLE_BRACKET_COLON_PARAM,
        BYTE_CODE_MARK_PARENTHESIS_PERCENT_PARAM,
        BYTE_CODE_MARK_BRACKET_PERCENT_PARAM,
        BYTE_CODE_MARK_BRACE_PERCENT_PARAM,
        BYTE_CODE_ANGLE_BRACKET_PERCENT_PARAM,
        BYTE_CODE_COLON_COLON_PARAM,
        BYTE_CODE_SET_FUNCTION_ID,
        BYTE_CODE_ADD_FUNCTION,
        BYTE_CODE_BEGIN_STATEMENT,
        BYTE_CODE_END_STATEMENT,
        BYTE_CODE_BUILD_OPERATOR,
        BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR,
        BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR,
        BYTE_CODE_BUILD_NULLABLE_OPERATOR,
        BYTE_CODE_MARK_POINTER_PARAM,
        BYTE_CODE_MARK_PERIOD_STAR_PARAM,
        BYTE_CODE_MARK_POINTER_STAR_PARAM,
        BYTE_CODE_MARK_SEPARATOR,
        BYTE_CODE_NUM
    };

    template<typename RealTypeT>
    class RuntimeBuilderT
    {
    public:
        RuntimeBuilderT(Interpreter& interpreter) :mThis(NULL), mInterpreter(&interpreter)
        {
            MyAssert(mInterpreter);
            mData.GetNullFunctionPtrRef() = mInterpreter->GetNullFunctionPtr();
        }
        inline void setEnvironmentObjRef(RealTypeT& thisObj)
        {
            mThis = &thisObj;
            MyAssert(mThis);
        }
    public:
        inline void    markSeparator();
        inline void    endStatement();
        inline void    buildOperator();
        inline void    buildFirstTernaryOperator();
        inline void    buildSecondTernaryOperator();
        inline void    beginStatement();
        inline void    addFunction();
        inline void    setFunctionId();
        inline void    buildNullableOperator();
        inline void    markParenthesisParam();
        inline void    buildHighOrderFunction();
        inline void    markBracketParam();
        inline void    markStatement();
        inline void    markExternScript();
        inline void    markBracketColonParam();
        inline void    markParenthesisColonParam();
        inline void    markAngleBracketColonParam();
        inline void    markBracePercentParam();
        inline void    markBracketPercentParam();
        inline void    markParenthesisPercentParam();
        inline void    markAngleBracketPercentParam();
        inline void    markColonColonParam();
        inline void    setExternScript();
        inline void    markPeriodParam();
        inline void    markPointerParam();
        inline void    markPeriodStarParam();
        inline void    markPointerStarParam();
    public:
        inline int     peekPairTypeStack()const { uint32_t tag = 0; return peekPairTypeStack(tag); }
        inline int     peekPairTypeStack(uint32_t& tag)const;
        inline int     getPairTypeStackSize()const;
        inline int     peekPairTypeStack(int ix)const { uint32_t tag = 0; return peekPairTypeStack(ix, tag); }
        inline int     peekPairTypeStack(int ix, uint32_t& tag)const;
	private:
        inline void    pushPairType(int type) { pushPairType(type, 0); };
        inline void    pushPairType(int type, uint32_t tag);
        inline void    popPairType();
    private:
        inline int wrapObjectMember(ISyntaxComponent& comp);
        inline int wrapObjectMemberInHighOrderFunction(FunctionData& arg);
        inline int wrapObjectMember(StatementData& arg);
        inline ISyntaxComponent& simplifyStatement(StatementData& data)const;
        inline ISyntaxComponent& simplifyStatement(FunctionData& data)const;
        inline bool	preconditionCheck()const
        {
            return NULL != mThis && NULL != mInterpreter;
        }
    protected:
        RuntimeBuilderData mData;
        ActionApi mApi;
        Interpreter* mInterpreter;
        RealTypeT* mThis;
    };

    template<typename RealTypeT>
    class GeneratorT
    {
    public:
        GeneratorT() :mThis(NULL)
        {}
        inline void    setEnvironmentObjRef(RealTypeT& thisObj)
        {
            mThis = &thisObj;
            MyAssert(mThis);
        }
        inline const char* getByteCode(int& len)const
        {
            return mData.getByteCode(len);
        }
    public:
        inline void setExternScript()
        {
            genLastToken();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_SET_EXTERN_SCRIPT);
        }
        inline void buildOperator()
        {
            genLastLineNumber();
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_OPERATOR);
        }
        inline void buildNullableOperator()
        {
            genLastLineNumber();
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_NULLABLE_OPERATOR);
        }
        inline void buildFirstTernaryOperator()
        {
            genLastLineNumber();
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR);
        }
        inline void buildSecondTernaryOperator()
        {
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR);
        }
        //--------------------------------------------------------------------------------------	
        inline void markSeparator()
        {
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_SEPARATOR);
        }
        inline void beginStatement()
        {
            genLastLineNumber();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BEGIN_STATEMENT);
        }
        inline void endStatement()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_END_STATEMENT);
        }
        inline void addFunction()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_ADD_FUNCTION);
        }
        inline void setFunctionId()
        {
            genPush();
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_SET_FUNCTION_ID);
        }
        inline void buildHighOrderFunction()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_HIGHORDER_FUNCTION);
        }
        inline void markParenthesisParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESIS_PARAM);
        }
        inline void markBracketParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_PARAM);
        }
        inline void markPeriodParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PERIOD_PARAM);
        }
        inline void markPointerParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_POINTER_PARAM);
        }
        inline void markPeriodStarParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PERIOD_STAR_PARAM);
        }
        inline void markPointerStarParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_POINTER_STAR_PARAM);
        }
        inline void markStatement()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_STATEMENT);
        }
        inline void markExternScript()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_EXTERN_SCRIPT);
        }
        inline void markBracketColonParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_COLON_PARAM);
        }
        inline void markParenthesisColonParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESIS_COLON_PARAM);
        }
        inline void markAngleBracketColonParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_ANGLE_BRACKET_COLON_PARAM);
        }
        inline void markBracePercentParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACE_PERCENT_PARAM);
        }
        inline void markBracketPercentParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_PERCENT_PARAM);
        }
        inline void markParenthesisPercentParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESIS_PERCENT_PARAM);
        }
        inline void markAngleBracketPercentParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_ANGLE_BRACKET_PERCENT_PARAM);
        }
        inline void markColonColonParam()
        {
            genCode(SimpleScriptByteCodeEnum::BYTE_CODE_COLON_COLON_PARAM);
        }
    private:
        inline bool preconditionCheck()const
        {
            return NULL != mThis;
        }
        inline void genLastLineNumber()
        {
            //This information will not be generated to save space.
            //if(!preconditionCheck())return;
            //mData.genByteCode(static_cast<unsigned char>(SimpleScriptByteCodeEnum::BYTE_CODE_SET_LAST_LINE_NUMBER));
            //mData.genByteCode(mThis->getLastLineNumber());
        }
        inline void genLastToken()
        {
            if (!preconditionCheck())return;
            mData.genByteCode(static_cast<unsigned char>(SimpleScriptByteCodeEnum::BYTE_CODE_SET_LAST_TOKEN));
            char* pStr = mThis->getLastToken();
            if (NULL != pStr) {
                mData.genByteCode(pStr);
            }
            else {
                mData.genByteCode("");
            }
        }
        inline void genPush()
        {
            if (!preconditionCheck())return;
            RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
            if (TRUE != tokenInfo.IsValid())return;
            mData.genByteCode(static_cast<unsigned char>(SimpleScriptByteCodeEnum::BYTE_CODE_PUSH_TOKEN));
            mData.genByteCode(static_cast<unsigned char>(tokenInfo.mType));
            if (RuntimeBuilderData::STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::DOLLAR_STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN == tokenInfo.mType)
                mData.genByteCode(tokenInfo.mString);
            else if (RuntimeBuilderData::FLOAT_TOKEN == tokenInfo.mType)
                mData.genByteCode(tokenInfo.mFloat);
            else if (RuntimeBuilderData::BOOL_TOKEN == tokenInfo.mType)
                mData.genByteCode(tokenInfo.mBool);
            else
                mData.genByteCode(tokenInfo.mInteger);
        }
        inline void genCode(int code)
        {
            if (!preconditionCheck())return;
            mData.genByteCode(static_cast<unsigned char>(code));
        }
    protected:
        RuntimeBuilderData mData;
        RealTypeT* mThis;
    };
}

#include "ByteCode.inl"

#endif //__ByteCode_H__