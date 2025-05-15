#include "ByteCodeScript.h"
#include "ByteCode.h"

//--------------------------------------------------------------------------------------
class ActionForByteCodeScript : public RuntimeBuilderT<ActionForByteCodeScript>
{
    static const int s_c_MaxStringSize = 4 * 1024;
    using BaseType = RuntimeBuilderT<ActionForByteCodeScript>;
public:
    inline int setLastToken(const char* token)
    {
        int size = 0;
        if (NULL != mInterpreter && NULL != token) {
            size = (int)strlen(token);
            if (size > 0) {
                char* pStr = mInterpreter->AllocString(size);
                if (NULL != pStr) {
                    for (int i = 0; i < size; ++i) {
                        pStr[i] = RuntimeBuilderData::Decode(token[i], __SERVERVERSION__);
                    }
                    pStr[size] = 0;
                }
            }
        }
        return size;
    }
    inline char* getLastToken() const { return m_LastToken; }
    inline void setLastLineNumber(int number) { m_LastNumber = number; }
    inline int getLastLineNumber() const { return m_LastNumber; }
    inline void setCanFinish(int val) { val; }
    inline void setStringDelimiter(const char* begin, const char* end) { begin, end; }
    inline void setScriptDelimiter(const char* begin, const char* end) { begin, end; }
    inline int pushToken(int type, const char* val)
    {
        int size = 0;
        if (NULL != mInterpreter && NULL != val) {
            size = (int)strlen(val);
            if (size > 0) {
                char* pStr = mInterpreter->AllocString(size);
                if (NULL != pStr) {
                    for (int i = 0; i < size; ++i) {
                        pStr[i] = RuntimeBuilderData::Decode(val[i], __SERVERVERSION__);
                    }
                    pStr[size] = 0;
                    mData.push(RuntimeBuilderData::TokenInfo(pStr, type));
                }
            }
        }
        return size;
    }
    inline void pushToken(int type, int val)
    {
        mData.push(RuntimeBuilderData::TokenInfo(val, type));
    }
    inline void pushToken(float val)
    {
        mData.push(RuntimeBuilderData::TokenInfo(val));
    }
    inline void pushToken(bool val)
    {
        mData.push(RuntimeBuilderData::TokenInfo(val));
    }
public:
    ActionForByteCodeScript(Interpreter& interpreter) :BaseType(interpreter), m_LastToken(NULL), m_LastNumber(0)
    {
        setEnvironmentObjRef(*this);
    }
private:
    char* m_LastToken;
    int		m_LastNumber;
};
//--------------------------------------------------------------------------------------

