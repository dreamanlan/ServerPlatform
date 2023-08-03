#include "SourceCodeScript.h"
#include "SlkInc.h"
#include "SlkParse.h"
#include "ByteCode.h"

#define MAX_ACTION_NUM	44

//--------------------------------------------------------------------------------------
class ActionForSourceCodeScript : public SlkAction, public RuntimeBuilderT<ActionForSourceCodeScript>
{
    using BaseType = RuntimeBuilderT<ActionForSourceCodeScript>;
public:
    inline char* getLastToken() const;
    inline int getLastLineNumber() const;
    inline void setCanFinish(int val);
    inline void setStringDelimiter(const char* begin, const char* end);
    inline void setScriptDelimiter(const char* begin, const char* end);
public:
    ActionForSourceCodeScript(SlkToken& scanner, FunctionScript::Interpreter& interpreter);
public:
    inline void    pushId();
    inline void    pushStr();
    inline void    pushNum();
    inline void    pushComma();
    inline void    pushSemiColon();
    void    (ActionForSourceCodeScript::* Action[MAX_ACTION_NUM]) ();
    inline void    initialize_table();
    inline void	execute(int  number) { (this->*Action[number]) (); }
private:
    SlkToken* mScanner;
};
//--------------------------------------------------------------------------------------
inline char* ActionForSourceCodeScript::getLastToken() const
{
    if (NULL != mScanner) {
        return mScanner->getLastToken();
    }
    else {
        return NULL;
    }
}
inline int ActionForSourceCodeScript::getLastLineNumber() const
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
inline void ActionForSourceCodeScript::pushId()
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
    }
}
inline void ActionForSourceCodeScript::pushNum()
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
inline void ActionForSourceCodeScript::pushStr()
{
    const char* token = getLastToken();
    if (strcmp(token, "true") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(true));
    else if (strcmp(token, "false") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(false));
    else
        mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForSourceCodeScript::pushComma()
{
    mData.push(RuntimeBuilderData::TokenInfo(",", RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForSourceCodeScript::pushSemiColon()
{
    mData.push(RuntimeBuilderData::TokenInfo(";", RuntimeBuilderData::STRING_TOKEN));
}
//--------------------------------------------------------------------------------------
inline ActionForSourceCodeScript::ActionForSourceCodeScript(SlkToken& scanner, FunctionScript::Interpreter& interpreter) :mScanner(&scanner), BaseType(interpreter)
{
    initialize_table();
    setEnvironmentObjRef(*this);
}
//--------------------------------------------------------------------------------------
inline void ActionForSourceCodeScript::initialize_table()
{
    Action[0] = 0;
    Action[1] = &ActionForSourceCodeScript::markSeparator;
    Action[2] = &ActionForSourceCodeScript::endStatement;
    Action[3] = &ActionForSourceCodeScript::pushId;
    Action[4] = &ActionForSourceCodeScript::buildOperator;
    Action[5] = &ActionForSourceCodeScript::buildFirstTernaryOperator;
    Action[6] = &ActionForSourceCodeScript::buildSecondTernaryOperator;
    Action[7] = &ActionForSourceCodeScript::beginStatement;
    Action[8] = &ActionForSourceCodeScript::addFunction;
    Action[9] = &ActionForSourceCodeScript::setFunctionId;
    Action[10] = &ActionForSourceCodeScript::markParenthesisParam;
    Action[11] = &ActionForSourceCodeScript::buildHighOrderFunction;
    Action[12] = &ActionForSourceCodeScript::markBracketParam;
    Action[13] = &ActionForSourceCodeScript::markQuestionParenthesisParam;
    Action[14] = &ActionForSourceCodeScript::markQuestionBracketParam;
    Action[15] = &ActionForSourceCodeScript::markQuestionBraceParam;
    Action[16] = &ActionForSourceCodeScript::markStatement;
    Action[17] = &ActionForSourceCodeScript::markExternScript;
    Action[18] = &ActionForSourceCodeScript::setExternScript;
    Action[19] = &ActionForSourceCodeScript::markBracketColonParam;
    Action[20] = &ActionForSourceCodeScript::markParenthesisColonParam;
    Action[21] = &ActionForSourceCodeScript::markAngleBracketColonParam;
    Action[22] = &ActionForSourceCodeScript::markBracePercentParam;
    Action[23] = &ActionForSourceCodeScript::markBracketPercentParam;
    Action[24] = &ActionForSourceCodeScript::markParenthesisPercentParam;
    Action[25] = &ActionForSourceCodeScript::markAngleBracketPercentParam;
    Action[26] = &ActionForSourceCodeScript::markColonColonParam;
    Action[27] = &ActionForSourceCodeScript::setMemberId;
    Action[28] = &ActionForSourceCodeScript::markColonColonParenthesisParam;
    Action[29] = &ActionForSourceCodeScript::markColonColonBracketParam;
    Action[30] = &ActionForSourceCodeScript::markColonColonBraceParam;
    Action[31] = &ActionForSourceCodeScript::markPeriodParam;
    Action[32] = &ActionForSourceCodeScript::markPeriodParenthesisParam;
    Action[33] = &ActionForSourceCodeScript::markPeriodBracketParam;
    Action[34] = &ActionForSourceCodeScript::markPeriodBraceParam;
    Action[35] = &ActionForSourceCodeScript::markQuestionPeriodParam;
    Action[36] = &ActionForSourceCodeScript::markPointerParam;
    Action[37] = &ActionForSourceCodeScript::markPeriodStarParam;
    Action[38] = &ActionForSourceCodeScript::markQuestionPeriodStarParam;
    Action[39] = &ActionForSourceCodeScript::markPointerStarParam;
    Action[40] = &ActionForSourceCodeScript::pushStr;
    Action[41] = &ActionForSourceCodeScript::pushNum;
    Action[42] = &ActionForSourceCodeScript::pushComma;
    Action[43] = &ActionForSourceCodeScript::pushSemiColon;
}
//--------------------------------------------------------------------------------------
class ActionForGenerator : public SlkAction, public GeneratorT<ActionForGenerator>
{
    using BaseType = GeneratorT<ActionForGenerator>;
public:
    inline char* getLastToken() const;
    inline int getLastLineNumber() const;
    inline void setCanFinish(int val);
    inline void setStringDelimiter(const char* begin, const char* end);
    inline void setScriptDelimiter(const char* begin, const char* end);
public:
    ActionForGenerator(SlkToken& scanner);
public:
    inline void    pushId();
    inline void    pushStr();
    inline void    pushNum();
    inline void    pushComma();
    inline void    pushSemiColon();
    void    (ActionForGenerator::* Action[MAX_ACTION_NUM]) ();
    inline void    initialize_table();
    inline void	execute(int  number) { (this->*Action[number]) (); }
private:
    SlkToken* mScanner;
};
//--------------------------------------------------------------------------------------
inline char* ActionForGenerator::getLastToken() const
{
    if (NULL != mScanner) {
        return mScanner->getLastToken();
    }
    else {
        return NULL;
    }
}
inline int ActionForGenerator::getLastLineNumber() const
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
inline void ActionForGenerator::pushId()
{
    char* lastToken = getLastToken();
    if (NULL != lastToken) {
        mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
    }
}
inline void ActionForGenerator::pushNum()
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
inline void ActionForGenerator::pushStr()
{
    const char* token = getLastToken();
    if (strcmp(token, "true") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(true));
    else if (strcmp(token, "false") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(false));
    else
        mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForGenerator::pushComma()
{
    mData.push(RuntimeBuilderData::TokenInfo(",", RuntimeBuilderData::STRING_TOKEN));
}
inline void ActionForGenerator::pushSemiColon()
{
    mData.push(RuntimeBuilderData::TokenInfo(";", RuntimeBuilderData::STRING_TOKEN));
}
//--------------------------------------------------------------------------------------
inline ActionForGenerator::ActionForGenerator(SlkToken& scanner) :mScanner(&scanner)
{
    initialize_table();
    setEnvironmentObjRef(*this);
}
//--------------------------------------------------------------------------------------
inline void ActionForGenerator::initialize_table()
{
    Action[0] = 0;
    Action[1] = &ActionForGenerator::markSeparator;
    Action[2] = &ActionForGenerator::endStatement;
    Action[3] = &ActionForGenerator::pushId;
    Action[4] = &ActionForGenerator::buildOperator;
    Action[5] = &ActionForGenerator::buildFirstTernaryOperator;
    Action[6] = &ActionForGenerator::buildSecondTernaryOperator;
    Action[7] = &ActionForGenerator::beginStatement;
    Action[8] = &ActionForGenerator::addFunction;
    Action[9] = &ActionForGenerator::setFunctionId;
    Action[10] = &ActionForGenerator::markParenthesisParam;
    Action[11] = &ActionForGenerator::buildHighOrderFunction;
    Action[12] = &ActionForGenerator::markBracketParam;
    Action[13] = &ActionForGenerator::markQuestionParenthesisParam;
    Action[14] = &ActionForGenerator::markQuestionBracketParam;
    Action[15] = &ActionForGenerator::markQuestionBraceParam;
    Action[16] = &ActionForGenerator::markStatement;
    Action[17] = &ActionForGenerator::markExternScript;
    Action[18] = &ActionForGenerator::setExternScript;
    Action[19] = &ActionForGenerator::markBracketColonParam;
    Action[20] = &ActionForGenerator::markParenthesisColonParam;
    Action[21] = &ActionForGenerator::markAngleBracketColonParam;
    Action[22] = &ActionForGenerator::markBracePercentParam;
    Action[23] = &ActionForGenerator::markBracketPercentParam;
    Action[24] = &ActionForGenerator::markParenthesisPercentParam;
    Action[25] = &ActionForGenerator::markAngleBracketPercentParam;
    Action[26] = &ActionForGenerator::markColonColonParam;
    Action[27] = &ActionForGenerator::setMemberId;
    Action[28] = &ActionForGenerator::markColonColonParenthesisParam;
    Action[29] = &ActionForGenerator::markColonColonBracketParam;
    Action[30] = &ActionForGenerator::markColonColonBraceParam;
    Action[31] = &ActionForGenerator::markPeriodParam;
    Action[32] = &ActionForGenerator::markPeriodParenthesisParam;
    Action[33] = &ActionForGenerator::markPeriodBracketParam;
    Action[34] = &ActionForGenerator::markPeriodBraceParam;
    Action[35] = &ActionForGenerator::markQuestionPeriodParam;
    Action[36] = &ActionForGenerator::markPointerParam;
    Action[37] = &ActionForGenerator::markPeriodStarParam;
    Action[38] = &ActionForGenerator::markQuestionPeriodStarParam;
    Action[39] = &ActionForGenerator::markPointerStarParam;
    Action[40] = &ActionForGenerator::pushStr;
    Action[41] = &ActionForGenerator::pushNum;
    Action[42] = &ActionForGenerator::pushComma;
    Action[43] = &ActionForGenerator::pushSemiColon;
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
        virtual int Load()
        {
            return FALSE;
        }
        virtual const char* GetBuffer()const
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
        SlkError error(m_ErrorAndStringBuffer);
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
        SlkError error(m_Interpreter.GetErrorAndStringBuffer());
        ActionForSourceCodeScript action(tokens, m_Interpreter);
        SlkParse(action, tokens, error, 0);

        m_Interpreter.PrepareRuntimeObject();
    }
}