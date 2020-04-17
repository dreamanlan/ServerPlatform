/*****************************************************************************

calc.h

******************************************************************************/

#ifndef _CALC_H
#define _CALC_H

#include "Type.h"
#include "Queue.h"
#include "Hashtable.h"

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

        DELTA_FUNCTION_PARAM = 2,
        DELTA_FUNCTION_LOCAL = 4,
        DELTA_FUNCTION_STATEMENT = 8,
        DELTA_STATEMENT_FUNCTION = 1,
    };

    enum
    {
        MAX_FUNCTION_DIMENSION_NUM = 8,
        MAX_LOCAL_NUM = 256,
        MAX_STATEMENT_NUM = 1024,
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
        InterpreterOptions(void) :
            m_MaxFunctionDimensionNum(MAX_FUNCTION_DIMENSION_NUM),
            m_MaxLocalNum(MAX_LOCAL_NUM),
            m_MaxStatementNum(MAX_STATEMENT_NUM),
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
        int GetMaxStatementNum() const { return m_MaxStatementNum; }
        void SetMaxStatementNum(int val) { m_MaxStatementNum = val; }
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
        int GetSyntaxComponentPoolSize(void) const { return m_SyntaxComponentPoolSize; }
        void SetSyntaxComponentPoolSize(int val) { m_SyntaxComponentPoolSize = val; }
    private:
        int	m_MaxFunctionDimensionNum;
        int	m_MaxLocalNum;
        int	m_MaxStatementNum;
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
    class ISyntaxComponent;
    class Value
    {
    public:
        enum
        {
            TYPE_INVALID = -1,
            TYPE_INT = 0,
            TYPE_INT64,
            TYPE_FLOAT,
            TYPE_DOUBLE,
            TYPE_BOOL,
            TYPE_STRING,
            TYPE_ALLOC_STRING,
            TYPE_IDENTIFIER,
            TYPE_ARG_INDEX,
            TYPE_LOCAL_INDEX,
            TYPE_INDEX,
            TYPE_EXPRESSION,
            TYPE_STATEMENT,
            TYPE_SYNTAX_COMPONENT,
            TYPE_PTR,
        };

        Value(void) :m_Type(TYPE_INVALID), m_Int64Val(0), m_Line(0) {}
        explicit Value(int val) :m_Type(TYPE_INT), m_IntVal(val), m_Line(0) {}
        explicit Value(long long val) :m_Type(TYPE_INT64), m_Int64Val(val), m_Line(0) {}
        explicit Value(float val) :m_Type(TYPE_FLOAT), m_FloatVal(val), m_Line(0) {}
        explicit Value(double val) :m_Type(TYPE_DOUBLE), m_DoubleVal(val), m_Line(0) {}
        explicit Value(bool val) :m_Type(TYPE_BOOL), m_BoolVal(val), m_Line(0) {}
        explicit Value(char* val) :m_Type(TYPE_STRING), m_StringVal(val), m_Line(0) {}
        explicit Value(const char* val) :m_Type(TYPE_STRING), m_ConstStringVal(val), m_Line(0) {}
        explicit Value(ExpressionApi* val) :m_Type(TYPE_EXPRESSION), m_Expression(val), m_Line(0) {}
        explicit Value(StatementApi* val) :m_Type(TYPE_STATEMENT), m_Statement(val), m_Line(0) {}
        explicit Value(ISyntaxComponent* val) :m_Type(TYPE_SYNTAX_COMPONENT), m_SyntaxComponent(val), m_Line(0) {}
        explicit Value(void* val) :m_Type(TYPE_PTR), m_Ptr(val), m_Line(0) {}
        explicit Value(int val, int type) :m_Type(type), m_IntVal(val), m_Line(0) {}
        explicit Value(char* val, int type) :m_Type(type), m_StringVal(val), m_Line(0) {}
        explicit Value(const char* val, int type) :m_Type(type), m_ConstStringVal(val), m_Line(0) {}
        ~Value(void)
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

        int IsInvalid(void)const { return (m_Type == TYPE_INVALID ? TRUE : FALSE); }
        int IsInt(void)const { return (m_Type == TYPE_INT ? TRUE : FALSE); }
        int IsInt64(void)const { return (m_Type == TYPE_INT64 ? TRUE : FALSE); }
        int IsFloat(void)const { return (m_Type == TYPE_FLOAT ? TRUE : FALSE); }
        int IsDouble(void)const { return (m_Type == TYPE_DOUBLE ? TRUE : FALSE); }
        int IsBool(void)const { return (m_Type == TYPE_BOOL ? TRUE : FALSE); }
        int IsString(void)const { return ((m_Type == TYPE_STRING || m_Type == TYPE_ALLOC_STRING) ? TRUE : FALSE); }
        int IsIdentifier(void)const { return (m_Type == TYPE_IDENTIFIER ? TRUE : FALSE); }
        int IsArgIndex(void)const { return (m_Type == TYPE_ARG_INDEX ? TRUE : FALSE); }
        int IsLocalIndex(void)const { return (m_Type == TYPE_LOCAL_INDEX ? TRUE : FALSE); }
        int IsIndex(void)const { return (m_Type == TYPE_INDEX ? TRUE : FALSE); }
        int IsExpression(void)const { return (m_Type == TYPE_EXPRESSION ? TRUE : FALSE); }
        int IsStatement(void)const { return (m_Type == TYPE_STATEMENT ? TRUE : FALSE); }
        int IsSyntaxComponent(void)const { return (m_Type == TYPE_SYNTAX_COMPONENT ? TRUE : FALSE); }
        int IsPtr(void)const { return (m_Type == TYPE_PTR ? TRUE : FALSE); }
        int GetLine(void)const { return m_Line; }
        int GetType(void)const { return m_Type; }
        int GetInt(void)const { return m_IntVal; }
        long long GetInt64(void)const { return m_Int64Val; }
        float GetFloat(void)const { return m_FloatVal; }
        double GetDouble(void)const { return m_DoubleVal; }
        bool GetBool(void)const { return m_BoolVal; }
        char* GetString(void)const { return m_StringVal; }
        ExpressionApi* GetExpression(void)const { return m_Expression; }
        StatementApi* GetStatement(void)const { return m_Statement; }
        ISyntaxComponent* GetSyntaxComponent(void)const { return m_SyntaxComponent; }
        void* GetPtr(void)const { return m_Ptr; }
        void SetLine(int line)
        {
            m_Line = line;
        }
        void SetInvalid(void)
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
        void SetInt64(long long val)
        {
            FreeString();
            m_Type = TYPE_INT64;
            m_Int64Val = val;
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
        void SetExpression(ExpressionApi* exp)
        {
            FreeString();
            m_Type = TYPE_EXPRESSION;
            m_Expression = exp;
        }
        void SetStatement(StatementApi* statement)
        {
            FreeString();
            m_Type = TYPE_STATEMENT;
            m_Statement = statement;
        }
        void SetSyntaxComponent(ISyntaxComponent* comp)
        {
            FreeString();
            m_Type = TYPE_SYNTAX_COMPONENT;
            m_SyntaxComponent = comp;
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
        int ToInt(void)const
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
        long long ToInt64(void)const
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
        float ToFloat(void)const
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
                return m_BoolVal ? 1 : 0;
            case TYPE_STRING:
            case TYPE_ALLOC_STRING:
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
        double ToDouble(void)const
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
        bool ToBool(void)const
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
            }
            else {
                m_Type = other.GetType();
                m_Int64Val = other.m_Int64Val;
            }
        }
        void FreeString(void)
        {
            if (TYPE_ALLOC_STRING == m_Type) {
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
            ExpressionApi* m_Expression;
            StatementApi* m_Statement;
            ISyntaxComponent* m_SyntaxComponent;
            void* m_Ptr;
            const char* m_ConstStringVal;//�ڽű�����m_StringVal������ͬ,����ʵ���Զ�const_cast
        };
        int m_Line;
    public:
        static Value& GetInvalidValueRef(void)
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
            TYPE_NULL,
            TYPE_VALUE,
            TYPE_CALL,
            TYPE_FUNCTION,
            TYPE_STATEMENT,
        };
    public:
        explicit ISyntaxComponent(int syntaxType, Interpreter& interpreter) :m_Interpreter(&interpreter) {}
        virtual ~ISyntaxComponent(void) {}
    public:
        virtual int IsValid(void) const = 0;
        virtual const char* GetId(void)const = 0;
        virtual int GetIdType(void) const = 0;
        virtual int GetLine(void) const = 0;
        virtual void PrepareRuntimeObject(void) = 0;
        virtual const Value& GetRuntimeObject(void)const = 0;
	public:
        int GetSyntaxType(void) const { return m_SyntaxType; }
    public:
        Interpreter& GetInterpreter(void)const { return *m_Interpreter; }
    protected:
        int	m_SyntaxType;
        Interpreter* m_Interpreter;
    };

    class NullSyntax : public ISyntaxComponent
    {
    public:
        NullSyntax(Interpreter& interpreter) : ISyntaxComponent(ISyntaxComponent::TYPE_NULL, interpreter) {}
    public:
        virtual int IsValid(void) const { return FALSE; }
        virtual const char* GetId(void) const { return ""; }
        virtual int GetIdType(void) const { return Value::TYPE_IDENTIFIER; }
        virtual int GetLine(void) const { return 0; }
        virtual void PrepareRuntimeObject(void) {}
        virtual const Value& GetRuntimeObject(void)const
        {
            return Value::GetInvalidValueRef();
        }
    private:
        NullSyntax(const NullSyntax&) = delete;
        NullSyntax& operator=(const NullSyntax&) = delete;
    };

    //��ΪValue����������ʱ��ֵ��ʾ����Ҫ��PODһ������������ר��Ϊ�﷨���ṩһ�����ʾ�﷨��ı�ʶ���������������Ͳ�����
    //�������Ҫ������ISyntaxComponent���ʵ����Σ�����CallData�ڽӿ����ṩ��ֱ�Ӳ���Value�ķ�����
    class ValueData : public ISyntaxComponent
    {
    public:
        virtual int IsValid(void)const
        {
            return FALSE == m_Value.IsInvalid();
        }
        virtual const char*	GetId(void)const
        {
            if (m_Value.IsString())
                return m_Value.GetString();
            return 0;
        }
        virtual int GetIdType(void) const { return m_Value.GetType(); }
        virtual int GetLine(void)const { return m_Value.GetLine(); }
        virtual void PrepareRuntimeObject(void) {}
        virtual const Value& GetRuntimeObject(void)const { return m_Value; }
    public:
        void SetValue(const Value& val) { m_Value = val; }
        Value& GetValue(void) { return m_Value; }
        const Value& GetValue(void)const { return m_Value; }
    public:
        bool HaveId()const { return IsValid(); }
        int IsHighOrder(void)const { return m_Value.IsSyntaxComponent(); }
    public:
        explicit ValueData(Interpreter& interpreter);
        virtual ~ValueData(void) {}
    private:
        ValueData(const ValueData& other) = delete;
        ValueData operator=(const ValueData& other) = delete;
    private:
        Value m_Value;
    private:
        InterpreterValuePool* m_pInnerValuePool;
    };

    class CallData : public ISyntaxComponent
    {
    public:
        enum
        {
            PARAM_CLASS_NOTHING = 0,
            PARAM_CLASS_PARENTHESIS,
            PARAM_CLASS_BRACKET,
            PARAM_CLASS_PERIOD,
            PARAM_CLASS_PERIOD_PARENTHESIS,
            PARAM_CLASS_PERIOD_BRACKET,
            PARAM_CLASS_PERIOD_BRACE,
            PARAM_CLASS_QUESTION_PERIOD,
            PARAM_CLASS_QUESTION_PARENTHESIS,
            PARAM_CLASS_QUESTION_BRACKET,
            PARAM_CLASS_QUESTION_BRACE,
            PARAM_CLASS_POINTER,
            PARAM_CLASS_PERIOD_STAR,
            PARAM_CLASS_QUESTION_PERIOD_STAR,
            PARAM_CLASS_POINTER_STAR,
            PARAM_CLASS_OPERATOR,
            PARAM_CLASS_TERNARY_OPERATOR,
            PARAM_CLASS_MAX,
            PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK = 0x00010000,
            PARAM_CLASS_UNMASK = 0x0000FFFF,
        };
        typedef ISyntaxComponent* SyntaxComponentPtr;
    public:
        virtual int IsValid(void)const
        {
            if (HaveId())
                return TRUE;
            else if (HaveParam())
                return TRUE;
            else
                return FALSE;
        }
        virtual int GetIdType(void)const { return m_Name.GetIdType(); }
        virtual const char* GetId(void)const { return m_Name.GetId(); }
        virtual int GetLine(void)const { return m_Name.GetLine(); }
        void SetNameValue(const Value& val) { m_Name.SetValue(val); }
        Value& GetNameValue(void) { return m_Name.GetValue(); }
        ValueData& GetName(void) { return m_Name; }
        void ClearParams(void) { m_ParamNum = 0; }
        void AddParam(ISyntaxComponent*	pVal)
        {
            if (0 == pVal || m_ParamNum < 0 || m_ParamNum >= MAX_FUNCTION_PARAM_NUM)
                return;
            PrepareParams();
            if (0 == m_Params || m_ParamNum >= m_ParamSpace)
                return;
            m_Params[m_ParamNum] = pVal;
            ++m_ParamNum;
        }
        void SetParam(int index, ISyntaxComponent* pVal)
        {
            if (NULL == pVal || index < 0 || index >= MAX_FUNCTION_PARAM_NUM)
                return;
            m_Params[index] = pVal;
        }
        void SetParamClass(int v) { m_ParamClass = v; }
        int GetParamClass(void)const { return m_ParamClass; }
        int HaveId(void)const { return m_Name.HaveId(); }
        int HaveParam(void)const { return m_ParamClass != PARAM_CLASS_NOTHING; }
        int IsHighOrder(void)const { return m_Name.IsHighOrder(); }
    public:
        const Value& GetNameValue(void)const { return m_Name.GetValue(); }
        const ValueData& GetName(void)const { return m_Name; }
        int GetParamNum(void)const { return m_ParamNum; }
        ISyntaxComponent* GetParam(int index)const
        {
            if (0 == m_Params || index < 0 || index >= m_ParamNum || index >= MAX_FUNCTION_PARAM_NUM)
                return 0;
            return m_Params[index];
        }
        const char* GetParamId(int index)const
        {
            if (0 == m_Params || index < 0 || index >= m_ParamNum || index >= MAX_FUNCTION_PARAM_NUM)
                return 0;
            return m_Params[index]->GetId();
        }
    public:
        virtual void PrepareRuntimeObject(void);
        virtual const Value& GetRuntimeObject(void)const;
    public:
        explicit CallData(Interpreter& interpreter);
        virtual ~CallData(void);
    private:
        CallData(const CallData& other) = delete;
        CallData operator=(const CallData& other) = delete;
    private:
        void PrepareParams(void);
        void ReleaseParams(void);
    private:
        ValueData m_Name;
        ISyntaxComponent** m_Params;
        int m_ParamNum;
        int m_ParamSpace;
        int m_ParamClass;
        int m_MaxLocalNum;
    private:
        Value m_RuntimeFunctionCall;
        int m_RuntimeObjectPrepared;

        InterpreterValuePool* m_pInnerValuePool;
    public:
        static CallData*& GetNullCallPtrRef(void)
        {
            static CallData* s_P = 0;
            return s_P;
        }
    };

    /*
    * FunctionData��StatementData�������Ϣ�Ǳ���ʱ��Ϣ��������ʱ��˵������ֻ��Ԫ���ݡ�
    *
    * ����FunctionData�������п���Ҳ��FunctionData����һ��Value������FunctionData���������ͽ����ڱ���ʱ��������������ʱ�߼���һ���֡�
    * ���Ƶģ�����FunctionData��StatementData���ɵ�����ʱ�����п�����һ����ֵͨ��Ҳ������һ��StatementApi������Ҳ��һ��Value����
    * ��StatementApi����������ͬ�������ɱ���ʱ��������������ʱ�߼���һ���֡�
    *
    * ������ʱ��StatementData���߶�Ӧ��һ���ڲ����StatementApi��������һ��RuntimeStatement����������ʽ��䣨���һ��
    * StatementDataֻ��һ���������ã������˻�Ϊһ��RuntimeFunction����һ�����������ֻ��һ�����֣������˻�Ϊһ����ֵͨ����
    *
    * ������ʱ�����ɺ���������ò������ɵ�FunctionData���߶�Ӧ��һ����ͨ��ֵValue��������һ��RuntimeFunctionCall���������ڲ��������á�
    * ���к���������FunctionData������ʱ��Ӧ��StatementApi��RuntimeStatement��
    *
    * RuntimeStatementBlock���ڸ���StatementApi��RuntimeStatementʵ�������ִ�С�
    *
    * ʵ��ʹ��ʱ��Ҫע�⣬һ����StatementApi::IsMatch��������ʵ���Value���Ǳ���ʱ����Ϣ��StatementApi::PrepareRuntimeObject
    * �����������StatementData::PrepareRuntimeObjectWithFunctions�����Ѿ���Ϊ����ʱ��Ϣ��֮ǰ�����Ǳ���ʱ��Ϣ����StatementApi::Execute
    * ��ExpressionApi::Execute��ʱ��϶�������ʱ��Ϣ�ˡ�
    *
    * ��������������ʱ������Ӧ�ñ���ʵ�ָ��õ��������ܡ�
    */
    class StatementData;
    class RuntimeStatementBlock;
    class FunctionData : public ISyntaxComponent
    {
    public:
        enum
        {
            EXTENT_CLASS_NOTHING = 0,
            EXTENT_CLASS_STATEMENT,
            EXTENT_CLASS_EXTERN_SCRIPT,
            EXTENT_CLASS_MAX,
        };
        typedef StringKeyT<MAX_TOKEN_NAME_SIZE> StringKey;
        typedef HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> > LocalIndexes;
        typedef ISyntaxComponent* SyntaxComponentPtr;
    public:
        virtual int IsValid(void)const
        {
            if (m_Call.IsValid())
                return TRUE;
            else if (HaveStatement() || HaveExternScript())
                return TRUE;
            else
                return FALSE;
        }
        virtual int GetIdType(void)const { return m_Call.GetIdType(); }
        virtual const char* GetId(void)const { return m_Call.GetId(); }
        virtual int GetLine(void)const { return m_Call.GetLine(); }
    public:
        CallData& GetCall(void) { return m_Call; }
        void ClearStatements(void) { m_StatementNum = 0; }
        void AddStatement(ISyntaxComponent* pVal)
        {
            if (0 == pVal || m_StatementNum < 0 || m_StatementNum >= m_MaxStatementNum)
                return;
            PrepareStatements();
            if (0 == m_Statements || m_StatementNum >= m_StatementSpace)
                return;
            m_Statements[m_StatementNum] = pVal;
            ++m_StatementNum;
        }
        void SetStatement(int index, ISyntaxComponent* pVal)
        {
            if (NULL == pVal || index < 0 || index >= m_MaxStatementNum)
                return;
            m_Statements[index] = pVal;
        }
        void SetExternScript(const char* scp) { m_ExternScript = scp; }
        void SetExtentClass(int v) { m_ExtentClass = v; }
        int GetExtentClass(void)const { return m_ExtentClass; }
        int HaveId(void)const { return m_Call.HaveId(); }
        int HaveParam(void)const { return m_Call.HaveParam(); }
        int HaveStatement(void)const { return m_ExtentClass == EXTENT_CLASS_STATEMENT; }
        int HaveExternScript(void)const { return m_ExtentClass == EXTENT_CLASS_EXTERN_SCRIPT; }
    public:
        const CallData&	GetCall(void)const { return m_Call; }
        int GetStatementNum(void)const { return m_StatementNum; }
        ISyntaxComponent* GetStatement(int index)const
        {
            if (0 == m_Statements || index < 0 || index >= m_StatementNum || index >= m_MaxStatementNum)
                return 0;
            return m_Statements[index];
        }
        const char*	GetExternScript(void)const { return m_ExternScript; }
        int CalculateStackSize(void)const;
    public:
        int AllocLocalIndex(const char* id);
        int GetLocalIndex(const char* id)const;
        const char* GetLocalName(int index)const;
    public:
        virtual void PrepareRuntimeObject(void);
        virtual const Value& GetRuntimeObject(void)const;
        RuntimeStatementBlock* GetRuntimeFunctionBody(void)const { return m_RuntimeStatementBlock; }
    public:
        explicit FunctionData(Interpreter& interpreter);
        virtual ~FunctionData(void);
    private:
        FunctionData(const FunctionData&) = delete;
        FunctionData& operator=(const FunctionData&) = delete;
    private:
        void PrepareStatements(void);
        void ReleaseStatements(void);
        void PrepareLocalIndexes(void);
        void ClearLocalIndexes(void);
    private:
        CallData m_Call;
        ISyntaxComponent** m_Statements;
        int m_StatementNum;
        int m_StatementSpace;
        int m_MaxStatementNum;
        const char* m_ExternScript;
        int m_ExtentClass;
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
    public:
        static FunctionData*& GetNullFunctionPtrRef(void)
        {
            static FunctionData* s_P = 0;
            return s_P;
        }
    };

    class StatementData : public ISyntaxComponent
    {
    public:
        virtual int IsValid(void)const
        {
            if (NULL != m_Functions && m_FunctionNum > 0 && m_Functions[0]->IsValid())
                return TRUE;
            else
                return FALSE;
        }
        virtual int GetIdType(void)const
        {
            int type = Value::TYPE_IDENTIFIER;
            if (IsValid()) {
                type = m_Functions[0]->GetIdType();
            }
            return type;
        }
        virtual const char* GetId(void)const
        {
            const char* str = "";
            if (IsValid()) {
                str = m_Functions[0]->GetId();
            }
            return str;
        }
        virtual int GetLine(void)const
        {
            int line = 0;
            if (IsValid()) {
                line = m_Functions[0]->GetLine();
            }
            return line;
        }
    public:
        virtual void PrepareRuntimeObject(void);
        const Value& GetRuntimeObject(void)const;
        void PrepareRuntimeObjectWithFunctions(void);
    public:
        void ClearFunctions(void) { m_FunctionNum = 0; }
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
        FunctionData*& GetLastFunctionRef(void)const
        {
            if (NULL == m_Functions || 0 == m_FunctionNum)
                return FunctionData::GetNullFunctionPtrRef();
            else
                return m_Functions[m_FunctionNum - 1];
        }
    public:
        int GetFunctionNum(void)const { return m_FunctionNum; }
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
        virtual ~StatementData(void)
        {
            ReleaseFunctions();
        }
    private:
        StatementData(const StatementData&) = delete;
        StatementData& operator=(const StatementData&) = delete;
    private:
        void PrepareFunctions(void);
        void ReleaseFunctions(void);
    private:
        FunctionData**	m_Functions;
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
            m_ValuePool = new Value*[numValue];
            if (m_ValueMemory && m_ValuePool) {
                m_ValueNum = numValue;
                for (int i = 0; i < numValue; ++i) {
                    m_ValuePool[i] = &m_ValueMemory[i];
                }
            }
            m_ValuesPool = new Value*[numValues];
            if (m_ValuesPool) {
                m_ValuesNum = numValues;
                for (int i = 0; i < numValues; ++i) {
                    m_ValuesPool[i] = new Value[MAX_FUNCTION_PARAM_NUM];
                }
            }
        }
        ~InterpreterValuePool(void)
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
        void Reset(void)
        {
            m_UsedValueNum = 0;
            m_UsedValuesNum = 0;
        }
        Value* AllocValue(void)
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
        Value* AllocValues(void)
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
        ~AutoInterpreterValuePoolValueOperation(void)
        {
            m_Pool.RecycleValue(&m_Value);
        }
    public:
        Value& Get(void) const { return m_Value; }
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
        ~AutoInterpreterValuePoolValuesOperation(void)
        {
            m_Pool.RecycleValues(m_Value);
        }
    public:
        Value* Get(void) const { return m_Value; }
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
        virtual ~RuntimeComponent(void) {}
    protected:
        void ReplaceVariableWithValue(Value* pParams, int num)const;
        void ReplaceVariableWithValue(Value& p)const;
        void SetVariableValue(const Value& p, const Value& val)const;
        const Value& GetVariableValue(const Value& p)const;
    protected:
        Interpreter* m_Interpreter;
    };

    /*
    * ������ʵ��APIʱע�������и�Լ�������в�������API�Լ��������ֵ�����ݵ���ԭʼ������Ϣ����������ʵ�ָ�ֵ��out�������ԣ�
    * �ر�ģ��Զ�������ִ��ʱ����API�������Լ��������ǲ���ҪΪAPI�������ֵ�ġ�
    * ��һ�����������������һ��StatementApi������Ҫִ�к󴫵��䷵��ֵ��
    * ����һ��Լ����Value����ΪStatementApi��FunctionData�ģ������ɽű��߼���������2�����͵�ֵ�Ǳ��뻷����һ���֣�����ֻ���ڱ�
    * ��ʱ�Ż����������Value��Ҳ����˵һ��Value������Ϊ��ֵ����StatementApi��FunctionData�����ڲ�������ΪStatementApi��FunctionData
    * ��Value���������ڵ��þ���APIʱ����ת��Ϊ��ͨ��ֵ������RuntimeComponent��ļ������ֵ�ķ�������Ҫ������Щ���Σ�������
    * ���ݲ�����Ϣ�滻�ɶ�Ӧ�Ĳ���ֵ��
    *
    * ExpressionApi::Execute
    * ���淽������APIʵ��ʱ��ʵ����Ҫ���ϴ�const���ε����壬��ΪAPIע���һ��ʵ�������ط�ʹ�á��ӿ��޷���Ӵ�Լ������
    * Ϊʵ����Щ�ӿڵĲ�ֻ��ע���API,����һЩ���Բ������ʵ��������ʱ����
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

    class StatementApiFactory
    {
    public:
        virtual int IsMatch(const StatementData& statement)const = 0;
        virtual StatementApi* PrepareRuntimeObject(StatementData& statement)const = 0;
    };

    class Closure : public ExpressionApi
    {
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetDefinitionRef(const Value* pArguments, int argumentNum, const StatementData& statement);
    public:
        explicit Closure(Interpreter& interpreter) :ExpressionApi(interpreter), m_pArguments(NULL), m_pDefinition(NULL), m_StackSize(0), m_Statements(NULL) {}
    private:
        const Value* m_pArguments;
        int m_ArgumentNum;
        const StatementData* m_pDefinition;
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
        ExpressionApi* GetObject(void)const { return m_Object; }
        void SetMemberIndex(int memberIndex) { m_MemberIndex = memberIndex; }
        int GetMemberIndex(void) const { return m_MemberIndex; }
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
    typedef MemberAccessor* MemberAccessorPtr;

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

            MemberInfo(void) :m_Name(0)
            {
            }
            void Reset(void)
            {
                m_Name = 0;
                m_Value.SetInvalid();
            }
        };
        typedef StringKeyT<32> StringKey;
        typedef HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> > NameIndexMap;
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        explicit ObjectBase(Interpreter& interpreter, int customInnerMemberNum);
        virtual ~ObjectBase(void);
    protected:
        virtual int	 GetCustomInnerMemberIndex(const char* name)const { return -1; }
        virtual ExecuteResultEnum ExecuteCustomMember(int index, int paramClass, Value* pParams, int num, Value* pRetValue) { return EXECUTE_RESULT_NORMAL; }
    private:
        void Resize(void);
        int GetMemberIndex(const char* name)const;
        void ResetTemp(void);
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
        virtual ~Object(void);
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

            MemberInfo(void) :m_Offset(0), m_Size(0), m_Num(0), m_Name(0)
            {
            }
        };
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetDefinitionRef(const StatementData& statement);
        void Attach(unsigned int addr);
        Struct* Clone(void) const;
    public:
        explicit Struct(Interpreter& interpreter) :ExpressionApi(interpreter), m_pDefinition(0), m_MemberInfos(0), m_Accessors(0), m_MemberNum(0), m_Size(0), m_Addr(0) {}
        virtual ~Struct(void);
    private:
        int GetMemberIndex(const char* name)const;
    private:
        const StatementData* m_pDefinition;
        MemberInfo* m_MemberInfos;
        MemberAccessorPtr* m_Accessors;
        int m_MemberNum;

        unsigned int m_Size;
        unsigned int m_Addr;
    };

    class RuntimeFunctionCall : public StatementApi
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        explicit RuntimeFunctionCall(Interpreter& interpreter);
        virtual ~RuntimeFunctionCall(void);
        void Init(CallData& call);
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
        ~RuntimeStatementBlock(void);
    private:
        Interpreter* m_Interpreter;
        int m_StatementNum;
        StatementApi** m_Statements;

        InterpreterValuePool* m_pInnerValuePool;
    };

    class RuntimeStatement : public StatementApi
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        void PrepareRuntimeObject(StatementData& statement);
    public:
        explicit RuntimeStatement(Interpreter& interpreter);
        virtual ~RuntimeStatement(void);
    private:
        int m_FunctionNum;
        StatementApi** m_FunctionHeads;
        RuntimeStatementBlock** m_FunctionBodies;
    };

    class ErrorAndStringBuffer
    {
    public:
        void ClearErrorInfo(void);
        void AddError(const char* error);
        int HasError(void)const { return m_HasError; }
        int GetErrorNum(void)const { return m_ErrorNum; }
        const char*	GetErrorInfo(int index) const
        {
            if (index < 0 || index >= m_ErrorNum || index >= MAX_RECORD_ERROR_NUM)
                return "";
            return m_ErrorInfo[index];
        }
        char* NewErrorInfo(void)
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
        int GetUnusedStringLength(void)const
        {
            DebugAssert(m_pStringBuffer);
            DebugAssert(m_ppUnusedStringRef);
            return m_MaxStringBufferLength - int(*m_ppUnusedStringRef - m_pStringBuffer);
        }
        char*& GetUnusedStringPtrRef(void)
        {
            DebugAssert(m_ppUnusedStringRef);
            return *m_ppUnusedStringRef;
        }
    public:
        ErrorAndStringBuffer(void) :m_pStringBuffer(NULL), m_ppUnusedStringRef(NULL), m_MaxStringBufferLength(0)
        {
            ClearErrorInfo();
        }
        void Reset(char* pStringBuffer, char*& pUnusedStringRef, int maxStringBufferLength)
        {
            ClearErrorInfo();
            m_pStringBuffer = pStringBuffer;
            m_ppUnusedStringRef = &pUnusedStringRef;
            m_MaxStringBufferLength = maxStringBufferLength;
            DebugAssert(m_pStringBuffer);
            DebugAssert(m_ppUnusedStringRef);
        }
    private:
        int	m_HasError;
        char m_ErrorInfo[MAX_RECORD_ERROR_NUM][MAX_ERROR_INFO_CAPACITY];
        int m_ErrorNum;
        char* m_pStringBuffer;
        char**	m_ppUnusedStringRef;
        int m_MaxStringBufferLength;
    };

    /*
    * ���������ű����ԵĽ������ض�����C++����ջ�Ļ��ƣ����Բ��ʺ�ʵ��corountine���ƣ���Ϊ������C++��������ջ��ʵ�ֽű�ջ�����Խ�������һ��ִ�л���ȫĨ������ջ���ٴν���ʱ�޷�����֮ǰ��ջ�������ģ���
    * �˻��ƿ��Կ����ڽű���ʵ��������еķ�ʽ��ģ�⡣
    */
    class Interpreter
    {
        typedef ISyntaxComponent* SyntaxComponentPtr;
        typedef RuntimeComponent* RuntimeComponentPtr;
        typedef StatementData* StatementPtr;
        typedef StatementApi* StatementApiPtr;
        typedef StringKeyT<MAX_TOKEN_NAME_SIZE> StringKey;
        typedef HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> > ValueIndexes;
        typedef HashtableT<StringKey, ExpressionApi*, StringKey> Functions;
        typedef DequeT<StatementApiFactory*, MAX_FORM_NUM_PER_STATEMENT> StatementApiFactoryList;
        typedef HashtableT<StringKey, StatementApiFactoryList, StringKey> StatementApiFactories;
        typedef DequeT<FunctionData*, MAX_FUNCTION_LEVEL> FunctionDefinitionStack;

        struct StackInfo
        {
            int m_Start;
            Value* m_Params;
            Value m_Size;
            Value m_ParamNum;
            const StatementData* m_pDefinition;

            StackInfo(void) :m_Start(-1), m_Size(0), m_Params(0), m_ParamNum(0), m_pDefinition(0)
            {
            }
            int IsValid(void)const
            {
                return (m_Start >= 0 ? TRUE : FALSE);
            }
            static StackInfo& GetInvalidStackInfoRef(void);
        };
        typedef DequeT<StackInfo, MAX_STACK_LEVEL> StackInfos;
    public:
        void RegisterStatementApi(const char* id, StatementApiFactory* p);
        void RegisterPredefinedValue(const char* id, const Value& val);
        void PrepareRuntimeObject(void);
        ExecuteResultEnum Execute(Value* pRetValue);
        ExecuteResultEnum CallMember(ExpressionApi& obj, const Value& member, int isProperty, int paramClass, Value* pParams, int paramNum, Value* pRetValue);
    public:
        StatementApiFactory* GetLiteralArrayApi(void)const;
        StatementApiFactory* GetLiteralObjectApi(void)const;
        ExpressionApi* FindFunctionApi(const char* id)const;
        StatementApiFactory* FindStatementApi(const StatementData& statement)const;
        const Value& GetPredefinedValue(const char* id)const;
        void SetValue(const char* id, const Value& val);
        const Value& GetValue(const char* id)const;
        int GetValueIndex(const char* id)const;
        const char* GetValueName(int indexType, int index)const;
        void SetValue(int indexType, int index, const Value& val);
        const Value& GetValue(int indexType, int index)const;
        int GetValueNum(void)const { return m_ValueNum; }
    public:
        void PushStackInfo(Value* pParams, int paramNum, int stackSize, const StatementData& definition);
        void PopStackInfo(void);
        int IsStackEmpty(void)const;
        int IsStackFull(void)const;
        const StackInfo& GetTopStackInfo(void)const;
        int GetStackValueNum(void)const;
    public:
        int PushFunctionDefinition(FunctionData* pFunction);
        int PopFunctionDefinition(void);
        FunctionData* GetCurFunctionDefinition(void)const;
    public:
        void AddStatement(StatementData* p);
    public:
        FunctionData* AddNewFunctionComponent(void);
        StatementData* AddNewStatementComponent(void);
        RuntimeFunctionCall* AddNewRuntimeFunctionComponent(void);
        RuntimeStatement* AddNewRuntimeStatementComponent(void);
        Closure* AddNewClosureComponent(void);
        Object* AddNewObjectComponent(void);
        Struct* AddNewStructComponent(void);
    public:
        template<typename T> inline T* AddNewStatementApiComponent(void)
        {
            T* p = new T(*this);
            if (NULL != p) {
                AddRuntimeComponent(p);
            }
            return p;
        }
    public:
        void AddSyntaxComponent(ISyntaxComponent* p);
        int GetSyntaxComponentNum(void)const { return m_SyntaxComponentNum; }
        void AddRuntimeComponent(RuntimeComponent* p);
        int GetRuntimeComponentNum(void)const { return m_RuntimeComponentNum; }
    public:
        char* AllocString(int len);
        char* AllocString(const char* src);
        char* GetStringBuffer(void)const { return m_StringBuffer; }
        char*& GetUnusedStringPtrRef(void) { return m_UnusedStringPtr; }
    public:
        Interpreter(void);
        Interpreter(const InterpreterOptions& options);
        ~Interpreter(void);
        void Reset(void);
        int SetNextStatement(int ip)
        {
            if (ip < 0 || ip >= m_StatementNum)
                return FALSE;
            else
                m_NextStatement = ip;
        }
        int GetStatementNum(void)const { return m_StatementNum; }
    private:
        void Init(void);
        void Release(void);
        void InitInnerApis(void);
        void ReleaseInnerApis(void);
        void RegisterInnerFunctionApi(const char* id, ExpressionApi* p);
        void RegisterInnerStatementApi(const char* id, StatementApiFactory* p);
    private:
        char* m_StringBuffer;
        char* m_UnusedStringPtr;
        SyntaxComponentPtr* m_SyntaxComponentPool;
        int m_SyntaxComponentNum;
        RuntimeComponentPtr*  m_RuntimeComponentPool;
        int m_RuntimeComponentNum;
        StatementPtr* m_Program;
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
    public://�����ڽ����������ݳأ������ں��������������������ֲ�������alloc��recycle����ɶԲ��ҷ���ջ�Ĳ���˳�򣨺���ȳ���������
        InterpreterValuePool& GetInnerValuePool(void) { return m_InterpreterValuePool; }
    private:
        InterpreterValuePool m_InterpreterValuePool;
    public:
        void EnableDebugInfo(void) { m_IsDebugInfoEnable = TRUE; }
        void DisableDebugInfo(void) { m_IsDebugInfoEnable = FALSE; }
        int IsDebugInfoEnable(void)const { return m_IsDebugInfoEnable; }
        int IsRunFlagEnable(void)const
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
        void ClearErrorInfo(void) { m_ErrorAndStringBuffer.ClearErrorInfo(); }
        void AddError(const char* error) { m_ErrorAndStringBuffer.AddError(error); }
        int HasError(void)const { return m_ErrorAndStringBuffer.HasError(); }
        int GetErrorNum(void) { return m_ErrorAndStringBuffer.GetErrorNum(); }
        const char*	GetErrorInfo(int index) const { return m_ErrorAndStringBuffer.GetErrorInfo(index); }
        char* NewErrorInfo(void) { return m_ErrorAndStringBuffer.NewErrorInfo(); }
    public:
        ErrorAndStringBuffer& GetErrorAndStringBuffer(void) { return m_ErrorAndStringBuffer; }
        const ErrorAndStringBuffer&	GetErrorAndStringBuffer(void)const { return m_ErrorAndStringBuffer; }
    private:
        ErrorAndStringBuffer m_ErrorAndStringBuffer;
    public:
        InterpreterOptions& GetOptions(void) { return m_Options; }
        const InterpreterOptions& GetOptions(void)const { return m_Options; }
    private:
        InterpreterOptions m_Options;
    public:
        NullSyntax& GetNullSyntaxRef()
        {
            return m_NullSyntax;
        }
    private:
        NullSyntax m_NullSyntax;
    };

    class IScriptSource
    {
    public:
        virtual ~IScriptSource(void) {}
    public:
        class Iterator
        {
        public:
            const char& operator * (void) const
            {
                return Peek(0);
            }
            Iterator& operator ++ (void)
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
            int Load(void)
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
            const char* GetBuffer(void)const
            {
                return m_pSource->GetBuffer();
            }
            const char* GetLeft(void)const
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
            Iterator(void) :m_pSource(NULL), m_Iterator(""), m_EndIterator(m_Iterator)
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
            void Advance(void)
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
        Iterator GetIterator(void)
        {
            return Iterator(*this);
        }
    protected:
        virtual int Load(void) = 0;
        virtual const char* GetBuffer(void)const = 0;
    };
}
using namespace FunctionScript;

#endif
