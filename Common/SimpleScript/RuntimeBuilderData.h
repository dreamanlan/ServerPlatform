#ifndef __RuntimeBuilderData_H__
#define __RuntimeBuilderData_H__

#include "Type.h"
#include "Queue.h"
#include "calc.h"

#define __SERVERVERSION__	0x0600110
#define STACKSIZE			255

namespace FunctionScript
{
    class FunctionData;
    class StatementData;
    class Interpreter;
    class Value;
}
class RuntimeBuilderData
{
public:
    static const int s_c_MaxByteCodeLength = 4 * 1024;
    enum
    {
        UNKNOWN_TOKEN = -1,
        VARIABLE_TOKEN = 0,
        INT_TOKEN,
        FLOAT_TOKEN,
        STRING_TOKEN,
        DOLLAR_STRING_TOKEN,
        BOOL_TOKEN,
    };
    struct TokenInfo
    {
        union
        {
            char* mString;
            int mInteger;
            float mFloat;
            bool mBool;
        };
        int mType;

        TokenInfo() :mString(0), mType(UNKNOWN_TOKEN)
        {}
        TokenInfo(char* pstr, int type) :mString(pstr), mType(type)
        {}
        TokenInfo(int val, int type) :mInteger(val), mType(type)
        {}
        TokenInfo(float val) :mFloat(val), mType(FLOAT_TOKEN)
        {}
        TokenInfo(bool val) :mBool(val), mType(BOOL_TOKEN)
        {}
        int IsValid()const
        {
            if (UNKNOWN_TOKEN != mType)
                return TRUE;
            else
                return FALSE;
        }
        FunctionScript::Value ToValue()const;
    };
private:
    using TokenStack = DequeT<TokenInfo, STACKSIZE>;
    using SemanticStack = DequeT<FunctionScript::StatementData*, STACKSIZE>;
    using PairTypeStack = DequeT<uint32_t, STACKSIZE>;
public:
    RuntimeBuilderData();
private:
    RuntimeBuilderData(const RuntimeBuilderData& other) = delete;
    RuntimeBuilderData& operator=(const RuntimeBuilderData& other) = delete;
    RuntimeBuilderData(const RuntimeBuilderData&& other) = delete;
    RuntimeBuilderData& operator=(const RuntimeBuilderData&& other) = delete;
public:
    void push(const TokenInfo& info);
    TokenInfo pop();
    int isSemanticStackEmpty()const;
    void pushStatement(FunctionScript::StatementData* p);
    FunctionScript::StatementData* popStatement();
    FunctionScript::StatementData* getCurStatement();
    FunctionScript::FunctionData*& getLastFunctionRef();
    uint32_t peekPairType()const;
    void pushPairType(uint32_t pairType);
    uint32_t popPairType();
    const PairTypeStack& getPairTypeStack()const;
    PairTypeStack& getPairTypeStack();
private:
    TokenStack mTokenStack;
    SemanticStack mSemanticStack;
    PairTypeStack   mPairTypeStack;
public:
    static FunctionData*& GetNullFunctionPtrRef()
    {
        static FunctionData* s_Ptr = 0;
        return s_Ptr;
    }
};
#endif //__RuntimeBuilderData_H__