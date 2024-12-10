/*****************************************************************************

calc.h

******************************************************************************/

#ifndef _CALC_H
#define _CALC_H

#include "Type.h"
#include "Queue.h"
#include "Hashtable.h"
#include <cstdint>
#include <new>
#include <string>
#include <vector>
#include <unordered_map>

class ActionForSourceCodeScript;
namespace FunctionScript
{
    template<typename DestT>
    struct ReinterpretCast
    {
        template<typename SrcT>
        static DestT From(const SrcT& v)
        {
            union
            {
                SrcT	m_Src;
                DestT	m_Dest;
            } tmp;
            tmp.m_Src = v;
            return tmp.m_Dest;
        }
    };

    typedef enum
    {
        EXECUTE_RESULT_NORMAL = 0,
        EXECUTE_RESULT_RETURN,
        EXECUTE_RESULT_BREAK,
        EXECUTE_RESULT_CONTINUE,
        EXECUTE_RESULT_GOTO,
    }ExecuteResultEnum;

    enum
    {
        MAX_ERROR_INFO_CAPACITY = 256,
        MAX_RECORD_ERROR_NUM = 16,
        MAX_TOKEN_NAME_SIZE = 32,
        MAX_NUMBER_STRING_SIZE = 32,
        MAX_FORM_NUM_PER_STATEMENT = 8,
        MAX_FUNCTION_LEVEL = 8,
        MAX_FUNCTION_PARAM_NUM = 32,
        MAX_STACK_LEVEL = 1024,

        INIT_FUNCTION_PARAM = 1,
        DELTA_FUNCTION_PARAM = 2,
        DELTA_FUNCTION_STATEMENT = 16,
        INIT_STATEMENT_FUNCTION = 1,
        DELTA_STATEMENT_FUNCTION = 1,
        INIT_FUNCTION_LOCAL = 1,
        DELTA_FUNCTION_LOCAL = 4,
    };

    enum
    {
        MAX_FUNCTION_DIMENSION_NUM = 8,
        MAX_LOCAL_NUM = 256,
        MAX_PARAM_NUM = 1024,
        MAX_PROGRAM_SIZE = 16 * 1024,
        MAX_INNER_FUNCTION_API_NUM = 256,
        MAX_INNER_STATEMENT_API_NUM = 64,
        MAX_STATEMENT_API_NUM = 256,
        MAX_PREDEFINED_VALUE_NUM = 1024,
        STACK_VALUE_POOL_SIZE = 16 * 1024,
        VALUE_POOL_SIZE = 16 * 1024,
        EXPRESSION_POOL_SIZE = 64 * 1024,
        STRING_BUFFER_SIZE = 1024 * 1024,
        SYNTAXCOMPONENT_POOL_SIZE = 16 * 1024,
    };

    class InterpreterOptions
    {
    public:
        InterpreterOptions() :
            m_MaxFunctionDimensionNum(MAX_FUNCTION_DIMENSION_NUM),
            m_MaxLocalNum(MAX_LOCAL_NUM),
            m_MaxParamNum(MAX_PARAM_NUM),
            m_MaxProgramSize(MAX_PROGRAM_SIZE),
            m_MaxInnerFunctionApiNum(MAX_INNER_FUNCTION_API_NUM),
            m_MaxInnerStatementApiNum(MAX_INNER_STATEMENT_API_NUM),
            m_MaxStatementApiNum(MAX_STATEMENT_API_NUM),
            m_MaxPredefinedValueNum(MAX_PREDEFINED_VALUE_NUM),
            m_StackValuePoolSize(STACK_VALUE_POOL_SIZE),
            m_ValuePoolSize(VALUE_POOL_SIZE),
            m_ExpressionPoolSize(EXPRESSION_POOL_SIZE),
            m_StringBufferSize(STRING_BUFFER_SIZE),
            m_SyntaxComponentPoolSize(SYNTAXCOMPONENT_POOL_SIZE)
        {
        }
    public:
        int GetMaxFunctionDimensionNum() const { return m_MaxFunctionDimensionNum; }
        void SetMaxFunctionDimensionNum(int val) { m_MaxFunctionDimensionNum = val; }
        int GetMaxLocalNum() const { return m_MaxLocalNum; }
        void SetMaxLocalNum(int val) { m_MaxLocalNum = val; }
        int GetMaxParamNum() const { return m_MaxParamNum; }
        void SetMaxParamNum(int val) { m_MaxParamNum = val; }
        int GetMaxProgramSize() const { return m_MaxProgramSize; }
        void SetMaxProgramSize(int val) { m_MaxProgramSize = val; }
        int GetMaxInnerFunctionApiNum() const { return m_MaxInnerFunctionApiNum; }
        void SetMaxInnerFunctionApiNum(int val) { m_MaxInnerFunctionApiNum = val; }
        int GetMaxInnerStatementApiNum() const { return m_MaxInnerStatementApiNum; }
        void SetMaxInnerStatementApiNum(int val) { m_MaxInnerStatementApiNum = val; }
        int GetMaxStatementApiNum() const { return m_MaxStatementApiNum; }
        void SetMaxStatementApiNum(int val) { m_MaxStatementApiNum = val; }
        int GetMaxPredefinedValueNum() const { return m_MaxPredefinedValueNum; }
        void SetMaxPredefinedValueNum(int val) { m_MaxPredefinedValueNum = val; }
        int GetStackValuePoolSize() const { return m_StackValuePoolSize; }
        void SetStackValuePoolSize(int val) { m_StackValuePoolSize = val; }
        int GetValuePoolSize() const { return m_ValuePoolSize; }
        void SetValuePoolSize(int val) { m_ValuePoolSize = val; }
        int GetExpressionPoolSize() const { return m_ExpressionPoolSize; }
        void SetExpressionPoolSize(int val) { m_ExpressionPoolSize = val; }
        int GetStringBufferSize() const { return m_StringBufferSize; }
        void SetStringBufferSize(int val) { m_StringBufferSize = val; }
        int GetSyntaxComponentPoolSize() const { return m_SyntaxComponentPoolSize; }
        void SetSyntaxComponentPoolSize(int val) { m_SyntaxComponentPoolSize = val; }
    private:
        int	m_MaxFunctionDimensionNum;
        int	m_MaxLocalNum;
        int	m_MaxParamNum;
        int	m_MaxProgramSize;
        int	m_MaxInnerFunctionApiNum;
        int	m_MaxInnerStatementApiNum;
        int	m_MaxStatementApiNum;
        int m_MaxPredefinedValueNum;
        int	m_StackValuePoolSize;
        int	m_ValuePoolSize;
        int	m_ExpressionPoolSize;
        int	m_StringBufferSize;
        int m_SyntaxComponentPoolSize;
    };

    class ExpressionApi;
    class StatementApi;
    class FunctionData;
    class Value
    {
    public:
        enum
        {
            TYPE_INVALID = -1,
            TYPE_INT = 0,
            TYPE_UINT,
            TYPE_INT64,
            TYPE_UINT64,
            TYPE_FLOAT,
            TYPE_DOUBLE,
            TYPE_BOOL,
            TYPE_STRING,
            TYPE_DOLLAR_STRING,
            TYPE_ALLOC_STRING,
            TYPE_ALLOC_DOLLAR_STRING,
            TYPE_IDENTIFIER,
            TYPE_ARG_INDEX,
            TYPE_LOCAL_INDEX,
            TYPE_INDEX,
            TYPE_EXPRESSION_API,
            TYPE_STATEMENT_API,
            TYPE_FUNCTION,
            TYPE_PTR,
        };

