#include "RuntimeBuilderData.h"
#include "ScriptableData.h"

#define BYTECODE_PRINTF	nullprintf

//--------------------------------------------------------------------------------------
ScriptableData::Value RuntimeBuilderData::TokenInfo::ToValue(void)const
{
  switch (mType) {
  case VARIABLE_TOKEN:
  {
    CHAR* pStr = mString;
    if (0 != pStr) {
      Value val(pStr, Value::TYPE_VARIABLE_NAME);
      return val;
    }
  }
    break;
  case STRING_TOKEN:
  {
    CHAR* pStr = mString;
    if (0 != pStr) {
      Value val(pStr, Value::TYPE_STRING);
      return val;
    }
  }
    break;
  case INT_TOKEN:
  {
    CHAR* pStr = mString;
    if (0 != pStr) {
      Value val(pStr, Value::TYPE_INT);
      return val;
    }
  }
    break;
  case FLOAT_TOKEN:
  {
    CHAR* pStr = mString;
    if (0 != pStr) {
      Value val(pStr, Value::TYPE_FLOAT);
      return val;
    }
  }
    break;
  }
  return Value();
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::push(const TokenInfo& info)
{
  mTokenStack.PushBack(info);
}
RuntimeBuilderData::TokenInfo RuntimeBuilderData::pop(void)
{
  if (mTokenStack.Empty())
    return TokenInfo();
  TokenInfo info = mTokenStack.Back();
  mTokenStack.PopBack();
  return info;
}
BOOL RuntimeBuilderData::isSemanticStackEmpty(void)const
{
  return mSemanticStack.Empty();
}
void RuntimeBuilderData::pushStatement(ScriptableData::Statement* p)
{
  mSemanticStack.PushBack(p);
}
ScriptableData::Statement* RuntimeBuilderData::popStatement(void)
{
  if (mSemanticStack.Empty()) {
    return 0;
  }
  Statement* cdata = mSemanticStack.Back();
  mSemanticStack.PopBack();
  return cdata;
}
ScriptableData::Statement* RuntimeBuilderData::getCurStatement(void)const
{
  if (mSemanticStack.Empty())
    return 0;
  Statement* topData = mSemanticStack.Back();
  return topData;
}
ScriptableData::Function*& RuntimeBuilderData::getLastFunctionRef(void)const
{
  Statement* statement = getCurStatement();
  if (0 != statement)
    return statement->GetLastFunctionRef();
  else
    return Function::GetNullFunctionPtrRef();
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::genByteCode(UCHAR data)
{
  BYTECODE_PRINTF("[\\x%2.2X]", data);
  if (mByteCodeLength + sizeof(data) <= s_c_MaxByteCodeLength) {
    *reinterpret_cast<UCHAR*>(&mByteCode[mByteCodeLength]) = data;
    mByteCode[mByteCodeLength] = Encode(mByteCode[mByteCodeLength], __SERVERVERSION__);
    mByteCodeLength += sizeof(data);
  }
}
void RuntimeBuilderData::genByteCode(USHORT data)
{
  BYTECODE_PRINTF("[\\x%2.2X\\x%2.2X]", (data & 0xff), ((data & 0xff00) >> 8));
  if (mByteCodeLength + sizeof(data) <= s_c_MaxByteCodeLength) {
    *reinterpret_cast<USHORT*>(&mByteCode[mByteCodeLength]) = data;
    for (INT i = mByteCodeLength; i < static_cast<INT>(mByteCodeLength + sizeof(data)); ++i) {
      mByteCode[i] = Encode(mByteCode[i], __SERVERVERSION__);
    }
    mByteCodeLength += sizeof(data);
  }
}
void RuntimeBuilderData::genByteCode(UINT data)
{
  BYTECODE_PRINTF("[\\x%2.2X\\x%2.2X\\x%2.2X\\x%2.2X]", (data & 0xff), ((data & 0xff00) >> 8), ((data & 0xff0000) >> 16), ((data & 0xff000000) >> 24));
  if (mByteCodeLength + sizeof(data) <= s_c_MaxByteCodeLength) {
    *reinterpret_cast<UINT*>(&mByteCode[mByteCodeLength]) = data;
    for (INT i = mByteCodeLength; i < static_cast<INT>(mByteCodeLength + sizeof(data)); ++i) {
      mByteCode[i] = Encode(mByteCode[i], __SERVERVERSION__);
    }
    mByteCodeLength += sizeof(data);
  }
}
void RuntimeBuilderData::genByteCode(const CHAR* data)
{
  if (NULL == data)
    return;
  BYTECODE_PRINTF("[%s]", data);
  size_t len = strlen(data) + 1;
  if (mByteCodeLength + len <= s_c_MaxByteCodeLength) {
    for (INT i = mByteCodeLength; i < static_cast<INT>(mByteCodeLength + static_cast<INT>(len)); ++i) {
      mByteCode[i] = Encode(data[i - mByteCodeLength], __SERVERVERSION__);
    }
    mByteCodeLength += (INT)len;
  }
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::resetByteCode(void)
{
  mByteCodeLength = 0;
  memset(mByteCode, 0, sizeof(mByteCode));
}
void RuntimeBuilderData::setByteCode(const CHAR* pByteCode, INT len)
{
  if (NULL != pByteCode && len <= s_c_MaxByteCodeLength) {
    memcpy(mByteCode, pByteCode, len);
    mByteCodeLength = len;
  }
}
const CHAR* RuntimeBuilderData::getByteCode(INT& len)const
{
  len = mByteCodeLength; return mByteCode;
}
//--------------------------------------------------------------------------------------
RuntimeBuilderData::RuntimeBuilderData(void)
{
  resetByteCode();
}