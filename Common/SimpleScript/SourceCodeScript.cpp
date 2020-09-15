#include "SourceCodeScript.h"
#include "SlkInc.h"
#include "SlkParse.h"
#include "ByteCode.h"

#define MAX_ACTION_NUM	41

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
    const char* token = getLastToken();
    if(strcmp(token, "true")==0)
        mData.push(RuntimeBuilderData::TokenInfo(true));
    else if(strcmp(token, "false")==0)
        mData.push(RuntimeBuilderData::TokenInfo(false));
    else
        mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
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
	Action[2] = &ActionForSourceCodeScript::pushId;
	Action[3] = &ActionForSourceCodeScript::buildOperator;
	Action[4] = &ActionForSourceCodeScript::buildFirstTernaryOperator;
	Action[5] = &ActionForSourceCodeScript::buildSecondTernaryOperator;
	Action[6] = &ActionForSourceCodeScript::beginStatement;
	Action[7] = &ActionForSourceCodeScript::addFunction;
	Action[8] = &ActionForSourceCodeScript::setFunctionId;
	Action[9] = &ActionForSourceCodeScript::markParenthesisParam;
	Action[10] = &ActionForSourceCodeScript::buildHighOrderFunction;
	Action[11] = &ActionForSourceCodeScript::markBracketParam;
	Action[12] = &ActionForSourceCodeScript::markQuestionParenthesisParam;
	Action[13] = &ActionForSourceCodeScript::markQuestionBracketParam;
	Action[14] = &ActionForSourceCodeScript::markQuestionBraceParam;
    Action[15] = &ActionForSourceCodeScript::markStatement;
    Action[16] = &ActionForSourceCodeScript::markExternScript;
    Action[17] = &ActionForSourceCodeScript::setExternScript;
    Action[18] = &ActionForSourceCodeScript::markBracketColonParam;
    Action[19] = &ActionForSourceCodeScript::markParenthesisColonParam;
    Action[20] = &ActionForSourceCodeScript::markAngleBracketColonParam;
    Action[21] = &ActionForSourceCodeScript::markBracePercentParam;
    Action[22] = &ActionForSourceCodeScript::markBracketPercentParam;
    Action[23] = &ActionForSourceCodeScript::markParenthesisPercentParam;
    Action[24] = &ActionForSourceCodeScript::markAngleBracketPercentParam;
    Action[25] = &ActionForSourceCodeScript::markColonColonParam;
    Action[26] = &ActionForSourceCodeScript::setMemberId;
    Action[27] = &ActionForSourceCodeScript::markColonColonParenthesisParam;
    Action[28] = &ActionForSourceCodeScript::markColonColonBracketParam;
    Action[29] = &ActionForSourceCodeScript::markColonColonBraceParam;
	Action[30] = &ActionForSourceCodeScript::markPeriodParam;
	Action[31] = &ActionForSourceCodeScript::markPeriodParenthesisParam;
	Action[32] = &ActionForSourceCodeScript::markPeriodBracketParam;
	Action[33] = &ActionForSourceCodeScript::markPeriodBraceParam;
	Action[34] = &ActionForSourceCodeScript::markQuestionPeriodParam;
	Action[35] = &ActionForSourceCodeScript::markPointerParam;
	Action[36] = &ActionForSourceCodeScript::markPeriodStarParam;
	Action[37] = &ActionForSourceCodeScript::markQuestionPeriodStarParam;
	Action[38] = &ActionForSourceCodeScript::markPointerStarParam;
	Action[39] = &ActionForSourceCodeScript::pushStr;
	Action[40] = &ActionForSourceCodeScript::pushNum;
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
    const char* token = getLastToken();
    if (strcmp(token, "true") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(true));
    else if (strcmp(token, "false") == 0)
        mData.push(RuntimeBuilderData::TokenInfo(false));
    else
        mData.push(RuntimeBuilderData::TokenInfo(getLastToken(), RuntimeBuilderData::STRING_TOKEN));
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
    Action[2] = &ActionForGenerator::pushId;
    Action[3] = &ActionForGenerator::buildOperator;
    Action[4] = &ActionForGenerator::buildFirstTernaryOperator;
    Action[5] = &ActionForGenerator::buildSecondTernaryOperator;
    Action[6] = &ActionForGenerator::beginStatement;
    Action[7] = &ActionForGenerator::addFunction;
    Action[8] = &ActionForGenerator::setFunctionId;
    Action[9] = &ActionForGenerator::markParenthesisParam;
    Action[10] = &ActionForGenerator::buildHighOrderFunction;
    Action[11] = &ActionForGenerator::markBracketParam;
    Action[12] = &ActionForGenerator::markQuestionParenthesisParam;
    Action[13] = &ActionForGenerator::markQuestionBracketParam;
    Action[14] = &ActionForGenerator::markQuestionBraceParam;
    Action[15] = &ActionForGenerator::markStatement;
    Action[16] = &ActionForGenerator::markExternScript;
    Action[17] = &ActionForGenerator::setExternScript;
    Action[18] = &ActionForGenerator::markBracketColonParam;
    Action[19] = &ActionForGenerator::markParenthesisColonParam;
    Action[20] = &ActionForGenerator::markAngleBracketColonParam;
    Action[21] = &ActionForGenerator::markBracePercentParam;
    Action[22] = &ActionForGenerator::markBracketPercentParam;
    Action[23] = &ActionForGenerator::markParenthesisPercentParam;
    Action[24] = &ActionForGenerator::markAngleBracketPercentParam;
    Action[25] = &ActionForGenerator::markColonColonParam;
    Action[26] = &ActionForGenerator::setMemberId;
    Action[27] = &ActionForGenerator::markColonColonParenthesisParam;
    Action[28] = &ActionForGenerator::markColonColonBracketParam;
    Action[29] = &ActionForGenerator::markColonColonBraceParam;
    Action[30] = &ActionForGenerator::markPeriodParam;
    Action[31] = &ActionForGenerator::markPeriodParenthesisParam;
    Action[32] = &ActionForGenerator::markPeriodBracketParam;
    Action[33] = &ActionForGenerator::markPeriodBraceParam;
    Action[34] = &ActionForGenerator::markQuestionPeriodParam;
    Action[35] = &ActionForGenerator::markPointerParam;
    Action[36] = &ActionForGenerator::markPeriodStarParam;
    Action[37] = &ActionForGenerator::markQuestionPeriodStarParam;
    Action[38] = &ActionForGenerator::markPointerStarParam;
    Action[39] = &ActionForGenerator::pushStr;
    Action[40] = &ActionForGenerator::pushNum;
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