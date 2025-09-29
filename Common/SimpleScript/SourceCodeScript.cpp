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
    inline void    pushDollarStr();
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
    if (nullptr != mScanner) {
        mScanner->setCanFinish(val);
    }
}
inline void ActionForSourceCodeScript::setStringDelimiter(const char* begin, const char* end)
{
    if (nullptr != mScanner) {
        mScanner->setStringDelimiter(begin, end);
    }
}
inline void ActionForSourceCodeScript::setScriptDelimiter(const char* begin, const char* end)
{
    if (nullptr != mScanner) {
        mScanner->setScriptDelimiter(begin, end);
    }
}
//--------------------------------------------------------------------------------------
//identifier
inline void ActionForSourceCodeScript::pushId()
{
    char* lastToken = getLastToken();
    if (nullptr != lastToken) {
        mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
    }
}
inline void ActionForSourceCodeScript::pushNum()
{
    char* lastToken = getLastToken();
    if (nullptr != lastToken) {
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
inline void ActionForSourceCodeScript::pushDollarStr()
{
    const char* token = getLastToken();
    if (strcmp(token, "true") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(true));
    else if (strcmp(token, "false") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(false));
    else
        mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::DOLLAR_STRING_TOKEN));
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
    mApi.SetImpl(this);
    scanner.SetDslActionApi(mApi);
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
    Action[10] = &ActionForSourceCodeScript::buildNullableOperator;
    Action[11] = &ActionForSourceCodeScript::markParenthesesParam;
    Action[12] = &ActionForSourceCodeScript::markParenthesesParamEnd;
    Action[13] = &ActionForSourceCodeScript::buildHighOrderFunction;
    Action[14] = &ActionForSourceCodeScript::markBracketParam;
    Action[15] = &ActionForSourceCodeScript::markBracketParamEnd;
    Action[16] = &ActionForSourceCodeScript::markStatement;
    Action[17] = &ActionForSourceCodeScript::markStatementEnd;
    Action[18] = &ActionForSourceCodeScript::markExternScript;
    Action[19] = &ActionForSourceCodeScript::setExternScript;
    Action[20] = &ActionForSourceCodeScript::markBracketColonParam;
    Action[21] = &ActionForSourceCodeScript::markBracketColonParamEnd;
    Action[22] = &ActionForSourceCodeScript::markParenthesesColonParam;
    Action[23] = &ActionForSourceCodeScript::markParenthesesColonParamEnd;
    Action[24] = &ActionForSourceCodeScript::markAngleBracketColonParam;
    Action[25] = &ActionForSourceCodeScript::markAngleBracketColonParamEnd;
    Action[26] = &ActionForSourceCodeScript::markBracePercentParam;
    Action[27] = &ActionForSourceCodeScript::markBracePercentParamEnd;
    Action[28] = &ActionForSourceCodeScript::markBracketPercentParam;
    Action[29] = &ActionForSourceCodeScript::markBracketPercentParamEnd;
    Action[30] = &ActionForSourceCodeScript::markParenthesesPercentParam;
    Action[31] = &ActionForSourceCodeScript::markParenthesesPercentParamEnd;
    Action[32] = &ActionForSourceCodeScript::markAngleBracketPercentParam;
    Action[33] = &ActionForSourceCodeScript::markAngleBracketPercentParamEnd;
    Action[34] = &ActionForSourceCodeScript::markColonColonParam;
    Action[35] = &ActionForSourceCodeScript::markPeriodParam;
    Action[36] = &ActionForSourceCodeScript::markPointerParam;
    Action[37] = &ActionForSourceCodeScript::markPeriodStarParam;
    Action[38] = &ActionForSourceCodeScript::markPointerStarParam;
    Action[39] = &ActionForSourceCodeScript::pushStr;
    Action[40] = &ActionForSourceCodeScript::pushNum;
    Action[41] = &ActionForSourceCodeScript::pushDollarStr;
    Action[42] = &ActionForSourceCodeScript::pushComma;
    Action[43] = &ActionForSourceCodeScript::pushSemiColon;
}
//--------------------------------------------------------------------------------------
int ActionApi::peekPairTypeStack()const
{
    if (!m_Impl)
        return FunctionData::PAIR_TYPE_NONE;
    return m_Impl->peekPairTypeStack();
}
int ActionApi::peekPairTypeStack(uint32_t& tag)const
{
    tag = 0;
    if (!m_Impl)
        return FunctionData::PAIR_TYPE_NONE;
    return m_Impl->peekPairTypeStack(tag);
}
int ActionApi::getPairTypeStackSize()const
{
    if (!m_Impl)
        return 0;
    return m_Impl->getPairTypeStackSize();
}
int ActionApi::peekPairTypeStack(int ix)const
{
    if (!m_Impl)
        return FunctionData::PAIR_TYPE_NONE;
    return m_Impl->peekPairTypeStack(ix);
}
int ActionApi::peekPairTypeStack(int ix, uint32_t& tag)const
{
    tag = 0;
    if (!m_Impl)
        return FunctionData::PAIR_TYPE_NONE;
    return m_Impl->peekPairTypeStack(ix, tag);
}
void ActionApi::beginStatement()const
{
    if (!m_Impl)
        return;
    m_Impl->beginStatement();
}
void ActionApi::endStatement()const
{
    if (!m_Impl)
        return;
    m_Impl->endStatement();
}
StatementData* ActionApi::getCurStatement()const
{
    if (!m_Impl)
        return 0;
    return m_Impl->getCurStatement();
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
    void SourceCodeScript::Parse(const char* buf)
    {
        if (0 == buf)
            return;
        CachedScriptSource source(buf);
        Parse(source);
    }

    void SourceCodeScript::Parse(IScriptSource& source)
    {
        m_Interpreter.NameTagsRef() = ParserFineTuneHelper::ForSimpleScript().NameTags();
        m_Interpreter.ClearErrorInfo();
        SlkToken tokens(source, m_Interpreter.GetErrorAndStringBuffer());
        SlkError error(m_Interpreter.GetErrorAndStringBuffer());
        ActionForSourceCodeScript action(tokens, m_Interpreter);
        SlkParse(action, tokens, error, 0);

        m_Interpreter.PrepareRuntimeObject();
    }
}