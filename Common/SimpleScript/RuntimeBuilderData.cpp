#include "RuntimeBuilderData.h"
#include "calc.h"

#define BYTECODE_PRINTF	nullprintf

//--------------------------------------------------------------------------------------
FunctionScript::Value RuntimeBuilderData::TokenInfo::ToValue()const
{
    switch (mType) {
    case VARIABLE_TOKEN:
    {
        char* pStr = mString;
        if (0 != pStr) {
            Value val(pStr, Value::TYPE_IDENTIFIER);
            return val;
        }
    }
    break;
    case STRING_TOKEN:
    {
        char* pStr = mString;
        if (0 != pStr) {
            Value val(pStr, Value::TYPE_STRING);
            return val;
        }
    }
    break;
    case DOLLAR_STRING_TOKEN:
    {
        char* pStr = mString;
        if (0 != pStr) {
            Value val(pStr, Value::TYPE_DOLLAR_STRING);
            return val;
        }
    }
    break;
    case INT_TOKEN:
    {
        Value val(mInteger);
        return val;
    }
    break;
    case FLOAT_TOKEN:
    {
        Value val(mFloat);
        return val;
    }
    break;
    case BOOL_TOKEN:
    {
        Value val(mBool);
        return val;
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
RuntimeBuilderData::TokenInfo RuntimeBuilderData::pop()
{
    if (mTokenStack.Empty())
        return TokenInfo();
    TokenInfo info = mTokenStack.Back();
    mTokenStack.PopBack();
    return info;
}
int RuntimeBuilderData::isSemanticStackEmpty()const
{
    return mSemanticStack.Empty();
}
void RuntimeBuilderData::pushStatement(FunctionScript::StatementData* p)
{
    mSemanticStack.PushBack(p);
}
FunctionScript::StatementData* RuntimeBuilderData::popStatement()
{
    if (mSemanticStack.Empty()) {
        return 0;
    }
    StatementData* cdata = mSemanticStack.Back();
    mSemanticStack.PopBack();
    return cdata;
}
FunctionScript::StatementData* RuntimeBuilderData::getCurStatement()
{
    if (mSemanticStack.Empty())
        return 0;
    StatementData* topData = mSemanticStack.Back();
    return topData;
}
FunctionScript::FunctionData*& RuntimeBuilderData::getLastFunctionRef()
{
    StatementData* statement = getCurStatement();
    if (0 != statement)
        return statement->GetLastFunctionRef();
    else
        return GetNullFunctionPtrRef();
}
uint32_t RuntimeBuilderData::peekPairType()const
{
    if (mPairTypeStack.Empty())
        return FunctionData::PAIR_TYPE_NONE;
    return mPairTypeStack.Back();
}
void RuntimeBuilderData::pushPairType(uint32_t pairType)
{
    mPairTypeStack.PushBack(pairType);
}
uint32_t RuntimeBuilderData::popPairType()
{
    if (mPairTypeStack.Empty())
        return FunctionData::PAIR_TYPE_NONE;
    return mPairTypeStack.PopBack();
}
const RuntimeBuilderData::PairTypeStack& RuntimeBuilderData::getPairTypeStack()const
{
    return mPairTypeStack;
}
RuntimeBuilderData::PairTypeStack& RuntimeBuilderData::getPairTypeStack()
{
    return mPairTypeStack;
}
//--------------------------------------------------------------------------------------
RuntimeBuilderData::RuntimeBuilderData()
{
}