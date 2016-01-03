#ifndef __RuntimeBuilderData_H__
#define __RuntimeBuilderData_H__

#include "BaseType.h"
#include "Queue.h"

#define __SERVERVERSION__	0x0600110
#define STACKSIZE			255

namespace ScriptableData
{
  class Function;
  class Statement;
  class ScriptableDataFile;
  class Value;
}
class RuntimeBuilderData
{
public:
  static const INT s_c_MaxByteCodeLength = 4 * 1024;
  enum
  {
    UNKNOWN_TOKEN = -1,
    VARIABLE_TOKEN = 0,
    INT_TOKEN,
    FLOAT_TOKEN,
    STRING_TOKEN,
  };
  struct TokenInfo
  {
    CHAR*	mString;
    INT mType;

    TokenInfo(void) :mString(0), mType(UNKNOWN_TOKEN)
    {}
    TokenInfo(CHAR* pstr, INT type) :mString(pstr), mType(type)
    {}
    BOOL IsValid(void)const
    {
      if (UNKNOWN_TOKEN != mType)
        return TRUE;
      else
        return FALSE;
    }
    ScriptableData::Value ToValue(void)const;
  };
private:
  typedef DequeT<TokenInfo, STACKSIZE> TokenStack;
  typedef DequeT<ScriptableData::Statement*, STACKSIZE> SemanticStack;
public:
  void resetByteCode(void);
  void setByteCode(const CHAR* pByteCode, INT len);
  const CHAR* getByteCode(INT& len)const;
public:
  RuntimeBuilderData(void);
public:
  void push(const TokenInfo& info);
  TokenInfo pop(void);
  BOOL isSemanticStackEmpty(void)const;
  void pushStatement(ScriptableData::Statement* p);
  ScriptableData::Statement* popStatement(void);
  ScriptableData::Statement* getCurStatement(void)const;
  ScriptableData::Function*& getLastFunctionRef(void)const;
public:
  inline void genByteCode(CHAR data){ genByteCode(static_cast<UCHAR>(data)); }
  inline void genByteCode(SHORT data){ genByteCode(static_cast<USHORT>(data)); }
  inline void genByteCode(INT data){ genByteCode(static_cast<UINT>(data)); }
  inline void genByteCode(FLOAT data){ genByteCode(*reinterpret_cast<UINT*>(&data)); }
  inline void genByteCode(const UCHAR* data){ genByteCode(reinterpret_cast<const CHAR*>(data)); }
  void genByteCode(UCHAR data);
  void genByteCode(USHORT data);
  void genByteCode(UINT data);
  void genByteCode(const CHAR* data);
public:
  static const UINT s_c_PrimeRoll = 0xa3;
  static inline CHAR Encode(CHAR c, UINT seed)
  {
    static const UINT s_c_mod = 0xff;
    static const UINT s_c_roll = ((seed % s_c_PrimeRoll) == 0 ? (seed % s_c_PrimeRoll) : s_c_PrimeRoll);
    if (0 == c)
      return c;
    else {
      UINT b = (UINT)(s_c_mod + 1 - (BYTE)c);
      b = (UINT)((b - 1 + s_c_roll) % s_c_mod + 1);
      return (CHAR)(b & 0xff);
    }
  }
  static inline CHAR Decode(CHAR c, UINT seed)
  {
    static const UINT s_c_mod = 0xff;
    static const UINT s_c_roll = ((seed % s_c_PrimeRoll) == 0 ? (seed % s_c_PrimeRoll) : s_c_PrimeRoll);
    if (0 == c)
      return c;
    else {
      UINT b = (UINT)(BYTE)c;
      b = (UINT)((b - 1 + s_c_mod - s_c_roll) % s_c_mod + 1);
      b = (UINT)(s_c_mod + 1 - b);
      return (CHAR)(b & 0xff);
    }
  }
private:
  TokenStack		mTokenStack;
  SemanticStack	mSemanticStack;

  INT				mByteCodeLength;
  CHAR			mByteCode[s_c_MaxByteCodeLength];
};

#endif //__RuntimeBuilderData_H__