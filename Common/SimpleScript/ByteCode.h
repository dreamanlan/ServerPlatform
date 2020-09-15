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
        BYTE_CODE_MARK_PERIOD_PARENTHESIS_PARAM,
        BYTE_CODE_MARK_PERIOD_BRACKET_PARAM,
        BYTE_CODE_MARK_PERIOD_BRACE_PARAM,
        BYTE_CODE_SET_MEMBER_ID,
        BYTE_CODE_MARK_PERIOD_PARAM,
        BYTE_CODE_MARK_BRACKET_PARAM,
        BYTE_CODE_BUILD_HIGHORDER_FUNCTION,
        BYTE_CODE_MARK_PARENTHESIS_PARAM,
        BYTE_CODE_SET_EXTERN_SCRIPT,
        BYTE_CODE_MARK_STATEMENT,
        BYTE_CODE_MARK_EXTERN_SCRIPT,
        BYTE_CODE_MARK_PARENTHESIS_ATTR_PARAM,
        BYTE_CODE_MARK_BRACKET_ATTR_PARAM,
        BYTE_CODE_ANGLE_BRACKET_ATTR_PARAM,
        BYTE_CODE_COLON_COLON_PARAM,
        BYTE_CODE_COLON_COLON_PARENTHESIS_PARAM,
        BYTE_CODE_COLON_COLON_BRACKET_PARAM,
        BYTE_CODE_COLON_COLON_BRACE_PARAM,
        BYTE_CODE_SET_FUNCTION_ID,
        BYTE_CODE_ADD_FUNCTION,
        BYTE_CODE_BEGIN_STATEMENT,
        BYTE_CODE_END_STATEMENT,
        BYTE_CODE_BUILD_OPERATOR,
        BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR,
        BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR,
        BYTE_CODE_MARK_QUESTION_PERIOD_PARAM,
        BYTE_CODE_MARK_QUESTION_PARENTHESIS_PARAM,
        BYTE_CODE_MARK_QUESTION_BRACKET_PARAM,
        BYTE_CODE_MARK_QUESTION_BRACE_PARAM,
        BYTE_CODE_MARK_POINTER_PARAM,
        BYTE_CODE_MARK_PERIOD_STAR_PARAM,
        BYTE_CODE_MARK_QUESTION_PERIOD_STAR_PARAM,
        BYTE_CODE_MARK_POINTER_STAR_PARAM,
        BYTE_CODE_NUM
    };

    template<typename RealTypeT>
    class RuntimeBuilderT
    {
    public:
        RuntimeBuilderT(Interpreter& interpreter) :mThis(NULL), mInterpreter(&interpreter)
        {
            DebugAssert(mInterpreter);
            mData.GetNullFunctionPtrRef() = mInterpreter->GetNullFunctionPtr();
        }
        inline void setEnvironmentObjRef(RealTypeT& thisObj)
        {
            mThis = &thisObj;
            DebugAssert(mThis);
        }
    public:
        inline void    endStatement(void);
        inline void    buildOperator(void);
        inline void    buildFirstTernaryOperator(void);
        inline void    buildSecondTernaryOperator(void);
        inline void    beginStatement(void);
        inline void    addFunction(void);
        inline void    setFunctionId(void);
        inline void    markParenthesisParam(void);
        inline void    buildHighOrderFunction(void);
        inline void    markBracketParam(void);
        inline void    markQuestionParenthesisParam(void);
        inline void    markQuestionBracketParam(void);
        inline void    markQuestionBraceParam(void);
        inline void    markStatement(void);
        inline void    markExternScript(void);
        inline void    markBracketAttrParam(void);
        inline void    markParenthesisAttrParam(void);
        inline void    markAngleBracketAttrParam(void);
        inline void    markColonColonParam(void);
        inline void    markColonColonParenthesisParam(void);
        inline void    markColonColonBracketParam(void);
        inline void    markColonColonBraceParam(void);
        inline void    setExternScript(void);
        inline void    markPeriodParam(void);
        inline void    setMemberId(void);
        inline void    markPeriodParenthesisParam(void);
        inline void    markPeriodBracketParam(void);
        inline void    markPeriodBraceParam(void);
        inline void    markQuestionPeriodParam(void);
        inline void    markPointerParam(void);
        inline void    markPeriodStarParam(void);
        inline void    markQuestionPeriodStarParam(void);
        inline void    markPointerStarParam(void);
    private:
        inline int wrapObjectMember(ISyntaxComponent& comp);
        inline int wrapObjectMemberInHighOrderFunction(FunctionData& arg);
        inline int wrapObjectMember(StatementData& arg);
        inline ISyntaxComponent& simplifyStatement(StatementData& data)const;
        inline ISyntaxComponent& simplifyStatement(FunctionData& data)const;
        inline bool	preconditionCheck(void)const
        {
            return NULL != mThis && NULL != mInterpreter;
        }
    protected:
        RuntimeBuilderData mData;
        Interpreter* mInterpreter;
        RealTypeT* mThis;
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
            DebugAssert(mThis);
        }
        inline const char* getByteCode(int& len)const
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
        inline void addFunction(void)
        {
            genCode(BYTE_CODE_ADD_FUNCTION);
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
        inline void markPeriod(void)
        {
            genCode(BYTE_CODE_MARK_PERIOD);
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
        inline void markOperator(void)
        {
            genCode(BYTE_CODE_MARK_OPERATOR);
        }
        inline void markQuestion(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION);
        }
        inline void markQuestionPeriodParam(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION_PERIOD_PARAM);
        }
        inline void markQuestionParenthesisParam(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION_PARENTHESIS_PARAM);
        }
        inline void markQuestionBracketParam(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION_BRACKET_PARAM);
        }
        inline void markQuestionBraceParam(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION_BRACE_PARAM);
        }
        inline void markPointer(void)
        {
            genCode(BYTE_CODE_MARK_POINTER);
        }
        inline void markPointerParam(void)
        {
            genCode(BYTE_CODE_MARK_POINTER_PARAM);
        }
        inline void markPeriodStarParam(void)
        {
            genCode(BYTE_CODE_MARK_PERIOD_STAR_PARAM);
        }
        inline void markQuestionPeriodStarParam(void)
        {
            genCode(BYTE_CODE_MARK_QUESTION_PERIOD_STAR_PARAM);
        }
        inline void markPointerStarParam(void)
        {
            genCode(BYTE_CODE_MARK_POINTER_STAR_PARAM);
        }
        inline void markStatement(void)
        {
            genCode(BYTE_CODE_MARK_STATEMENT);
        }
        inline void markExternScript(void)
        {
            genCode(BYTE_CODE_MARK_EXTERN_SCRIPT);
        }
        inline void markBracketAttrParam(void)
        {
            genCode(BYTE_CODE_MARK_BRACKET_ATTR_PARAM);
        }
        inline void markParenthesisAttrParam(void)
        {
            genCode(BYTE_CODE_MARK_PARENTHESIS_ATTR_PARAM);
        }
        inline void markAngleBracketAttrParam(void)
        {
            genCode(BYTE_CODE_ANGLE_BRACKET_ATTR_PARAM);
        }
        inline void markColonColonParam(void)
        {
            genCode(BYTE_CODE_COLON_COLON_PARAM);
        }
        inline void markColonColonParenthesisParam(void)
        {
            genCode(BYTE_CODE_COLON_COLON_PARENTHESIS_PARAM);
        }
        inline void markColonColonBracketParam(void)
        {
            genCode(BYTE_CODE_COLON_COLON_BRACKET_PARAM);
        }
        inline void markColonColonBraceParam(void)
        {
            genCode(BYTE_CODE_COLON_COLON_BRACE_PARAM);
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
            //mData.genByteCode(static_cast<unsigned char>(BYTE_CODE_SET_LAST_LINE_NUMBER));
            //mData.genByteCode(mThis->getLastLineNumber());
        }
        inline void genLastToken(void)
        {
            if (!preconditionCheck())return;
            mData.genByteCode(static_cast<unsigned char>(BYTE_CODE_SET_LAST_TOKEN));
            char* pStr = mThis->getLastToken();
            if (NULL != pStr) {
                mData.genByteCode(pStr);
            }
            else {
                mData.genByteCode("");
            }
        }
        inline void genPush(void)
        {
            if (!preconditionCheck())return;
            RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
            if (TRUE != tokenInfo.IsValid())return;
            mData.genByteCode(static_cast<unsigned char>(BYTE_CODE_PUSH_TOKEN));
            mData.genByteCode(static_cast<unsigned char>(tokenInfo.mType));
            if (RuntimeBuilderData::STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN == tokenInfo.mType)
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