        Value() :m_Type(TYPE_INVALID), m_Int64Val(0), m_Line(0) {}
        explicit Value(int val) :m_Type(TYPE_INT), m_IntVal(val), m_Line(0) {}
        explicit Value(unsigned int val) :m_Type(TYPE_UINT), m_IntVal(static_cast<int>(val)), m_Line(0) {}
        explicit Value(long long val) :m_Type(TYPE_INT64), m_Int64Val(val), m_Line(0) {}
        explicit Value(unsigned long long val) :m_Type(TYPE_UINT64), m_Int64Val(static_cast<long long>(val)), m_Line(0) {}
        explicit Value(float val) :m_Type(TYPE_FLOAT), m_FloatVal(val), m_Line(0) {}
        explicit Value(double val) :m_Type(TYPE_DOUBLE), m_DoubleVal(val), m_Line(0) {}
        explicit Value(bool val) :m_Type(TYPE_BOOL), m_BoolVal(val), m_Line(0) {}
        explicit Value(char* val) :m_Type(TYPE_STRING), m_StringVal(val), m_Line(0) {}
        explicit Value(const char* val) :m_Type(TYPE_STRING), m_ConstStringVal(val), m_Line(0) {}
        explicit Value(ExpressionApi* val) :m_Type(TYPE_EXPRESSION_API), m_ExpressionApiVal(val), m_Line(0) {}
        explicit Value(StatementApi* val) :m_Type(TYPE_STATEMENT_API), m_StatementApiVal(val), m_Line(0) {}
        explicit Value(FunctionData* val) :m_Type(TYPE_FUNCTION), m_FunctionVal(val), m_Line(0) {}
        explicit Value(void* val) :m_Type(TYPE_PTR), m_Ptr(val), m_Line(0) {}
        explicit Value(int val, int type) :m_Type(type), m_IntVal(val), m_Line(0) {}
        explicit Value(long long val, int type) :m_Type(type), m_Int64Val(val), m_Line(0) {}
        explicit Value(char* val, int type) :m_Type(type), m_StringVal(val), m_Line(0) {}
        explicit Value(const char* val, int type) :m_Type(type), m_ConstStringVal(val), m_Line(0) {}
        ~Value()
        {
            FreeString();
        }
        Value(const Value& other) :m_Type(TYPE_INVALID), m_Int64Val(0), m_Line(0)
        {
            CopyFrom(other);
        }
        Value& operator=(const Value& other)
        {
            if (this == &other)
                return *this;
            FreeString();
            CopyFrom(other);
            return *this;
        }
        int InitString(int len)
        {
            char* p = new char[len + 1];
            if (NULL != p) {
                m_StringVal = p;
                m_Type = TYPE_ALLOC_STRING;
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        void InitString(const char* pstr)
        {
            int len = (int)strlen(pstr);
            if (InitString(len)) {
                strcpy(m_StringVal, pstr);
            }
        }
        int InitDollarString(int len)
        {
            char* p = new char[len + 1];
            if (NULL != p) {
                m_StringVal = p;
                m_Type = TYPE_ALLOC_DOLLAR_STRING;
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        void InitDollarString(const char* pstr)
        {
            int len = (int)strlen(pstr);
            if (InitDollarString(len)) {
                strcpy(m_StringVal, pstr);
            }
        }

        int IsInvalid()const { return (m_Type == TYPE_INVALID ? TRUE : FALSE); }
        int IsInt()const { return (m_Type == TYPE_INT ? TRUE : FALSE); }
        int IsUInt()const { return (m_Type == TYPE_UINT ? TRUE : FALSE); }
        int IsInt64()const { return (m_Type == TYPE_INT64 ? TRUE : FALSE); }
        int IsUInt64()const { return (m_Type == TYPE_UINT64 ? TRUE : FALSE); }
        int IsFloat()const { return (m_Type == TYPE_FLOAT ? TRUE : FALSE); }
        int IsDouble()const { return (m_Type == TYPE_DOUBLE ? TRUE : FALSE); }
        int IsBool()const { return (m_Type == TYPE_BOOL ? TRUE : FALSE); }
        int IsString()const { return ((m_Type == TYPE_STRING || m_Type == TYPE_ALLOC_STRING) ? TRUE : FALSE); }
        int IsDollarString()const { return ((m_Type == TYPE_DOLLAR_STRING || m_Type == TYPE_ALLOC_DOLLAR_STRING) ? TRUE : FALSE); }
        int IsIdentifier()const { return (m_Type == TYPE_IDENTIFIER ? TRUE : FALSE); }
        int IsArgIndex()const { return (m_Type == TYPE_ARG_INDEX ? TRUE : FALSE); }
        int IsLocalIndex()const { return (m_Type == TYPE_LOCAL_INDEX ? TRUE : FALSE); }
        int IsIndex()const { return (m_Type == TYPE_INDEX ? TRUE : FALSE); }
        int IsExpressionApi()const { return (m_Type == TYPE_EXPRESSION_API ? TRUE : FALSE); }
        int IsStatementApi()const { return (m_Type == TYPE_STATEMENT_API ? TRUE : FALSE); }
        int IsFunction()const { return (m_Type == TYPE_FUNCTION ? TRUE : FALSE); }
        int IsPtr()const { return (m_Type == TYPE_PTR ? TRUE : FALSE); }
        int GetLine()const { return m_Line; }
        int GetType()const { return m_Type; }
        int GetInt()const { return m_IntVal; }
        long long GetInt64()const { return m_Int64Val; }
        float GetFloat()const { return m_FloatVal; }
        double GetDouble()const { return m_DoubleVal; }
        bool GetBool()const { return m_BoolVal; }
        char* GetString()const { return m_StringVal; }
        ExpressionApi* GetExpressionApi()const { return m_ExpressionApiVal; }
        StatementApi* GetStatementApi()const { return m_StatementApiVal; }
        FunctionData* GetFunction()const { return m_FunctionVal; }
        void* GetPtr()const { return m_Ptr; }
        void SetLine(int line)
        {
            m_Line = line;
        }
        void SetInvalid()
        {
            FreeString();
            m_Type = TYPE_INVALID;
            m_Int64Val = 0;
        }
        void SetInt(int val)
        {
            FreeString();
            m_Type = TYPE_INT;
            m_IntVal = val;
        }
        void SetUInt(unsigned int val)
        {
            FreeString();
            m_Type = TYPE_INT;
            m_IntVal = static_cast<int>(val);
        }
        void SetInt64(long long val)
        {
            FreeString();
            m_Type = TYPE_INT64;
            m_Int64Val = val;
        }
        void SetUInt64(unsigned long long val)
        {
            FreeString();
            m_Type = TYPE_INT64;
            m_Int64Val = static_cast<long long>(val);
        }
        void SetFloat(float val)
        {
            FreeString();
            m_Type = TYPE_FLOAT;
            m_FloatVal = val;
        }
        void SetDouble(double val)
        {
            FreeString();
            m_Type = TYPE_DOUBLE;
            m_DoubleVal = val;
        }
        void SetBool(bool val)
        {
            FreeString();
            m_Type = TYPE_BOOL;
            m_BoolVal = val;
        }
        void SetExpressionApi(ExpressionApi* expApi)
        {
            FreeString();
            m_Type = TYPE_EXPRESSION_API;
            m_ExpressionApiVal = expApi;
        }
        void SetStatementApi(StatementApi* statementApi)
        {
            FreeString();
            m_Type = TYPE_STATEMENT_API;
            m_StatementApiVal = statementApi;
        }
        void SetFunction(FunctionData* funcData)
        {
            FreeString();
            m_Type = TYPE_FUNCTION;
            m_FunctionVal = funcData;
        }
        void SetPtr(void* ptr)
        {
            FreeString();
            m_Type = TYPE_PTR;
            m_Ptr = ptr;
        }
        void SetIndex(int index)
        {
            FreeString();
            m_Type = TYPE_INDEX;
            m_IntVal = index;
        }
        void SetLocalIndex(int index)
        {
            FreeString();
            m_Type = TYPE_LOCAL_INDEX;
            m_IntVal = index;
        }
        void SetArgIndex(int index)
        {
            FreeString();
            m_Type = TYPE_ARG_INDEX;
            m_IntVal = index;
        }
        void SetIdentifier(char* name)
        {
            FreeString();
            m_Type = TYPE_IDENTIFIER;
            m_StringVal = name;
        }
        void SetWeakRefString(char* pstr)
        {
            FreeString();
            m_Type = TYPE_STRING;
            m_StringVal = pstr;
        }
        void SetWeakRefConstString(const char* pstr)
        {
            FreeString();
            m_Type = TYPE_STRING;
            m_ConstStringVal = pstr;
        }
        int AllocString(int len)
        {
            char* p = new char[len + 1];
            if (NULL != p) {
                FreeString();
                m_StringVal = p;
                m_Type = TYPE_ALLOC_STRING;
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        void AllocString(const char* pstr)
        {
            int len = (int)strlen(pstr);
            if (AllocString(len)) {
                strcpy(m_StringVal, pstr);
            }
        }
        void SetWeakRefDollarString(char* pstr)
        {
            FreeString();
            m_Type = TYPE_DOLLAR_STRING;
            m_StringVal = pstr;
        }
        void SetWeakRefConstDollarString(const char* pstr)
        {
            FreeString();
            m_Type = TYPE_DOLLAR_STRING;
            m_ConstStringVal = pstr;
        }
        int AllocDollarString(int len)
        {
            char* p = new char[len + 1];
            if (NULL != p) {
                FreeString();
                m_StringVal = p;
                m_Type = TYPE_ALLOC_DOLLAR_STRING;
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        void AllocDollarString(const char* pstr)
        {
            int len = (int)strlen(pstr);
            if (AllocDollarString(len)) {
                strcpy(m_StringVal, pstr);
            }
        }
        int ToInt()const
        {
            switch (m_Type) {
            case TYPE_INT:
            case TYPE_ARG_INDEX:
            case TYPE_LOCAL_INDEX:
            case TYPE_INDEX:
                return m_IntVal;
            case TYPE_INT64:
                return (int)m_Int64Val;
            case TYPE_FLOAT:
                return (int)m_FloatVal;
            case TYPE_DOUBLE:
                return (int)m_DoubleVal;
            case TYPE_BOOL:
                return m_BoolVal ? 1 : 0;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                int val = 0;
                if (m_StringVal) {
                    val = atoi(m_StringVal);
                }
                return val;
            }
            default:
                return 0;
            }
        }
        long long ToInt64()const
        {
            switch (m_Type) {
            case TYPE_INT:
            case TYPE_ARG_INDEX:
            case TYPE_LOCAL_INDEX:
            case TYPE_INDEX:
                return (long long)m_IntVal;
            case TYPE_INT64:
                return m_Int64Val;
            case TYPE_FLOAT:
                return (long long)m_FloatVal;
            case TYPE_DOUBLE:
                return (long long)m_DoubleVal;
            case TYPE_BOOL:
                return m_BoolVal ? 1 : 0;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                long long val = 0;
                if (m_StringVal) {
                    sscanf(m_StringVal, "%lld", &val);
                }
                return val;
            }
            default:
                return 0;
            }
        }
        float ToFloat()const
        {
            switch (m_Type) {
            case TYPE_INT:
                return(float)m_IntVal;
            case TYPE_INT64:
                return (float)m_Int64Val;
            case TYPE_FLOAT:
                return m_FloatVal;
            case TYPE_DOUBLE:
                return (float)m_DoubleVal;
            case TYPE_BOOL:
                return m_BoolVal ? 1.0f : 0.0f;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                float val = 0;
                if (m_StringVal) {
                    val = (float)atof(m_StringVal);
                }
                return val;
            }
            default:
                return 0;
            }
        }
        double ToDouble()const
        {
            switch (m_Type) {
            case TYPE_INT:
                return(double)m_IntVal;
            case TYPE_INT64:
                return (double)m_Int64Val;
            case TYPE_FLOAT:
                return (double)m_FloatVal;
            case TYPE_DOUBLE:
                return m_DoubleVal;
            case TYPE_BOOL:
                return m_BoolVal ? 1 : 0;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                double val = 0;
                if (m_StringVal) {
                    val = (double)atof(m_StringVal);
                }
                return val;
            }
            default:
                return 0;
            }
        }
        bool ToBool()const
        {
            switch (m_Type) {
            case TYPE_INT:
                return m_IntVal != 0;
            case TYPE_INT64:
                return m_Int64Val != 0;
            case TYPE_FLOAT:
                return (int)m_FloatVal != 0;
            case TYPE_DOUBLE:
                return (int)m_DoubleVal != 0;
            case TYPE_BOOL:
                return m_BoolVal;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                return strcmp(m_StringVal, "true") == 0;
            }
            default:
                return 0;
            }
        }
        const char* ToString(char* buf, int capacity)const
        {
            switch (m_Type) {
            case TYPE_INT:
            case TYPE_ARG_INDEX:
            case TYPE_LOCAL_INDEX:
            case TYPE_INDEX:
            {
                tsnprintf(buf, capacity, "%d", m_IntVal);
                return buf;
            }
            case TYPE_INT64:
            {
                tsnprintf(buf, capacity, "%d", m_Int64Val);
                return buf;
            }
            case TYPE_FLOAT:
            {
                tsnprintf(buf, capacity, "%f", m_FloatVal);
                return buf;
            }
            case TYPE_DOUBLE:
            {
                tsnprintf(buf, capacity, "%f", m_DoubleVal);
                return buf;
            }
            case TYPE_BOOL:
            {
                if (m_BoolVal)
                    return "true";
                else
                    return "false";
            }
            case TYPE_IDENTIFIER:
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
            case TYPE_DOLLAR_STRING:
            case TYPE_ALLOC_DOLLAR_STRING:
            {
                if (0 == m_StringVal)
                    return "";
                else
                    return m_StringVal;
            }
            default:
                return "";
            }
        }
    private:
        void CopyFrom(const Value& other)
        {
            if (TYPE_ALLOC_STRING == other.m_Type) {
                InitString(other.GetString());
                m_Line = other.m_Line;
            }
            else if (TYPE_ALLOC_DOLLAR_STRING == other.m_Type) {
                InitDollarString(other.GetString());
                m_Line = other.m_Line;
            }
            else {
                m_Type = other.GetType();
                m_Int64Val = other.m_Int64Val;
                m_Line = other.m_Line;
            }
        }
        void FreeString()
        {
            if (TYPE_ALLOC_STRING == m_Type || TYPE_ALLOC_DOLLAR_STRING == m_Type) {
                delete[] m_StringVal;
            }
        }
    private:
        int m_Type;
        union
        {
            int m_IntVal;
            long long m_Int64Val;
            float m_FloatVal;
            double m_DoubleVal;
            bool m_BoolVal;
            char* m_StringVal;
            ExpressionApi* m_ExpressionApiVal;
            StatementApi* m_StatementApiVal;
            FunctionData* m_FunctionVal;
            void* m_Ptr;
            const char* m_ConstStringVal;//The same type as m_StringVal in the script,
                                        // used to implement automatic const_cast
        };
        int m_Line;
    public:
        static Value& GetInvalidValueRef()
        {
            static Value s_Val;
            s_Val.SetInvalid();
            return s_Val;
        }
    };

    class Interpreter;
    class InterpreterValuePool;
    class ISyntaxComponent
    {
    public:
        enum
        {
            SEPARATOR_NOTHING = 0,
            SEPARATOR_COMMA,
            SEPARATOR_SEMICOLON,
        };
        enum
        {
            TYPE_NULL = 0,
            TYPE_VALUE,
            TYPE_FUNCTION,
            TYPE_STATEMENT,
        };
    public:
        explicit ISyntaxComponent(int syntaxType, Interpreter& interpreter) :m_SyntaxType(syntaxType), m_Separator(SEPARATOR_NOTHING), m_Interpreter(&interpreter) {}
        virtual ~ISyntaxComponent() {}
    public:
        virtual int IsValid() const = 0;
        virtual const char* GetId()const = 0;
        virtual int GetIdType() const = 0;
        virtual int GetLine() const = 0;
        //The compile-time data is converted into run-time data. The main purpose is that
        // the run-time only cares about the work that needs to be done at run-time and
        // improves efficiency.
        //Construct the runtime object here and ensure that it is constructed only once.
        //If the current syntax unit is StatementApi, it will be processed here.
        //Otherwise call PrepareGeneralRuntimeObject
        virtual void PrepareRuntimeObject() = 0;
        //Get the corresponding runtime object. There are three types of runtime objects:
        // single value in the usual sense, ExpressionApi and StatementApi, all recorded
        // in Value.
        virtual const Value& GetRuntimeObject()const = 0;
        //The current syntax unit may be a StatementApi or a component of a StatementApi,
        // both of which require preparation of runtime information.
        //Complete the preparation of regular runtime information here, that is,
        // the processing of runtime information except that the whole is a StatementApi
        // instance.
        virtual void PrepareGeneralRuntimeObject() = 0;
    public:
        int GetSyntaxType() const { return m_SyntaxType; }
        void SetSeparator(int sep) { m_Separator = sep; }
        int GetSeparator() const { return m_Separator; }
        const char* GetSepChar() const
        {
            switch (m_Separator) {
            case SEPARATOR_COMMA:
                return ",";
            case SEPARATOR_SEMICOLON:
                return ";";
            default:
                return " ";
            }
        }
    public:
        Interpreter& GetInterpreter()const { return *m_Interpreter; }
    protected:
        int	m_SyntaxType;
        int m_Separator;
        Interpreter* m_Interpreter;
    };

    class NullSyntax : public ISyntaxComponent
    {
    public:
        NullSyntax(Interpreter& interpreter) : ISyntaxComponent(ISyntaxComponent::TYPE_NULL, interpreter) {}
    public:
        virtual int IsValid() const { return FALSE; }
        virtual const char* GetId() const { return ""; }
        virtual int GetIdType() const { return Value::TYPE_IDENTIFIER; }
        virtual int GetLine() const { return 0; }
        virtual void PrepareRuntimeObject() {}
        virtual const Value& GetRuntimeObject()const
        {
            return Value::GetInvalidValueRef();
        }
        virtual void PrepareGeneralRuntimeObject() {}
    private:
        NullSyntax(const NullSyntax&) = delete;
        NullSyntax& operator=(const NullSyntax&) = delete;
    };

    //Because the Value class is used for value representation at runtime and needs to work
    // like a POD, a class is specially provided for the syntax layer to represent
    // the identifiers, constants, variables and operators in the syntax.
    //This class is mainly used for access with ISyntaxComponent, so FunctionData provides
    // methods for directly operating Value on the interface.
    class ValueData : public ISyntaxComponent
    {
    public:
        virtual int IsValid()const;
        virtual const char* GetId()const;
        virtual int GetIdType()const;
        virtual int GetLine()const;
        virtual void PrepareRuntimeObject();
        virtual const Value& GetRuntimeObject()const { return m_Value; }
        virtual void PrepareGeneralRuntimeObject();
    public:
        void SetValue(const Value& val) { m_Value = val; }
        Value& GetValue() { return m_Value; }
        const Value& GetValue()const { return m_Value; }
    public:
        int HaveId()const;
        int IsHighOrder()const { return m_Value.IsFunction(); }
    public:
        explicit ValueData(Interpreter& interpreter);
        virtual ~ValueData() {}
    private:
        ValueData(const ValueData& other) = delete;
        ValueData operator=(const ValueData& other) = delete;
    private:
        Value m_Value;
        int m_MaxLocalNum;
    private:
        Value m_RuntimeFunctionCall;
        int m_RuntimeObjectPrepared;

        InterpreterValuePool* m_pInnerValuePool;
    };

    /*
    * The information represented by ValueData, FunctionData and StatementData is compile-time information. For runtime, they are just metadata.
    * Because FunctionData can have high-order representation, its name may also be a FunctionData. This recursive relationship relies on Value to establish (note that in order to avoid wasting space, it is not placed on ValueData)
    * Value can be a FunctionData which is used only at compile time and should not be used at runtime.
    *
    * The three types of runtime: single value, ExpressionApi and StatementApi are all recorded in Value.
    *
    * The independent runtime should theoretically facilitate better running performance.
    *
    * DSL syntax does not distinguish between function parameters and statement blocks, both are in the form of parameter lists (this is to make the syntax more flexible, statement blocks can appear wherever parameter lists appear).
    *
    * A function with both a parameter list and a statement block is generally used to represent a function definition, which is a high-order function representation in syntax.
    */
    class StatementData;
    class RuntimeStatementBlock;
    class FunctionData : public ISyntaxComponent
    {
    public:
        enum
        {
            PARAM_CLASS_NOTHING = 0,
            PARAM_CLASS_PARENTHESIS,
            PARAM_CLASS_BRACKET,
            PARAM_CLASS_PERIOD,
            PARAM_CLASS_POINTER,
            PARAM_CLASS_STATEMENT,
            PARAM_CLASS_EXTERN_SCRIPT,
            PARAM_CLASS_PARENTHESIS_COLON,
            PARAM_CLASS_BRACKET_COLON,
            PARAM_CLASS_ANGLE_BRACKET_COLON,
            PARAM_CLASS_PARENTHESIS_PERCENT,
            PARAM_CLASS_BRACKET_PERCENT,
            PARAM_CLASS_BRACE_PERCENT,
            PARAM_CLASS_ANGLE_BRACKET_PERCENT,
            PARAM_CLASS_COLON_COLON,
            PARAM_CLASS_PERIOD_STAR,
            PARAM_CLASS_POINTER_STAR,
            PARAM_CLASS_OPERATOR,
            PARAM_CLASS_TERNARY_OPERATOR,
            PARAM_CLASS_QUESTION_NULLABLE_OPERATOR,
            PARAM_CLASS_EXCLAMATION_NULLABLE_OPERATOR,
            PARAM_CLASS_MAX,
            PARAM_CLASS_WRAP_INFIX_CALL_MASK = 0x20,
            PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK = 0x00040,
            PARAM_CLASS_UNMASK = 0x1F,
        };
        enum
        {
            PAIR_TYPE_NONE = 0,
            PAIR_TYPE_QUESTION_COLON,
            PAIR_TYPE_PARENTHESIS,
            PAIR_TYPE_BRACKET,
            PAIR_TYPE_BRACE,
            PAIR_TYPE_BRACKET_COLON,
            PAIR_TYPE_PARENTHESIS_COLON,
            PAIR_TYPE_ANGLE_BRACKET_COLON,
            PAIR_TYPE_BRACE_PERCENT,
            PAIR_TYPE_BRACKET_PERCENT,
            PAIR_TYPE_PARENTHESIS_PERCENT,
            PAIR_TYPE_ANGLE_BRACKET_PERCENT,
            PAIR_TYPE_MAXNUM
        };
        using StringKey = StringKeyT<MAX_TOKEN_NAME_SIZE>;
        using LocalIndexes = HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> >;
        using SyntaxComponentPtr = ISyntaxComponent*;
    public:
        virtual int IsValid()const
        {
            if (m_Name.IsValid())
                return TRUE;
            else if (HaveParamOrStatement())
                return TRUE;
            else
                return FALSE;
        }
        virtual int GetIdType()const { return m_Name.GetIdType(); }
        virtual const char* GetId()const { return m_Name.GetId(); }
        virtual int GetLine()const { return m_Name.GetLine(); }
        virtual void PrepareRuntimeObject();
        virtual const Value& GetRuntimeObject()const;
        virtual void PrepareGeneralRuntimeObject();
    public:
        RuntimeStatementBlock* GetRuntimeFunctionBody()const { return m_RuntimeStatementBlock; }
    public:
        void SetNameValue(const Value& val) { m_Name.SetValue(val); }
        Value& GetNameValue() { return m_Name.GetValue(); }
        ValueData& GetName() { return m_Name; }
        void ClearParams() { m_ParamNum = 0; }
        void AddParam(ISyntaxComponent* pVal)
        {
            if (0 == pVal || m_ParamNum < 0 || m_ParamNum >= m_MaxParamNum)
                return;
            PrepareParams();
            if (0 == m_Params || m_ParamNum >= m_ParamSpace)
                return;
            m_Params[m_ParamNum] = pVal;
            ++m_ParamNum;
        }
        void SetParam(int index, ISyntaxComponent* pVal)
        {
            if (NULL == pVal || index < 0 || index >= m_MaxParamNum)
                return;
            m_Params[index] = pVal;
        }
        void SetParamClass(int v) { m_ParamClass = v; }
        int GetParamClass()const { return m_ParamClass; }
        int GetParamClassUnmasked()const
        {
            int paramClass = (m_ParamClass & (int)PARAM_CLASS_UNMASK);
            return paramClass;
        }
        int HaveParamClassInfixFlag()const
        {
            int infix = (m_ParamClass & (int)PARAM_CLASS_WRAP_INFIX_CALL_MASK);
            return infix == (int)PARAM_CLASS_WRAP_INFIX_CALL_MASK ? TRUE : FALSE;
        }
        void SetInfixOperatorParamClass()
        {
            m_ParamClass = (int)(PARAM_CLASS_WRAP_INFIX_CALL_MASK | PARAM_CLASS_OPERATOR);
        }
        int HaveParamClassWrapObjectMemberFlag()const
        {
            int infix = (m_ParamClass & (int)PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK);
            return infix == (int)PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK ? TRUE : FALSE;
        }
        void SetWrapObjectMemberParamClass(int paramClass)
        {
            m_ParamClass = (int)(PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
        }
        void SetOperatorParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_OPERATOR;
        }
        void SetQuestionNullableOperatorParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_QUESTION_NULLABLE_OPERATOR;
        }
        void SetExclamationNullableOperatorParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_EXCLAMATION_NULLABLE_OPERATOR;
        }
        void SetTernaryOperatorParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_TERNARY_OPERATOR;
        }
        void SetParenthesisParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_PARENTHESIS;
        }
        void SetBracketParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_BRACKET;
        }
        void SetColonColonParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_COLON_COLON;
        }
        void SetPeriodParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_PERIOD;
        }
        void SetPeriodStarParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_PERIOD_STAR;
        }
        void SetPointerParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_POINTER;
        }
        void SetPointerStarParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_POINTER_STAR;
        }
        void SetParenthesisColonParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_PARENTHESIS_COLON;
        }
        void SetBracketColonParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_BRACKET_COLON;
        }
        void SetAngleBracketColonParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_ANGLE_BRACKET_COLON;
        }
        void SetParenthesisPercentParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_PARENTHESIS_PERCENT;
        }
        void SetBracketPercentParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_BRACKET_PERCENT;
        }
        void SetBracePercentParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_BRACE_PERCENT;
        }
        void SetAngleBracketPercentParamClass()
        {
            m_ParamClass = (int)PARAM_CLASS_ANGLE_BRACKET_PERCENT;
        }
        int IsOperatorParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_OPERATOR ? TRUE : FALSE;
        }
        int IsQuestionNullableOperatorParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_QUESTION_NULLABLE_OPERATOR ? TRUE : FALSE;
        }
        int IsExclamationNullableOperatorParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_EXCLAMATION_NULLABLE_OPERATOR ? TRUE : FALSE;
        }
        int IsTernaryOperatorParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_TERNARY_OPERATOR ? TRUE : FALSE;
        }
        bool IsParenthesisParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_PARENTHESIS;
        }
        bool IsBracketParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_BRACKET;
        }
        bool IsColonColonParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_COLON_COLON;
        }
        bool IsPeriodParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_PERIOD;
        }
        bool IsPeriodStarParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_PERIOD_STAR;
        }
        bool IsPointerParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_POINTER;
        }
        bool IsPointerStarParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_POINTER_STAR;
        }
        bool IsParenthesisColonParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_PARENTHESIS_COLON;
        }
        bool IsBracketColonParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_BRACKET_COLON;
        }
        bool IsAngleBracketColonParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_ANGLE_BRACKET_COLON;
        }
        bool IsParenthesisPercentParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_PARENTHESIS_PERCENT;
        }
        bool IsBracketPercentParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_BRACKET_PERCENT;
        }
        bool IsBracePercentParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_BRACE_PERCENT;
        }
        bool IsAngleBracketPercentParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return paramClass == (int)PARAM_CLASS_ANGLE_BRACKET_PERCENT;
        }
        int IsMemberParamClass()const
        {
            int paramClass = GetParamClassUnmasked();
            return (paramClass == (int)PARAM_CLASS_COLON_COLON ||
                paramClass == (int)PARAM_CLASS_PERIOD ||
                paramClass == (int)PARAM_CLASS_PERIOD_STAR ||
                paramClass == (int)PARAM_CLASS_POINTER ||
                paramClass == (int)PARAM_CLASS_POINTER_STAR) ? TRUE : FALSE;
        }
        int HaveId()const { return m_Name.HaveId(); }
        int HaveParamOrStatement()const { return m_ParamClass != PARAM_CLASS_NOTHING ? TRUE : FALSE; }
        int HaveParam()const { return HaveParamOrStatement() && !HaveStatement() && !HaveExternScript(); }
        int HaveStatement()const { return m_ParamClass == PARAM_CLASS_STATEMENT ? TRUE : FALSE; }
        int HaveExternScript()const { return m_ParamClass == PARAM_CLASS_EXTERN_SCRIPT ? TRUE : FALSE; }
        int IsHighOrder()const { return m_Name.IsHighOrder(); }
        FunctionData* GetLowerOrderFunction()const
        {
            auto fptr = m_Name.GetValue().GetFunction();
            if (IsHighOrder() && fptr) {
                return fptr;
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        const FunctionData* GetThisOrLowerOrderCall()const
        {
            if (HaveParam()) {
                return this;
            }
            else if (HaveLowerOrderParam()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        FunctionData* GetThisOrLowerOrderCall()
        {
            if (HaveParam()) {
                return this;
            }
            else if (HaveLowerOrderParam()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        const FunctionData* GetThisOrLowerOrderBody()const
        {
            if (HaveStatement()) {
                return this;
            }
            else if (HaveLowerOrderStatement()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        FunctionData* GetThisOrLowerOrderBody()
        {
            if (HaveStatement()) {
                return this;
            }
            else if (HaveLowerOrderStatement()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        const FunctionData* GetThisOrLowerOrderScript()const
        {
            if (HaveExternScript()) {
                return this;
            }
            else if (HaveLowerOrderExternScript()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        FunctionData* GetThisOrLowerOrderScript()
        {
            if (HaveExternScript()) {
                return this;
            }
            else if (HaveLowerOrderExternScript()) {
                return m_Name.GetValue().GetFunction();
            }
            else {
                return GetNullFunctionPtr();
            }
        }
        int HaveLowerOrderParam()const
        {
            auto fptr = m_Name.GetValue().GetFunction();
            if (IsHighOrder() && fptr && fptr->HaveParam())
                return TRUE;
            else
                return FALSE;
        }
        int HaveLowerOrderStatement()const
        {
            auto fptr = m_Name.GetValue().GetFunction();
            if (IsHighOrder() && fptr && fptr->HaveStatement())
                return TRUE;
            else
                return FALSE;
        }
        int HaveLowerOrderExternScript()const
        {
            auto fptr = m_Name.GetValue().GetFunction();
            if (IsHighOrder() && fptr && fptr->HaveExternScript())
                return TRUE;
            else
                return FALSE;
        }
    public:
        const Value& GetNameValue()const { return m_Name.GetValue(); }
        const ValueData& GetName()const { return m_Name; }
        int GetParamNum()const { return m_ParamNum; }
        ISyntaxComponent* GetParam(int index)const
        {
            if (0 == m_Params || index < 0 || index >= m_ParamNum)
                return GetNullSyntaxPtr();
            return m_Params[index];
        }
        const char* GetParamId(int index)const
        {
            if (0 == m_Params || index < 0 || index >= m_ParamNum)
                return "";
            return m_Params[index]->GetId();
        }
    public:
        int CalculateStackSize()const;
    public:
        int AllocLocalIndex(const char* id);
        int GetLocalIndex(const char* id)const;
        const char* GetLocalName(int index)const;
    public:
        explicit FunctionData(Interpreter& interpreter);
        virtual ~FunctionData();
    private:
        FunctionData(const FunctionData& other) = delete;
        FunctionData operator=(const FunctionData& other) = delete;
    private:
        void PrepareParams();
        void ReleaseParams();
        void PrepareLocalIndexes();
        void ClearLocalIndexes();
    private:
        NullSyntax* GetNullSyntaxPtr()const;
        FunctionData* GetNullFunctionPtr()const;
    private:
        ValueData m_Name;
        ISyntaxComponent** m_Params;
        int m_ParamNum;
        int m_ParamSpace;
        int m_MaxParamNum;
        int m_ParamClass;
    private:
        LocalIndexes m_LocalIndexes;
        int m_LocalNum;
        int m_LocalSpace;
        int m_MaxLocalNum;
    private:
        Value m_RuntimeFunctionCall;
        RuntimeStatementBlock* m_RuntimeStatementBlock;
        int m_RuntimeObjectPrepared;

        InterpreterValuePool* m_pInnerValuePool;
    };

    /* Memo: Why do members of StatementData not use ISyntaxComponent[] but FunctionData[]?
    * 1. Although FunctionData here can be degenerated into FunctionData and ValueData in syntax,
    * it cannot be StatementData, so it cannot be conceptually equivalent to ISyntaxComponent.
    * 2. In terms of design, FunctionData should take the degradation situation into consideration and
    * try not to occupy additional space in the degradation situation.
    */
    class StatementData : public ISyntaxComponent
    {
    public:
        virtual int IsValid()const
        {
            if (NULL != m_Functions && m_FunctionNum > 0 && m_Functions[0]->IsValid())
                return TRUE;
            else
                return FALSE;
        }
        virtual int GetIdType()const
        {
            int type = Value::TYPE_IDENTIFIER;
            if (IsValid()) {
                type = m_Functions[0]->GetIdType();
            }
            return type;
        }
        virtual const char* GetId()const
        {
            const char* str = "";
            if (IsValid()) {
                str = m_Functions[0]->GetId();
            }
            return str;
        }
        virtual int GetLine()const
        {
            int line = 0;
            if (IsValid()) {
                line = m_Functions[0]->GetLine();
            }
            return line;
        }
        virtual void PrepareRuntimeObject();
        virtual const Value& GetRuntimeObject()const;
        virtual void PrepareGeneralRuntimeObject();
    public:
        void ClearFunctions() { m_FunctionNum = 0; }
        void AddFunction(FunctionData* pVal)
        {
            if (NULL == pVal || m_FunctionNum < 0 || m_FunctionNum >= m_MaxFunctionNum)
                return;
            PrepareFunctions();
            if (NULL == m_Functions || m_FunctionNum >= m_FunctionSpace)
                return;
            m_Functions[m_FunctionNum] = pVal;
            ++m_FunctionNum;
        }
        FunctionData*& GetLastFunctionRef()const
        {
            if (NULL == m_Functions || 0 == m_FunctionNum)
                return GetNullFunctionPtrRef();
            else
                return m_Functions[m_FunctionNum - 1];
        }
    public:
        int GetFunctionNum()const { return m_FunctionNum; }
        FunctionData* GetFunction(int index)const
        {
            if (NULL == m_Functions || index < 0 || index >= m_FunctionNum || index >= m_MaxFunctionNum)
                return 0;
            return m_Functions[index];
        }
        const char* GetFunctionId(int index)const
        {
            if (0 == m_Functions || index < 0 || index >= m_FunctionNum || index >= m_MaxFunctionNum)
                return 0;
            return m_Functions[index]->GetId();
        }
    public:
        explicit StatementData(Interpreter& interpreter);
        virtual ~StatementData()
        {
            ReleaseFunctions();
        }
    private:
        StatementData(const StatementData&) = delete;
        StatementData& operator=(const StatementData&) = delete;
    private:
        void PrepareFunctions();
        void ReleaseFunctions();
    private:
        FunctionData*& GetNullFunctionPtrRef()const;
    private:
        FunctionData** m_Functions;
        int m_FunctionNum;
        int m_FunctionSpace;
        int m_MaxFunctionNum;
    private:
        Value m_RuntimeObject;
        int m_RuntimeObjectPrepared;
    };

    class InterpreterValuePool
    {
        friend class AutoInterpreterValuePoolValueOperation;
        friend class AutoInterpreterValuePoolValuesOperation;
        friend class Interpreter;
    private:
        InterpreterValuePool(int numValue, int numValues) :m_ValueMemory(NULL), m_ValuePool(NULL), m_ValueNum(0), m_ValuesPool(NULL), m_ValuesNum(0), m_UsedValueNum(0), m_UsedValuesNum(0)
        {
            m_ValueMemory = new Value[numValue];
            m_ValuePool = new Value * [numValue];
            if (m_ValueMemory && m_ValuePool) {
                m_ValueNum = numValue;
                for (int i = 0; i < numValue; ++i) {
                    m_ValuePool[i] = &m_ValueMemory[i];
                }
            }
            m_ValuesPool = new Value * [numValues];
            if (m_ValuesPool) {
                m_ValuesNum = numValues;
                for (int i = 0; i < numValues; ++i) {
                    m_ValuesPool[i] = new Value[MAX_FUNCTION_PARAM_NUM];
                }
            }
        }
        ~InterpreterValuePool()
        {
            if (m_ValueMemory) {
                delete[] m_ValueMemory;
                m_ValueMemory = NULL;
            }
            if (m_ValuePool) {
                delete[] m_ValuePool;
                m_ValuePool = NULL;
            }
            if (m_ValuesPool) {
                for (int i = 0; i < m_ValuesNum; ++i) {
                    delete[] m_ValuesPool[i];
                }
                delete[] m_ValuesPool;
                m_ValuesPool = NULL;
            }
            m_UsedValueNum = 0;
            m_ValueNum = 0;
            m_UsedValuesNum = 0;
            m_ValuesNum = 0;
        }
    private:
        void Reset()
        {
            m_UsedValueNum = 0;
            m_UsedValuesNum = 0;
        }
        Value* AllocValue()
        {
            Value* p = NULL;
            if (m_UsedValuesNum < m_ValueNum) {
                p = m_ValuePool[m_UsedValueNum++];
            }
            return p;
        }
        void RecycleValue(Value* p)
        {
            if (m_UsedValueNum) {
                /*INT newNum=m_UsedValueNum-1;
                for(INT i=newNum;i>=0;--i)
                {
                if(m_ValuePool[i]==p)
                {
                m_ValuePool[i]=m_ValuePool[newNum];
                m_ValuePool[newNum]=p;
                m_UsedValueNum=newNum;
                break;
                }
                }*/
                if (m_ValuePool[--m_UsedValueNum] != p) {
                    throw;
                }
            }
        }
        Value* AllocValues()
        {
            Value* p = NULL;
            if (m_UsedValuesNum < m_ValuesNum) {
                p = m_ValuesPool[m_UsedValuesNum++];
            }
            return p;
        }
        void RecycleValues(Value* p)
        {
            if (m_UsedValuesNum) {
                /*INT newNum=m_UsedValuesNum-1;
                for(INT i=newNum;i>=0;--i)
                {
                if(m_ValuesPool[i]==p)
                {
                m_ValuesPool[i]=m_ValuesPool[newNum];
                m_ValuesPool[newNum]=p;
                m_UsedValuesNum=newNum;
                break;
                }
                }*/
                if (m_ValuesPool[--m_UsedValuesNum] != p) {
                    throw;
                }
            }
        }
    private:
        Value* m_ValueMemory;
        Value** m_ValuePool;
        int m_ValueNum;
        int m_UsedValueNum;
        Value** m_ValuesPool;
        int m_ValuesNum;
        int m_UsedValuesNum;
    };

    class AutoInterpreterValuePoolValueOperation
    {
    public:
        explicit AutoInterpreterValuePoolValueOperation(InterpreterValuePool& pool) :m_Pool(pool), m_Value(*pool.AllocValue())
        {
        }
        ~AutoInterpreterValuePoolValueOperation()
        {
            m_Pool.RecycleValue(&m_Value);
        }
    public:
        Value& Get() const { return m_Value; }
    private:
        AutoInterpreterValuePoolValueOperation(const AutoInterpreterValuePoolValueOperation&);
        AutoInterpreterValuePoolValueOperation& operator=(const AutoInterpreterValuePoolValueOperation&);
    private:
        InterpreterValuePool& m_Pool;
        Value& m_Value;
    };

    class AutoInterpreterValuePoolValuesOperation
    {
    public:
        explicit AutoInterpreterValuePoolValuesOperation(InterpreterValuePool& pool) :m_Pool(pool), m_Value(pool.AllocValues())
        {
        }
        ~AutoInterpreterValuePoolValuesOperation()
        {
            m_Pool.RecycleValues(m_Value);
        }
    public:
        Value* Get() const { return m_Value; }
    private:
        AutoInterpreterValuePoolValuesOperation(const AutoInterpreterValuePoolValuesOperation&);
        AutoInterpreterValuePoolValuesOperation& operator=(const AutoInterpreterValuePoolValuesOperation&);
    private:
        InterpreterValuePool& m_Pool;
        Value* m_Value;
    };

    class Interpreter;
    class StatementData;
    class RuntimeComponent
    {
    public:
        explicit RuntimeComponent(Interpreter& interpreter) :m_Interpreter(&interpreter) {}
        virtual ~RuntimeComponent() {}
    protected:
        void ReplaceVariableWithValue(Value* pParams, int num)const;
        void ReplaceVariableWithValue(Value& p)const;
        void SetVariableValue(const Value& p, const Value& val)const;
        const Value& GetVariableValue(const Value& p)const;
    protected:
        Interpreter* m_Interpreter;
    };

    /*
    * Memo: When implementing the API, please note that there is a convention here. All parameters are calculated by the API itself, and the original parameter information is passed. This makes it easy to implement assignment or out parameter features!
    * In particular, when a custom statement is executed, the API is called. According to this convention, the statement does not need to calculate parameter values for the API.
    * One exception is that if the parameter is a StatementApi, its return value needs to be passed after execution.
    * There is also a convention that the Value types of StatementApi and FunctionData will not be generated by script logic. These two types of values are part of the compilation environment, so they will only be generated during compilation.
    * Such a Value will be generated during translation, which means that a Value will not become StatementApi or FunctionData after evaluation. For parameter types of StatementApi or FunctionData
    * Value, the interpreter will convert it to a common value when calling a specific API, so the method of calculating the parameter value of the RuntimeComponent class does not need to consider these situations, just
    * Replace with the corresponding parameter value based on the parameter information.
    *
    * ExpressionApi::Execute
    * When the above method is used for API implementation, the implementation must comply with const-modified semantics, because after API registration, an instance is used in many places. The interface cannot add this constraint because
    * To implement these interfaces are not only registered APIs, but also some runtime objects that can generate multiple instances.
    */
    class ExpressionApi : public RuntimeComponent
    {
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue) = 0;
    public:
        explicit ExpressionApi(Interpreter& interpreter) :RuntimeComponent(interpreter)
        {
        }
    };

    class StatementApi : public RuntimeComponent
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const = 0;
    public:
        explicit StatementApi(Interpreter& interpreter) :RuntimeComponent(interpreter) {}
    };

    //Each type of statement is different in form, so an api factory is needed to customize
    //the corresponding api instance and the work that relies on compile-time information is
    //completed here.
    class StatementApiFactory
    {
    public:
        virtual ~StatementApiFactory() = default;
    public:
        virtual int IsMatch(const ISyntaxComponent& statement)const = 0;
        //Statements are different from expressions. All expressions are a function representation.
        //The output is calculated given the input and does not rely on other compile-time information.
        //Each type of statement is different in form, and the runtime object needs to be customized
        //based on compile-time information for each statement.
        virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const = 0;
    };

    class Closure : public ExpressionApi
    {
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetDefinitionRef(const Value* pArguments, int argumentNum, const ISyntaxComponent& comp);
    public:
        explicit Closure(Interpreter& interpreter) :ExpressionApi(interpreter), m_pArguments(NULL), m_pDefinition(NULL), m_StackSize(0), m_Statements(NULL) {}
    private:
        const Value* m_pArguments;
        int m_ArgumentNum;
        const ISyntaxComponent* m_pDefinition;
        int m_StackSize;
        RuntimeStatementBlock* m_Statements;
    };

    class MemberAccessor : public ExpressionApi
    {
    public:
        static const int MARK_ACCESSOR_CALL = 0x12345678;
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetObject(ExpressionApi& object) { m_Object = &object; }
        ExpressionApi* GetObject()const { return m_Object; }
        void SetMemberIndex(int memberIndex) { m_MemberIndex = memberIndex; }
        int GetMemberIndex() const { return m_MemberIndex; }
    public:
        MemberAccessor(Interpreter& interpreter) :ExpressionApi(interpreter), m_Object(NULL), m_MemberIndex(-1)
        {
        }
        MemberAccessor(Interpreter& interpreter, ExpressionApi& object, int memberIndex) :ExpressionApi(interpreter), m_Object(&object), m_MemberIndex(memberIndex)
        {
            //CrashAssert(0!=m_Object);
        }
    private:
        ExpressionApi* m_Object;
        int m_MemberIndex;
    };
    using MemberAccessorPtr = MemberAccessor*;

    class ObjectBase : public ExpressionApi
    {
    protected:
        static const int MEMBER_INFO_CAPACITY_DELTA_SIZE = 8;
        static const int MAX_MEMBER_NUM = 10240;
        enum
        {
            INNER_MEMBER_INDEX_SIZE = 0,
            INNER_MEMBER_INDEX_NUM
        };
        struct MemberInfo
        {
            const char* m_Name;
            Value m_Value;

            MemberInfo() :m_Name(0)
            {
            }
            void Reset()
            {
                m_Name = 0;
                m_Value.SetInvalid();
            }
        };
        using StringKey = StringKeyT<32>;
        using NameIndexMap = HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> >;
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        explicit ObjectBase(Interpreter& interpreter, int customInnerMemberNum);
        virtual ~ObjectBase();
    protected:
        virtual int	 GetCustomInnerMemberIndex(const char* name)const { name; return -1; }
        virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue) { index, paramClass, pParams, num, pRetValue; return EXECUTE_RESULT_NORMAL; }
    private:
        void Resize();
        int GetMemberIndex(const char* name)const;
        void ResetTemp();
        void AddTempToMember(int skipNum);
    private:
        NameIndexMap* m_NameIndexMap;
        MemberInfo* m_MemberInfos;
        MemberAccessorPtr* m_Accessors;
        int m_MemberNum;
        int m_Capacity;
    private://
        MemberAccessorPtr* m_InnerMemberAccessors;
        int m_InnerMemberNum;
    private:
        MemberInfo m_TempMemberInfo;
        MemberAccessor m_TempAccessor;
    };

    class Object : public ObjectBase
    {
    public:
        explicit Object(Interpreter& interpreter);
        virtual ~Object();
    };

    class Struct : public ExpressionApi
    {
        enum
        {
            INNER_MEMBER_INDEX_ATTACH = 0,
            INNER_MEMBER_INDEX_CLONE,
            INNER_MEMBER_INDEX_ADDR,
            INNER_MEMBER_INDEX_SIZE,
            INNER_MEMBER_INDEX_NUM
        };
        enum
        {
            INNER_ARG_INDEX_OFFSET = -3,
            INNER_ARG_INDEX_SIZE,
            INNER_ARG_INDEX_NUM,
        };
        struct MemberInfo
        {
            int m_Offset;
            int m_Size;
            int m_Num;
            const char* m_Name;

            MemberInfo() :m_Offset(0), m_Size(0), m_Num(0), m_Name(0)
            {
            }
        };
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetDefinitionRef(const FunctionData& statement);
        void Attach(unsigned int addr);
        Struct* Clone() const;
    public:
        explicit Struct(Interpreter& interpreter) :ExpressionApi(interpreter), m_pDefinition(0), m_MemberInfos(0), m_Accessors(0), m_MemberNum(0), m_Size(0), m_Addr(0) {}
        virtual ~Struct();
    private:
        int GetMemberIndex(const char* name)const;
    private:
        const FunctionData* m_pDefinition;
        MemberInfo* m_MemberInfos;
        MemberAccessorPtr* m_Accessors;
        int m_MemberNum;

        unsigned int m_Size;
        unsigned int m_Addr;
    };

    //Each function call is a RuntimeFunctionCall at runtime, which records the function pointer and corresponding call parameters.
    //API：RuntimeFunctionCall->ExpressionApi
    //Custom function：RuntimeFunctionCall->Closure->FunctionStatement
    //Custom object method：RuntimeFunctionCall->MemberAccessor->ObjectBase->Closure->RuntimeStatementBlock
    //Custom object property：RuntimeFunctionCall->MemberAccessor->ObjectBase
    //Custom struct member：RuntimeFunctionCall->MemberAccessor->ObjectBase
    class RuntimeFunctionCall : public StatementApi
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        explicit RuntimeFunctionCall(Interpreter& interpreter);
        virtual ~RuntimeFunctionCall();
        void Init(FunctionData& call);
    private:
        Value m_Name;
        int m_ParamClass;
        int m_ParamNum;
        Value* m_Params;

        InterpreterValuePool* m_pInnerValuePool;
    };

    class RuntimeStatementBlock
    {
    public:
        ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        RuntimeStatementBlock(Interpreter& interpreter, FunctionData& func);
        ~RuntimeStatementBlock();
    private:
        Interpreter* m_Interpreter;
        int m_StatementNum;
        StatementApi** m_Statements;

        InterpreterValuePool* m_pInnerValuePool;
    };

    /*
    * This class is separated here for use in the syntax parsing part, because there are two forms of scripts:
    * SourceCodeScript and ByteCodeScript,
    * the parsing part cannot rely on Interpreter but must be a shared class ErrorAndStringBuffer
    */
    class ErrorAndStringBuffer
    {
    public:
        void ClearErrorInfo();
        void AddError(const char* error);
        int HasError()const { return m_HasError; }
        int GetErrorNum()const { return m_ErrorNum; }
        const char* GetErrorInfo(int index) const
        {
            if (index < 0 || index >= m_ErrorNum || index >= MAX_RECORD_ERROR_NUM)
                return "";
            return m_ErrorInfo[index];
        }
        char* NewErrorInfo()
        {
            m_HasError = TRUE;
            if (m_ErrorNum < MAX_RECORD_ERROR_NUM) {
                ++m_ErrorNum;
                return m_ErrorInfo[m_ErrorNum - 1];
            }
            else {
                return 0;
            }
        }
    public:
        int GetUnusedStringLength()const
        {
            MyAssert(m_pStringBuffer);
            MyAssert(m_ppUnusedStringRef);
            return m_MaxStringBufferLength - int(*m_ppUnusedStringRef - m_pStringBuffer);
        }
        char*& GetUnusedStringPtrRef()
        {
            MyAssert(m_ppUnusedStringRef);
            return *m_ppUnusedStringRef;
        }
    public:
        ErrorAndStringBuffer() :m_pStringBuffer(NULL), m_ppUnusedStringRef(NULL), m_MaxStringBufferLength(0)
        {
            ClearErrorInfo();
        }
        void Reset(char* pStringBuffer, char*& pUnusedStringRef, int maxStringBufferLength)
        {
            ClearErrorInfo();
            m_pStringBuffer = pStringBuffer;
            m_ppUnusedStringRef = &pUnusedStringRef;
            m_MaxStringBufferLength = maxStringBufferLength;
            MyAssert(m_pStringBuffer);
            MyAssert(m_ppUnusedStringRef);
        }
    private:
        int	m_HasError;
        char m_ErrorInfo[MAX_RECORD_ERROR_NUM][MAX_ERROR_INFO_CAPACITY];
        int m_ErrorNum;
        char* m_pStringBuffer;
        char** m_ppUnusedStringRef;
        int m_MaxStringBufferLength;
    };

    class ActionApi final
    {
    public:
        int peekPairTypeStack()const;
        int peekPairTypeStack(uint32_t& tag)const;
        int getPairTypeStackSize()const;
        int peekPairTypeStack(int ix)const;
        int peekPairTypeStack(int ix, uint32_t& tag)const;
    public:
        inline ActionApi() :m_Impl(0) {}
        inline void SetImpl(ActionForSourceCodeScript* p) { m_Impl = p; }
    private:
        ActionApi(const ActionApi& other) = delete;
        ActionApi(ActionApi&& other) noexcept = delete;
        ActionApi& operator=(const ActionApi& other) = delete;
        ActionApi& operator=(ActionApi&& other) noexcept = delete;
    private:
        ActionForSourceCodeScript* m_Impl;
    };
    /*
    * Memo: The interpreter of this script language relies heavily on the C++ function stack mechanism, so it is not suitable
    * for implementing the corountine mechanism (because the C++ function call stack is used to implement the script stack, one execution of the interpreter will completely erase the call stack. The previous stack and context cannot be continued when entering again).
    * This mechanism can be simulated by implementing a command queue in a script.
    */
    class Interpreter
    {
        using NameTags = std::unordered_map<std::string, uint32_t>;
        using SyntaxComponentPtr = ISyntaxComponent*;
        using RuntimeComponentPtr = RuntimeComponent*;
        using StatementApiPtr = StatementApi*;
        using StringKey = StringKeyT<MAX_TOKEN_NAME_SIZE>;
        using ValueIndexes = HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> >;
        using Functions = HashtableT<StringKey, ExpressionApi*, StringKey>;
        using StatementApiFactoryList = DequeT<StatementApiFactory*, MAX_FORM_NUM_PER_STATEMENT>;
        using StatementApiFactories = HashtableT<StringKey, StatementApiFactoryList, StringKey>;
        using FunctionDefinitionStack = DequeT<FunctionData*, MAX_FUNCTION_LEVEL>;

        struct StackInfo
        {
            int m_Start;
            Value* m_Params;
            Value m_Size;
            Value m_ParamNum;
            const ISyntaxComponent* m_pDefinition;

            StackInfo() :m_Start(-1), m_Size(0), m_Params(0), m_ParamNum(0), m_pDefinition(0)
            {
            }
            int IsValid()const
            {
                return (m_Start >= 0 ? TRUE : FALSE);
            }
            static StackInfo& GetInvalidStackInfoRef();
        };
        using StackInfos = DequeT<StackInfo, MAX_STACK_LEVEL>;
    public:
        void RegisterStatementApi(const char* id, StatementApiFactory* p);
        void RegisterPredefinedValue(const char* id, const Value& val);
        void PrepareRuntimeObject();
        ExecuteResultEnum Execute(Value* pRetValue);
        ExecuteResultEnum CallMember(ExpressionApi& obj, const Value& member, int isProperty, int paramClass, Value* pParams, int paramNum, Value* pRetValue);
    public:
        StatementApiFactory* GetLiteralArrayApi()const;
        StatementApiFactory* GetLiteralObjectApi()const;
        ExpressionApi* FindFunctionApi(const char* id)const;
        StatementApiFactory* FindStatementApi(const ISyntaxComponent& statement)const;
        const Value& GetPredefinedValue(const char* id)const;
        void SetValue(const char* id, const Value& val);
        const Value& GetValue(const char* id)const;
        int GetValueIndex(const char* id)const;
        const char* GetValueName(int indexType, int index)const;
        void SetValue(int indexType, int index, const Value& val);
        const Value& GetValue(int indexType, int index)const;
        int GetValueNum()const { return m_ValueNum; }
    public:
        void PushStackInfo(Value* pParams, int paramNum, int stackSize, const ISyntaxComponent& definition);
        void PopStackInfo();
        int IsStackEmpty()const;
        int IsStackFull()const;
        const StackInfo& GetTopStackInfo()const;
        int GetStackValueNum()const;
    public:
        int PushFunctionDefinition(FunctionData* pFunction);
        int PopFunctionDefinition();
        FunctionData* GetCurFunctionDefinition()const;
    public:
        void AddStatement(ISyntaxComponent* p);
    public:
        ValueData* AddNewValueComponent();
        FunctionData* AddNewFunctionComponent();
        StatementData* AddNewStatementComponent();
        RuntimeFunctionCall* AddNewRuntimeFunctionComponent();
        Closure* AddNewClosureComponent();
        Object* AddNewObjectComponent();
        Struct* AddNewStructComponent();
    public:
        template<typename T> inline T* AddNewStatementApiComponent()
        {
            T* p = new T(*this);
            if (NULL != p) {
                AddRuntimeComponent(p);
            }
            return p;
        }
    public:
        void AddSyntaxComponent(ISyntaxComponent* p);
        int GetSyntaxComponentNum()const { return m_SyntaxComponentNum; }
        void AddRuntimeComponent(RuntimeComponent* p);
        int GetRuntimeComponentNum()const { return m_RuntimeComponentNum; }
    public:
        NameTags& NameTagsRef() { return m_NameTags; }
        const NameTags& NameTagsRef()const { return m_NameTags; }
    public:
        char* AllocString(int len);
        char* AllocString(const char* src);
        char* GetStringBuffer()const { return m_StringBuffer; }
        char*& GetUnusedStringPtrRef() { return m_UnusedStringPtr; }
    public:
        Interpreter() :Interpreter(InterpreterOptions()) {}
        Interpreter(const InterpreterOptions& options);
        ~Interpreter();
        void Reset();
        int SetNextStatement(int ip)
        {
            if (ip < 0 || ip >= m_StatementNum)
                return FALSE;
            else
                m_NextStatement = ip;
        }
        int GetStatementNum()const { return m_StatementNum; }
        SyntaxComponentPtr GetStatement(int ix)const
        {
            if (ix >= 0 && ix < m_StatementNum)
                return m_Program[ix];
            else
                return nullptr;
        }
    private:
        void Init();
        void Release();
        void InitInnerApis();
        void ReleaseInnerApis();
        void RegisterInnerFunctionApi(const char* id, ExpressionApi* p);
        void RegisterInnerStatementApi(const char* id, StatementApiFactory* p);
    private:
        NameTags m_NameTags;
        char* m_StringBuffer;
        char* m_UnusedStringPtr;
        SyntaxComponentPtr* m_SyntaxComponentPool;
        int m_SyntaxComponentNum;
        RuntimeComponentPtr* m_RuntimeComponentPool;
        int m_RuntimeComponentNum;
        SyntaxComponentPtr* m_Program;
        StatementApiPtr* m_RuntimeProgram;
        int m_StatementNum;
        Functions m_InnerFunctionApis;
        StatementApiFactories m_StatementApis;
        StatementApiFactories m_InnerStatementApis;
        Value* m_PredefinedValue;
        int m_PredefinedValueNum;
        ValueIndexes m_PredefinedValueIndexes;
        Value* m_ValuePool;
        int m_ValueNum;
        ValueIndexes m_ValueIndexes;
        Value* m_StackValuePool;
        StackInfos m_StackInfos;
        FunctionDefinitionStack	m_FunctionDefinitionStack;

        int m_NextStatement;
    public://The data pool is only used for the interpreter. It is only used in functions or statements to replace local variables.
            //Alloc and recycle must be paired and comply with the operation order of the stack (last in, first out)! ! !
        InterpreterValuePool& GetInnerValuePool() { return m_InterpreterValuePool; }
    private:
        InterpreterValuePool m_InterpreterValuePool;
    public:
        void EnableDebugInfo() { m_IsDebugInfoEnable = TRUE; }
        void DisableDebugInfo() { m_IsDebugInfoEnable = FALSE; }
        int IsDebugInfoEnable()const { return m_IsDebugInfoEnable; }
        int IsRunFlagEnable()const
        {
            if (0 == m_pRunFlag || *m_pRunFlag)
                return TRUE;
            else
                return FALSE;
        }
        void SetExternRunFlagRef(int& flag) { m_pRunFlag = &flag; }
    private:
        int m_IsDebugInfoEnable;
        const int* m_pRunFlag;
    public:
        void ClearErrorInfo() { m_ErrorAndStringBuffer.ClearErrorInfo(); }
        void AddError(const char* error) { m_ErrorAndStringBuffer.AddError(error); }
        int HasError()const { return m_ErrorAndStringBuffer.HasError(); }
        int GetErrorNum() { return m_ErrorAndStringBuffer.GetErrorNum(); }
        const char* GetErrorInfo(int index) const { return m_ErrorAndStringBuffer.GetErrorInfo(index); }
        char* NewErrorInfo() { return m_ErrorAndStringBuffer.NewErrorInfo(); }
    public:
        ErrorAndStringBuffer& GetErrorAndStringBuffer() { return m_ErrorAndStringBuffer; }
        const ErrorAndStringBuffer& GetErrorAndStringBuffer()const { return m_ErrorAndStringBuffer; }
    private:
        ErrorAndStringBuffer m_ErrorAndStringBuffer;
    public:
        InterpreterOptions& GetOptions() { return m_Options; }
        const InterpreterOptions& GetOptions()const { return m_Options; }
    private:
        InterpreterOptions m_Options;
    public:
        NullSyntax* GetNullSyntaxPtr()const
        {
            return m_pNullSyntax;
        }
        FunctionData* GetNullFunctionPtr()const
        {
            m_pNullFunction->GetName().GetValue().SetInvalid();
            m_pNullFunction->SetParamClass(FunctionData::PARAM_CLASS_NOTHING);
            m_pNullFunction->ClearParams();
            return m_pNullFunction;
        }
        FunctionData*& GetNullFunctionPtrRef()const
        {
            auto fptr = *m_ppNullFunction;
            fptr->GetName().GetValue().SetInvalid();
            fptr->SetParamClass(FunctionData::PARAM_CLASS_NOTHING);
            fptr->ClearParams();
            return *m_ppNullFunction;
        }
    private:
        NullSyntax* m_pNullSyntax;
        FunctionData* m_pNullFunction;
        FunctionData** m_ppNullFunction;
    };

    class IScriptSource
    {
    public:
        virtual ~IScriptSource() {}
    public:
        class Iterator
        {
        public:
            const char& operator * () const
            {
                return Peek(0);
            }
            Iterator& operator ++ ()
            {
                Advance();
                return *this;
            }
            const Iterator operator ++ (int)
            {
                Iterator it = *this;
                ++(*this);
                return it;
            }
            const Iterator operator + (int val) const
            {
                Iterator it = *this;
                for (int ix = 0; ix < val; ++ix)
                    it.Advance();
                return it;
            }
            int Load()
            {
                if (NULL != m_pSource) {
                    int r = m_pSource->Load();
                    if (r) {
                        *this = m_pSource->GetIterator();
                    }
                    return r;
                }
                else {
                    return FALSE;
                }
            }
            const char* GetBuffer()const
            {
                return m_pSource->GetBuffer();
            }
            const char* GetLeft()const
            {
                return m_Iterator;
            }
        public:
            int operator ==(const Iterator& other) const
            {
                return m_pSource == other.m_pSource && m_Iterator == other.m_Iterator && m_EndIterator == other.m_EndIterator;
            }
            int operator !=(const Iterator& other) const
            {
                return !(operator ==(other));
            }
        public:
            Iterator() :m_pSource(NULL), m_Iterator(""), m_EndIterator(m_Iterator)
            {
            }
            explicit Iterator(IScriptSource& source) :m_pSource(&source)
            {
                const char* p = m_pSource->GetBuffer();
                if (0 == p) {
                    m_Iterator = "";
                    m_EndIterator = m_Iterator;
                }
                else {
                    m_Iterator = p;
                    m_EndIterator = m_Iterator + strlen(p);
                }
            }
            Iterator(const Iterator& other)
            {
                CopyFrom(other);
            }
            Iterator& operator=(const Iterator& other)
            {
                if (this == &other)
                    return *this;
                CopyFrom(other);
                return *this;
            }
        private:
            const char& Peek(int index)const
            {
                if (m_Iterator + index < m_EndIterator)
                    return *(m_Iterator + index);
                else
                    return *m_EndIterator;
            }
            void Advance()
            {
                if (m_Iterator < m_EndIterator)
                    ++m_Iterator;
            }
            void CopyFrom(const Iterator& other)
            {
                m_pSource = other.m_pSource;
                m_Iterator = other.m_Iterator;
                m_EndIterator = other.m_EndIterator;
            }
        private:
            IScriptSource* m_pSource;
            const char* m_Iterator;
            const char* m_EndIterator;
        };
        friend class Iterator;
    public:
        Iterator GetIterator()
        {
            return Iterator(*this);
        }
    protected:
        virtual int Load() = 0;
        virtual const char* GetBuffer()const = 0;
    };
}
using namespace FunctionScript;

#endif
