/**************************************************************************

    SlkToken.h

    **************************************************************************/

#ifndef _SLK_SLKTOKEN_H
#define _SLK_SLKTOKEN_H

#include "calc.h"

class SlkToken
{
public:
  short get(void);
  short peek(int level);
  char* getCurToken(void) const
  {
    return mCurToken;
  }
  char* getLastToken(void) const
  {
    return mLastToken;
  }
  int getLineNumber(void) const
  {
    return mLineNumber;
  }
  int getLastLineNumber(void) const
  {
    return mLastLineNumber;
  }
public:
  void setCanFinish(int val)
  {
    mCanFinish = val;
  }
private:
  void getOperatorToken(void);
  short getOperatorTokenValue(void)const;
  int isCanFinish(void)const
  {
    return mCanFinish;
  }
  int isWhiteSpace(char c) const;
  int isDelimiter(char c) const;
  int isBeginParentheses(char c) const;
  int isEndParentheses(char c) const;
  int isOperator(char c) const;
  int isSpecialChar(char c) const;
private:
  void newToken(void);
  void pushTokenChar(char c);
  void endToken(void);
  void endTokenWithEof(void);
public:
  SlkToken(FunctionScript::IScriptSource& source, FunctionScript::ErrorAndStringBuffer& errorAndStringBuffer);
private:
  FunctionScript::IScriptSource* mSource;
  FunctionScript::IScriptSource::Iterator mIterator;

  char* mCurToken;
  char* mLastToken;
  int mTokenCharIndex;

  FunctionScript::ErrorAndStringBuffer* mErrorAndStringBuffer;

  int mLineNumber;
  int mLastLineNumber;
  int mIsExternScript;
  int mCanFinish;

  const char* mWhiteSpaces;
  const char* mDelimiters;
  const char* mBeginParentheses;
  const char* mEndParentheses;
  const char* mOperators;
  const char* mSpecialChars;
};

#endif

