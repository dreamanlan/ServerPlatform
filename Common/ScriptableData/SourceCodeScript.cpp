#include "SourceCodeScript.h"
#include "SlkInc.h"
#include "SlkParse.h"
#include "ByteCode.h"

#define MAX_ACTION_NUM	23

//--------------------------------------------------------------------------------------
class ActionForSourceCodeScript : public SlkAction, public RuntimeBuilderT < ActionForSourceCodeScript >
{
  typedef RuntimeBuilderT<ActionForSourceCodeScript> BaseType;
public:
  inline CHAR* getLastToken(void) const;
  inline INT getLastLineNumber(void) const;
  inline void setCanFinish(BOOL val);
public:
  ActionForSourceCodeScript(SlkToken &scanner, ScriptableData::ScriptableDataFile& dataFile);
public:
  inline void    pushId(void);
  inline void    pushStr(void);
  inline void    pushNum(void);
  void    (ActionForSourceCodeScript::*Action[MAX_ACTION_NUM]) (void);
  inline void    initialize_table(void);
  inline void	execute(INT  number)   { (this->*Action[number]) (); }
private:
  SlkToken   *mScanner;
};
//--------------------------------------------------------------------------------------
inline CHAR* ActionForSourceCodeScript::getLastToken(void) const
{
  if (NULL != mScanner) {
    return mScanner->getLastToken();
  } else {
    return NULL;
  }
}
inline INT ActionForSourceCodeScript::getLastLineNumber(void) const
{
  if (NULL != mScanner) {
    return mScanner->getLastLineNumber();
  } else {
    return -1;
  }
}
inline void ActionForSourceCodeScript::setCanFinish(BOOL val)
{
  if (NULL != mScanner) {
    mScanner->setCanFinish(val);
  }
}
//--------------------------------------------------------------------------------------
//标识符
inline void ActionForSourceCodeScript::pushId(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
  }
}
inline void ActionForSourceCodeScript::pushNum(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::INT_TOKEN));
  }
}
inline void ActionForSourceCodeScript::pushStr(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::STRING_TOKEN));
  }
}
//--------------------------------------------------------------------------------------
inline ActionForSourceCodeScript::ActionForSourceCodeScript(SlkToken &scanner, ScriptableData::ScriptableDataFile& dataFile) :mScanner(&scanner), BaseType(dataFile)
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
  Action[7] = &ActionForSourceCodeScript::beginFunction;
  Action[8] = &ActionForSourceCodeScript::endFunction;
  Action[9] = &ActionForSourceCodeScript::setFunctionId;
  Action[10] = &ActionForSourceCodeScript::markHaveStatement;
  Action[11] = &ActionForSourceCodeScript::markHaveExternScript;
  Action[12] = &ActionForSourceCodeScript::setExternScript;
  Action[13] = &ActionForSourceCodeScript::markParenthesisParam;
  Action[14] = &ActionForSourceCodeScript::buildHighOrderFunction;
  Action[15] = &ActionForSourceCodeScript::markBracketParam;
  Action[16] = &ActionForSourceCodeScript::markPeriodParam;
  Action[17] = &ActionForSourceCodeScript::setMemberId;
  Action[18] = &ActionForSourceCodeScript::markPeriodParenthesisParam;
  Action[19] = &ActionForSourceCodeScript::markPeriodBracketParam;
  Action[20] = &ActionForSourceCodeScript::markPeriodBraceParam;
  Action[21] = &ActionForSourceCodeScript::pushStr;
  Action[22] = &ActionForSourceCodeScript::pushNum;
}
//--------------------------------------------------------------------------------------
class ActionForGenerator : public SlkAction, public GeneratorT < ActionForGenerator >
{
  typedef GeneratorT<ActionForGenerator> BaseType;
public:
  inline CHAR* getLastToken(void) const;
  inline INT getLastLineNumber(void) const;
  inline void setCanFinish(BOOL val);
public:
  ActionForGenerator(SlkToken &scanner);
public:
  inline void    pushId(void);
  inline void    pushStr(void);
  inline void    pushNum(void);
  void    (ActionForGenerator::*Action[MAX_ACTION_NUM]) (void);
  inline void    initialize_table(void);
  inline void	execute(INT  number)   { (this->*Action[number]) (); }
private:
  SlkToken   *mScanner;
};
//--------------------------------------------------------------------------------------
inline CHAR* ActionForGenerator::getLastToken(void) const
{
  if (NULL != mScanner) {
    return mScanner->getLastToken();
  } else {
    return NULL;
  }
}
inline INT ActionForGenerator::getLastLineNumber(void) const
{
  if (NULL != mScanner) {
    return mScanner->getLastLineNumber();
  } else {
    return -1;
  }
}
inline void ActionForGenerator::setCanFinish(BOOL val)
{
  if (NULL != mScanner) {
    mScanner->setCanFinish(val);
  }
}
//--------------------------------------------------------------------------------------
//标识符
inline void ActionForGenerator::pushId(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::VARIABLE_TOKEN));
  }
}
inline void ActionForGenerator::pushNum(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::INT_TOKEN));
  }
}
inline void ActionForGenerator::pushStr(void)
{
  CHAR* lastToken = getLastToken();
  if (NULL != lastToken) {
    mData.push(RuntimeBuilderData::TokenInfo(lastToken, RuntimeBuilderData::STRING_TOKEN));
  }
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
  Action[7] = &ActionForGenerator::beginFunction;
  Action[8] = &ActionForGenerator::endFunction;
  Action[9] = &ActionForGenerator::setFunctionId;
  Action[10] = &ActionForGenerator::markHaveStatement;
  Action[11] = &ActionForGenerator::markHaveExternScript;
  Action[12] = &ActionForGenerator::setExternScript;
  Action[13] = &ActionForGenerator::markParenthesisParam;
  Action[14] = &ActionForGenerator::buildHighOrderFunction;
  Action[15] = &ActionForGenerator::markBracketParam;
  Action[16] = &ActionForGenerator::markPeriodParam;
  Action[17] = &ActionForGenerator::setMemberId;
  Action[18] = &ActionForGenerator::markPeriodParenthesisParam;
  Action[19] = &ActionForGenerator::markPeriodBracketParam;
  Action[20] = &ActionForGenerator::markPeriodBraceParam;
  Action[21] = &ActionForGenerator::pushStr;
  Action[22] = &ActionForGenerator::pushNum;
}
//--------------------------------------------------------------------------------------
namespace ScriptableData
{
  class CachedScriptSource : public IScriptSource
  {
  public:
    explicit CachedScriptSource(const CHAR* p) :m_Source(p)
    {}
  protected:
    virtual BOOL Load(void)
    {
      return FALSE;
    }
    virtual const CHAR* GetBuffer(void)const
    {
      return m_Source;
    }
  private:
    const CHAR* m_Source;
  };
  //------------------------------------------------------------------------------------------------------
  INT ByteCodeGenerator::Parse(const CHAR* buf, CHAR* pByteCode, INT codeBufferLen)
  {
    if (0 == buf)
      return 0;
    CachedScriptSource source(buf);
    return Parse(source, pByteCode, codeBufferLen);
  }
  INT ByteCodeGenerator::Parse(IScriptSource& source, CHAR* pByteCode, INT codeBufferLen)
  {
    m_ErrorAndStringBuffer.ClearErrorInfo();
    SlkToken tokens(source, m_ErrorAndStringBuffer);
    SlkError error(tokens, m_ErrorAndStringBuffer);
    ActionForGenerator action(tokens);
    SlkParse(action, tokens, error, 0);
    INT len = 0;
    const CHAR* p = action.getByteCode(len);
    if (NULL != p && len <= codeBufferLen) {
      memcpy(pByteCode, p, len);
    }
    return len;
  }
  //------------------------------------------------------------------------------------------------------
  void ParseText(const CHAR* buf, ScriptableDataFile& file)
  {
    if (0 == buf)
      return;
    CachedScriptSource source(buf);
    ParseText(source, file);
  }

  void ParseText(IScriptSource& source, ScriptableDataFile& file)
  {
    file.ClearErrorInfo();
    SlkToken tokens(source, file.GetErrorAndStringBuffer());
    SlkError error(tokens, file.GetErrorAndStringBuffer());
    ActionForSourceCodeScript action(tokens, file);
    SlkParse(action, tokens, error, 0);
  }
}