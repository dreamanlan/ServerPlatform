/**************************************************************************

    SlkError.h

**************************************************************************/

#ifndef _SLKERROR_H
#define _SLKERROR_H

#include "Type.h"

namespace FunctionScript
{
    class ErrorAndStringBuffer;
}
class SlkToken;
class SlkError
{
public:
  SlkError(FunctionScript::ErrorAndStringBuffer& errorBuffer);
  short mismatch(short symbol, short token, SlkToken& tokens);
  short no_entry(short entry, short nonterminal, short token, int level, SlkToken& tokens);
  void input_left(SlkToken& tokens);
private:
    FunctionScript::ErrorAndStringBuffer* mErrorBuffer;
};

#endif

