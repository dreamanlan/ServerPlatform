/*************************************************************************

    SlkAction.h

*************************************************************************/

#ifndef _SLKACTION_H
#define _SLKACTION_H

#include "Type.h"

class SlkAction
{
public:
	virtual ~SlkAction(void){}
public:
	virtual void predict(unsigned short entry, unsigned short production, unsigned short token, int level, const char* lastTok, int lastLine, const char* curTok, int curLine){}
	virtual void execute(int number)=0;
};

#endif

