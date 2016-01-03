#ifndef __ByteCodeScript_H__
#define __ByteCodeScript_H__

#include "ScriptableData.h"

namespace ScriptableData
{
  void ParseBinary(const CHAR* buf, INT size, ScriptableDataFile& file);
}

using namespace ScriptableData;

#endif //__ByteCodeScript_H__