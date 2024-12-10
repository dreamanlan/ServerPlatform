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
    void resetByteCode();
    void setByteCode(const char* pByteCode, int len);
    const char* getByteCode(int& len)const;
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
public:
    inline void genByteCode(char data) { genByteCode(static_cast<unsigned char>(data)); }
    inline void genByteCode(short data) { genByteCode(static_cast<unsigned short>(data)); }
    inline void genByteCode(int data) { genByteCode(static_cast<unsigned int>(data)); }
    inline void genByteCode(float data) { genByteCode(*reinterpret_cast<unsigned int*>(&data)); }
    inline void genByteCode(const unsigned char* data) { genByteCode(reinterpret_cast<const char*>(data)); }
    void genByteCode(unsigned char data);
    void genByteCode(unsigned short data);
    void genByteCode(unsigned int data);
    void genByteCode(const char* data);
public:
    static const unsigned int s_c_PrimeRoll = 0xa3;
    static inline char Encode(char c, unsigned int seed)
    {
        static const unsigned int s_c_mod = 0xff;
        static const unsigned int s_c_roll = ((seed % s_c_PrimeRoll) == 0 ? (seed % s_c_PrimeRoll) : s_c_PrimeRoll);
        if (0 == c)
            return c;
        else {
            unsigned int b = (unsigned int)(s_c_mod + 1 - (unsigned char)c);
            b = (unsigned int)((b - 1 + s_c_roll) % s_c_mod + 1);
            return (char)(b & 0xff);
        }
    }
    static inline char Decode(char c, unsigned int seed)
    {
        static const unsigned int s_c_mod = 0xff;
        static const unsigned int s_c_roll = ((seed % s_c_PrimeRoll) == 0 ? (seed % s_c_PrimeRoll) : s_c_PrimeRoll);
        if (0 == c)
            return c;
        else {
            unsigned int b = (unsigned int)(unsigned char)c;
            b = (unsigned int)((b - 1 + s_c_mod - s_c_roll) % s_c_mod + 1);
            b = (unsigned int)(s_c_mod + 1 - b);
            return (char)(b & 0xff);
        }
    }
private:
    TokenStack mTokenStack;
    SemanticStack mSemanticStack;
    PairTypeStack   mPairTypeStack;

    int mByteCodeLength;
    char mByteCode[s_c_MaxByteCodeLength];
public:
    static FunctionData*& GetNullFunctionPtrRef()
    {
        static FunctionData* s_Ptr = 0;
        return s_Ptr;
    }
};
#endif //__RuntimeBuilderData_H__