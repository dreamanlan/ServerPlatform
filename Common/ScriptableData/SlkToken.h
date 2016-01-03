/**************************************************************************

    SlkToken.h

    **************************************************************************/

#ifndef _SLK_SLKTOKEN_H
#define _SLK_SLKTOKEN_H

#include "ScriptableData.h"

class SlkToken
{
public:
  SHORT get(void);
  SHORT peek(INT level);
  CHAR* getCurToken(void) const
  {
    return mCurToken;
  }
  CHAR* getLastToken(void) const
  {
    return mLastToken;
  }
  INT getLineNumber(void) const
  {
    return mLineNumber;
  }
  INT getLastLineNumber(void) const
  {
    return mLastLineNumber;
  }
public:
  void setCanFinish(BOOL val)
  {
    mCanFinish = val;
  }
private:
  void getOperatorToken(void);
  SHORT getOperatorTokenValue(void)const;
  BOOL isCanFinish(void)const
  {
    return mCanFinish;
  }
  BOOL isWhiteSpace(CHAR c) const;
  BOOL isDelimiter(CHAR c) const;
  BOOL isOperator(CHAR c) const;
private:
  void newToken(void);
  void pushTokenChar(CHAR c);
  void endToken(void);
  void endTokenWithEof(void);
public:
  SlkToken(ScriptableData::IScriptSource& source, ScriptableData::ErrorAndStringBuffer& errorAndStringBuffer);
private:
  ScriptableData::IScriptSource* mSource;
  ScriptableData::IScriptSource::Iterator mIterator;

  CHAR* mCurToken;
  CHAR* mLastToken;
  INT mTokenCharIndex;

  ScriptableData::ErrorAndStringBuffer* mErrorAndStringBuffer;

  INT mLineNumber;
  INT mLastLineNumber;
  BOOL mIsExternScript;
  BOOL mCanFinish;

  const CHAR* mWhiteSpaces;
  const CHAR* mDelimiters;
  const CHAR* mOperators;
};

#endif