namespace FunctionScript
{
    void ByteCodeScript::Parse(const char* buf, int size)
    {
        ActionForByteCodeScript action(m_Interpreter);
        for (int ix = 0; ix < size;) {
            unsigned char c = static_cast<unsigned char>(RuntimeBuilderData::Decode(buf[ix], __SERVERVERSION__));
            ix += sizeof(c);
            switch (static_cast<SimpleScriptByteCodeEnum>(c)) {
            case SimpleScriptByteCodeEnum::BYTE_CODE_SET_LAST_TOKEN:
            {
                const char* pStr = buf + ix;
                int tokenSize = action.setLastToken(pStr);
                ix += tokenSize + 1;
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_SET_LAST_LINE_NUMBER:
            {
                int lineNumber = *reinterpret_cast<const int*>(buf + ix);
                for (int byteIndex = 0; byteIndex < sizeof(lineNumber); ++byteIndex) {
                    reinterpret_cast<char*>(&lineNumber)[byteIndex] = RuntimeBuilderData::Decode(reinterpret_cast<char*>(&lineNumber)[byteIndex], __SERVERVERSION__);
                }
                ix += sizeof(lineNumber);
                action.setLastLineNumber(lineNumber);
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_PUSH_TOKEN:
            {
                unsigned char type = static_cast<unsigned char>(RuntimeBuilderData::Decode(*(buf + ix), __SERVERVERSION__));
                ix += sizeof(type);
                switch (type) {
                case RuntimeBuilderData::STRING_TOKEN:
                case RuntimeBuilderData::DOLLAR_STRING_TOKEN:
                case RuntimeBuilderData::VARIABLE_TOKEN:
                {
                    const char* pStr = buf + ix;
                    int tokenSize = action.pushToken(type, pStr);
                    ix += tokenSize + 1;
                }
                break;
                case RuntimeBuilderData::INT_TOKEN:
                {
                    int val = *reinterpret_cast<const int*>(buf + ix);
                    for (int byteIndex = 0; byteIndex < sizeof(val); ++byteIndex) {
                        reinterpret_cast<char*>(&val)[byteIndex] = RuntimeBuilderData::Decode(reinterpret_cast<char*>(&val)[byteIndex], __SERVERVERSION__);
                    }
                    ix += sizeof(val);
                    action.pushToken(type, val);
                }
                break;
                case RuntimeBuilderData::FLOAT_TOKEN:
                {
                    float val = *reinterpret_cast<const float*>(buf + ix);
                    for (int byteIndex = 0; byteIndex < sizeof(val); ++byteIndex) {
                        reinterpret_cast<char*>(&val)[byteIndex] = RuntimeBuilderData::Decode(reinterpret_cast<char*>(&val)[byteIndex], __SERVERVERSION__);
                    }
                    ix += sizeof(val);
                    action.pushToken(val);
                }
                break;
                case RuntimeBuilderData::BOOL_TOKEN:
                {
                    bool val = *reinterpret_cast<const bool*>(buf + ix);
                    for (int byteIndex = 0; byteIndex < sizeof(val); ++byteIndex) {
                        reinterpret_cast<char*>(&val)[byteIndex] = RuntimeBuilderData::Decode(reinterpret_cast<char*>(&val)[byteIndex], __SERVERVERSION__);
                    }
                    ix += sizeof(val);
                    action.pushToken(val);
                }
                break;
                }
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PERIOD_PARAM:
            {
                action.markPeriodParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_PARAM:
            {
                action.markBracketParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_HIGHORDER_FUNCTION:
            {
                action.buildHighOrderFunction();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESES_PARAM:
            {
                action.markParenthesesParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_SET_EXTERN_SCRIPT:
            {
                action.setExternScript();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_STATEMENT:
            {
                action.markStatement();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_EXTERN_SCRIPT:
            {
                action.markExternScript();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_COLON_PARAM:
            {
                action.markBracketColonParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESES_COLON_PARAM:
            {
                action.markParenthesesColonParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_ANGLE_BRACKET_COLON_PARAM:
            {
                action.markAngleBracketColonParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PARENTHESES_PERCENT_PARAM:
            {
                action.markParenthesesPercentParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACKET_PERCENT_PARAM:
            {
                action.markBracketPercentParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_BRACE_PERCENT_PARAM:
            {
                action.markBracePercentParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_ANGLE_BRACKET_PERCENT_PARAM:
            {
                action.markAngleBracketPercentParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_COLON_COLON_PARAM:
            {
                action.markColonColonParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_SET_FUNCTION_ID:
            {
                action.setFunctionId();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_ADD_FUNCTION:
            {
                action.addFunction();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BEGIN_STATEMENT:
            {
                action.beginStatement();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_END_STATEMENT:
            {
                action.endStatement();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_OPERATOR:
            {
                action.buildOperator();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_NULLABLE_OPERATOR:
            {
                action.buildNullableOperator();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_FIRST_TERNARY_OPERATOR:
            {
                action.buildFirstTernaryOperator();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_BUILD_SECOND_TERNARY_OPERATOR:
            {
                action.buildSecondTernaryOperator();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_POINTER_PARAM:
            {
                action.markPointerParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_PERIOD_STAR_PARAM:
            {
                action.markPeriodStarParam();
            }
            break;
            case SimpleScriptByteCodeEnum::BYTE_CODE_MARK_POINTER_STAR_PARAM:
            {
                action.markPointerStarParam();
            }
            break;
            };
        }

        m_Interpreter.PrepareRuntimeObject();
    }
}