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
    class Function;
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
            TYPE_STRING,
            TYPE_ALLOC_STRING,
            TYPE_VARIABLE_NAME,
            TYPE_ARG_INDEX,
            TYPE_LOCAL_INDEX,
            TYPE_INDEX,
            TYPE_EXPRESSION,
            TYPE_STATEMENT,
            TYPE_FUNCTION,
            TYPE_PTR,
        };

        Value(void) :m_Type(TYPE_INVALID), m_Int64Val(0) {}
        explicit Value(int val) :m_Type(TYPE_INT), m_IntVal(val) {}
        explicit Value(long long val) :m_Type(TYPE_INT64), m_Int64Val(val) {}
        explicit Value(float val) :m_Type(TYPE_FLOAT), m_FloatVal(val) {}
        explicit Value(double val) :m_Type(TYPE_DOUBLE), m_DoubleVal(val) {}
        explicit Value(char* val) :m_Type(TYPE_STRING), m_StringVal(val) {}
        explicit Value(const char* val) :m_Type(TYPE_STRING), m_ConstStringVal(val) {}
        explicit Value(ExpressionApi* val) :m_Type(TYPE_EXPRESSION), m_Expression(val) {}
        explicit Value(StatementApi* val) :m_Type(TYPE_STATEMENT), m_Statement(val) {}
        explicit Value(Function* val) :m_Type(TYPE_FUNCTION), m_Function(val) {}
        explicit Value(void* val) :m_Type(TYPE_PTR), m_Ptr(val) {}
        explicit Value(int val, int type) :m_Type(type), m_IntVal(val) {}
        explicit Value(char* val, int type) :m_Type(type), m_StringVal(val) {}
        explicit Value(const char* val, int type) :m_Type(type), m_ConstStringVal(val) {}
        ~Value(void)
        {
            FreeString();
        }
        Value(const Value& other) :m_Type(TYPE_INVALID), m_Int64Val(0)
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
            } else {
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
        int IsString(void)const { return ((m_Type == TYPE_STRING || m_Type == TYPE_ALLOC_STRING) ? TRUE : FALSE); }
        int IsVariableName(void)const { return (m_Type == TYPE_VARIABLE_NAME ? TRUE : FALSE); }
        int IsArgIndex(void)const { return (m_Type == TYPE_ARG_INDEX ? TRUE : FALSE); }
        int IsLocalIndex(void)const { return (m_Type == TYPE_LOCAL_INDEX ? TRUE : FALSE); }
        int IsIndex(void)const { return (m_Type == TYPE_INDEX ? TRUE : FALSE); }
        int IsExpression(void)const { return (m_Type == TYPE_EXPRESSION ? TRUE : FALSE); }
        int IsStatement(void)const { return (m_Type == TYPE_STATEMENT ? TRUE : FALSE); }
        int IsFunction(void)const { return (m_Type == TYPE_FUNCTION ? TRUE : FALSE); }
        int IsPtr(void)const { return (m_Type == TYPE_PTR ? TRUE : FALSE); }
        int GetType(void)const { return m_Type; }
        int GetInt(void)const { return m_IntVal; }
        long long GetInt64(void)const { return m_Int64Val; }
        float GetFloat(void)const { return m_FloatVal; }
        double GetDouble(void)const { return m_DoubleVal; }
        char* GetString(void)const { return m_StringVal; }
        ExpressionApi* GetExpression(void)const { return m_Expression; }
        StatementApi* GetStatement(void)const { return m_Statement; }
        Function* GetFunction(void)const { return m_Function; }
        void* GetPtr(void)const { return m_Ptr; }
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
        void SetFunction(Function* func)
        {
            FreeString();
            m_Type = TYPE_FUNCTION;
            m_Function = func;
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
        void SetVariableName(char* name)
        {
            FreeString();
            m_Type = TYPE_VARIABLE_NAME;
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
            } else {
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
                return(int)m_FloatVal;
            case TYPE_DOUBLE:
                return(int)m_DoubleVal;
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
            case TYPE_VARIABLE_NAME:
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
            } else {
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
        int					m_Type;
        union
        {
            int				m_IntVal;
            long long		m_Int64Val;
            float			m_FloatVal;
            double			m_DoubleVal;
            char*			m_StringVal;
            ExpressionApi*	m_Expression;
            StatementApi* m_Statement;
            Function*		m_Function;
            void*			m_Ptr;
            const char*		m_ConstStringVal;//在脚本里与m_StringVal类型相同,用于实现自动const_cast
        };
    public:
        static Value& GetInvalidValueRef(void)
        {
            static Value s_Val;
            s_Val.SetInvalid();
            return s_Val;
        }
    };

    class Interpreter;
    class SyntaxComponent
    {
    public:
        explicit SyntaxComponent(Interpreter& interpreter) :m_Interpreter(&interpreter) {}
        virtual ~SyntaxComponent(void) {}
    public:
        Interpreter& GetInterpreter(void)const { return *m_Interpreter; }
    protected:
        Interpreter*	m_Interpreter;
    };

    /*
    * Function与Statement代表的信息是编译时信息，对运行时来说，它们只是元数据。
    *
    * 由于Function的名称有可能也是Function，有一种Value类型是Function，这种类型仅仅在编译时产生，不是运行时逻辑的一部分。
    * 类似的，根据Function与Statement生成的运行时对象有可能是一个普通值，也可能是一个StatementApi，所以也有一种Value类型
    * 是StatementApi，这种类型同样仅仅由编译时产生，不是运行时逻辑的一部分。
    *
    * 在运行时，Statement或者对应到一个内部语句StatementApi，或者是一个RuntimeStatement用来处理表达式语句（如果一个
    * Statement只是一个函数调用，可以退化为一个RuntimeFunction，进一步，如果函数只有一个名字，可以退化为一个普通值）。
    *
    * 在运行时，仅由函数名与调用参数构成的Function或者对应到一个普通的值Value，或者是一个RuntimeFunctionCall用来处理内部函数调用。
    * 带有函数语句体的Function在运行时对应到StatementApi或RuntimeStatement。
    *
    * RuntimeStatementBlock用于辅助StatementApi与RuntimeStatement实现语句块的执行。
    *
    * 实际使用时需要注意，一般在StatementApi::IsMatch方法里访问到的Value都是编译时的信息，StatementApi::PrepareRuntimeObject
    * 里如果调用了Statement::PrepareRuntimeObjectWithFunctions，则已经变为运行时信息，之前则仍是编译时信息，在StatementApi::Execute
    * 与ExpressionApi::Execute里时则肯定是运行时信息了。
    *
    * 独立出来的运行时理论上应该便于实现更好的运行性能。
    */
    class Statement;
    class RuntimeStatementBlock;
    class InterpreterValuePool;
    class Function : public SyntaxComponent
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
            PARAM_CLASS_OPERATOR,
            PARAM_CLASS_TERNARY_OPERATOR,
            PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK = 0x00010000,
            PARAM_CLASS_UNMASK = 0x0000FFFF,
        };
        enum
        {
            EXTENT_CLASS_NOTHING = 0,
            EXTENT_CLASS_STATEMENT,
            EXTENT_CLASS_EXTERN_SCRIPT,
        };
        typedef StringKeyT<MAX_TOKEN_NAME_SIZE> StringKey;
        typedef HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> > LocalIndexes;
        typedef Statement* StatementPtr;
    public:
        void PrepareRuntimeObject(void);
        const Value& GetRuntimeFunctionHead(void)const;
        RuntimeStatementBlock* GetRuntimeFunctionBody(void)const { return m_RuntimeStatementBlock; }
    public:
        void		SetName(const Value& val) { m_Name = val; }
        void		ClearParams(void) { m_ParamNum = 0; }
        void		AddParam(Statement*	pVal)
        {
            if (0 == pVal || m_ParamNum < 0 || m_ParamNum >= MAX_FUNCTION_PARAM_NUM)
                return;
            PrepareParams();
            if (0 == m_Params || m_ParamNum >= m_ParamSpace)
                return;
            m_Params[m_ParamNum] = pVal;
            ++m_ParamNum;
        }
        void		ClearStatements(void) { m_StatementNum = 0; }
        void		AddStatement(Statement* pVal)
        {
            if (0 == pVal || m_StatementNum < 0 || m_StatementNum >= m_MaxStatementNum)
                return;
            PrepareStatements();
            if (0 == m_Statements || m_StatementNum >= m_StatementSpace)
                return;
            m_Statements[m_StatementNum] = pVal;
            ++m_StatementNum;
        }
        void		SetExternScript(const char* scp) { m_ExternScript = scp; }
        void		SetParamClass(int v) { m_ParamClass = v; }
        int		GetParamClass(void)const { return m_ParamClass; }
        int		HaveName(void)const { return !m_Name.IsInvalid(); }
        int		HaveParam(void)const { return m_ParamClass != PARAM_CLASS_NOTHING; }
        void		SetExtentClass(int v) { m_ExtentClass = v; }
        int		GetExtentClass(void)const { return m_ExtentClass; }
        int		HaveStatement(void)const { return m_ExtentClass == EXTENT_CLASS_STATEMENT; }
        int		HaveExternScript(void)const { return m_ExtentClass == EXTENT_CLASS_EXTERN_SCRIPT; }
        int		IsValid(void)const
        {
            if (HaveName())
                return TRUE;
            else if (HaveParam())
                return TRUE;
            else if (HaveStatement() || HaveExternScript())
                return TRUE;
            else
                return FALSE;
        }
    public:
        const Value&	GetName(void)const { return m_Name; }
        int			GetParamNum(void)const { return m_ParamNum; }
        Statement*	GetParam(int index)const
        {
            if (0 == m_Params || index < 0 || index >= m_ParamNum || index >= MAX_FUNCTION_PARAM_NUM)
                return 0;
            return m_Params[index];
        }
        int			GetStatementNum(void)const { return m_StatementNum; }
        Statement*	GetStatement(int index)const
        {
            if (0 == m_Statements || index < 0 || index >= m_StatementNum || index >= m_MaxStatementNum)
                return 0;
            return m_Statements[index];
        }
        const char*	GetExternScript(void)const { return m_ExternScript; }
        int				CalculateStackSize(void)const;
    public:
        int             AllocLocalIndex(const char* id);
        int             GetLocalIndex(const char* id)const;
        const char*     GetLocalName(int index)const;
    public:
        explicit Function(Interpreter& interpreter);
        virtual ~Function(void);
    private:
        void            PrepareParams(void);
        void            ReleaseParams(void);
        void            PrepareStatements(void);
        void            ReleaseStatements(void);
        void            PrepareLocalIndexes(void);
        void            ClearLocalIndexes(void);
    private:
        Value			m_Name;
        Statement**		m_Params;
        int				m_ParamNum;
        int				m_ParamSpace;
        Statement**		m_Statements;
        int				m_StatementNum;
        int				m_StatementSpace;
        int				m_MaxStatementNum;
        const char*		m_ExternScript;
        int				m_ParamClass;
        int				m_ExtentClass;
    private:
        LocalIndexes    m_LocalIndexes;
        int             m_LocalNum;
        int				m_LocalSpace;
        int				m_MaxLocalNum;
    private:
        Value	    m_RuntimeFunctionCall;
        RuntimeStatementBlock* m_RuntimeStatementBlock;
        int			m_RuntimeObjectPrepared;

        InterpreterValuePool*	m_pInnerValuePool;
    public:
        static Function*& GetNullFunctionPtrRef(void)
        {
            static Function* s_P = 0;
            return s_P;
        }
    };

    class Statement : public SyntaxComponent
    {
    public:
        void PrepareRuntimeObject(void);
        const Value& GetRuntimeObject(void)const;
    public:
        void PrepareRuntimeObjectWithFunctions(void);
    public:
        void		ClearFunctions(void) { m_FunctionNum = 0; }
        void		AddFunction(Function* pVal)
        {
            if (NULL == pVal || m_FunctionNum < 0 || m_FunctionNum >= m_MaxFunctionNum)
                return;
            PrepareFunctions();
            if (NULL == m_Functions || m_FunctionNum >= m_FunctionSpace)
                return;
            m_Functions[m_FunctionNum] = pVal;
            ++m_FunctionNum;
        }
        Function*&	GetLastFunctionRef(void)
        {
            if (NULL == m_Functions || 0 == m_FunctionNum)
                return Function::GetNullFunctionPtrRef();
            else
                return m_Functions[m_FunctionNum - 1];
        }
        void		SetLine(int line) { m_Line = line; }
        int		GetLine(void)const { return m_Line; }
        int		IsValid(void)const
        {
            if (NULL != m_Functions && m_FunctionNum > 0 && m_Functions[0]->IsValid())
                return TRUE;
            else
                return FALSE;
        }
    public:
        int			GetFunctionNum(void)const { return m_FunctionNum; }
        Function*	GetFunction(int index)const
        {
            if (NULL == m_Functions || index < 0 || index >= m_FunctionNum || index >= m_MaxFunctionNum)
                return 0;
            return m_Functions[index];
        }
    public:
        explicit Statement(Interpreter& interpreter);
        virtual ~Statement(void)
        {
            ReleaseFunctions();
        }
    private:
        void				PrepareFunctions(void);
        void				ReleaseFunctions(void);
    private:
        Function**	m_Functions;
        int					m_FunctionNum;
        int					m_FunctionSpace;
        int					m_MaxFunctionNum;
    private:
        int					m_Line;
    private:
        Value   		m_RuntimeObject;
        int				m_RuntimeObjectPrepared;
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
    class Statement;
    class RuntimeComponent
    {
    public:
        explicit RuntimeComponent(Interpreter& interpreter) :m_Interpreter(&interpreter) {}
        virtual ~RuntimeComponent(void) {}
    protected:
        void			ReplaceVariableWithValue(Value* pParams, int num)const;
        void			ReplaceVariableWithValue(Value& p)const;
        void			SetVariableValue(const Value& p, const Value& val)const;
        const Value&	GetVariableValue(const Value& p)const;
    protected:
        Interpreter*	m_Interpreter;
    };

    /*
    * 备忘：实现API时注意这里有个约定，所有参数都由API自己计算参数值，传递的是原始参数信息，这样便于实现赋值或out参数特性！
    * 特别的，自定义的语句执行时调用API，按这个约定，语句是不需要为API计算参数值的。
    * 有一个例外是如果参数是一个StatementApi，则需要执行后传递其返回值。
    * 还有一个约定，Value类型为StatementApi与Function的，不会由脚本逻辑产生，这2种类型的值是编译环境的一部分，所以只会在编
    * 译时才会产生这样的Value，也就是说一个Value不会因为求值后变成StatementApi或Function，对于参数类型为StatementApi或Function
    * 的Value，解释器在调用具体API时都会转换为普通的值，所以RuntimeComponent类的计算参数值的方法不需要考虑这些情形，仅仅是
    * 根据参数信息替换成对应的参数值。
    *
    * ExpressionApi::Execute
    * 上面方法用于API实现时，实现上要符合带const修饰的语义，因为API注册后，一个实例在许多地方使用。接口无法添加此约束，因
    * 为实现这些接口的不只是注册的API,还有一些可以产生多个实例的运行时对象。
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
        virtual int IsMatch(const Statement& statement)const = 0;
        virtual StatementApi* PrepareRuntimeObject(Statement& statement)const = 0;
    };

    class Closure : public ExpressionApi
    {
    public:
        virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue);
    public:
        void SetDefinitionRef(const Value* pArguments, int argumentNum, const Statement& statement);
    public:
        explicit Closure(Interpreter& interpreter) :ExpressionApi(interpreter), m_pArguments(NULL), m_pDefinition(NULL), m_StackSize(0), m_Statements(NULL) {}
    private:
        const Value* m_pArguments;
        int m_ArgumentNum;
        const Statement* m_pDefinition;
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
        ExpressionApi*	m_Object;
        int 			m_MemberIndex;
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
        NameIndexMap*			m_NameIndexMap;
        MemberInfo*				m_MemberInfos;
        MemberAccessorPtr*		m_Accessors;
        int						m_MemberNum;
        int						m_Capacity;
    private://
        MemberAccessorPtr*		m_InnerMemberAccessors;
        int						m_InnerMemberNum;
    private:
        MemberInfo				m_TempMemberInfo;
        MemberAccessor			m_TempAccessor;
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
        void SetDefinitionRef(const Statement& statement);
        void Attach(unsigned int addr);
        Struct* Clone(void) const;
    public:
        explicit Struct(Interpreter& interpreter) :ExpressionApi(interpreter), m_pDefinition(0), m_MemberInfos(0), m_Accessors(0), m_MemberNum(0), m_Size(0), m_Addr(0) {}
        virtual ~Struct(void);
    private:
        int GetMemberIndex(const char* name)const;
    private:
        const Statement*	m_pDefinition;
        MemberInfo*			m_MemberInfos;
        MemberAccessorPtr*	m_Accessors;
        int					m_MemberNum;

        unsigned int				m_Size;
        unsigned int				m_Addr;
    };

    class RuntimeFunctionCall : public StatementApi
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        explicit RuntimeFunctionCall(Interpreter& interpreter);
        virtual ~RuntimeFunctionCall(void);
        void Init(Function& func);
    private:
        Value m_Name;
        int m_ParamClass;
        int m_ParamNum;
        Value* m_Params;

        InterpreterValuePool*	m_pInnerValuePool;
    };

    class RuntimeStatementBlock
    {
    public:
        ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        RuntimeStatementBlock(Interpreter& interpreter, Function& func);
        ~RuntimeStatementBlock(void);
    private:
        Interpreter* m_Interpreter;
        int m_StatementNum;
        StatementApi** m_Statements;

        InterpreterValuePool*	m_pInnerValuePool;
    };

    class RuntimeStatement : public StatementApi
    {
    public:
        virtual ExecuteResultEnum Execute(Value* pRetValue)const;
    public:
        void PrepareRuntimeObject(Statement& statement);
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
        void				    ClearErrorInfo(void);
        void				    AddError(const char* error);
        int			HasError(void)const { return m_HasError; }
        int			GetErrorNum(void)const { return m_ErrorNum; }
        const char*	GetErrorInfo(int index) const
        {
            if (index < 0 || index >= m_ErrorNum || index >= MAX_RECORD_ERROR_NUM)
                return "";
            return m_ErrorInfo[index];
        }
        char*		NewErrorInfo(void)
        {
            m_HasError = TRUE;
            if (m_ErrorNum < MAX_RECORD_ERROR_NUM) {
                ++m_ErrorNum;
                return m_ErrorInfo[m_ErrorNum - 1];
            } else {
                return 0;
            }
        }
    public:
        int			GetUnusedStringLength(void)const
        {
            DebugAssert(m_pStringBuffer);
            DebugAssert(m_ppUnusedStringRef);
            return m_MaxStringBufferLength - int(*m_ppUnusedStringRef - m_pStringBuffer);
        }
        char*&		GetUnusedStringPtrRef(void)
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
        char	m_ErrorInfo[MAX_RECORD_ERROR_NUM][MAX_ERROR_INFO_CAPACITY];
        int		m_ErrorNum;
        char*	m_pStringBuffer;
        char**	m_ppUnusedStringRef;
        int		m_MaxStringBufferLength;
    };

    /*
    * 备忘：本脚本语言的解释器重度依赖C++函数栈的机制，所以不适合实现corountine机制（因为利用了C++函数调用栈来实现脚本栈，所以解释器的一次执行会完全抹掉调用栈，再次进入时无法继续之前的栈与上下文）。
    * 此机制可以考虑在脚本里实现命令队列的方式来模拟。
    */
    class Interpreter
    {
        typedef SyntaxComponent* SyntaxComponentPtr;
        typedef RuntimeComponent* RuntimeComponentPtr;
        typedef Statement* StatementPtr;
        typedef StatementApi* StatementApiPtr;
        typedef StringKeyT<MAX_TOKEN_NAME_SIZE> StringKey;
        typedef HashtableT<StringKey, int, StringKey, IntegerValueWorkerT<int> > ValueIndexes;
        typedef HashtableT<StringKey, ExpressionApi*, StringKey> Functions;
        typedef DequeT<StatementApiFactory*, MAX_FORM_NUM_PER_STATEMENT> StatementApiFactoryList;
        typedef HashtableT<StringKey, StatementApiFactoryList, StringKey> StatementApiFactories;
        typedef DequeT<Function*, MAX_FUNCTION_LEVEL> FunctionDefinitionStack;

        struct StackInfo
        {
            int                 m_Start;
            Value*              m_Params;
            Value	              m_Size;
            Value               m_ParamNum;
            const Statement*    m_pDefinition;

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
        void				  RegisterStatementApi(const char* id, StatementApiFactory* p);
        void				  RegisterPredefinedValue(const char* id, const Value& val);
        void				  PrepareRuntimeObject(void);
        ExecuteResultEnum	Execute(Value* pRetValue);
        ExecuteResultEnum	CallMember(ExpressionApi& obj, const Value& member, int isProperty, int paramClass, Value* pParams, int paramNum, Value* pRetValue);
    public:
        StatementApiFactory*	GetLiteralArrayApi(void)const;
        StatementApiFactory*	GetLiteralObjectApi(void)const;
        ExpressionApi*  FindFunctionApi(const char* id)const;
        StatementApiFactory*	FindStatementApi(const Statement& statement)const;
        const Value&	GetPredefinedValue(const char* id)const;
        void				  SetValue(const char* id, const Value& val);
        const Value&	GetValue(const char* id)const;
        int					  GetValueIndex(const char* id)const;
        const char*   GetValueName(int indexType, int index)const;
        void				  SetValue(int indexType, int index, const Value& val);
        const Value&	GetValue(int indexType, int index)const;
        int           GetValueNum(void)const { return m_ValueNum; }
    public:
        void				  PushStackInfo(Value* pParams, int paramNum, int stackSize, const Statement& definition);
        void				  PopStackInfo(void);
        int				  IsStackEmpty(void)const;
        int				  IsStackFull(void)const;
        const StackInfo&	GetTopStackInfo(void)const;
        int           GetStackValueNum(void)const;
    public:
        int				  PushFunctionDefinition(Function* pFunction);
        int				  PopFunctionDefinition(void);
        Function*			GetCurFunctionDefinition(void)const;
    public:
        void				  AddStatement(Statement* p);
    public:
        Function*			AddNewFunctionComponent(void);
        Statement*		AddNewStatementComponent(void);
        RuntimeFunctionCall* AddNewRuntimeFunctionComponent(void);
        RuntimeStatement* AddNewRuntimeStatementComponent(void);
        Closure*      AddNewClosureComponent(void);
        Object*				AddNewObjectComponent(void);
        Struct*				AddNewStructComponent(void);
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
        void				  AddSyntaxComponent(SyntaxComponent* p);
        int           GetSyntaxComponentNum(void)const { return m_SyntaxComponentNum; }
        void				  AddRuntimeComponent(RuntimeComponent* p);
        int           GetRuntimeComponentNum(void)const { return m_RuntimeComponentNum; }
    public:
        char*				  AllocString(int len);
        char*				  AllocString(const char* src);
        char*	        GetStringBuffer(void)const { return m_StringBuffer; }
        char*&	      GetUnusedStringPtrRef(void) { return m_UnusedStringPtr; }
    public:
        Interpreter(void);
        Interpreter(const InterpreterOptions& options);
        ~Interpreter(void);
        void				  Reset(void);
        int		      SetNextStatement(int ip)
        {
            if (ip < 0 || ip >= m_StatementNum)
                return FALSE;
            else
                m_NextStatement = ip;
        }
        int		        GetStatementNum(void)const { return m_StatementNum; }
    private:
        void				  Init(void);
        void				  Release(void);
        void				  InitInnerApis(void);
        void				  ReleaseInnerApis(void);
        void				  RegisterInnerFunctionApi(const char* id, ExpressionApi* p);
        void				  RegisterInnerStatementApi(const char* id, StatementApiFactory* p);
    private:
        char*				  m_StringBuffer;
        char*				  m_UnusedStringPtr;
        SyntaxComponentPtr* m_SyntaxComponentPool;
        int					  m_SyntaxComponentNum;
        RuntimeComponentPtr*  m_RuntimeComponentPool;
        int					  m_RuntimeComponentNum;
        StatementPtr*	m_Program;
        StatementApiPtr* m_RuntimeProgram;
        int					  m_StatementNum;
        Functions			m_InnerFunctionApis;
        StatementApiFactories			m_StatementApis;
        StatementApiFactories			m_InnerStatementApis;
        Value*				m_PredefinedValue;
        int					  m_PredefinedValueNum;
        ValueIndexes	m_PredefinedValueIndexes;
        Value*				m_ValuePool;
        int					  m_ValueNum;
        ValueIndexes	m_ValueIndexes;
        Value*        m_StackValuePool;
        StackInfos    m_StackInfos;
        FunctionDefinitionStack	m_FunctionDefinitionStack;

        int					m_NextStatement;
    public://仅用于解释器的数据池，仅用于函数或语句里面用来替代局部变量，alloc与recycle必须成对并且符合栈的操作顺序（后进先出）！！！
        InterpreterValuePool&	GetInnerValuePool(void) { return m_InterpreterValuePool; }
    private:
        InterpreterValuePool	m_InterpreterValuePool;
    public:
        void		EnableDebugInfo(void) { m_IsDebugInfoEnable = TRUE; }
        void		DisableDebugInfo(void) { m_IsDebugInfoEnable = FALSE; }
        int		IsDebugInfoEnable(void)const { return m_IsDebugInfoEnable; }
        int		IsRunFlagEnable(void)const
        {
            if (0 == m_pRunFlag || *m_pRunFlag)
                return TRUE;
            else
                return FALSE;
        }
        void		SetExternRunFlagRef(int& flag) { m_pRunFlag = &flag; }
    private:
        int				  m_IsDebugInfoEnable;
        const int*		m_pRunFlag;
    public:
        void		ClearErrorInfo(void) { m_ErrorAndStringBuffer.ClearErrorInfo(); }
        void		AddError(const char* error) { m_ErrorAndStringBuffer.AddError(error); }
        int		HasError(void)const { return m_ErrorAndStringBuffer.HasError(); }
        int		GetErrorNum(void) { return m_ErrorAndStringBuffer.GetErrorNum(); }
        const char*	GetErrorInfo(int index) const { return m_ErrorAndStringBuffer.GetErrorInfo(index); }
        char*	NewErrorInfo(void) { return m_ErrorAndStringBuffer.NewErrorInfo(); }
    public:
        ErrorAndStringBuffer&		    GetErrorAndStringBuffer(void) { return m_ErrorAndStringBuffer; }
        const ErrorAndStringBuffer&	GetErrorAndStringBuffer(void)const { return m_ErrorAndStringBuffer; }
    private:
        ErrorAndStringBuffer m_ErrorAndStringBuffer;
    public:
        InterpreterOptions&			  GetOptions(void) { return m_Options; }
        const InterpreterOptions&	GetOptions(void)const { return m_Options; }
    private:
        InterpreterOptions	m_Options;
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
                } else {
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
                } else {
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
