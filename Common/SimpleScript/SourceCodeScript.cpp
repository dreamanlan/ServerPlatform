#include "SourceCodeScript.h"
#include "SlkInc.h"
#include "SlkParse.h"
#include "ByteCode.h"

#define MAX_ACTION_NUM	37

//--------------------------------------------------------------------------------------
class ActionForSourceCodeScript : public SlkAction, public RuntimeBuilderT<ActionForSourceCodeScript>
{
    typedef RuntimeBuilderT<ActionForSourceCodeScript> BaseType;
public:
    inline char* getLastToken(void) const;
    inline int getLastLineNumber(void) const;
    inline void setCanFinish(int val);
    inline void setStringDelimiter(const char* begin, const char* end);
    inline void setScriptDelimiter(const char* begin, const char* end);
public:
    ActionForSourceCodeScript(SlkToken &scanner, FunctionScript::Interpreter& interpreter);
public:
    inline void    pushId(void);
    inline void    pushStr(void);
    inline void    pushNum(void);
    inline void	   pushTrue(void);
    inline void	   pushFalse(void);
    void    (ActionForSourceCodeScript::*Action[MAX_ACTION_NUM]) (void);
    inline void    initialize_table(void);
    inline void	execute(int  number) { (this->*Action[number]) (); }
private:
    SlkToken   *mScanner;
};
//--------------------------------------------------------------------------------------
inline char* ActionForSourceCodeScript::getLastToken(void) const
{
    if (NULL != mScanner) {
        return mScanner->getLastToken();
    }
    else {
        return NULL;
    }
}
inline int ActionForSourceCodeScript::getLastLineNumber(void) const
{
    if (NULL != mScanner) {
        return mScanner->getLastLineNumber();
    }
    else {
        return -1;
    }
}
inline void ActionForSourceCodeScript::setCanFinish(int val)
{
    if (NULL != mScanner) {
        mScanner->setCanFinish(val);
    }
}
inline void ActionForSourceCodeScript::setStringDelimiter(const char* begin, const char* end)
{
    if (NULL != mScanner) {
        mScanner->setStringDelimiter(begin, end);
    }
}
inline void ActionForSourceCodeScript::setScriptDelimiter(const char* begin, const char* end)
{
    if (NULL != mScanner) {
        mScanner->setScriptDelimiter(begin, end);
    }
}
//--------------------------------------------------------------------------------------
//标识符
inline void ActionForSourceCodeScript::pushId(void)
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
    }
}
inline void ActionForSourceCodeScript::pushNum(void)
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        if (strchr(lastToken, '.') == 0) {
            int val = 0;
            if (lastToken[0] == '0' && lastToken[1] == 'x') {
                sscanf(lastToken + 2, "%x", &val);
            }
            else {
                val = atoi(lastToken);
            }
            mData.push(RuntimeBuilderData::TokenInfo(val, RuntimeBuilderData::INT_TOKEN));
        }
        else {
            float val = static_cast<float>(atof(lastToken));
            mData.push(RuntimeBuilderData::TokenInfo(val));
        }
    }
}
inline void ActionForSourceCodeScript::pushStr(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForSourceCodeScript::pushTrue(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(true));
}
inline void ActionForSourceCodeScript::pushFalse(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(false));
}
//--------------------------------------------------------------------------------------
inline ActionForSourceCodeScript::ActionForSourceCodeScript(SlkToken &scanner, FunctionScript::Interpreter& interpreter) :mScanner(&scanner), BaseType(interpreter)
{
    initialize_table();
    setEnvironmentObjRef(*this);
}
//--------------------------------------------------------------------------------------
inline void ActionForSourceCodeScript::initialize_table(void)
{
    Action[0] = 0;
    Action[1] = &ActionForSourceCodeScript::endStatement;
    Action[2] = &ActionForSourceCodeScript::markOperator;
    Action[3] = &ActionForSourceCodeScript::pushId;
    Action[4] = &ActionForSourceCodeScript::buildOperator;
    Action[5] = &ActionForSourceCodeScript::buildFirstTernaryOperator;
    Action[6] = &ActionForSourceCodeScript::buildSecondTernaryOperator;
    Action[7] = &ActionForSourceCodeScript::beginStatement;
    Action[8] = &ActionForSourceCodeScript::beginFunction;
    Action[9] = &ActionForSourceCodeScript::endFunction;
    Action[10] = &ActionForSourceCodeScript::setFunctionId;
    Action[11] = &ActionForSourceCodeScript::markHaveStatement;
    Action[12] = &ActionForSourceCodeScript::markHaveExternScript;
    Action[13] = &ActionForSourceCodeScript::setExternScript;
    Action[14] = &ActionForSourceCodeScript::markParenthesisParam;
    Action[15] = &ActionForSourceCodeScript::buildHighOrderFunction;
    Action[16] = &ActionForSourceCodeScript::markBracketParam;
    Action[17] = &ActionForSourceCodeScript::markPeriod;
    Action[18] = &ActionForSourceCodeScript::markQuestion;
    Action[19] = &ActionForSourceCodeScript::markQuestionParenthesisParam;
    Action[20] = &ActionForSourceCodeScript::markQuestionBracketParam;
    Action[21] = &ActionForSourceCodeScript::markQuestionBraceParam;
    Action[22] = &ActionForSourceCodeScript::markPointer;
    Action[23] = &ActionForSourceCodeScript::markPeriodParam;
    Action[24] = &ActionForSourceCodeScript::setMemberId;
    Action[25] = &ActionForSourceCodeScript::markPeriodParenthesisParam;
    Action[26] = &ActionForSourceCodeScript::markPeriodBracketParam;
    Action[27] = &ActionForSourceCodeScript::markPeriodBraceParam;
    Action[28] = &ActionForSourceCodeScript::markQuestionPeriodParam;
    Action[29] = &ActionForSourceCodeScript::markPointerParam;
    Action[30] = &ActionForSourceCodeScript::markPeriodStarParam;
    Action[31] = &ActionForSourceCodeScript::markQuestionPeriodStarParam;
    Action[32] = &ActionForSourceCodeScript::markPointerStarParam;
    Action[33] = &ActionForSourceCodeScript::pushStr;
    Action[34] = &ActionForSourceCodeScript::pushNum;
    Action[35] = &ActionForSourceCodeScript::pushTrue;
    Action[36] = &ActionForSourceCodeScript::pushFalse;
}
//--------------------------------------------------------------------------------------
class ActionForGenerator : public SlkAction, public GeneratorT<ActionForGenerator>
{
    typedef GeneratorT<ActionForGenerator> BaseType;
public:
    inline char* getLastToken(void) const;
    inline int getLastLineNumber(void) const;
    inline void setCanFinish(int val);
    inline void setStringDelimiter(const char* begin, const char* end);
    inline void setScriptDelimiter(const char* begin, const char* end);
public:
    ActionForGenerator(SlkToken &scanner);
public:
    inline void    pushId(void);
    inline void    pushStr(void);
    inline void    pushNum(void);
    inline void    pushTrue(void);
    inline void    pushFalse(void);
    void    (ActionForGenerator::*Action[MAX_ACTION_NUM]) (void);
    inline void    initialize_table(void);
    inline void	execute(int  number) { (this->*Action[number]) (); }
private:
    SlkToken   *mScanner;
};
//--------------------------------------------------------------------------------------
inline char* ActionForGenerator::getLastToken(void) const
{
    if (NULL != mScanner) {
        return mScanner->getLastToken();
    }
    else {
        return NULL;
    }
}
inline int ActionForGenerator::getLastLineNumber(void) const
{
    if (NULL != mScanner) {
        return mScanner->getLastLineNumber();
    }
    else {
        return -1;
    }
}
inline void ActionForGenerator::setCanFinish(int val)
{
    if (NULL != mScanner) {
        mScanner->setCanFinish(val);
    }
}
inline void ActionForGenerator::setStringDelimiter(const char* begin, const char* end)
{
    if (NULL != mScanner) {
        mScanner->setStringDelimiter(begin, end);
    }
}
inline void ActionForGenerator::setScriptDelimiter(const char* begin, const char* end)
{
    if (NULL != mScanner) {
        mScanner->setScriptDelimiter(begin, end);
    }
}
//--------------------------------------------------------------------------------------
//标识符
inline void ActionForGenerator::pushId(void)
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
    }
}
inline void ActionForGenerator::pushNum(void)
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        if (strchr(lastToken, '.') == 0) {
            int val = 0;
            if (lastToken[0] == '0' && lastToken[1] == 'x') {
                sscanf(lastToken + 2, "%x", &val);
            }
            else {
                val = atoi(lastToken);
            }
            mData.push(RuntimeBuilderData::TokenInfo(val, RuntimeBuilderData::INT_TOKEN));
        }
        else {
            float val = static_cast<float>(atof(lastToken));
            mData.push(RuntimeBuilderData::TokenInfo(val));
        }
    }
}
inline void ActionForGenerator::pushStr(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForGenerator::pushTrue(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(true));
}
inline void ActionForGenerator::pushFalse(void)
{
    mData.push(RuntimeBuilderData::TokenInfo(false));
}
//--------------------------------------------------------------------------------------
inline ActionForGenerator::ActionForGenerator(SlkToken &scanner) :mScanner(&scanner)
{
    initialize_table();
    setEnvironmentObjRef(*this);
}
//--------------------------------------------------------------------------------------
inline void ActionForGenerator::initialize_table(void)
{
    Action[0] = 0;
    Action[1] = &ActionForGenerator::endStatement;
    Action[2] = &ActionForGenerator::markOperator;
    Action[3] = &ActionForGenerator::pushId;
    Action[4] = &ActionForGenerator::buildOperator;
    Action[5] = &ActionForGenerator::buildFirstTernaryOperator;
    Action[6] = &ActionForGenerator::buildSecondTernaryOperator;
    Action[7] = &ActionForGenerator::beginStatement;
    Action[8] = &ActionForGenerator::beginFunction;
    Action[9] = &ActionForGenerator::endFunction;
    Action[10] = &ActionForGenerator::setFunctionId;
    Action[11] = &ActionForGenerator::markHaveStatement;
    Action[12] = &ActionForGenerator::markHaveExternScript;
    Action[13] = &ActionForGenerator::setExternScript;
    Action[14] = &ActionForGenerator::markParenthesisParam;
    Action[15] = &ActionForGenerator::buildHighOrderFunction;
    Action[16] = &ActionForGenerator::markBracketParam;
    Action[17] = &ActionForGenerator::markPeriod;
    Action[18] = &ActionForGenerator::markQuestion;
    Action[19] = &ActionForGenerator::markQuestionParenthesisParam;
    Action[20] = &ActionForGenerator::markQuestionBracketParam;
    Action[21] = &ActionForGenerator::markQuestionBraceParam;
    Action[22] = &ActionForGenerator::markPointer;
    Action[23] = &ActionForGenerator::markPeriodParam;
    Action[24] = &ActionForGenerator::setMemberId;
    Action[25] = &ActionForGenerator::markPeriodParenthesisParam;
    Action[26] = &ActionForGenerator::markPeriodBracketParam;
    Action[27] = &ActionForGenerator::markPeriodBraceParam;
    Action[28] = &ActionForGenerator::markQuestionPeriodParam;
    Action[29] = &ActionForGenerator::markPointerParam;
    Action[30] = &ActionForGenerator::markPeriodStarParam;
    Action[31] = &ActionForGenerator::markQuestionPeriodStarParam;
    Action[32] = &ActionForGenerator::markPointerStarParam;
    Action[33] = &ActionForGenerator::pushStr;
    Action[34] = &ActionForGenerator::pushNum;
    Action[35] = &ActionForGenerator::pushTrue;
    Action[36] = &ActionForGenerator::pushFalse;
}
//--------------------------------------------------------------------------------------
namespace FunctionScript
{
    class CachedScriptSource : public IScriptSource
    {
    public:
        explicit CachedScriptSource(const char* p) :m_Source(p)
        {}
    protected:
        virtual int Load(void)
        {
            return FALSE;
        }
        virtual const char* GetBuffer(void)const
        {
            return m_Source;
        }
    private:
        const char* m_Source;
    };
    //------------------------------------------------------------------------------------------------------
    int ByteCodeGenerator::Parse(const char* buf, char* pByteCode, int codeBufferLen)
    {
        if (0 == buf)
            return 0;
        CachedScriptSource source(buf);
        return Parse(source, pByteCode, codeBufferLen);
    }
    int ByteCodeGenerator::Parse(IScriptSource& source, char* pByteCode, int codeBufferLen)
    {
        m_ErrorAndStringBuffer.ClearErrorInfo();
        SlkToken tokens(source, m_ErrorAndStringBuffer);
        SlkError error(tokens, m_ErrorAndStringBuffer);
        ActionForGenerator action(tokens);
        SlkParse(action, tokens, error, 0);
        int len = 0;
        const char* p = action.getByteCode(len);
        if (NULL != p && len <= codeBufferLen) {
            memcpy(pByteCode, p, len);
        }
        return len;
    }
    //------------------------------------------------------------------------------------------------------
    void SourceCodeScript::Parse(const char* buf)
    {
        if (0 == buf)
            return;
        CachedScriptSource source(buf);
        Parse(source);
    }

    void SourceCodeScript::Parse(IScriptSource& source)
    {
        m_Interpreter.ClearErrorInfo();
        SlkToken tokens(source, m_Interpreter.GetErrorAndStringBuffer());
        SlkError error(tokens, m_Interpreter.GetErrorAndStringBuffer());
        ActionForSourceCodeScript action(tokens, m_Interpreter);
        SlkParse(action, tokens, error, 0);

        m_Interpreter.PrepareRuntimeObject();
    }
}