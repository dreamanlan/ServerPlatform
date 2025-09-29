/*****************************************************************************

calc.cpp

******************************************************************************/
#include "calc.h"

#if defined(__LINUX__)
#include <ctype.h>
#endif

#define GET_PRODUCTION_NAME    SlkGetProductionName
#define GET_SYMBOL_NAME        SlkGetSymbolName

namespace FunctionScript
{
    static const char* g_ArgNames[] = { "$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31", "$32" };
    //------------------------------------------------------------------------------------------------------
    void RuntimeComponent::ReplaceVariableWithValue(Value* pParams, int num)const
    {
        if (0 == pParams)
            return;
        for (int ix = 0; ix < num; ++ix) {
            ReplaceVariableWithValue(pParams[ix]);
        }
    }

    void RuntimeComponent::ReplaceVariableWithValue(Value& p)const
    {
        if (NULL != m_Interpreter) {
            if (p.IsIndex() || p.IsArgIndex() || p.IsLocalIndex()) {
                const Value& t = m_Interpreter->GetValue(p.GetType(), p.GetInt());
                if ((&t) != (&p)) {
                    p = t;
                    if (t.IsIndex() || t.IsArgIndex() || t.IsLocalIndex()) {
                        ReplaceVariableWithValue(p);
                    }
                }
            }
            else if (p.IsIdentifier()) {
                p = m_Interpreter->GetValue(p.GetString());
            }
        }
    }

    void RuntimeComponent::SetVariableValue(const Value& p, const Value& val)const
    {
        if (NULL != m_Interpreter) {
            if (p.IsIndex() || p.IsArgIndex() || p.IsLocalIndex()) {
                m_Interpreter->SetValue(p.GetType(), p.GetInt(), val);
            }
            else if (p.IsIdentifier()) {
                m_Interpreter->SetValue(p.GetString(), val);
            }
        }
    }

    const Value& RuntimeComponent::GetVariableValue(const Value& p)const
    {
        if (NULL != m_Interpreter) {
            if (p.IsIndex() || p.IsArgIndex() || p.IsLocalIndex()) {
                const Value& t = m_Interpreter->GetValue(p.GetType(), p.GetInt());
                if ((&t) != (&p) && (t.IsIndex() || t.IsArgIndex() || t.IsLocalIndex())) {
                    return GetVariableValue(t);
                }
                else {
                    return t;
                }
            }
            else if (p.IsIdentifier())
                return m_Interpreter->GetValue(p.GetString());
            else
                return p;
        }
        else {
            return Value::GetInvalidValueRef();
        }
    }
    //------------------------------------------------------------------------------------------------------
    namespace InnerApi
    {
        class DebugBreakApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass, pParams, num;
                if (NULL != pRetValue) {
                    pRetValue->SetInvalid();
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit DebugBreakApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ExprApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                else {
                    ReplaceVariableWithValue(pParams, num);
                    if (NULL != pRetValue) {
                        if (num > 0)
                            *pRetValue = pParams[num - 1];
                        else
                            pRetValue->SetInvalid();
                    }
                    return EXECUTE_RESULT_NORMAL;
                }
            }
        public:
            explicit ExprApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class AssignOpApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                if (2 != num || 0 == pParams || 0 == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                else {
                    ExpressionApi* pExp = m_pProxyApi;
                    if (0 == pExp) {
                        Value val = GetVariableValue(pParams[1]);
                        if (pParams[0].IsIndex() && (pParams[1].IsArgIndex() || pParams[1].IsLocalIndex() || pParams[1].IsIdentifier())) {
                            SetVariableValue(pParams[0], val);
                        }
                        else {
                            SetVariableValue(pParams[0], pParams[1]);
                        }
                        if (NULL != pRetValue) {
                            *pRetValue = val;
                        }
                    }
                    else {
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        Value params[] = { pParams[0], pParams[1] };
                        pExp->Execute(paramClass, params, 2, &val);
                        SetVariableValue(pParams[0], val);
                        if (NULL != pRetValue) {
                            *pRetValue = val;
                        }
                    }
                    return EXECUTE_RESULT_NORMAL;
                }
            }
        public:
            explicit AssignOpApi(Interpreter& interpreter, ExpressionApi* pProxyApi) :ExpressionApi(interpreter), m_pProxyApi(pProxyApi) {}
        private:
            ExpressionApi* m_pProxyApi;
        };

        class PrintApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass, pRetValue;
                if (0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                else {
                    /*for(INT ix=0;ix<num;++ix)
                    {
                    if(pParams[ix].IsIndex() || pParams[ix].IsArgIndex() || pParams[ix].IsLocalIndex())
                    {
                    printf("[%d:%s]\n",ix,m_Interpreter->GetValueName(pParams[ix].GetType(),pParams[ix].GetInt()));
                    }
                    }*/
                    ReplaceVariableWithValue(pParams, num);
                    for (int ix = 0; ix < num; ++ix) {
                        char buf[MAX_NUMBER_STRING_SIZE];
                        const char* pBuf = pParams[ix].ToString(buf, MAX_NUMBER_STRING_SIZE);
                        if (0 != pBuf) {
                            printf("%s", pBuf);
                        }
                        if (ix < num - 1) {
                            printf(",");
                        }
                        else {
                            printf("\n");
                        }
                    }
                    return EXECUTE_RESULT_NORMAL;
                }
            }
        public:
            explicit PrintApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class AddApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    if (num > 1) {
                        for (int ix = 1; ix < num; ++ix) {
                            if (val.IsString() || pParams[ix].IsString()) {
                                Value temp = val;
                                char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                                const char* pStr1 = temp.ToString(p1, MAX_NUMBER_STRING_SIZE);
                                const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                                if (0 == pStr1 || 0 == pStr2) {
                                    val.SetWeakRefString("");
                                }
                                else {
                                    int len = (int)strlen(pStr1) + (int)strlen(pStr2);
                                    val.AllocString(len);
                                    tsnprintf(val.GetString(), len + 1, "%s%s", pStr1, pStr2);
                                }
                            }
                            else if (val.IsFloat() || pParams[ix].IsFloat()) {
                                val.SetFloat(val.ToFloat() + pParams[ix].ToFloat());
                            }
                            else if (val.IsInt() && pParams[ix].IsInt()) {
                                val.SetInt(val.ToInt() + pParams[ix].ToInt());
                            }
                            else {
                                val.SetInvalid();
                                break;
                            }
                        }
                    }
                    else {
                        if (val.IsString() || val.IsFloat() || val.IsInt()) {
                        }
                        else {
                            val.SetInvalid();
                        }
                    }
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit AddApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class AddOpApi : public AddApi
        {
        public:
            explicit AddOpApi(Interpreter& interpreter) :AddApi(interpreter) {}
        };

        class SubApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    if (num > 1) {
                        for (int ix = 1; ix < num; ++ix) {
                            if (val.IsFloat() || pParams[ix].IsFloat()) {
                                val.SetFloat(val.ToFloat() - pParams[ix].ToFloat());
                            }
                            else if (val.IsInt() && pParams[ix].IsInt()) {
                                val.SetInt(val.ToInt() - pParams[ix].ToInt());
                            }
                            else {
                                val.SetInvalid();
                                break;
                            }
                        }
                    }
                    else {
                        if (val.IsFloat()) {
                            val.SetFloat(-val.GetFloat());
                        }
                        else if (val.IsInt()) {
                            val.SetInt(-val.GetInt());
                        }
                        else {
                            val.SetInvalid();
                        }
                    }
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit SubApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class SubOpApi : public SubApi
        {
        public:
            explicit SubOpApi(Interpreter& interpreter) :SubApi(interpreter) {}
        };

        class MulApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsFloat() || pParams[ix].IsFloat()) {
                            val.SetFloat(val.ToFloat() * pParams[ix].ToFloat());
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt() * pParams[ix].ToInt());
                        }
                        else {
                            val.SetInvalid();
                            break;
                        }
                    }
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit MulApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class MulOpApi : public MulApi
        {
        public:
            explicit MulOpApi(Interpreter& interpreter) :MulApi(interpreter) {}
        };

        class DivApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsFloat() || pParams[ix].IsFloat()) {
                            val.SetFloat(val.ToFloat() / pParams[ix].ToFloat());
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt() / pParams[ix].ToInt());
                        }
                        else {
                            val.SetInvalid();
                            break;
                        }
                    }
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit DivApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class DivOpApi : public DivApi
        {
        public:
            explicit DivOpApi(Interpreter& interpreter) :DivApi(interpreter) {}
        };

        class ModApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt() % pParams[ix].ToInt());
                        }
                        else {
                            val.SetInvalid();
                            break;
                        }
                    }
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ModApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ModOpApi : public ModApi
        {
        public:
            explicit ModOpApi(Interpreter& interpreter) :ModApi(interpreter) {}
        };

        class EqApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    const Value& val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) == 0 ? 1 : 0);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta >= -0.000001f && delta <= 0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() == pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as it is not equal once, there is no need to judge the subsequent ones.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit EqApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class EqOpApi : public EqApi
        {
        public:
            explicit EqOpApi(Interpreter& interpreter) :EqApi(interpreter) {}
        };

        class NotEqApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    const Value& val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) != 0 ? 1 : 0);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta < -0.000001f || delta>0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() != pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as there is equality once, there is no need to
                        // judge the subsequent ones.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NotEqApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NotEqOpApi : public NotEqApi
        {
        public:
            explicit NotEqOpApi(Interpreter& interpreter) :NotEqApi(interpreter) {}
        };

        class LessApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    for (int ix = 1; ix < num; ++ix) {
                        const Value& val = pParams[ix - 1];
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) < 0 ? 1 : 0);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta < -0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() < pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as it is not less than once, there is no need to judge the rest.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LessApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class LessOpApi : public LessApi
        {
        public:
            explicit LessOpApi(Interpreter& interpreter) :LessApi(interpreter) {}
        };

        class GreatApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    for (int ix = 1; ix < num; ++ix) {
                        const Value& val = pParams[ix - 1];
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) > 0 ? 1 : 0);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta > 0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() > pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as it is not greater than once, there is no need to
                        // judge the subsequent ones.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit GreatApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class GreatOpApi : public GreatApi
        {
        public:
            explicit GreatOpApi(Interpreter& interpreter) :GreatApi(interpreter) {}
        };

        class NotGreatApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    for (int ix = 1; ix < num; ++ix) {
                        const Value& val = pParams[ix - 1];
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) > 0 ? 0 : 1);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta <= 0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() <= pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as it is not less than or equal to once, there is no need
                        // to judge the subsequent ones.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NotGreatApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NotGreatOpApi : public NotGreatApi
        {
        public:
            explicit NotGreatOpApi(Interpreter& interpreter) :NotGreatApi(interpreter) {}
        };

        class NotLessApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    for (int ix = 1; ix < num; ++ix) {
                        const Value& val = pParams[ix - 1];
                        if (val.IsString() || pParams[ix].IsString()) {
                            char p1[MAX_NUMBER_STRING_SIZE], p2[MAX_NUMBER_STRING_SIZE];
                            const char* pStr1 = val.ToString(p1, MAX_NUMBER_STRING_SIZE);
                            const char* pStr2 = pParams[ix].ToString(p2, MAX_NUMBER_STRING_SIZE);
                            if (0 == pStr1 || 0 == pStr2)
                                pRetValue->SetInt(0);
                            else
                                pRetValue->SetInt(strcmp(pStr1, pStr2) < 0 ? 0 : 1);
                        }
                        else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta >= -0.000001f) ? 1 : 0);
                        }
                        else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() >= pParams[ix].ToInt()) ? 1 : 0);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                        //As long as one time is not greater than or equal to, there is no need
                        // to judge the subsequent ones.
                        if (pRetValue->GetInt() == 0)
                            break;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NotLessApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NotLessOpApi : public NotLessApi
        {
        public:
            explicit NotLessOpApi(Interpreter& interpreter) :NotLessApi(interpreter) {}
        };

        class AndApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetInt(1);
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (pParams[ix].GetInt() == 0) {
                                //If one of them is 0, there will be no need to calculate the rest.
                                pRetValue->SetInt(0);
                                break;
                            }
                        }
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit AndApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class AndOpApi : public AndApi
        {
        public:
            explicit AndOpApi(Interpreter& interpreter) :AndApi(interpreter) {}
        };

        class OrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetInt(0);
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (pParams[ix].GetInt() == 1) {
                                //If one of them is 1, there will be no need to calculate the rest.
                                pRetValue->SetInt(1);
                                break;
                            }
                        }
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit OrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class OrOpApi : public OrApi
        {
        public:
            explicit OrOpApi(Interpreter& interpreter) :OrApi(interpreter) {}
        };

        class NotApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    if (pParams[0].IsInt()) {
                        pRetValue->SetInt((pParams[0].ToInt() == 0) ? 1 : 0);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NotApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NotOpApi : public NotApi
        {
        public:
            explicit NotOpApi(Interpreter& interpreter) :NotApi(interpreter) {}
        };

        class IncApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                for (int ix = 0; ix < num; ++ix) {
                    int writeback = FALSE;
                    Value val = pParams[ix];
                    if (val.IsIdentifier() || val.IsIndex() || val.IsArgIndex() || val.IsLocalIndex()) {
                        ReplaceVariableWithValue(val);
                        writeback = TRUE;
                    }
                    if (val.IsInvalid())
                        continue;
                    if (val.IsFloat())
                        val.SetFloat(val.GetFloat() + 1);
                    else if (val.IsInt())
                        val.SetInt(val.GetInt() + 1);
                    if (writeback)
                        SetVariableValue(pParams[ix], val);

                    if (0 != pRetValue && ix == num - 1) {
                        *pRetValue = val;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit IncApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class IncOpApi : public IncApi
        {
        public:
            explicit IncOpApi(Interpreter& interpreter) :IncApi(interpreter) {}
        };

        class DecApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                for (int ix = 0; ix < num; ++ix) {
                    int writeback = FALSE;
                    Value val = pParams[ix];
                    if (val.IsIdentifier() || val.IsIndex() || val.IsArgIndex() || val.IsLocalIndex()) {
                        ReplaceVariableWithValue(val);
                        writeback = TRUE;
                    }
                    if (val.IsInvalid())
                        continue;
                    if (val.IsFloat())
                        val.SetFloat(val.GetFloat() - 1);
                    else if (val.IsInt())
                        val.SetInt(val.GetInt() - 1);
                    if (writeback)
                        SetVariableValue(pParams[ix], val);

                    if (0 != pRetValue && ix == num - 1) {
                        *pRetValue = val;
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit DecApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class DecOpApi : public DecApi
        {
        public:
            explicit DecOpApi(Interpreter& interpreter) :DecApi(interpreter) {}
        };

        class IntApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetInt(pParams[0].ToInt());
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit IntApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class FloatApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetFloat(pParams[0].ToFloat());
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit FloatApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StringApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    char buf[MAX_NUMBER_STRING_SIZE];
                    const char* pStr = pParams[0].ToString(buf, MAX_NUMBER_STRING_SIZE);
                    if (0 == pStr)
                        pRetValue->SetWeakRefString("");
                    else
                        pRetValue->AllocString(pStr);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StringApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class Int2HexApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    int val = pParams[0].ToInt();
                    char temp[MAX_NUMBER_STRING_SIZE];
                    temp[0] = '0';
                    temp[1] = 'x';
                    tsnprintf(temp + 2, MAX_NUMBER_STRING_SIZE - 2, "%x", val);
                    pRetValue->AllocString(temp);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit Int2HexApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class Hex2IntApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue && pParams[0].IsString() && 0 != pParams[0].GetString()) {
                    const char* p = pParams[0].GetString();
                    if (p[0] == '0' && p[1] == 'x')
                        p += 2;
                    int val = 0;
                    sscanf(p, "%x", &val);
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit Hex2IntApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class SystemApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue && pParams[0].IsString() && 0 != pParams[0].GetString()) {
                    const char* p = pParams[0].GetString();
                    pRetValue->SetInt(system(p));
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit SystemApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class BitAndApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    int val = 0xffffffff;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (val == 0) {
                                //The intermediate result of the operation is 0, and
                                // there is no need to calculate the rest.
                                break;
                            }
                            else {
                                val &= pParams[ix].GetInt();
                            }
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit BitAndApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class BitAndOpApi : public BitAndApi
        {
        public:
            explicit BitAndOpApi(Interpreter& interpreter) :BitAndApi(interpreter) {}
        };

        class BitOrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (val == (int)0xffffffff) {
                                //The intermediate result of the operation is 0xffffffff,
                                // and there is no need to calculate the rest.
                                break;
                            }
                            else {
                                val |= pParams[ix].GetInt();
                            }
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit BitOrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class BitOrOpApi : public BitOrApi
        {
        public:
            explicit BitOrOpApi(Interpreter& interpreter) :BitOrApi(interpreter) {}
        };

        class BitXorApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    bool first = true;
                    int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (first) {
                                val = pParams[0].GetInt();
                                first = false;
                                continue;
                            }
                            val ^= pParams[ix].GetInt();
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit BitXorApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class BitXorOpApi : public BitXorApi
        {
        public:
            explicit BitXorOpApi(Interpreter& interpreter) :BitXorApi(interpreter) {}
        };

        class NegApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    if (pParams[0].IsInt()) {
                        pRetValue->SetInt(~pParams[0].ToInt());
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NegApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NegOpApi : public NegApi
        {
        public:
            explicit NegOpApi(Interpreter& interpreter) :NegApi(interpreter) {}
        };

        class LeftShiftApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    bool first = true;
                    int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (first) {
                                val = pParams[0].GetInt();
                                first = false;
                                continue;
                            }
                            val <<= pParams[ix].GetInt();
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LeftShiftApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class LeftShiftOpApi : public LeftShiftApi
        {
        public:
            explicit LeftShiftOpApi(Interpreter& interpreter) :LeftShiftApi(interpreter) {}
        };

        class RightShiftApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    bool first = true;
                    int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (first) {
                                val = pParams[0].GetInt();
                                first = false;
                                continue;
                            }
                            val >>= pParams[ix].GetInt();
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit RightShiftApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class RightShiftOpApi : public RightShiftApi
        {
        public:
            explicit RightShiftOpApi(Interpreter& interpreter) :RightShiftApi(interpreter) {}
        };

        class UnsignedRightShiftApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    bool first = true;
                    unsigned int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (first) {
                                val = (unsigned int)pParams[0].GetInt();
                                first = false;
                                continue;
                            }
                            val >>= pParams[ix].GetInt();
                        }
                    }
                    pRetValue->SetInt(val);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit UnsignedRightShiftApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class UnsignedRightShiftOpApi : public UnsignedRightShiftApi
        {
        public:
            explicit UnsignedRightShiftOpApi(Interpreter& interpreter) :UnsignedRightShiftApi(interpreter) {}
        };

        class StrlenApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    const char* p = pParams[0].GetString();
                    if (0 != p) {
                        int len = (int)strlen(p);
                        pRetValue->SetInt(len);
                    }
                    else {
                        pRetValue->SetInt(0);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrlenApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StrstrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest) {
                        const char* p = strstr(pSrc, pDest);
                        if (0 == p)
                            pRetValue->SetInt(-1);
                        else
                            pRetValue->SetInt((int)(p - pSrc));
                    }
                    else {
                        pRetValue->SetInt(-1);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrstrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class SubstrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num && 3 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsInt()) {
                    const char* pSrc = pParams[0].GetString();
                    int start = pParams[1].GetInt();
                    int size = 0;
                    if (3 == num && pParams[2].IsInt())
                        size = pParams[2].GetInt();
                    if (0 != pSrc) {
                        int len = (int)strlen(pSrc);
                        if (0 == size) {
                            size = len - start;
                        }
                        if (start >= 0 && start < len && size >= 0 && size < len) {
                            char* p = NULL;
                            if (TRUE == pRetValue->AllocString(size)) {
                                p = pRetValue->GetString();
                            }
                            if (0 == p)
                                pRetValue->SetWeakRefString("");
                            else {
                                memcpy(p, pSrc + start, size);
                                p[size] = 0;
                            }
                        }
                        else {
                            pRetValue->SetWeakRefString("");
                        }
                    }
                    else {
                        pRetValue->SetWeakRefString("");
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit SubstrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StrchrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int c = pDest[0];
                        const char* p = strchr(pSrc, c);
                        if (NULL != p)
                            pRetValue->SetInt((int)(p - pSrc));
                        else
                            pRetValue->SetInt(-1);
                    }
                    else {
                        pRetValue->SetInt(-1);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrchrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StrrchrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int c = pDest[0];
                        const char* p = strrchr(pSrc, c);
                        if (NULL != p)
                            pRetValue->SetInt((int)(p - pSrc));
                        else
                            pRetValue->SetInt(-1);
                    }
                    else {
                        pRetValue->SetInt(-1);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrrchrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StrcspnApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int ix = (int)strcspn(pSrc, pDest);
                        pRetValue->SetInt(ix);
                    }
                    else {
                        pRetValue->SetInt(-1);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrcspnApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class StrspnApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int ix = (int)strspn(pSrc, pDest);
                        pRetValue->SetInt(ix);
                    }
                    else {
                        pRetValue->SetInt(-1);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StrspnApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class TolowerApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    if (0 != pSrc) {
                        int len = (int)strlen(pSrc);
                        char* p = NULL;
                        if (TRUE == pRetValue->AllocString(len)) {
                            p = pRetValue->GetString();
                        }
                        if (p) {
                            for (int ix = 0; ix < len; ++ix) {
                                p[ix] = (char)tolower(pSrc[ix]);
                            }
                            p[len] = 0;
                        }
                        else {
                            pRetValue->SetWeakRefString("");
                        }
                    }
                    else {
                        pRetValue->SetWeakRefString("");
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit TolowerApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ToupperApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    if (0 != pSrc) {
                        int len = (int)strlen(pSrc);
                        char* p = NULL;
                        if (TRUE == pRetValue->AllocString(len)) {
                            p = pRetValue->GetString();
                        }
                        if (p) {
                            for (int ix = 0; ix < len; ++ix) {
                                p[ix] = (char)toupper(pSrc[ix]);
                            }
                            p[len] = 0;
                        }
                        else {
                            pRetValue->SetWeakRefString("");
                        }
                    }
                    else {
                        pRetValue->SetWeakRefString("");
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ToupperApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ByteAtApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (1 != num && 2 != num && 3 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    char* pSrc = pParams[0].GetString();
                    int index = 0;
                    if (2 <= num && pParams[1].IsInt())
                        index = pParams[1].GetInt();
                    if (0 != pSrc) {
                        int len = (int)strlen(pSrc);
                        if (index >= 0 && index < len) {
                            int c = pSrc[index];
                            if (3 == num) {
                                if (pParams[2].IsInt()) {
                                    pSrc[index] = (char)pParams[2].GetInt();
                                }
                                else if (pParams[2].IsString()) {
                                    const char* p = pParams[2].GetString();
                                    if (0 != p && 0 != p[0]) {
                                        pSrc[index] = p[0];
                                    }
                                }
                            }
                            pRetValue->SetInt(c);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                    }
                    else {
                        pRetValue->SetInt(0);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ByteAtApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class Bytes2StrApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue) {
                    if (0 < num) {
                        if (TRUE == pRetValue->AllocString(num)) {
                            char* p = pRetValue->GetString();
                            if (NULL != p) {
                                p[num] = 0;
                                for (int ix = 0; ix < num; ++ix) {
                                    if (pParams[ix].IsInt()) {
                                        p[ix] = (char)pParams[ix].GetInt();
                                    }
                                    else if (pParams[ix].IsString()) {
                                        const char* pN = pParams[ix].GetString();
                                        if (0 != pN && 0 != pN[0]) {
                                            p[ix] = pN[0];
                                        }
                                    }
                                    else {
                                        p[ix] = ' ';
                                    }
                                }
                            }
                            else {
                                pRetValue->SetWeakRefString("");
                            }
                        }
                        else {
                            pRetValue->SetWeakRefString("");
                        }
                    }
                    else {
                        pRetValue->SetWeakRefString("");
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit Bytes2StrApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ByteRepApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (3 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    char* pSrc = pParams[0].GetString();
                    char s = 0;
                    if (pParams[1].IsInt()) {
                        s = (char)pParams[1].GetInt();
                    }
                    else if (pParams[1].IsString()) {
                        const char* pTmp = pParams[1].GetString();
                        if (pTmp && pTmp[0]) {
                            s = pTmp[0];
                        }
                    }
                    char d = 0;
                    if (pParams[2].IsInt()) {
                        d = (char)pParams[2].GetInt();
                    }
                    else if (pParams[2].IsString()) {
                        const char* pTmp = pParams[2].GetString();
                        if (pTmp && pTmp[0]) {
                            d = pTmp[0];
                        }
                    }
                    if (0 != pSrc && 0 != s && 0 != d) {
                        int len = (int)strlen(pSrc);
                        pRetValue->AllocString(pSrc);
                        char* pRet = pRetValue->GetString();
                        if (NULL != pRet) {
                            for (int ix = 0; ix < len; ++ix) {
                                if (pRet[ix] == s)
                                    pRet[ix] = d;
                            }
                        }
                    }
                    else {
                        pRetValue->SetWeakRefString("");
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ByteRepApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ObjectApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass, pParams, num;
                if (0 != m_Interpreter && 0 != pRetValue) {
                    Object* pObject = m_Interpreter->AddNewObjectComponent();
                    if (0 != pObject) {
                        pRetValue->SetExpressionApi(pObject);
                    }
                    else {
                        pRetValue->SetInt(0);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ObjectApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class NewApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && pParams[0].IsExpressionApi() && 0 != pRetValue) {
                    ExpressionApi* p = pParams[0].GetExpressionApi();
                    if (0 != p) {
                        Object* pObject = m_Interpreter->AddNewObjectComponent();
                        if (0 != pObject) {
                            AutoInterpreterValuePoolValuesOperation op(m_Interpreter->GetInnerValuePool());
                            Value* params = op.Get();
                            params[0] = Value(pObject);
                            for (int ix = 1; ix < num; ++ix) {
                                params[ix] = pParams[ix];
                            }
                            AutoInterpreterValuePoolValueOperation op2(m_Interpreter->GetInnerValuePool());
                            Value& temp = op2.Get();
                            p->Execute(paramClass, params, num, &temp);
                            pRetValue->SetExpressionApi(pObject);
                        }
                        else {
                            pRetValue->SetInt(0);
                        }
                    }
                    else {
                        pRetValue->SetInt(0);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit NewApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };

        class ArgApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
                paramClass;
                if (0 != m_Interpreter && 0 != pRetValue && 0 != pParams) {
                    ReplaceVariableWithValue(pParams, num);
                    if (1 == num && pParams[0].IsInt()) {
                        *pRetValue = m_Interpreter->GetValue(Value::TYPE_ARG_INDEX, pParams[0].GetInt());
                    }
                    else if (2 == num && pParams[0].IsInt()) {
                        m_Interpreter->SetValue(Value::TYPE_ARG_INDEX, pParams[0].GetInt(), pParams[1]);
                        *pRetValue = pParams[1];
                    }
                    else {
                        pRetValue->SetInvalid();
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ArgApi(Interpreter& interpreter) :ExpressionApi(interpreter) {}
        };
    }
    //------------------------------------------------------------------------------------------------------
    class AutoFunctionDefinitionStackOperation
    {
    public:
        AutoFunctionDefinitionStackOperation(Interpreter& interpreter, FunctionData* pFunction) :m_Interpreter(&interpreter), m_Function(pFunction), m_NeedPop(FALSE)
        {
            if (NULL != m_Function && NULL != m_Interpreter) {
                m_NeedPop = m_Interpreter->PushFunctionDefinition(m_Function);
            }
        }
        ~AutoFunctionDefinitionStackOperation()
        {
            if (NULL != m_Function && NULL != m_Interpreter && TRUE == m_NeedPop) {
                m_Interpreter->PopFunctionDefinition();
            }
        }
    private:
        Interpreter* m_Interpreter;
        FunctionData* m_Function;
        int            m_NeedPop;
    };
    //------------------------------------------------------------------------------------------------------
    namespace InnerApi
    {
        class ForStatementFactory;
        class ForStatement : public StatementApi
        {
            friend class ForStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                if (m_Exp1.IsStatementApi() && m_Exp1.GetStatementApi()) {
                    m_Exp1.GetStatementApi()->Execute(NULL);
                }
                for (; m_Interpreter->IsRunFlagEnable();) {
                    if (!m_Exp2.IsInvalid()) {
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (m_Exp2.IsStatementApi() && m_Exp2.GetStatementApi()) {
                            m_Exp2.GetStatementApi()->Execute(&val);
                        }
                        else {
                            val = m_Exp2;
                            ReplaceVariableWithValue(val);
                        }
                        if (!val.IsInt() || val.GetInt() == 0) {
                            break;
                        }
                    }
                    ExecuteResultEnum ret = m_Statements->Execute(pRetValue);
                    if (EXECUTE_RESULT_RETURN == ret)
                        return ret;
                    else if (EXECUTE_RESULT_BREAK == ret)
                        break;
                    if (m_Exp3.IsStatementApi() && m_Exp3.GetStatementApi()) {
                        m_Exp3.GetStatementApi()->Execute(0);
                    }
                    if (EXECUTE_RESULT_CONTINUE == ret)
                        continue;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ForStatement(Interpreter& interpreter) :StatementApi(interpreter), m_Statements(NULL)
            {
            }
            virtual ~ForStatement()
            {
                m_Statements = NULL;
            }
        private:
            Value m_Exp1;
            Value m_Exp2;
            Value m_Exp3;
            RuntimeStatementBlock* m_Statements;
        };
        class ForStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == pLoFunc)
                    return FALSE;
                if (pLoFunc->GetParamNum() != 3)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                statement.PrepareGeneralRuntimeObject();
                FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                ForStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<ForStatement>();
                if (NULL != pApi) {
                    const FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    ISyntaxComponent* p1 = pLoFunc->GetParam(0);
                    ISyntaxComponent* p2 = pLoFunc->GetParam(1);
                    ISyntaxComponent* p3 = pLoFunc->GetParam(2);
                    if (NULL != p1)
                        pApi->m_Exp1 = p1->GetRuntimeObject();
                    if (NULL != p2)
                        pApi->m_Exp2 = p2->GetRuntimeObject();
                    if (NULL != p3)
                        pApi->m_Exp3 = p3->GetRuntimeObject();
                    pApi->m_Statements = pFunc->GetRuntimeFunctionBody();
                }
                return pApi;
            }
        };
        class ForStatementFactory;

        class ForEachStatement : public StatementApi
        {
            friend class ForEachStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                for (int i = 0; i < m_ExpNum && m_Interpreter->IsRunFlagEnable(); ++i) {
                    if (!m_Exps[i].IsInvalid()) {
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (m_Exps[i].IsStatementApi() && m_Exps[i].GetStatementApi()) {
                            m_Exps[i].GetStatementApi()->Execute(&val);
                        }
                        else {
                            val = m_Exps[i];
                            ReplaceVariableWithValue(val);
                        }
                        if (m_IterIndex >= 0) {
                            m_Interpreter->SetValue(m_IterIndexType, m_IterIndex, val);
                        }
                    }
                    ExecuteResultEnum ret = m_Statements->Execute(pRetValue);
                    if (EXECUTE_RESULT_RETURN == ret)
                        return ret;
                    else if (EXECUTE_RESULT_BREAK == ret)
                        break;
                    if (EXECUTE_RESULT_CONTINUE == ret)
                        continue;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit ForEachStatement(Interpreter& interpreter) :StatementApi(interpreter), m_Exps(NULL), m_ExpNum(0), m_IterIndexType(Value::TYPE_INVALID), m_IterIndex(-1), m_Statements(NULL)
            {
            }
            virtual ~ForEachStatement()
            {
                if (m_Exps) {
                    delete[] m_Exps;
                    m_Exps = NULL;
                    m_ExpNum = 0;
                }
                m_Statements = NULL;
            }
            void SetIterator(int indexType, int index)
            {
                m_IterIndexType = indexType;
                m_IterIndex = index;
            }
        private:
            Value* m_Exps;
            int m_ExpNum;
            int m_IterIndexType;
            int m_IterIndex;
            RuntimeStatementBlock* m_Statements;
        };
        class ForEachStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == pLoFunc)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                statement.PrepareGeneralRuntimeObject();
                FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                ForEachStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<ForEachStatement>();
                if (NULL != pApi) {
                    const FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    pApi->m_ExpNum = pLoFunc->GetParamNum();
                    pApi->m_Exps = new Value[pApi->m_ExpNum];
                    for (int i = 0; i < pApi->m_ExpNum; ++i) {
                        ISyntaxComponent* p = pLoFunc->GetParam(i);
                        if (NULL != p)
                            pApi->m_Exps[i] = p->GetRuntimeObject();
                    }
                    pApi->m_Statements = pFunc->GetRuntimeFunctionBody();

                    FunctionData* pFuncDef = statement.GetInterpreter().GetCurFunctionDefinition();
                    if (NULL != pFuncDef) {
                        pApi->SetIterator(Value::TYPE_LOCAL_INDEX, pFuncDef->GetLocalIndex("$iterv"));
                    }
                    else {
                        statement.GetInterpreter().SetValue("@iterv", Value());
                        pApi->SetIterator(Value::TYPE_INDEX, statement.GetInterpreter().GetValueIndex("@iterv"));
                    }
                }
                return pApi;
            }
        };

        class WhileStatementFactory;
        class WhileStatement : public StatementApi
        {
            friend class WhileStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                while (m_Interpreter->IsRunFlagEnable()) {
                    if (!m_Exp.IsInvalid()) {
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (m_Exp.IsStatementApi() && m_Exp.GetStatementApi()) {
                            m_Exp.GetStatementApi()->Execute(&val);
                        }
                        else {
                            val = m_Exp;
                            ReplaceVariableWithValue(val);
                        }
                        if (!val.IsInt() || val.GetInt() == 0) {
                            break;
                        }
                    }
                    ExecuteResultEnum ret = m_Statements->Execute(pRetValue);
                    if (EXECUTE_RESULT_RETURN == ret)
                        return ret;
                    else if (EXECUTE_RESULT_BREAK == ret)
                        break;
                    if (EXECUTE_RESULT_CONTINUE == ret)
                        continue;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit WhileStatement(Interpreter& interpreter) :StatementApi(interpreter), m_Statements(NULL)
            {
            }
            virtual ~WhileStatement()
            {
                m_Statements = NULL;
            }
        private:
            Value m_Exp;
            RuntimeStatementBlock* m_Statements;
        };
        class WhileStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == pLoFunc)
                    return FALSE;
                if (pLoFunc->GetParamNum() != 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                statement.PrepareGeneralRuntimeObject();
                FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                WhileStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<WhileStatement>();
                if (NULL != pApi) {
                    const FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    ISyntaxComponent* p = pLoFunc->GetParam(0);
                    if (NULL != p)
                        pApi->m_Exp = p->GetRuntimeObject();
                    pApi->m_Statements = pFunc->GetRuntimeFunctionBody();
                }
                return pApi;
            }
        };

        class LoopStatementFactory;
        class LoopStatement : public StatementApi
        {
            friend class LoopStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                int ct = 0;
                if (!m_Exp.IsInvalid()) {
                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                    Value& val = op.Get();
                    if (m_Exp.IsStatementApi() && m_Exp.GetStatementApi()) {
                        m_Exp.GetStatementApi()->Execute(&val);
                    }
                    else {
                        val = m_Exp;
                        ReplaceVariableWithValue(val);
                    }
                    if (val.IsInt()) {
                        ct = val.GetInt();
                    }
                }
                for (int i = 0; i < ct && m_Interpreter->IsRunFlagEnable(); ++i) {
                    if (m_IterIndex >= 0) {
                        m_Interpreter->SetValue(m_IterIndexType, m_IterIndex, Value(i, Value::TYPE_INT));
                    }
                    ExecuteResultEnum ret = m_Statements->Execute(pRetValue);
                    if (EXECUTE_RESULT_RETURN == ret)
                        return ret;
                    else if (EXECUTE_RESULT_BREAK == ret)
                        break;
                    if (EXECUTE_RESULT_CONTINUE == ret)
                        continue;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LoopStatement(Interpreter& interpreter) :StatementApi(interpreter), m_IterIndexType(Value::TYPE_INVALID), m_IterIndex(-1), m_Statements(NULL)
            {
            }
            virtual ~LoopStatement()
            {
                m_Statements = NULL;
            }
            void SetIterator(int indexType, int index)
            {
                m_IterIndexType = indexType;
                m_IterIndex = index;
            }
        private:
            Value m_Exp;
            int m_IterIndexType;
            int m_IterIndex;
            RuntimeStatementBlock* m_Statements;
        };
        class LoopStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == pLoFunc)
                    return FALSE;
                if (pLoFunc->GetParamNum() != 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                statement.PrepareGeneralRuntimeObject();
                FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                LoopStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LoopStatement>();
                if (NULL != pApi) {
                    const FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    ISyntaxComponent* p = pLoFunc->GetParam(0);
                    if (NULL != p)
                        pApi->m_Exp = p->GetRuntimeObject();
                    pApi->m_Statements = pFunc->GetRuntimeFunctionBody();

                    FunctionData* pFuncDef = statement.GetInterpreter().GetCurFunctionDefinition();
                    if (NULL != pFuncDef) {
                        pApi->SetIterator(Value::TYPE_LOCAL_INDEX, pFuncDef->GetLocalIndex("$iterv"));
                    }
                    else {
                        statement.GetInterpreter().SetValue("@iterv", Value());
                        pApi->SetIterator(Value::TYPE_INDEX, statement.GetInterpreter().GetValueIndex("@iterv"));
                    }
                }
                return pApi;
            }
        };

        class LoopListStatementFactory;
        class LoopListStatement : public StatementApi
        {
            friend class LoopListStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                ObjectBase* pList = NULL;
                if (!m_Exp.IsInvalid()) {
                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                    Value& val = op.Get();
                    if (m_Exp.IsStatementApi() && m_Exp.GetStatementApi()) {
                        m_Exp.GetStatementApi()->Execute(&val);
                    }
                    else {
                        val = m_Exp;
                        ReplaceVariableWithValue(val);
                    }
                    if(val.IsExpressionApi()) {
                        ExpressionApi* pApi = val.GetExpressionApi();
                        if (pApi && pApi->GetTypeTag() == RuntimeComponent::TYPE_TAG_OBJECT) {
                            pList = static_cast<ObjectBase*>(pApi);
                        }
                    }
                }
                int ct = pList ? pList->GetDynamicMemberNum() : 0;
                for (int i = 0; i < ct && m_Interpreter->IsRunFlagEnable(); ++i) {
                    const ObjectBase::MemberInfo* pInfo = pList->GetDynamicMember(i);
                    if (m_IterIndexKey >= 0) {
                        m_Interpreter->SetValue(m_IterIndexType, m_IterIndexKey, Value(pInfo->m_Name, Value::TYPE_STRING));
                    }
                    if (m_IterIndexVal >= 0) {
                        m_Interpreter->SetValue(m_IterIndexType, m_IterIndexVal, pInfo->m_Value);
                    }
                    ExecuteResultEnum ret = m_Statements->Execute(pRetValue);
                    if (EXECUTE_RESULT_RETURN == ret)
                        return ret;
                    else if (EXECUTE_RESULT_BREAK == ret)
                        break;
                    if (EXECUTE_RESULT_CONTINUE == ret)
                        continue;
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LoopListStatement(Interpreter& interpreter) :StatementApi(interpreter), m_IterIndexType(Value::TYPE_INVALID), m_IterIndexKey(-1), m_IterIndexVal(-1), m_Statements(NULL)
            {
            }
            virtual ~LoopListStatement()
            {
                m_Statements = NULL;
            }
            void SetIterator(int indexType, int indexKey, int indexValue)
            {
                m_IterIndexType = indexType;
                m_IterIndexKey = indexKey;
                m_IterIndexVal = indexValue;
            }
        private:
            Value m_Exp;
            int m_IterIndexType;
            int m_IterIndexKey;
            int m_IterIndexVal;
            RuntimeStatementBlock* m_Statements;
        };
        class LoopListStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == pLoFunc)
                    return FALSE;
                if (pLoFunc->GetParamNum() != 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                statement.PrepareGeneralRuntimeObject();
                FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                LoopListStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LoopListStatement>();
                if (NULL != pApi) {
                    const FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    ISyntaxComponent* p = pLoFunc->GetParam(0);
                    if (NULL != p)
                        pApi->m_Exp = p->GetRuntimeObject();
                    pApi->m_Statements = pFunc->GetRuntimeFunctionBody();

                    FunctionData* pFuncDef = statement.GetInterpreter().GetCurFunctionDefinition();
                    if (NULL != pFuncDef) {
                        pApi->SetIterator(Value::TYPE_LOCAL_INDEX, pFuncDef->GetLocalIndex("$iterk"), pFuncDef->GetLocalIndex("$iterv"));
                    }
                    else {
                        statement.GetInterpreter().SetValue("@iterk", Value());
                        statement.GetInterpreter().SetValue("@iterv", Value());
                        pApi->SetIterator(Value::TYPE_INDEX, statement.GetInterpreter().GetValueIndex("@iterk"), statement.GetInterpreter().GetValueIndex("@iterv"));
                    }
                }
                return pApi;
            }
        };

        class IfElseStatementFactory;
        class IfElseStatement : public StatementApi
        {
            friend class IfElseStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;

                AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                Value& val = op.Get();
                if (m_Exp.IsStatementApi() && m_Exp.GetStatementApi()) {
                    m_Exp.GetStatementApi()->Execute(&val);
                }
                else {
                    val = m_Exp;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (NULL != m_pIf)
                        return m_pIf->Execute(pRetValue);
                }
                else {
                    int num = m_ElseIfNum;
                    for (int ix = 0; ix < num; ++ix) {
                        AutoInterpreterValuePoolValueOperation op2(m_Interpreter->GetInnerValuePool());
                        Value& val2 = op2.Get();
                        if (m_pElseIfExp[ix].IsStatementApi() && NULL != m_pElseIfExp[ix].GetStatementApi()) {
                            m_pElseIfExp[ix].GetStatementApi()->Execute(&val2);
                        }
                        else {
                            val2 = m_pElseIfExp[ix];
                            ReplaceVariableWithValue(val2);
                        }
                        if (0 != val2.GetInt()) {
                            return m_pElseIf[ix]->Execute(pRetValue);
                        }
                    }
                    if (NULL != m_pElse) {
                        return m_pElse->Execute(pRetValue);
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit IfElseStatement(Interpreter& interpreter) :StatementApi(interpreter), m_pIf(NULL), m_ElseIfNum(0), m_pElseIf(NULL), m_pElse(NULL)
            {
            }
            virtual ~IfElseStatement()
            {
                m_pIf = NULL;
                if (m_ElseIfNum > 0) {
                    if (NULL != m_pElseIfExp) {
                        delete[] m_pElseIfExp;
                        m_pElseIfExp = NULL;
                    }
                    if (NULL != m_pElseIf) {
                        delete[] m_pElseIf;
                        m_pElseIf = NULL;
                    }
                }
                m_pElse = NULL;
            }
        private:
            Value m_Exp;
            RuntimeStatementBlock* m_pIf;
            int m_ElseIfNum;
            Value* m_pElseIfExp;
            RuntimeStatementBlock** m_pElseIf;
            RuntimeStatementBlock* m_pElse;
        };
        class IfElseStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                    const FunctionData& func = static_cast<const FunctionData&>(statement);
                    const FunctionData* pLoFunc = func.GetLowerOrderFunction();
                    if (0 == pLoFunc)
                        return FALSE;
                    if (pLoFunc->GetParamNum() != 1)
                        return FALSE;
                }
                else if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                    const StatementData& stData = static_cast<const StatementData&>(statement);
                    FunctionData* pFunc0 = stData.GetFunction(0);
                    if (0 == pFunc0)
                        return FALSE;
                    const FunctionData* pLoFunc0 = pFunc0->GetLowerOrderFunction();
                    if (0 == pLoFunc0)
                        return FALSE;
                    if (pLoFunc0->GetParamNum() != 1)
                        return FALSE;
                    int num = stData.GetFunctionNum();
                    for (int ix = 1; ix < num; ++ix) {
                        FunctionData* pFunc1 = stData.GetFunction(ix);
                        if (0 == pFunc1)
                            return FALSE;
                        const ValueData& funcName1 = pFunc1->GetName();
                        if (funcName1.IsHighOrder()) {
                            const char* pName = funcName1.GetId();
                            if (0 != pName) {
                                if (0 != strcmp("elseif", pName))
                                    return FALSE;
                                else if (funcName1.GetValue().GetFunction()->GetParamNum() != 1)
                                    return FALSE;
                            }
                            else {
                                return FALSE;
                            }
                        }
                        else {
                            const char* pName = funcName1.GetId();
                            if (0 != pName) {
                                if (0 != strcmp("else", pName)) {
                                    return FALSE;
                                }
                            }
                            else {
                                return FALSE;
                            }
                        }
                    }
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                IfElseStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<IfElseStatement>();
                if (NULL != pApi) {
                    //Takes the name of the last function syntax before the statement is processed at runtime.
                    // Once processed at runtime, the name becomes the local variable index.
                    Value name;
                    if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                        StatementData* pStatement = static_cast<StatementData*>(&statement);
                        int num = pStatement->GetFunctionNum();
                        FunctionData* pLastFunc = pStatement->GetFunction(num - 1);
                        name = pLastFunc->GetNameValue();
                    }
                    statement.PrepareGeneralRuntimeObject();
                    if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                        FunctionData* pFunc = static_cast<FunctionData*>(&statement);
                        FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                        ISyntaxComponent* p = pLoFunc->GetParam(0);
                        if (NULL != p)
                            pApi->m_Exp = p->GetRuntimeObject();
                        pApi->m_pIf = pFunc->GetRuntimeFunctionBody();
                    }
                    else if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                        StatementData* pStatement = static_cast<StatementData*>(&statement);
                        FunctionData* pFunc = pStatement->GetFunction(0);
                        FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();

                        ISyntaxComponent* p = pLoFunc->GetParam(0);
                        if (NULL != p)
                            pApi->m_Exp = p->GetRuntimeObject();
                        pApi->m_pIf = pFunc->GetRuntimeFunctionBody();

                        int num = pStatement->GetFunctionNum();
                        FunctionData* pLastFunc = pStatement->GetFunction(num - 1);
                        if (name.IsIdentifier() && 0 == strcmp(name.GetString(), "else")) {
                            pApi->m_pElse = pLastFunc->GetRuntimeFunctionBody();
                            pApi->m_ElseIfNum = num - 2;
                        }
                        else {
                            pApi->m_ElseIfNum = num - 1;
                        }
                        if (pApi->m_ElseIfNum > 0) {
                            pApi->m_pElseIfExp = new Value[pApi->m_ElseIfNum];
                            pApi->m_pElseIf = new RuntimeStatementBlock * [pApi->m_ElseIfNum];
                            for (int ix = 0; ix < pApi->m_ElseIfNum; ++ix) {
                                FunctionData* pElseIfFunc = pStatement->GetFunction(ix + 1);
                                FunctionData* pLoElseIfFunc = pElseIfFunc->GetLowerOrderFunction();
                                if (NULL != pLoElseIfFunc && NULL != pLoElseIfFunc->GetParam(0)) {
                                    ISyntaxComponent* pExpStatement = pLoElseIfFunc->GetParam(0);
                                    pApi->m_pElseIfExp[ix] = pExpStatement->GetRuntimeObject();
                                    pApi->m_pElseIf[ix] = pElseIfFunc->GetRuntimeFunctionBody();
                                }
                                else {
                                    pApi->m_pElseIf[ix] = NULL;
                                }
                            }
                        }
                    }
                }
                return pApi;
            }
        };

        class CondExpStatementFactory;
        class CondExpStatement : public StatementApi
        {
            friend class CondExpStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;

                AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                Value& val = op.Get();
                if (m_Exp.IsStatementApi() && m_Exp.GetStatementApi()) {
                    m_Exp.GetStatementApi()->Execute(&val);
                }
                else {
                    val = m_Exp;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (m_True.IsStatementApi() && m_True.GetStatementApi()) {
                        m_True.GetStatementApi()->Execute(pRetValue);
                    }
                    else {
                        if (0 != pRetValue) {
                            *pRetValue = m_True;
                            ReplaceVariableWithValue(*pRetValue);
                        }
                    }
                }
                else {
                    if (m_False.IsStatementApi() && m_False.GetStatementApi()) {
                        m_False.GetStatementApi()->Execute(pRetValue);
                    }
                    else {
                        if (0 != pRetValue) {
                            *pRetValue = m_False;
                            ReplaceVariableWithValue(*pRetValue);
                        }
                    }
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit CondExpStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        private:
            Value m_Exp;
            Value m_True;
            Value m_False;
        };
        class CondExpStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_STATEMENT)
                    return FALSE;
                const StatementData& stData = static_cast<const StatementData&>(statement);
                if (stData.GetFunctionNum() != 2)
                    return FALSE;
                FunctionData* pFunc0 = stData.GetFunction(0);
                if (0 == pFunc0 || pFunc0->GetParamNum() != 1)
                    return FALSE;
                FunctionData* pExp = pFunc0->GetLowerOrderFunction();
                if (0 == pExp || pExp->GetParamNum() != 1)
                    return FALSE;
                FunctionData* pFunc1 = stData.GetFunction(1);
                if (0 == pFunc1 || pFunc1->GetParamNum() != 1)
                    return FALSE;
                const char* id = pFunc1->GetId();
                if (0 != id) {
                    if (0 != strcmp(id, ":"))
                        return FALSE;
                }
                else {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                CondExpStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<CondExpStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    StatementData* pStatement = static_cast<StatementData*>(&statement);
                    FunctionData* pFunc = pStatement->GetFunction(0);
                    FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    ISyntaxComponent* p = pLoFunc->GetParam(0);
                    if (NULL != p)
                        pApi->m_Exp = p->GetRuntimeObject();
                    ISyntaxComponent* pTrue = pFunc->GetParam(0);
                    if (NULL != pTrue)
                        pApi->m_True = pTrue->GetRuntimeObject();
                    FunctionData* pFunc2 = pStatement->GetFunction(1);
                    ISyntaxComponent* pFalse = pFunc2->GetParam(0);
                    if (NULL != pFalse)
                        pApi->m_False = pFalse->GetRuntimeObject();
                }
                return pApi;
            }
        };

        class IfGotoStatementFactory;
        class IfGotoStatement : public StatementApi
        {
            friend class IfGotoStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;

                AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                Value& val = op.Get();
                if (m_If.IsStatementApi() && NULL != m_If.GetStatementApi())
                    m_If.GetStatementApi()->Execute(&val);
                else {
                    val = m_If;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (m_Goto.IsStatementApi() && NULL != m_Goto.GetStatementApi()) {
                        m_Goto.GetStatementApi()->Execute(pRetValue);
                    }
                    return EXECUTE_RESULT_GOTO;
                }
                else {
                    return EXECUTE_RESULT_NORMAL;
                }
            }
        public:
            explicit IfGotoStatement(Interpreter& interpreter) :StatementApi(interpreter)
            {
            }
        private:
            Value m_If;
            Value m_Goto;
        };
        class IfGotoStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_STATEMENT)
                    return FALSE;
                const StatementData& stData = static_cast<const StatementData&>(statement);
                if (stData.GetFunctionNum() != 2)
                    return FALSE;
                FunctionData* pFunc0 = stData.GetFunction(0);
                if (0 == pFunc0 || pFunc0->GetParamNum() != 1)
                    return FALSE;
                FunctionData* pFunc1 = stData.GetFunction(1);
                if (0 == pFunc1 || pFunc1->GetParamNum() != 1)
                    return FALSE;
                const char* id = pFunc1->GetId();
                if (0 != id) {
                    if (0 != strcmp(id, "goto"))
                        return FALSE;
                }
                else {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                IfGotoStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<IfGotoStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    StatementData* pStatement = static_cast<StatementData*>(&statement);
                    FunctionData* pFunc0 = pStatement->GetFunction(0);
                    FunctionData* pFunc1 = pStatement->GetFunction(1);
                    ISyntaxComponent* p0 = pFunc0->GetParam(0);
                    ISyntaxComponent* p1 = pFunc1->GetParam(0);
                    pApi->m_If = p0->GetRuntimeObject();
                    pApi->m_Goto = p1->GetRuntimeObject();
                }
                return pApi;
            }
        };

        class GotoStatementFactory;
        class GotoStatement : public StatementApi
        {
            friend class GotoStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                if (m_Goto.IsInvalid())
                    return EXECUTE_RESULT_NORMAL;

                AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                Value& val = op.Get();
                if (m_Goto.IsStatementApi() && NULL != m_Goto.GetStatementApi()) {
                    m_Goto.GetStatementApi()->Execute(pRetValue);
                }
                else {
                    val = m_Goto;
                    ReplaceVariableWithValue(val);
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_GOTO;
            }
        public:
            explicit GotoStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        private:
            Value m_Goto;
        };
        class GotoStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                    const FunctionData& call = static_cast<const FunctionData&>(statement);
                    if (call.GetParamNum() != 1)
                        return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                GotoStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<GotoStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    FunctionData* pCall = static_cast<FunctionData*>(&statement);
                    ISyntaxComponent* p = pCall->GetParam(0);
                    pApi->m_Goto = p->GetRuntimeObject();
                }
                return pApi;
            }
        };

        class ReturnStatementFactory;
        class ReturnStatement : public StatementApi
        {
            friend class ReturnStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (NULL == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                if (m_Return.IsInvalid())
                    return EXECUTE_RESULT_NORMAL;

                AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                Value& val = op.Get();
                if (m_Return.IsStatementApi() && NULL != m_Return.GetStatementApi()) {
                    m_Return.GetStatementApi()->Execute(pRetValue);
                }
                else {
                    val = m_Return;
                    ReplaceVariableWithValue(val);
                    *pRetValue = val;
                }
                return EXECUTE_RESULT_RETURN;
            }
        public:
            explicit ReturnStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        private:
            Value m_Return;
        };
        class ReturnStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                    const StatementData& stData = static_cast<const StatementData&>(statement);
                    if (stData.GetFunctionNum() != 2)
                        return FALSE;
                    const FunctionData* pFunc = stData.GetFunction(1);
                    if (0 == pFunc)
                        return FALSE;
                    return TRUE;
                }
                else if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                    const FunctionData& call = static_cast<const FunctionData&>(statement);
                    if (call.GetParamNum() != 1)
                        return FALSE;
                }
                else if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_VALUE) {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                ReturnStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<ReturnStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                        const StatementData& stData = static_cast<const StatementData&>(statement);
                        const FunctionData& func1 = *stData.GetFunction(1);
                        pApi->m_Return = func1.GetRuntimeObject();
                    }
                    else if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                        const FunctionData& call = static_cast<const FunctionData&>(statement);
                        if (call.GetParamNum() == 1 && NULL != call.GetParam(0)) {
                            ISyntaxComponent* pStatement = call.GetParam(0);
                            pApi->m_Return = pStatement->GetRuntimeObject();
                        }
                    }
                    else if (statement.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                        pApi->m_Return.SetInvalid();
                    }
                }
                return pApi;
            }
        };

        class BreakStatementFactory;
        class BreakStatement : public StatementApi
        {
            friend class BreakStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                pRetValue;
                return EXECUTE_RESULT_BREAK;
            }
        public:
            explicit BreakStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        };
        class BreakStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_VALUE)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                if (NULL == m_pApi) {
                    m_pApi = statement.GetInterpreter().AddNewStatementApiComponent<BreakStatement>();
                }
                return m_pApi;
            }
        public:
            BreakStatementFactory() :m_pApi(NULL) {}
            virtual ~BreakStatementFactory()
            {
                if (NULL != m_pApi) {
                    delete m_pApi;
                    m_pApi = NULL;
                }
            }
        private:
            mutable BreakStatement* m_pApi;
        };

        class ContinueStatementFactory;
        class ContinueStatement : public StatementApi
        {
            friend class ContinueStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                pRetValue;
                return EXECUTE_RESULT_CONTINUE;
            }
        public:
            explicit ContinueStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        };
        class ContinueStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_VALUE)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                if (NULL == m_pApi) {
                    m_pApi = statement.GetInterpreter().AddNewStatementApiComponent<ContinueStatement>();
                }
                return m_pApi;
            }
        public:
            ContinueStatementFactory() :m_pApi(NULL) {}
            virtual ~ContinueStatementFactory()
            {
                if (NULL != m_pApi) {
                    delete m_pApi;
                    m_pApi = NULL;
                }
            }
        private:
            mutable ContinueStatement* m_pApi;
        };

        class FunctionStatementFactory;
        class FunctionStatement : public StatementApi
        {
            friend class FunctionStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                if (0 == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                //The function definition generates a Closure instance.
                ISyntaxComponent& statement = *m_pDefine;
                FunctionData* pFunc0 = 0;
                int syntaxType = statement.GetSyntaxType();
                if (syntaxType == ISyntaxComponent::TYPE_FUNCTION) {
                    pFunc0 = static_cast<FunctionData*>(&statement)->GetLowerOrderFunction();
                }
                else if (syntaxType == ISyntaxComponent::TYPE_STATEMENT) {
                    StatementData* pStatement = static_cast<StatementData*>(&statement);
                    pFunc0 = pStatement->GetFunction(0);
                }
                Closure* pClosure = m_Interpreter->AddNewClosureComponent();
                if (0 != pClosure) {
                    pClosure->SetDefinitionRef(m_pArguments, m_ArgumentNum, statement);
                    if (pFunc0->HaveParam()) {
                        ISyntaxComponent* pName = pFunc0->GetParam(0);
                        if (NULL != pName) {
                            Value nameVal = pName->GetRuntimeObject();
                            AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                            Value& val = op.Get();
                            if (nameVal.IsStatementApi() && NULL != nameVal.GetStatementApi()) {
                                nameVal.GetStatementApi()->Execute(&val);
                            }
                            else {
                                val = nameVal;
                            }
                            SetVariableValue(val, Value(pClosure));
                        }
                    }
                    if (NULL != pRetValue) {
                        pRetValue->SetExpressionApi(pClosure);
                    }
                }

                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit FunctionStatement(Interpreter& interpreter) :StatementApi(interpreter), m_pArguments(NULL), m_ArgumentNum(0), m_pDefine(NULL) {}
            virtual ~FunctionStatement()
            {
                if (0 != m_pArguments) {
                    delete[] m_pArguments;
                    m_pArguments = 0;
                }
            }
        private:
            Value* m_pArguments;
            int m_ArgumentNum;
            ISyntaxComponent* m_pDefine;
        };
        class FunctionStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                //Function definition syntax: function(name)args($a,$b,$c){};
                // where name and args($a,$b,$c) are optional
                int syntaxType = statement.GetSyntaxType();
                if (syntaxType == ISyntaxComponent::TYPE_FUNCTION) {
                    const FunctionData& func = static_cast<const FunctionData&>(statement);
                    if (FALSE == func.IsHighOrder())
                        return FALSE;
                    if (FALSE == func.HaveStatement())
                        return FALSE;
                }
                else if (syntaxType == ISyntaxComponent::TYPE_STATEMENT) {
                    const StatementData& stData = static_cast<const StatementData&>(statement);
                    if (stData.GetFunctionNum() != 2) {
                        return FALSE;
                    }
                    else {
                        FunctionData* pFunc0 = stData.GetFunction(0);
                        FunctionData* pFunc1 = stData.GetFunction(1);
                        if (0 != pFunc1) {
                            if (TRUE == pFunc0->IsHighOrder() || FALSE == pFunc1->IsHighOrder() || FALSE == pFunc1->HaveStatement())
                                return FALSE;
                            const char* id = pFunc1->GetId();
                            if (0 != id) {
                                if (0 != strcmp(id, "args"))
                                    return FALSE;
                            }
                            else {
                                return FALSE;
                            }
                        }
                    }
                }
                else {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                FunctionStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<FunctionStatement>();
                if (NULL != pApi) {
                    FunctionData* pFunc = 0;
                    int syntaxType = statement.GetSyntaxType();
                    if (syntaxType == ISyntaxComponent::TYPE_FUNCTION) {
                        pFunc = static_cast<FunctionData*>(&statement);
                    }
                    else if (syntaxType == ISyntaxComponent::TYPE_STATEMENT) {
                        StatementData* pStatement = static_cast<StatementData*>(&statement);
                        int num = pStatement->GetFunctionNum();
                        if (num > 0) {
                            pFunc = pStatement->GetFunction(num - 1);
                        }
                    }
                    FunctionData* pLoFunc = pFunc->GetLowerOrderFunction();
                    AutoFunctionDefinitionStackOperation autoDefStack(pFunc->GetInterpreter(), pFunc);
                    statement.PrepareGeneralRuntimeObject();
                    pApi->m_pDefine = &statement;

                    if (syntaxType == ISyntaxComponent::TYPE_STATEMENT && 0 != pLoFunc && pLoFunc->GetParamNum() > 0) {
                        pApi->m_ArgumentNum = pLoFunc->GetParamNum();
                        pApi->m_pArguments = new Value[pLoFunc->GetParamNum()];
                        for (int i = 0; i < pLoFunc->GetParamNum(); ++i) {
                            ISyntaxComponent* p = pLoFunc->GetParam(i);
                            if (0 != p) {
                                pApi->m_pArguments[i] = p->GetRuntimeObject();
                            }
                        }
                    }
                }
                return pApi;
            }
        };

        class LiteralArrayStatementFactory;
        class LiteralArrayStatement : public StatementApi
        {
            friend class LiteralArrayStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                Object* pObject = m_Interpreter->AddNewObjectComponent();
                if (0 != pObject) {
                    pRetValue->SetExpressionApi(pObject);
                    for (int i = 0; i < m_Count; ++i) {
                        Value value = m_pValues[i];
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (value.IsStatementApi() && NULL != value.GetStatementApi()) {
                            value.GetStatementApi()->Execute(&val);
                        }
                        else {
                            val = value;
                            ReplaceVariableWithValue(val);
                        }
                        AutoInterpreterValuePoolValueOperation ret(m_Interpreter->GetInnerValuePool());
                        Value& retVal = ret.Get();
                        Value key(i);
                        m_Interpreter->CallMember(*pObject, key, TRUE, FunctionData::PARAM_CLASS_OPERATOR, &val, 1, &retVal);
                    }
                }
                else {
                    pRetValue->SetInt(0);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LiteralArrayStatement(Interpreter& interpreter) :StatementApi(interpreter), m_Count(0), m_pValues(NULL) {}
        private:
            int m_Count;
            Value* m_pValues;
        };
        class LiteralArrayStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                LiteralArrayStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LiteralArrayStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    const FunctionData& call = static_cast<const FunctionData&>(statement);
                    pApi->m_Count = call.GetParamNum();
                    pApi->m_pValues = new Value[pApi->m_Count];
                    if (0 != pApi->m_pValues) {
                        for (int i = 0; i < call.GetParamNum(); ++i) {
                            ISyntaxComponent* p = call.GetParam(i);
                            if (0 == p)
                                continue;
                            pApi->m_pValues[i] = p->GetRuntimeObject();
                        }
                    }
                }
                return pApi;
            }
        };

        class LiteralObjectStatementFactory;
        class LiteralObjectStatement : public StatementApi
        {
            friend class LiteralObjectStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                Object* pObject = m_Interpreter->AddNewObjectComponent();
                if (0 != pObject) {
                    pRetValue->SetExpressionApi(pObject);
                    for (int i = 0; i < m_Count; ++i) {
                        Value key = m_pKeys[i];
                        Value value = m_pValues[i];
                        if (!key.IsInvalid()) {
                            AutoInterpreterValuePoolValueOperation op1(m_Interpreter->GetInnerValuePool());
                            AutoInterpreterValuePoolValueOperation op2(m_Interpreter->GetInnerValuePool());
                            Value& val1 = op1.Get();
                            Value& val2 = op2.Get();
                            if (key.IsStatementApi() && NULL != key.GetStatementApi()) {
                                key.GetStatementApi()->Execute(&val1);
                            }
                            else {
                                val1 = key;
                                ReplaceVariableWithValue(val1);
                            }
                            if (value.IsStatementApi() && NULL != value.GetStatementApi()) {
                                value.GetStatementApi()->Execute(&val2);
                            }
                            else {
                                val2 = value;
                                ReplaceVariableWithValue(val2);
                            }
                            AutoInterpreterValuePoolValueOperation ret(m_Interpreter->GetInnerValuePool());
                            Value& retVal = ret.Get();
                            m_Interpreter->CallMember(*pObject, val1, TRUE, FunctionData::PARAM_CLASS_OPERATOR, &val2, 1, &retVal);
                        }
                    }
                }
                else {
                    pRetValue->SetInt(0);
                }
                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit LiteralObjectStatement(Interpreter& interpreter) :StatementApi(interpreter), m_Count(0), m_pKeys(NULL), m_pValues(NULL) {}
        private:
            int m_Count;
            Value* m_pKeys;
            Value* m_pValues;
        };
        class LiteralObjectStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                const FunctionData& func = static_cast<const FunctionData&>(statement);
                if (func.IsHighOrder() || !func.HaveParamOrStatement())
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                LiteralObjectStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LiteralObjectStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    const FunctionData& call = static_cast<const FunctionData&>(statement);

                    pApi->m_Count = call.GetParamNum();
                    pApi->m_pKeys = new Value[pApi->m_Count];
                    pApi->m_pValues = new Value[pApi->m_Count];
                    if (0 != pApi->m_pKeys && 0 != pApi->m_pValues) {
                        for (int i = 0; i < call.GetParamNum(); ++i) {
                            ISyntaxComponent* pComp = call.GetParam(i);
                            if (0 == pComp || pComp->GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                                continue;
                            FunctionData* p = static_cast<FunctionData*>(pComp);
                            if (0 == p)
                                continue;
                            if (p->GetParamNum() != 2)
                                continue;
                            ISyntaxComponent* p1 = p->GetParam(0);
                            ISyntaxComponent* p2 = p->GetParam(1);
                            if (p1 && p2) {
                                pApi->m_pKeys[i] = p1->GetRuntimeObject();
                                pApi->m_pValues[i] = p2->GetRuntimeObject();
                            }
                        }
                    }
                }
                return pApi;
            }
        };

        class StructStatementFactory;
        class StructStatement : public StatementApi
        {
            friend class StructStatementFactory;
        public:
            virtual ExecuteResultEnum Execute(Value* pRetValue)const
            {
                FunctionData& func = *m_pDefine;
                FunctionData* pLoFunc = func.GetLowerOrderFunction();
                if (0 == m_Interpreter)
                    return EXECUTE_RESULT_NORMAL;
                int pnum = pLoFunc->GetParamNum();
                if (1 < pnum)
                    return EXECUTE_RESULT_NORMAL;
                Struct* pStruct = m_Interpreter->AddNewStructComponent();
                if (0 != pStruct) {
                    pStruct->SetDefinitionRef(func);
                    if (pLoFunc->HaveParam()) {
                        ISyntaxComponent* pName = pLoFunc->GetParam(0);
                        if (NULL != pName) {
                            Value nameVal = pName->GetRuntimeObject();
                            AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                            Value& val = op.Get();
                            if (nameVal.IsStatementApi() && NULL != nameVal.GetStatementApi()) {
                                nameVal.GetStatementApi()->Execute(&val);
                            }
                            else {
                                val = nameVal;
                            }
                            SetVariableValue(val, Value(pStruct));
                        }
                    }
                    if (NULL != pRetValue) {
                        pRetValue->SetExpressionApi(pStruct);
                    }
                }

                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StructStatement(Interpreter& interpreter) :StatementApi(interpreter), m_pDefine(NULL) {}
        private:
            FunctionData* m_pDefine;
        };
        class StructStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const ISyntaxComponent& statement)const
            {
                //Structure definition syntax: struct(name){member1(size,num);member2(size,num);...};
                // where size can be a number or char, short, int, ptr
                if (statement.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(ISyntaxComponent& statement)const
            {
                StructStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<StructStatement>();
                if (NULL != pApi) {
                    statement.PrepareGeneralRuntimeObject();
                    pApi->m_pDefine = static_cast<FunctionData*>(&statement);
                }
                return pApi;
            }
        };
    }
    //------------------------------------------------------------------------------------------------------
    void Interpreter::InitInnerApis()
    {
        using namespace InnerApi;
        //
        RegisterInnerFunctionApi("debugbreak", new DebugBreakApi(*this));
        RegisterInnerFunctionApi("expr", new ExprApi(*this));
        RegisterInnerFunctionApi("print", new PrintApi(*this));
        ExpressionApi* pAddOp = new AddApi(*this);
        ExpressionApi* pSubOp = new SubApi(*this);
        ExpressionApi* pMulOp = new MulApi(*this);
        ExpressionApi* pDivOp = new DivApi(*this);
        ExpressionApi* pModOp = new ModApi(*this);
        RegisterInnerFunctionApi("__add", pAddOp);
        RegisterInnerFunctionApi("__sub", pSubOp);
        RegisterInnerFunctionApi("__mul", pMulOp);
        RegisterInnerFunctionApi("__div", pDivOp);
        RegisterInnerFunctionApi("__mod", pModOp);
        RegisterInnerFunctionApi("__isequal", new EqApi(*this));
        RegisterInnerFunctionApi("__isnotequal", new NotEqApi(*this));
        RegisterInnerFunctionApi("__isless", new LessApi(*this));
        RegisterInnerFunctionApi("__isgreat", new GreatApi(*this));
        RegisterInnerFunctionApi("__isnotgreat", new NotGreatApi(*this));
        RegisterInnerFunctionApi("__isnotless", new NotLessApi(*this));
        RegisterInnerFunctionApi("__and", new AndApi(*this));
        RegisterInnerFunctionApi("__or", new OrApi(*this));
        RegisterInnerFunctionApi("__not", new NotApi(*this));
        RegisterInnerFunctionApi("__inc", new IncApi(*this));
        RegisterInnerFunctionApi("__dec", new DecApi(*this));
        RegisterInnerFunctionApi("toint", new IntApi(*this));
        RegisterInnerFunctionApi("tofloat", new FloatApi(*this));
        RegisterInnerFunctionApi("tostring", new StringApi(*this));
        RegisterInnerFunctionApi("int2hex", new Int2HexApi(*this));
        RegisterInnerFunctionApi("hex2int", new Hex2IntApi(*this));
        RegisterInnerFunctionApi("system", new SystemApi(*this));
        ExpressionApi* pBitAndOp = new BitAndApi(*this);
        ExpressionApi* pBitOrOp = new BitOrApi(*this);
        ExpressionApi* pBitXorOp = new BitXorApi(*this);
        ExpressionApi* pLeftShiftOp = new LeftShiftApi(*this);
        ExpressionApi* pRightShiftOp = new RightShiftApi(*this);
        ExpressionApi* pUnsignedRightShiftOp = new UnsignedRightShiftApi(*this);
        RegisterInnerFunctionApi("__bitand", pBitAndOp);
        RegisterInnerFunctionApi("&", new BitAndOpApi(*this));
        RegisterInnerFunctionApi("__bitor", pBitOrOp);
        RegisterInnerFunctionApi("|", new BitOrOpApi(*this));
        RegisterInnerFunctionApi("__bitxor", pBitXorOp);
        RegisterInnerFunctionApi("^", new BitXorOpApi(*this));
        RegisterInnerFunctionApi("__neg", new NegApi(*this));
        RegisterInnerFunctionApi("~", new NegOpApi(*this));
        RegisterInnerFunctionApi("__lshift", pLeftShiftOp);
        RegisterInnerFunctionApi("<<", new LeftShiftOpApi(*this));
        RegisterInnerFunctionApi("__rshift", pRightShiftOp);
        RegisterInnerFunctionApi(">>", new RightShiftOpApi(*this));
        RegisterInnerFunctionApi("__urshift", pUnsignedRightShiftOp);
        RegisterInnerFunctionApi(">>>", new UnsignedRightShiftOpApi(*this));

        RegisterInnerFunctionApi("+", new AddOpApi(*this));
        RegisterInnerFunctionApi("-", new SubOpApi(*this));
        RegisterInnerFunctionApi("*", new MulOpApi(*this));
        RegisterInnerFunctionApi("/", new DivOpApi(*this));
        RegisterInnerFunctionApi("%", new ModOpApi(*this));
        RegisterInnerFunctionApi("==", new EqOpApi(*this));
        RegisterInnerFunctionApi("!=", new NotEqOpApi(*this));
        RegisterInnerFunctionApi("<", new LessOpApi(*this));
        RegisterInnerFunctionApi(">", new GreatOpApi(*this));
        RegisterInnerFunctionApi("<=", new NotGreatOpApi(*this));
        RegisterInnerFunctionApi(">=", new NotLessOpApi(*this));
        RegisterInnerFunctionApi("&&", new AndOpApi(*this));
        RegisterInnerFunctionApi("||", new OrOpApi(*this));
        RegisterInnerFunctionApi("!", new NotOpApi(*this));
        RegisterInnerFunctionApi("++", new IncOpApi(*this));
        RegisterInnerFunctionApi("--", new DecOpApi(*this));

        RegisterInnerFunctionApi("=", new AssignOpApi(*this, 0));
        RegisterInnerFunctionApi("+=", new AssignOpApi(*this, pAddOp));
        RegisterInnerFunctionApi("-=", new AssignOpApi(*this, pSubOp));
        RegisterInnerFunctionApi("*=", new AssignOpApi(*this, pMulOp));
        RegisterInnerFunctionApi("/=", new AssignOpApi(*this, pDivOp));
        RegisterInnerFunctionApi("%=", new AssignOpApi(*this, pModOp));
        RegisterInnerFunctionApi("<<=", new AssignOpApi(*this, pLeftShiftOp));
        RegisterInnerFunctionApi(">>=", new AssignOpApi(*this, pRightShiftOp));
        RegisterInnerFunctionApi(">>>=", new AssignOpApi(*this, pUnsignedRightShiftOp));
        RegisterInnerFunctionApi("&=", new AssignOpApi(*this, pBitAndOp));
        RegisterInnerFunctionApi("|=", new AssignOpApi(*this, pBitOrOp));
        RegisterInnerFunctionApi("^=", new AssignOpApi(*this, pBitXorOp));
        //
        RegisterInnerFunctionApi("strlen", new StrlenApi(*this));
        RegisterInnerFunctionApi("strstr", new StrstrApi(*this));
        RegisterInnerFunctionApi("substr", new SubstrApi(*this));
        RegisterInnerFunctionApi("strchr", new StrchrApi(*this));
        RegisterInnerFunctionApi("strrchr", new StrrchrApi(*this));
        RegisterInnerFunctionApi("strcspn", new StrcspnApi(*this));
        RegisterInnerFunctionApi("strspn", new StrspnApi(*this));
        RegisterInnerFunctionApi("tolower", new TolowerApi(*this));
        RegisterInnerFunctionApi("toupper", new ToupperApi(*this));
        RegisterInnerFunctionApi("byteat", new ByteAtApi(*this));
        RegisterInnerFunctionApi("bytes2str", new Bytes2StrApi(*this));
        RegisterInnerFunctionApi("byterep", new ByteRepApi(*this));
        //
        RegisterInnerFunctionApi("object", new ObjectApi(*this));
        RegisterInnerFunctionApi("new", new NewApi(*this));
        RegisterInnerFunctionApi("arg", new ArgApi(*this));
        //
        RegisterInnerStatementApi("for", new ForStatementFactory());
        RegisterInnerStatementApi("while", new WhileStatementFactory());
        RegisterInnerStatementApi("foreach", new ForEachStatementFactory());
        RegisterInnerStatementApi("loop", new LoopStatementFactory());
        RegisterInnerStatementApi("looplist", new LoopListStatementFactory());
        RegisterInnerStatementApi("if", new IfElseStatementFactory());
        RegisterInnerStatementApi("?", new CondExpStatementFactory());
        RegisterInnerStatementApi("if", new IfGotoStatementFactory());
        RegisterInnerStatementApi("goto", new GotoStatementFactory());
        RegisterInnerStatementApi("return", new ReturnStatementFactory());
        RegisterInnerStatementApi("break", new BreakStatementFactory());
        RegisterInnerStatementApi("continue", new ContinueStatementFactory());
        RegisterInnerStatementApi("function", new FunctionStatementFactory());
        RegisterInnerStatementApi("literalarray", new LiteralArrayStatementFactory());
        RegisterInnerStatementApi("literalobject", new LiteralObjectStatementFactory());
#ifdef _GAMECLIENT_
        RegisterInnerStatementApi("struct", new StructStatementFactory());
#endif //_GAMECLIENT_
    }

    void Interpreter::ReleaseInnerApis()
    {
        for (Functions::Iterator it = m_InnerFunctionApis.First(); !it.IsNull(); ++it) {
            ExpressionApi* p = it->GetValue();
            if (0 != p) {
                delete p;
                p = NULL;
            }
        }
        m_InnerFunctionApis.CleanUp();

        for (StatementApiFactories::Iterator it = m_InnerStatementApis.First(); !it.IsNull(); ++it) {
            StatementApiFactoryList& list = it->GetValue();
            for (int id = list.FrontID(); TRUE == list.IsValidID(id); id = list.NextID(id)) {
                StatementApiFactory* p = list[id];
                if (0 != p)
                    delete p;
            }
        }
        m_InnerStatementApis.CleanUp();
    }

    void Interpreter::RegisterInnerFunctionApi(const char* id, ExpressionApi* p)
    {
        if (0 == p)
            return;
        ExpressionApi* pApi = m_InnerFunctionApis.Get(id);
        if (0 == pApi) {
            m_InnerFunctionApis.Add(id, p);
        }
    }

    void Interpreter::RegisterInnerStatementApi(const char* id, StatementApiFactory* p)
    {
        if (0 == p)
            return;
        StatementApiFactoryList& apiList = m_InnerStatementApis.Get(id);
        if (apiList.Size() == 0) {
            StatementApiFactoryList newlist;
            m_InnerStatementApis.Add(id, newlist);
        }
        StatementApiFactoryList& list = m_InnerStatementApis.Get(id);
        int find = FALSE;
        for (int sid = list.FrontID(); list.IsValidID(sid); sid = list.NextID(sid)) {
            StatementApiFactory* pApi = list[sid];
            if (pApi == p) {
                find = TRUE;
                break;
            }
        }
        if (!find) {
            list.PushBack(p);
        }
    }
    //------------------------------------------------------------------------------------------------------
    class AutoStackInfoStackOperation
    {
    public:
        AutoStackInfoStackOperation(Interpreter& interpreter, Value* pParams, int paramNum, int stackSize, const ISyntaxComponent& definition) :m_Interpreter(&interpreter)
        {
            if (NULL != m_Interpreter) {
                m_Interpreter->PushStackInfo(pParams, paramNum, stackSize, definition);
            }
        }
        ~AutoStackInfoStackOperation()
        {
            if (NULL != m_Interpreter) {
                m_Interpreter->PopStackInfo();
            }
        }
    private:
        Interpreter* m_Interpreter;
    };
    //------------------------------------------------------------------------------------------------------
    void Closure::SetDefinitionRef(const Value* pArguments, int argumentNum, const ISyntaxComponent& comp)
    {
        m_pArguments = pArguments;
        m_ArgumentNum = argumentNum;
        m_pDefinition = &comp;
        if (comp.GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
            const FunctionData& func = static_cast<const FunctionData&>(comp);
            m_StackSize = func.CalculateStackSize();
            m_Statements = func.GetRuntimeFunctionBody();
        }
        else if (comp.GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
            const StatementData& statement = static_cast<const StatementData&>(comp);
            int num = statement.GetFunctionNum();
            if (num > 1) {
                FunctionData* pFunc = statement.GetFunction(num - 1);
                if (NULL != pFunc) {
                    m_StackSize = pFunc->CalculateStackSize();
                    m_Statements = pFunc->GetRuntimeFunctionBody();
                }
            }
        }
    }
    ExecuteResultEnum Closure::Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
    {
        paramClass;
        //Closure implements the calling of custom functions.
        if (NULL != m_Statements && NULL != m_Interpreter) {
            ReplaceVariableWithValue(pParams, num);
            AutoStackInfoStackOperation op(*m_Interpreter, pParams, num, m_StackSize, *m_pDefinition);
            //Process parameter list
            if (NULL != m_pArguments) {
                for (int i = 0; i < m_ArgumentNum && i < num; ++i) {
                    SetVariableValue(m_pArguments[i], pParams[i]);
                }
            }
            return m_Statements->Execute(pRetValue);
        }
        return EXECUTE_RESULT_NORMAL;
    }

    ExecuteResultEnum MemberAccessor::Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
    {
        ExecuteResultEnum ret = EXECUTE_RESULT_NORMAL;
        if (0 != m_Object) {
            AutoInterpreterValuePoolValuesOperation op(m_Interpreter->GetInnerValuePool());
            Value* params = op.Get();
            int paramNum = num + 2;
            if (1 == num && pParams[0].IsInvalid()) {
                //_epsilon_Special handling of syntactic side effects
                num = 0;
                paramNum = 2;
            }
            params[0] = Value(MARK_ACCESSOR_CALL);
            params[1] = Value(m_MemberIndex);
            for (int ix = 0; ix < num; ++ix) {
                params[2 + ix] = pParams[ix];
            }
            ret = m_Object->Execute(paramClass, params, paramNum, pRetValue);
        }
        return ret;
    }

    ObjectBase::ObjectBase(Interpreter& interpreter, int customInnerMemberNum) :ExpressionApi(interpreter), m_NameIndexMap(0), m_MemberInfos(0), m_Accessors(0), m_MemberNum(0), m_Capacity(0), m_InnerMemberNum(INNER_MEMBER_INDEX_NUM + customInnerMemberNum), m_TempAccessor(interpreter)
    {
        m_InnerMemberAccessors = new MemberAccessorPtr[m_InnerMemberNum];
        if (m_InnerMemberAccessors) {
            for (int ix = 0; ix < m_InnerMemberNum; ++ix) {
                m_InnerMemberAccessors[ix] = new MemberAccessor(interpreter, *this, ix);
            }
        }
        m_TempAccessor.SetObject(*this);
    }

    ObjectBase::~ObjectBase()
    {
        if (0 != m_NameIndexMap) {
            delete m_NameIndexMap;
            m_NameIndexMap = 0;
        }
        if (0 != m_MemberInfos) {
            delete[] m_MemberInfos;
            m_MemberInfos = 0;
        }
        if (0 != m_Accessors) {
            for (int ix = 0; ix < m_MemberNum; ++ix) {
                if (0 != m_Accessors[ix])
                    delete m_Accessors[ix];
            }
            delete[] m_Accessors;
            m_Accessors = 0;
            m_MemberNum = 0;
        }
        if (0 != m_InnerMemberAccessors) {
            for (int ix = 0; ix < m_InnerMemberNum; ++ix) {
                if (0 != m_InnerMemberAccessors[ix]) {
                    delete m_InnerMemberAccessors[ix];
                    m_InnerMemberAccessors[ix] = 0;
                }
            }
            delete[] m_InnerMemberAccessors;
            m_InnerMemberAccessors = 0;
            m_InnerMemberNum = 0;
        }
    }

    ExecuteResultEnum ObjectBase::Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
    {
        if (0 != pParams && num > 0) {
            ReplaceVariableWithValue(pParams, num);
            if (pParams[0].IsInt() && pParams[0].GetInt() == MemberAccessor::MARK_ACCESSOR_CALL) {
                if (num > 1 && pParams[1].IsInt() && pParams[1].GetInt() >= 0) {
                    int _index = pParams[1].GetInt();
                    if (_index < m_InnerMemberNum) {
                        int index = _index;
                        switch (index) {
                        case INNER_MEMBER_INDEX_SIZE:
                        {
                            if (2 == num) {
                                if (0 != pRetValue)
                                    pRetValue->SetInt(m_MemberNum);
                            }
                        }
                        break;
                        default:
                        {
                            return ExecuteCustomMember(index - INNER_MEMBER_INDEX_NUM, paramClass, pParams + 2, num - 2, pRetValue);
                        }
                        break;
                        }
                    }
                    else {
                        int index = _index - m_InnerMemberNum;
                        if (index >= 0 && index < m_MemberNum && NULL != m_MemberInfos) {
                            switch (num) {
                            case 2:
                            {
                                if ((FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (0 != pRetValue) {
                                        *pRetValue = info.m_Value;
                                    }
                                }
                                else//obj.method()
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (info.m_Value.IsExpressionApi() && 0 != info.m_Value.GetExpressionApi()) {
                                        ExpressionApi& s_Exp = *(info.m_Value.GetExpressionApi());
                                        Value params[] = { Value(this) };
                                        s_Exp.Execute(paramClass, params, 1, pRetValue);
                                    }
                                }
                            }
                            break;
                            case 3:
                            {
                                if ((FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property=val
                                {
                                    MemberInfo& info = m_MemberInfos[index];
                                    info.m_Value = pParams[2];
                                    if (0 != pRetValue) {
                                        *pRetValue = pParams[2];
                                    }
                                }
                                else//obj.method(p1)
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (info.m_Value.IsExpressionApi() && 0 != info.m_Value.GetExpressionApi()) {
                                        ExpressionApi& s_Exp = *(info.m_Value.GetExpressionApi());
                                        Value params[] = { Value(this), pParams[2] };
                                        s_Exp.Execute(paramClass, params, 2, pRetValue);
                                    }
                                }
                            }
                            break;
                            default://obj.method(p1,p2,...)
                            {
                                const MemberInfo& info = m_MemberInfos[index];
                                if (info.m_Value.IsExpressionApi() && 0 != info.m_Value.GetExpressionApi()) {
                                    ExpressionApi& s_Exp = *(info.m_Value.GetExpressionApi());
                                    Value params[MAX_FUNCTION_PARAM_NUM];
                                    params[0] = Value(this);
                                    for (int ix = 0; ix < num - 2 && ix < MAX_FUNCTION_PARAM_NUM - 1; ++ix) {
                                        params[1 + ix] = pParams[2 + ix];
                                    }
                                    s_Exp.Execute(paramClass, params, num - 1, pRetValue);
                                }
                            }
                            break;
                            }
                        }
                        else if (index >= m_MemberNum) {
                            //New members are only allowed through obj.property=val method
                            if (3 == num && (FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK) {
                                MemberInfo& info = m_TempMemberInfo;
                                info.m_Value = pParams[2];
                                if (0 != pRetValue) {
                                    *pRetValue = pParams[2];
                                }
                                AddTempToMember(index - m_MemberNum);
                            }
                        }
                    }
                }
            }
            else {
                if (1 == num) {
                    if ((pParams[0].IsString() || pParams[0].IsIdentifier()) && 0 != pParams[0].GetString()) {
                        if (0 != pRetValue) {
                            const char* pName = pParams[0].GetString();
                            int index = GetMemberIndex(pName);
                            if (index >= 0 && index < m_InnerMemberNum && NULL != m_InnerMemberAccessors) {
                                pRetValue->SetExpressionApi(m_InnerMemberAccessors[index]);
                            }
                            else if (index >= m_InnerMemberNum && index < m_InnerMemberNum + m_MemberNum && NULL != m_Accessors) {
                                pRetValue->SetExpressionApi(m_Accessors[index - m_InnerMemberNum]);
                            }
                            else {
                                m_TempMemberInfo.m_Name = pName;
                                m_TempAccessor.SetMemberIndex(m_MemberNum + m_InnerMemberNum);
                                pRetValue->SetExpressionApi(&m_TempAccessor);
                            }
                        }
                    }
                    else if (pParams[0].IsInt())//If the value is given directly, the member explanation will
                                                //be defined according to the script.
                    {
                        if (0 != pRetValue) {
                            int index = pParams[0].GetInt();
                            if (index >= 0 && index < m_MemberNum && NULL != m_Accessors) {
                                pRetValue->SetExpressionApi(m_Accessors[index]);
                            }
                            else {
                                m_TempMemberInfo.m_Name = "";
                                m_TempAccessor.SetMemberIndex(index + m_InnerMemberNum);
                                pRetValue->SetExpressionApi(&m_TempAccessor);
                            }
                        }
                    }
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void ObjectBase::Resize()
    {
        if (0 != m_Interpreter) {
            int capacity = m_Capacity + MEMBER_INFO_CAPACITY_DELTA_SIZE;
            MemberInfo* pInfos = new MemberInfo[capacity];
            if (0 != pInfos) {
                NameIndexMap* pNameIndexMap = new NameIndexMap();
                if (0 != pNameIndexMap) {
                    MemberAccessorPtr* pAccessors = new MemberAccessorPtr[capacity];
                    if (0 != pAccessors) {
                        memset(pAccessors, 0, sizeof(MemberAccessorPtr) * capacity);
                        if (0 != m_MemberInfos) {
                            for (int ix = 0; ix < m_MemberNum; ++ix) {
                                pInfos[ix] = m_MemberInfos[ix];
                                pAccessors[ix] = new MemberAccessor(*m_Interpreter, *this, ix + m_InnerMemberNum);
                            }
                        }
                        pNameIndexMap->InitTable(capacity);
                        if (0 != m_NameIndexMap) {
                            for (NameIndexMap::Iterator it = m_NameIndexMap->First(); FALSE == it.IsNull(); ++it) {
                                pNameIndexMap->Add(it->GetKey(), it->GetValue());
                            }
                        }
                        if (0 != m_NameIndexMap) {
                            delete m_NameIndexMap;
                        }
                        if (0 != m_MemberInfos) {
                            delete[] m_MemberInfos;
                        }
                        if (0 != m_Accessors) {
                            for (int ix = 0; ix < m_MemberNum; ++ix) {
                                if (0 != m_Accessors[ix]) {
                                    delete m_Accessors[ix];
                                }
                            }
                            delete[] m_Accessors;
                        }
                        m_NameIndexMap = pNameIndexMap;
                        m_MemberInfos = pInfos;
                        m_Accessors = pAccessors;
                        m_Capacity = capacity;
                    }
                    else {
                        delete pNameIndexMap;
                        delete[] pInfos;
                    }
                }
                else {
                    delete[] pInfos;
                }
            }
        }
    }

    int ObjectBase::GetMemberIndex(const char* name)const
    {
        int index = -1;
        if (0 != name) {
            if (0 != m_NameIndexMap) {
                int ix = m_NameIndexMap->Get(name);
                if (ix >= 0) {
                    index = ix + m_InnerMemberNum;
                }
            }
            if (index < 0) {
                const char* pSizeName = "size";
                if (pSizeName && strcmp(name, pSizeName) == 0)
                    index = INNER_MEMBER_INDEX_SIZE;
                else {
                    index = GetCustomInnerMemberIndex(name);
                    if (index >= 0)
                        index += INNER_MEMBER_INDEX_NUM;
                }
            }
        }
        return index;
    }

    void ObjectBase::ResetTemp()
    {
        m_TempMemberInfo.Reset();
        m_TempAccessor.SetMemberIndex(-1);
    }

    void ObjectBase::AddTempToMember(int skipNum)
    {
        int needNum = m_MemberNum + skipNum;
        if (needNum < MAX_MEMBER_NUM) {
            while (needNum >= m_Capacity) {
                Resize();
            }
            if (needNum < m_Capacity && NULL != m_MemberInfos) {
                for (int ix = 0; ix < skipNum; ++ix) {
                    m_MemberInfos[m_MemberNum].Reset();
                    m_Accessors[m_MemberNum] = new MemberAccessor(*m_Interpreter, *this, m_MemberNum + m_InnerMemberNum);
                    ++m_MemberNum;
                }
                if (0 != m_MemberInfos && 0 != m_Accessors && 0 != m_Interpreter) {
                    if (NULL != m_NameIndexMap && NULL != m_TempMemberInfo.m_Name && m_TempMemberInfo.m_Name[0] != 0) {
                        int& ix = m_NameIndexMap->Get(m_TempMemberInfo.m_Name);
                        if (ix < 0) {
                            m_NameIndexMap->Add(m_TempMemberInfo.m_Name, m_MemberNum);
                        }
                        else {
                            //Repeating the name would make it impossible to get here!
                        }
                    }
                    m_MemberInfos[m_MemberNum] = m_TempMemberInfo;
                    m_Accessors[m_MemberNum] = new MemberAccessor(*m_Interpreter, *this, m_MemberNum + m_InnerMemberNum);
                    ++m_MemberNum;
                    ResetTemp();
                }
            }
        }
    }

    Object::Object(Interpreter& interpreter) :ObjectBase(interpreter, 0)
    {

    }

    Object::~Object()
    {

    }

    Struct::~Struct()
    {
        if (0 != m_MemberInfos) {
            delete[] m_MemberInfos;
            m_MemberInfos = 0;
        }
        if (0 != m_Accessors) {
            for (int ix = 0; ix < m_MemberNum + INNER_MEMBER_INDEX_NUM; ++ix) {
                if (0 != m_Accessors[ix])
                    delete m_Accessors[ix];
            }
            delete[] m_Accessors;
            m_Accessors = 0;
            m_MemberNum = 0;
        }
    }

    ExecuteResultEnum Struct::Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
    {
        if (0 != m_pDefinition && 0 != m_Accessors && 0 != m_MemberInfos && 0 != pParams && num > 0) {
            ReplaceVariableWithValue(pParams, num);
            if (pParams[0].IsInt() && pParams[0].GetInt() == MemberAccessor::MARK_ACCESSOR_CALL) {
                if (num > 1 && pParams[1].IsInt() && pParams[1].GetInt() >= 0) {
                    int _index = pParams[1].GetInt();
                    if (_index < INNER_MEMBER_INDEX_NUM) {
                        int index = _index;
                        switch (index) {
                        case INNER_MEMBER_INDEX_ATTACH:
                        {
                            if (3 == num && pParams[2].IsInt()) {
                                Attach(pParams[2].GetInt());
                            }
                        }
                        break;
                        case INNER_MEMBER_INDEX_CLONE:
                        {
                            if (2 == num) {
                                Struct* pNew = Clone();
                                if (0 != pNew && 0 != pRetValue)
                                    pRetValue->SetExpressionApi(pNew);
                            }
                        }
                        break;
                        case INNER_MEMBER_INDEX_ADDR:
                        {
                            if (2 == num) {
                                if (0 != pRetValue)
                                    pRetValue->SetInt(m_Addr);
                            }
                        }
                        break;
                        case INNER_MEMBER_INDEX_SIZE:
                        {
                            if (2 == num) {
                                if (0 != pRetValue)
                                    pRetValue->SetInt(m_Size);
                            }
                        }
                        break;
                        }
                    }
                    else {
                        int index = _index - INNER_MEMBER_INDEX_NUM;
                        if (index >= 0 && index < m_MemberNum) {
                            switch (num) {
                            case 2://obj.property
                            {
                                const MemberInfo& info = m_MemberInfos[index];
                                if (0 != m_Addr && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                    void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset);
                                    int val = 0;
                                    memcpy(&val, pAddr, info.m_Size);
                                    pRetValue->SetInt(val);
                                }
                            }
                            break;
                            case 3:
                            {
                                if ((FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property=val
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (0 != m_Addr && pParams[2].IsInt() && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                        void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset);
                                        int val = pParams[2].GetInt();
                                        memcpy(pAddr, &val, info.m_Size);
                                        pRetValue->SetInt(val);
                                    }
                                }
                                else//obj.property(index)
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (pParams[2].IsInt() && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                        int argIndex = pParams[2].GetInt();
                                        if (0 != m_Addr && argIndex >= 0 && argIndex < info.m_Num) {
                                            void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset + info.m_Size * argIndex);
                                            int val = 0;
                                            memcpy(&val, pAddr, info.m_Size);
                                            pRetValue->SetInt(val);
                                        }
                                        else if (argIndex == INNER_ARG_INDEX_OFFSET) {
                                            pRetValue->SetInt(info.m_Offset);
                                        }
                                        else if (argIndex == INNER_ARG_INDEX_SIZE) {
                                            pRetValue->SetInt(info.m_Size);
                                        }
                                        else if (argIndex == INNER_ARG_INDEX_NUM) {
                                            pRetValue->SetInt(info.m_Num);
                                        }
                                    }
                                }
                            }
                            break;
                            case 4://obj.property(index,val)
                            {
                                const MemberInfo& info = m_MemberInfos[index];
                                if (0 != m_Addr && pParams[2].IsInt() && pParams[3].IsInt() && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                    int argIndex = pParams[2].GetInt();
                                    if (argIndex >= 0 && argIndex < info.m_Num) {
                                        void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset + info.m_Size * argIndex);
                                        int val = pParams[3].GetInt();
                                        memcpy(pAddr, &val, info.m_Size);
                                        pRetValue->SetInt(val);
                                    }
                                }
                            }
                            break;
                            }
                        }
                    }
                }
            }
            else {
                if (1 == num) {
                    int index = -1;
                    if ((pParams[0].IsString() || pParams[0].IsIdentifier()) && 0 != pParams[0].GetString()) {
                        const char* pName = pParams[0].GetString();
                        index = GetMemberIndex(pName);
                    }
                    else if (pParams[0].IsInt())//Values are interpreted as script-defined members
                    {
                        index = pParams[0].GetInt() + INNER_MEMBER_INDEX_NUM;
                    }
                    if (index >= 0 && index < m_MemberNum + INNER_MEMBER_INDEX_NUM && 0 != pRetValue && NULL != m_Accessors) {
                        pRetValue->SetExpressionApi(m_Accessors[index]);
                    }
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void Struct::SetDefinitionRef(const FunctionData& func)
    {
        m_pDefinition = &func;
        //Nested structures are not supported first
        //todo:Analyze the definition, generate structural layout data, and initialize m_Accessors
        if (0 != m_Interpreter) {
            int memberNum = func.GetParamNum();
            m_MemberInfos = new MemberInfo[memberNum];
            if (0 != m_MemberInfos) {
                m_Accessors = new MemberAccessorPtr[memberNum + INNER_MEMBER_INDEX_NUM];
                if (0 != m_Accessors) {
                    m_MemberNum = memberNum;

                    memset(m_Accessors, 0, sizeof(MemberAccessorPtr) * (m_MemberNum + INNER_MEMBER_INDEX_NUM));
                    m_Accessors[INNER_MEMBER_INDEX_ATTACH] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_ATTACH);
                    m_Accessors[INNER_MEMBER_INDEX_CLONE] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_CLONE);
                    m_Accessors[INNER_MEMBER_INDEX_ADDR] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_ADDR);
                    m_Accessors[INNER_MEMBER_INDEX_SIZE] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_SIZE);
                    m_Size = 0;
                    for (int ix = 0; ix < m_MemberNum; ++ix) {
                        ISyntaxComponent* pComp = func.GetParam(ix);
                        if (NULL == pComp || pComp->GetSyntaxType() != ISyntaxComponent::TYPE_STATEMENT)
                            continue;
                        StatementData* pStatement = static_cast<StatementData*>(pComp);
                        if (0 != pStatement) {
                            FunctionData* pMember = pStatement->GetFunction(0);
                            if (0 != pMember) {

                                const Value& name = GetVariableValue(pMember->GetNameValue());
                                ISyntaxComponent* pParam0 = pMember->GetParam(0);
                                ISyntaxComponent* pParam1 = pMember->GetParam(1);
                                m_Accessors[ix + INNER_MEMBER_INDEX_NUM] = new MemberAccessor(*m_Interpreter, *this, ix + INNER_MEMBER_INDEX_NUM);

                                int size = 4;
                                int num = 1;
                                if (0 != pParam0) {
                                    Value param0 = pParam0->GetRuntimeObject();
                                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                                    Value& val = op.Get();
                                    if (param0.IsStatementApi() && NULL != param0.GetStatementApi()) {
                                        param0.GetStatementApi()->Execute(&val);
                                    }
                                    else {
                                        val = param0;
                                        ReplaceVariableWithValue(val);
                                    }
                                    if (val.IsInt())
                                        size = val.GetInt();
                                    else if (val.IsIdentifier() && 0 != val.GetString()) {
                                        const char* pName = val.GetString();
                                        if (0 == strcmp("char", pName)) {
                                            size = 1;
                                        }
                                        else if (0 == strcmp("short", pName)) {
                                            size = 2;
                                        }
                                        else if (0 == strcmp("int", pName)) {
                                            size = 4;
                                        }
                                        else if (0 == strcmp("ptr", pName)) {
                                            size = 4;
                                        }
                                    }
                                }
                                if (0 != pParam1) {
                                    Value param1 = pParam1->GetRuntimeObject();
                                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                                    Value& val = op.Get();
                                    if (param1.IsStatementApi() && NULL != param1.GetStatementApi()) {
                                        param1.GetStatementApi()->Execute(&val);
                                    }
                                    else {
                                        val = param1;
                                        ReplaceVariableWithValue(val);
                                    }
                                    if (val.IsInt())
                                        num = val.GetInt();
                                }
                                m_MemberInfos[ix].m_Offset = m_Size;
                                m_MemberInfos[ix].m_Size = size;
                                m_MemberInfos[ix].m_Num = num;
                                if (name.IsIdentifier()) {
                                    m_MemberInfos[ix].m_Name = name.GetString();
                                }
                                m_Size += size * num;
                            }
                        }
                    }
                }
                else {
                    delete[] m_MemberInfos;
                    m_MemberInfos = 0;
                }
            }
        }
    }

    void Struct::Attach(unsigned int addr)
    {
        m_Addr = addr;
    }

    Struct* Struct::Clone() const
    {
        Struct* pStruct = 0;
        if (0 != m_pDefinition && 0 != m_Interpreter) {
            pStruct = m_Interpreter->AddNewStructComponent();
            if (0 != pStruct) {
                pStruct->SetDefinitionRef(*m_pDefinition);
            }
        }
        return pStruct;
    }

    int Struct::GetMemberIndex(const char* name) const
    {
        int index = -1;
        if (0 != name) {
            if (0 != m_MemberInfos) {
                for (int ix = 0; ix < m_MemberNum; ++ix) {
                    const char* pStr = m_MemberInfos[ix].m_Name;
                    if (0 != pStr && strcmp(pStr, name) == 0) {
                        index = ix + INNER_MEMBER_INDEX_NUM;
                        break;
                    }
                }
            }
            if (index < 0) {
                const char* pAttachName = "attach";
                const char* pCloneName = "clone";
                const char* pAddrName = "addr";
                const char* pSizeName = "size";
                if (pAttachName && strcmp(name, pAttachName) == 0)
                    index = INNER_MEMBER_INDEX_ATTACH;
                else if (pCloneName && strcmp(name, pCloneName) == 0)
                    index = INNER_MEMBER_INDEX_CLONE;
                else if (pAddrName && strcmp(name, pAddrName) == 0)
                    index = INNER_MEMBER_INDEX_ADDR;
                else if (pSizeName && strcmp(name, pSizeName) == 0)
                    index = INNER_MEMBER_INDEX_SIZE;
            }
        }
        return index;
    }
    //------------------------------------------------------------------------------------------------------
    ValueData::ValueData(Interpreter& interpreter) :
        ISyntaxComponent(ISyntaxComponent::TYPE_VALUE, interpreter),
        m_RuntimeFunctionCall(0),
        m_RuntimeObjectPrepared(FALSE),
        m_pInnerValuePool(0)
    {
        const InterpreterOptions& options = interpreter.GetOptions();
        m_MaxLocalNum = options.GetMaxLocalNum();
        m_pInnerValuePool = &interpreter.GetInnerValuePool();
    }

    int ValueData::IsValid()const
    {
        if (m_Value.IsFunction()) {
            FunctionData* p = m_Value.GetFunction();
            if (0 != p)
                return p->IsValid();
            else
                return FALSE;
        }
        return HaveId();
    }
    int ValueData::GetIdType()const
    {
        if (m_Value.IsFunction()) {
            FunctionData* p = m_Value.GetFunction();
            if (0 != p)
                return p->GetIdType();
            else
                return Value::TYPE_INVALID;
        }
        return m_Value.GetType();
    }
    const char* ValueData::GetId()const
    {
        if (m_Value.IsFunction()) {
            FunctionData* p = m_Value.GetFunction();
            if (0 != p)
                return p->GetId();
            else
                return "";
        }
        else if (m_Value.IsString() || m_Value.IsIdentifier()) {
            return m_Value.GetString();
        }
        else {
            char temp[MAX_NUMBER_STRING_SIZE];
            const char* p = m_Value.ToString(temp, MAX_NUMBER_STRING_SIZE);
            return p;
        }
    }
    int ValueData::GetLine()const
    {
        if (m_Value.IsFunction()) {
            FunctionData* p = m_Value.GetFunction();
            if (0 != p)
                return p->GetLine();
            else
                return m_Value.GetLine();
        }
        return m_Value.GetLine();
    }
    int ValueData::HaveId()const
    {
        if (m_Value.IsFunction()) {
            FunctionData* p = m_Value.GetFunction();
            if (0 != p)
                return p->HaveId();
            else
                return FALSE;
        }
        else if (m_Value.IsString() || m_Value.IsIdentifier()) {
            const char* p = m_Value.GetString();
            return 0 != p ? TRUE : FALSE;
        }
        else if (m_Value.IsExpressionApi() || m_Value.IsStatementApi() || m_Value.IsPtr() || m_Value.IsInvalid()) {
            return FALSE;
        }
        else {
            return TRUE;
        }
    }

    void ValueData::PrepareRuntimeObject()
    {
        if (NULL == m_Interpreter)
            return;
        if (m_RuntimeObjectPrepared)
            return;
        StatementApiFactory* pApiFactory = m_Interpreter->FindStatementApi(*this);
        m_RuntimeObjectPrepared = TRUE;
        if (0 != pApiFactory) {
            //Let the API generate composite runtime objects.
            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
            m_RuntimeFunctionCall.SetStatementApi(pApi);
        }
        else {
            PrepareGeneralRuntimeObject();
        }
    }

    void ValueData::PrepareGeneralRuntimeObject()
    {
        Value& valOfName = GetValue();
        if (valOfName.IsIdentifier()) {
            char* pStr = valOfName.GetString();
            if (NULL != pStr) {
                if (pStr[0] == '$')//Arguments
                {
                    if (pStr[1] == '$' && pStr[2] == 0) {
                        valOfName.SetArgIndex(-1);
                    }
                    else if (isdigit(pStr[1])) {
                        int index = atoi(pStr + 1);
                        if (index >= 0 && index < MAX_FUNCTION_PARAM_NUM) {
                            valOfName.SetArgIndex(index);
                        }
                    }
                    else {
                        FunctionData* pFuncDef = m_Interpreter->GetCurFunctionDefinition();
                        if (0 != pFuncDef) {
                            int index = pFuncDef->GetLocalIndex(pStr);
                            if (index < 0) {
                                index = pFuncDef->AllocLocalIndex(pStr);
                            }
                            if (index >= 0 && index < m_MaxLocalNum) {
                                valOfName.SetLocalIndex(index);
                            }
                        }
                    }
                }
                else if (pStr[0] == '@' && pStr[1] == '@' && pStr[2] == 0)//Number of local variables
                {
                    valOfName.SetLocalIndex(-1);
                }
                else {
                    //Functions, global variables and predefined variables
                    if (0 == strcmp(pStr, "this")) {
                        //this <=> $0
                        valOfName.SetArgIndex(0);
                    }
                    else {
                        ExpressionApi* p = m_Interpreter->FindFunctionApi(pStr);
                        //normal variable
                        AutoInterpreterValuePoolValueOperation op(*m_pInnerValuePool);
                        Value& val = op.Get();
                        if (0 != p) {
                            //The internal api name is not processed as a variable, and the value is calculated directly
                            // (equivalent to a constant). There will be one less indirect processing of calculating
                            // the variable value during runtime.
                            val.SetExpressionApi(p);
                            valOfName = val;
                        }
                        else {
                            val = valOfName;
                            if (m_Interpreter->GetValueIndex(pStr) == -1 || 0 != p) {
                                m_Interpreter->SetValue(pStr, val);
                            }
                            valOfName.SetIndex(m_Interpreter->GetValueIndex(pStr));
                        }
                    }
                }
            }
        }
        else if (valOfName.IsFunction() && 0 != valOfName.GetFunction()) {
            valOfName.GetFunction()->PrepareRuntimeObject();
        }
    }

    FunctionData::FunctionData(Interpreter& interpreter) :
        ISyntaxComponent(ISyntaxComponent::TYPE_FUNCTION, interpreter),
        m_Name(interpreter),
        m_RuntimeFunctionCall(0),
        m_RuntimeStatementBlock(0),
        m_RuntimeObjectPrepared(FALSE),
        m_Params(0),
        m_ParamNum(0),
        m_ParamSpace(0),
        m_ParamClass(PARAM_CLASS_NOTHING),
        m_LocalNum(0),
        m_LocalSpace(0),
        m_pInnerValuePool(0)
    {
        const InterpreterOptions& options = interpreter.GetOptions();
        m_MaxParamNum = options.GetMaxParamNum();
        m_MaxLocalNum = options.GetMaxLocalNum();
        m_pInnerValuePool = &interpreter.GetInnerValuePool();
    }

    FunctionData::~FunctionData()
    {
        if (NULL != m_RuntimeStatementBlock) {
            delete m_RuntimeStatementBlock;
            m_RuntimeStatementBlock = NULL;
        }
        ReleaseParams();
        ClearLocalIndexes();
    }

    void FunctionData::PrepareParams()
    {
        if (NULL == m_Params && TRUE == HaveParam()) {
            m_Params = new SyntaxComponentPtr[INIT_FUNCTION_PARAM];
            if (m_Params) {
                m_ParamSpace = INIT_FUNCTION_PARAM;
            }
        }
        else if (HaveParamOrStatement() && m_ParamNum >= m_ParamSpace) {
            int delta = HaveStatement() ? DELTA_FUNCTION_STATEMENT : DELTA_FUNCTION_PARAM;
            int newSpace = m_ParamSpace + delta;
            if (newSpace <= m_MaxParamNum) {
                SyntaxComponentPtr* pNew = new SyntaxComponentPtr[newSpace];
                if (pNew) {
                    memcpy(pNew, m_Params, m_ParamNum * sizeof(SyntaxComponentPtr));
                    memset(pNew + m_ParamNum, 0, delta * sizeof(SyntaxComponentPtr));
                    delete[] m_Params;
                    m_Params = pNew;
                    m_ParamSpace = newSpace;
                }
            }
        }
    }

    void FunctionData::ReleaseParams()
    {
        if (NULL != m_Params) {
            delete[] m_Params;
            m_Params = NULL;
        }
    }

    NullSyntax* FunctionData::GetNullSyntaxPtr()const
    {
        return GetInterpreter().GetNullSyntaxPtr();
    }

    FunctionData* FunctionData::GetNullFunctionPtr()const
    {
        return GetInterpreter().GetNullFunctionPtr();
    }

    void FunctionData::PrepareRuntimeObject()
    {
        if (NULL == m_Interpreter)
            return;
        if (m_RuntimeObjectPrepared)
            return;
        StatementApiFactory* pApiFactory = m_Interpreter->FindStatementApi(*this);
        m_RuntimeObjectPrepared = TRUE;
        if (0 != pApiFactory) {
            //Let the API generate composite runtime objects.
            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
            m_RuntimeFunctionCall.SetStatementApi(pApi);
        }
        else {
            PrepareGeneralRuntimeObject();
        }
    }

    const Value& FunctionData::GetRuntimeObject()const
    {
        return m_RuntimeFunctionCall;
    }

    void FunctionData::PrepareGeneralRuntimeObject()
    {
        //Processing function name
        m_Name.PrepareGeneralRuntimeObject();
        m_RuntimeObjectPrepared = TRUE;
        //Params
        if (HaveParam()) {
            if (0 != m_Params) {
                for (int ix = 0; ix < m_ParamNum; ++ix) {
                    if (0 != m_Params[ix]) {
                        m_Params[ix]->PrepareRuntimeObject();
                    }
                }
            }
            //Finally, a composite runtime object is generated. Unlike a statement, a function
            // call (unless it degenerates into a Value, in which case the outer function is
            // combined into a RuntimeFunction) always generates a RuntimeFunction. Semantically
            // speaking, FunctionData actually represents a StatementApi. , instead of
            // ExpressionApi
            RuntimeFunctionCall* pRuntimeFunction = m_Interpreter->AddNewRuntimeFunctionComponent();
            if (NULL != pRuntimeFunction) {
                pRuntimeFunction->Init(*this);
                m_RuntimeFunctionCall.SetStatementApi(pRuntimeFunction);
            }
            else {
                m_RuntimeFunctionCall.SetInvalid();
            }
        }
        else if (HaveStatement()) {
            //statement
            if (0 != m_Params) {
                for (int ix = 0; ix < m_ParamNum; ++ix) {
                    if (0 != m_Params[ix]) {
                        m_Params[ix]->PrepareRuntimeObject();
                    }
                }
                if (m_ParamNum > 0) {
                    m_RuntimeStatementBlock = new RuntimeStatementBlock(*m_Interpreter, *this);
                }
            }
        }
        else if (HaveExternScript()) {

        }
        else {
            m_RuntimeFunctionCall = m_Name.GetRuntimeObject();
        }
    }

    void FunctionData::PrepareLocalIndexes()
    {
        if (FALSE == m_LocalIndexes.IsInited()) {
            m_LocalIndexes.InitTable(INIT_FUNCTION_LOCAL);
            m_LocalSpace = INIT_FUNCTION_LOCAL;
        }
        else if (m_LocalNum >= m_LocalSpace) {
            int newSpace = m_LocalSpace + DELTA_FUNCTION_LOCAL;
            if (newSpace <= m_MaxLocalNum) {
                struct LocalInfo
                {
                    char name[MAX_TOKEN_NAME_SIZE];
                    int index;
                };
                LocalInfo* pInfos = new LocalInfo[m_LocalNum];
                if (pInfos) {
                    int ix = 0;
                    for (LocalIndexes::Iterator it = m_LocalIndexes.First(); FALSE == it.IsNull(); ++it) {
                        const char* pKey = it->GetKey().GetString();
                        if (pKey) {
                            tsnprintf(pInfos[ix].name, MAX_TOKEN_NAME_SIZE, "%s", pKey);
                        }
                        else {
                            pInfos[ix].name[0] = 0;
                        }
                        pInfos[ix].index = it->GetValue();
                        ++ix;
                    }
                    m_LocalIndexes.CleanUp();
                    m_LocalIndexes.InitTable(newSpace);
                    m_LocalSpace = newSpace;
                    for (int i = 0; i < ix; ++i) {
                        m_LocalIndexes.Add(pInfos[i].name, pInfos[i].index);
                    }
                    delete[] pInfos;
                }
            }
        }
    }

    void FunctionData::ClearLocalIndexes()
    {
        if (TRUE == m_LocalIndexes.IsInited()) {
            m_LocalIndexes.CleanUp();
        }
    }

    int FunctionData::AllocLocalIndex(const char* id)
    {
        int index = -1;
        if (0 != id) {
            PrepareLocalIndexes();
            if (TRUE == m_LocalIndexes.IsInited()) {
                index = m_LocalIndexes.Get(id);
                if (index < 0 && m_LocalNum >= 0 && m_LocalNum < m_LocalSpace && m_LocalNum < m_MaxLocalNum) {
                    m_LocalIndexes.Add(id, m_LocalNum);
                    index = m_LocalNum;
                    ++m_LocalNum;
                }
            }
        }
        return index;
    }

    int FunctionData::GetLocalIndex(const char* id)const
    {
        int index = -1;
        if (0 != id && TRUE == m_LocalIndexes.IsInited()) {
            index = m_LocalIndexes.Get(id);
        }
        return index;
    }

    const char* FunctionData::GetLocalName(int index)const
    {
        if (TRUE == m_LocalIndexes.IsInited()) {
            if (index >= 0 && index < m_MaxLocalNum) {
                LocalIndexes::Iterator it = m_LocalIndexes.First();
                for (int ix = 0; ix < index && !it.IsNull(); ++ix, ++it) {}
                if (!it.IsNull()) {
                    return it->GetKey().GetString();
                }
            }
        }
        return "";
    }

    int	FunctionData::CalculateStackSize()const
    {
        return m_LocalIndexes.GetNum();
    }

    StatementData::StatementData(Interpreter& interpreter) :
        ISyntaxComponent(ISyntaxComponent::TYPE_STATEMENT, interpreter),
        m_RuntimeObject(0),
        m_RuntimeObjectPrepared(FALSE),
        m_Functions(0),
        m_FunctionNum(0),
        m_FunctionSpace(0)
    {
        const InterpreterOptions& options = interpreter.GetOptions();
        m_MaxFunctionNum = options.GetMaxFunctionDimensionNum();
    }

    FunctionData*& StatementData::GetNullFunctionPtrRef()const
    {
        return GetInterpreter().GetNullFunctionPtrRef();
    }

    void StatementData::PrepareFunctions()
    {
        if (NULL == m_Functions) {
            m_Functions = new FunctionData * [INIT_STATEMENT_FUNCTION];
            if (m_Functions) {
                m_FunctionSpace = INIT_STATEMENT_FUNCTION;
            }
        }
        else if (m_FunctionNum >= m_FunctionSpace) {
            int newSpace = m_FunctionSpace + DELTA_STATEMENT_FUNCTION;
            if (newSpace <= m_MaxFunctionNum) {
                FunctionData** pNew = new FunctionData * [newSpace];
                if (pNew) {
                    memcpy(pNew, m_Functions, m_FunctionNum * sizeof(FunctionData*));
                    memset(pNew + m_FunctionNum, 0, DELTA_STATEMENT_FUNCTION * sizeof(FunctionData*));
                    delete[] m_Functions;
                    m_Functions = pNew;
                    m_FunctionSpace = newSpace;
                }
            }
        }
    }

    void StatementData::ReleaseFunctions()
    {
        if (NULL != m_Functions) {
            delete[] m_Functions;
            m_Functions = NULL;
        }
    }

    void StatementData::PrepareRuntimeObject()
    {
        if (NULL == m_Interpreter)
            return;
        if (m_RuntimeObjectPrepared)
            return;
        StatementApiFactory* pApiFactory = m_Interpreter->FindStatementApi(*this);
        m_RuntimeObjectPrepared = TRUE;
        if (0 != pApiFactory) {
            //Let the API generate composite runtime objects.
            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
            m_RuntimeObject.SetStatementApi(pApi);
            return;
        }
        else {
            //detect object literal
            if (GetFunctionNum() == 1) {
                FunctionData* pFunc = GetFunction(0);
                if (0 != pFunc && !pFunc->HaveId()) {
                    if (pFunc->HaveParam() && pFunc->GetParamClass() == FunctionData::PARAM_CLASS_BRACKET) {
                        pApiFactory = m_Interpreter->GetLiteralArrayApi();
                        if (0 != pApiFactory) {
                            //Let the API generate composite runtime objects.
                            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
                            m_RuntimeObject.SetStatementApi(pApi);
                        }
                        return;
                    }
                    else if (!pFunc->HaveParam() && pFunc->HaveStatement()) {
                        pApiFactory = m_Interpreter->GetLiteralObjectApi();
                        if (0 != pApiFactory) {
                            //Let the API generate composite runtime objects.
                            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
                            m_RuntimeObject.SetStatementApi(pApi);
                        }
                        return;
                    }
                }
            }
            //Wrong syntax, all statements should have corresponding statement API
#ifndef _GAMECLIENT_
            char errBuf[1025];
            tsnprintf(errBuf, 1024, "Error statement, line %d !\n", GetLine());
            printf(errBuf);
            m_Interpreter->AddError(errBuf);
#endif
            m_RuntimeObject.SetInvalid();
        }
    }

    const Value& StatementData::GetRuntimeObject()const
    {
        return m_RuntimeObject;
    }

    void StatementData::PrepareGeneralRuntimeObject()
    {
        if (NULL == m_Functions)
            return;
        for (int ix = 0; ix < m_FunctionNum; ++ix) {
            FunctionData* pFunction = m_Functions[ix];
            if (0 != pFunction) {
                pFunction->PrepareGeneralRuntimeObject();
            }
        }
    }

    RuntimeStatementBlock::RuntimeStatementBlock(Interpreter& interpreter, FunctionData& func) :m_Interpreter(&interpreter), m_StatementNum(0), m_Statements(NULL), m_pInnerValuePool(NULL)
    {
        m_pInnerValuePool = &interpreter.GetInnerValuePool();

        m_StatementNum = func.GetParamNum();
        if (m_StatementNum > 0) {
            m_Statements = new StatementApi * [m_StatementNum];
            for (int ix = 0; ix < m_StatementNum; ++ix) {
                ISyntaxComponent* pStatement = func.GetParam(ix);
                if (NULL != pStatement) {
                    Value val = pStatement->GetRuntimeObject();
                    if (val.IsStatementApi())
                        m_Statements[ix] = val.GetStatementApi();
                    else
                        m_Statements[ix] = NULL;
                }
                else {
                    m_Statements[ix] = NULL;
                }
            }
        }
    }
    RuntimeStatementBlock::~RuntimeStatementBlock()
    {
        if (NULL != m_Statements) {
            delete[] m_Statements;
        }
        m_StatementNum = 0;
    }

    ExecuteResultEnum RuntimeStatementBlock::Execute(Value* pRetValue)const
    {
        if (NULL == m_Interpreter)
            return EXECUTE_RESULT_NORMAL;
        for (int ix = 0; ix < m_StatementNum && m_Interpreter->IsRunFlagEnable(); ++ix) {
            StatementApi* pStatement = m_Statements[ix];
            if (NULL != pStatement) {
                AutoInterpreterValuePoolValueOperation op(*m_pInnerValuePool);
                Value& val = op.Get();
                ExecuteResultEnum ret = pStatement->Execute(&val);
                if (0 != pRetValue)
                    *pRetValue = val;
                if (EXECUTE_RESULT_RETURN == ret || EXECUTE_RESULT_BREAK == ret || EXECUTE_RESULT_CONTINUE == ret) {
                    return ret;
                }
                else if (EXECUTE_RESULT_GOTO == ret && 0 != val.GetInt()) {
                    ix += val.GetInt() - 1;
                    continue;
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    RuntimeFunctionCall::RuntimeFunctionCall(Interpreter& interpreter) :StatementApi(interpreter), m_ParamClass(0), m_ParamNum(0), m_Params(NULL), m_pInnerValuePool(NULL)
    {
        m_pInnerValuePool = &interpreter.GetInnerValuePool();
    }

    RuntimeFunctionCall::~RuntimeFunctionCall()
    {
        if (NULL != m_Params) {
            delete[] m_Params;
        }
        m_ParamNum = 0;
    }

    void RuntimeFunctionCall::Init(FunctionData& call)
    {
        m_Name = call.GetNameValue();
        if (m_Name.IsFunction()) {
            FunctionData* pCall = m_Name.GetFunction();
            if (NULL != pCall) {
                m_Name = pCall->GetRuntimeObject();
            }
        }
        m_ParamClass = call.GetParamClass();
        m_ParamNum = call.GetParamNum();
        if (m_ParamNum > 0) {
            m_Params = new Value[m_ParamNum];
            for (int ix = 0; ix < m_ParamNum; ++ix) {
                ISyntaxComponent* pParam = call.GetParam(ix);
                if (NULL != pParam) {
                    m_Params[ix] = pParam->GetRuntimeObject();
                }
            }
        }
    }

    ExecuteResultEnum RuntimeFunctionCall::Execute(Value* pRetValue)const
    {
        if (NULL == m_Interpreter)
            return EXECUTE_RESULT_NORMAL;
        if (!m_Interpreter->IsRunFlagEnable()) {
            return EXECUTE_RESULT_NORMAL;
        }
        AutoInterpreterValuePoolValuesOperation op(*m_pInnerValuePool);
        Value* params = op.Get();
        //There is a convention here that all parameters are calculated by the processing
        // function itself, and the original parameter information is passed, which
        // facilitates the implementation of assignment or out parameter characteristics.
        //Copy parameter information
        if (0 != m_Params) {
            for (int ix = 0; ix < m_ParamNum && m_Interpreter->IsRunFlagEnable(); ++ix) {
                params[ix] = m_Params[ix];
                Value& param = params[ix];
                if (param.IsStatementApi() && NULL != param.GetStatementApi()) {
                    //The parameter is the statement and its return value needs to be passed
                    // after executing the statement.
                    ExecuteResultEnum ret = param.GetStatementApi()->Execute(&param);
                    if (EXECUTE_RESULT_NORMAL != ret) {
#ifndef _GAMECLIENT_
                        char errBuf[1025];
                        tsnprintf(errBuf, 1024, "Error break/continue/return, line %d !\n", param.GetLine());
                        printf(errBuf);
                        m_Interpreter->AddError(errBuf);
#endif
                    }
                }
            }
            if (!m_Interpreter->IsRunFlagEnable()) {
                return EXECUTE_RESULT_NORMAL;
            }
        }
        if (m_Name.IsStatementApi()) {
            StatementApi* pApi = m_Name.GetStatementApi();
            if (0 != pApi) {
                AutoInterpreterValuePoolValueOperation op1(*m_pInnerValuePool);
                Value& val = op1.Get();
                ExecuteResultEnum ret = pApi->Execute(&val);
                if (EXECUTE_RESULT_NORMAL != ret) {
#ifndef _GAMECLIENT_
                    char errBuf[1025];
                    tsnprintf(errBuf, 1024, "Error break/continue/return, line %d !\n", m_Name.GetLine());
                    printf(errBuf);
                    m_Interpreter->AddError(errBuf);
#endif
                }
                ExpressionApi* pExp = val.GetExpressionApi();
                if (val.IsExpressionApi() && 0 != pExp) {
                    /*if(0!=pRetValue)
                    pRetValue->SetInvalid();*/
                    pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
                }
            }
        }
        else if (m_Name.IsExpressionApi()) {
            ExpressionApi* pExp = m_Name.GetExpressionApi();
            if (0 != pExp) {
                /*if(0!=pRetValue)
                pRetValue->SetInvalid();*/
                pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
            }
        }
        else {
            if (m_Name.IsInvalid()) {
                //Comma expression implementation (now we are the implementation of the API
                // ourselves and have the responsibility for parameter evaluation)
                ReplaceVariableWithValue(params, m_ParamNum);
                //Comma expression returns the value of the last argument
                if (0 != pRetValue)
                    *pRetValue = params[m_ParamNum - 1];
            }
            else {
                Value val = m_Name;
                //Function call of variable reference function
                ReplaceVariableWithValue(val);
                ExpressionApi* pExp = val.GetExpressionApi();
                if (val.IsExpressionApi() && 0 != pExp) {
                    pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
                }
                else {
                    //The function name refers to something other than a function. The
                    // function name is ignored, treated as a comma expression, and returns
                    // the value of the last parameter.
                    //Comma expression implementation (now we are the implementation of the
                    // API ourselves and have the responsibility for parameter evaluation)
                    ReplaceVariableWithValue(params, m_ParamNum);
                    //Comma expression returns the value of the last argument
                    if (0 != pRetValue)
                        *pRetValue = params[m_ParamNum - 1];
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void Interpreter::PrepareRuntimeObject()
    {
        if (0 == m_Program)
            return;
        if (!HasError()) {
            for (int ix = m_NextStatement; ix < m_StatementNum; ++ix) {
                ISyntaxComponent* pStatement = m_Program[ix];
                if (0 != pStatement) {
                    pStatement->PrepareRuntimeObject();
                    Value val = pStatement->GetRuntimeObject();
                    if (val.IsStatementApi()) {
                        m_RuntimeProgram[ix] = val.GetStatementApi();
                    }
                    else {
                        m_RuntimeProgram[ix] = NULL;
                    }
                }
            }
        }
    }

    ExecuteResultEnum Interpreter::Execute(Value* pRetValue)
    {
        if (0 == m_RuntimeProgram)
            return EXECUTE_RESULT_NORMAL;
        for (; m_NextStatement < m_StatementNum && IsRunFlagEnable(); ++m_NextStatement) {
            StatementApi* pStatement = m_RuntimeProgram[m_NextStatement];
            if (0 != pStatement) {
                AutoInterpreterValuePoolValueOperation op(m_InterpreterValuePool);
                Value& val = op.Get();
                ExecuteResultEnum ret = pStatement->Execute(&val);
                if (0 != pRetValue)
                    *pRetValue = val;
                if (EXECUTE_RESULT_RETURN == ret || EXECUTE_RESULT_BREAK == ret || EXECUTE_RESULT_CONTINUE == ret) {
                    m_NextStatement = m_StatementNum;
                    break;
                }
                else if (EXECUTE_RESULT_GOTO == ret && 0 != val.GetInt()) {
                    m_NextStatement += val.GetInt() - 1;
                    continue;
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    ExecuteResultEnum Interpreter::CallMember(ExpressionApi& obj, const Value& member, int isProperty, int paramClass, Value* pParams, int paramNum, Value* pRetValue)
    {
        //Object member access is completed in two steps. The first step uses the member
        // name or index as a parameter to obtain the member access object. The second step
        // is to use the member access object to operate the object members to complete
        // the actual operation.
        //The parser combines these 2 steps into a second-order function call in the
        // wrapObjectMember/wrapObjectMemberInHighOrderFunction method.
        Value param = member;
        AutoInterpreterValuePoolValueOperation op(m_InterpreterValuePool);
        Value& memberAccessor = op.Get();
        ExecuteResultEnum r = obj.Execute(FunctionData::PARAM_CLASS_PERIOD, &param, 1, &memberAccessor);
        if (memberAccessor.IsExpressionApi() && 0 != memberAccessor.GetExpressionApi()) {
            ExpressionApi& mAccessor = *(memberAccessor.GetExpressionApi());
            int mask = 0;
            if (TRUE == isProperty)
                mask = FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK;
            return mAccessor.Execute(paramClass | mask, pParams, paramNum, pRetValue);
        }
        else {
            return r;
        }
    }

    void Interpreter::RegisterStatementApi(const char* id, StatementApiFactory* p)
    {
        if (0 == p)
            return;
        StatementApiFactoryList& apiList = m_StatementApis.Get(id);
        if (apiList.Size() == 0) {
            StatementApiFactoryList newlist;
            m_StatementApis.Add(id, newlist);
        }
        StatementApiFactoryList& list = m_StatementApis.Get(id);
        int find = FALSE;
        for (int sid = list.FrontID(); list.IsValidID(sid); sid = list.NextID(sid)) {
            StatementApiFactory* pApi = list[sid];
            if (pApi == p) {
                find = TRUE;
                break;
            }
        }
        if (!find) {
            list.PushBack(p);
        }
    }

    void Interpreter::RegisterPredefinedValue(const char* id, const Value& val)
    {
        if (0 == id || 0 == m_PredefinedValue)
            return;
        int index = m_PredefinedValueIndexes.Get(id);
        if (index >= 0 && index < m_Options.GetMaxPredefinedValueNum())
            m_PredefinedValue[index] = val;
        else if (m_PredefinedValueNum >= 0 && m_PredefinedValueNum < m_Options.GetMaxPredefinedValueNum()) {
            m_PredefinedValue[m_PredefinedValueNum] = val;
            m_PredefinedValueIndexes.Add(id, m_PredefinedValueNum);
            ++m_PredefinedValueNum;
        }
    }

    StatementApiFactory* Interpreter::GetLiteralArrayApi()const
    {
        const StatementApiFactoryList& innerList = m_InnerStatementApis.Get("literalarray");
        if (innerList.Size() == 1) {
            return innerList.Front();
        }
        else {
            return 0;
        }
    }

    StatementApiFactory* Interpreter::GetLiteralObjectApi()const
    {
        const StatementApiFactoryList& innerList = m_InnerStatementApis.Get("literalobject");
        if (innerList.Size() == 1) {
            return innerList.Front();
        }
        else {
            return 0;
        }
    }

    ExpressionApi* Interpreter::FindFunctionApi(const char* id)const
    {
        if (0 == id)
            return 0;
        ExpressionApi* p = m_InnerFunctionApis.Get(id);
        if (0 != p)
            return p;
        else
            return 0;
    }

    StatementApiFactory* Interpreter::FindStatementApi(const ISyntaxComponent& statement)const
    {
        if (!statement.IsValid() || statement.GetIdType() != Value::TYPE_IDENTIFIER)
            return 0;
        const char* id = statement.GetId();
        StatementApiFactory* p = 0;
        const StatementApiFactoryList& list = m_StatementApis.Get(id);
        if (list.Size() > 0) {
            for (int sid = list.FrontID(); list.IsValidID(sid); sid = list.NextID(sid)) {
                StatementApiFactory* pApi = list[sid];
                if (0 != pApi) {
                    if (pApi->IsMatch(statement)) {
                        p = pApi;
                        break;
                    }
                }
            }
        }
        else {
            const StatementApiFactoryList& innerList = m_InnerStatementApis.Get(id);
            if (innerList.Size() > 0) {
                for (int sid = innerList.FrontID(); innerList.IsValidID(sid); sid = innerList.NextID(sid)) {
                    StatementApiFactory* pApi = innerList[sid];
                    if (0 != pApi) {
                        if (pApi->IsMatch(statement)) {
                            p = pApi;
                            break;
                        }
                    }
                }
            }
        }
        return p;
    }

    const Value& Interpreter::GetPredefinedValue(const char* id)const
    {
        if (0 == id || 0 == m_PredefinedValue)
            return Value::GetInvalidValueRef();
        int index = m_PredefinedValueIndexes.Get(id);
        if (index >= 0 && index < m_Options.GetMaxPredefinedValueNum())
            return m_PredefinedValue[index];
        else
            return Value::GetInvalidValueRef();
    }

    void Interpreter::SetValue(const char* id, const Value& val)
    {
        if (0 == id || 0 == m_ValuePool)
            return;
        int index = m_ValueIndexes.Get(id);
        if (index >= 0 && index < m_Options.GetValuePoolSize())
            m_ValuePool[index] = val;
        else if (m_ValueNum >= 0 && m_ValueNum < m_Options.GetValuePoolSize()) {
            m_ValuePool[m_ValueNum] = val;
            m_ValueIndexes.Add(id, m_ValueNum);
            ++m_ValueNum;
        }
    }

    const Value& Interpreter::GetValue(const char* id)const
    {
        if (0 == id || 0 == m_ValuePool)
            return Value::GetInvalidValueRef();
        int index = m_ValueIndexes.Get(id);
        if (index >= 0 && index < m_Options.GetValuePoolSize())
            return m_ValuePool[index];
        else
            return Value::GetInvalidValueRef();
    }

    int Interpreter::GetValueIndex(const char* id)const
    {
        if (0 == id)
            return -1;
        else {
            int index = m_ValueIndexes.Get(id);
            if (index >= 0)
                return index;
            else {
                index = m_PredefinedValueIndexes.Get(id);
                if (index >= 0)
                    return -index - 2;
                else
                    return -1;
            }
        }
    }

    const char* Interpreter::GetValueName(int indexType, int index)const
    {
        switch (indexType) {
        case Value::TYPE_INDEX:
        {
            if (index >= 0 && index < m_Options.GetValuePoolSize()) {
                ValueIndexes::Iterator it = m_ValueIndexes.First();
                for (int ix = 0; ix < index && !it.IsNull(); ++ix, ++it) {}
                if (!it.IsNull()) {
                    return it->GetKey().GetString();
                }
            }
            else {
                int _index = -index - 2;
                if (_index >= 0 && _index < m_Options.GetMaxPredefinedValueNum()) {
                    ValueIndexes::Iterator it = m_PredefinedValueIndexes.First();
                    for (int ix = 0; ix < _index && !it.IsNull(); ++ix, ++it) {}
                    if (!it.IsNull()) {
                        return it->GetKey().GetString();
                    }
                }
            }
        }
        break;
        case Value::TYPE_ARG_INDEX:
        {
            if (index < 0) {
                return "$$";
            }
            else if (index < sizeof(g_ArgNames) / sizeof(const char*)) {
                return g_ArgNames[index];
            }
        }
        break;
        case Value::TYPE_LOCAL_INDEX:
        {
            if (index < 0) {
                return "@@";
            }
            else if (index < m_Options.GetMaxLocalNum() && FALSE == m_StackInfos.Empty()) {
                const StackInfo& info = m_StackInfos.Front();
                const ISyntaxComponent* pDefinition = info.m_pDefinition;
                if (NULL != pDefinition && index < info.m_Size.GetInt()) {
                    if (pDefinition->GetSyntaxType() == ISyntaxComponent::TYPE_FUNCTION) {
                        const FunctionData* pFunc = static_cast<const FunctionData*>(pDefinition);
                        return pFunc->GetLocalName(index);
                    }
                    else if (pDefinition->GetSyntaxType() == ISyntaxComponent::TYPE_STATEMENT) {
                        const StatementData* pDef = static_cast<const StatementData*>(pDefinition);
                        FunctionData* pFunc = pDef->GetFunction(pDef->GetFunctionNum() - 1);
                        if (NULL != pFunc) {
                            return pFunc->GetLocalName(index);
                        }
                    }
                }
            }
        }
        break;
        }
        return "";
    }

    void Interpreter::SetValue(int indexType, int index, const Value& val)
    {
        switch (indexType) {
        case Value::TYPE_INDEX:
        {
            if (index >= 0) {
                m_ValuePool[index] = val;
            }
            else {
                int _index = -index - 2;
                m_PredefinedValue[_index] = val;
            }
        }
        break;
        case Value::TYPE_ARG_INDEX:
        {
            if (index >= 0) {
                const StackInfo& info = m_StackInfos.Front();
                info.m_Params[index] = val;
            }
        }
        break;
        case Value::TYPE_LOCAL_INDEX:
        {
            if (index >= 0) {
                const StackInfo& info = m_StackInfos.Front();
                int _index = info.m_Start + index;
                m_StackValuePool[_index] = val;
            }
        }
        break;
        }
    }

    const Value& Interpreter::GetValue(int indexType, int index)const
    {
        switch (indexType) {
        case Value::TYPE_INDEX:
        {
            if (index >= 0 && index < m_ValueNum && 0 != m_ValuePool) {
                return m_ValuePool[index];
            }
            else {
                int _index = -index - 2;
                return m_PredefinedValue[_index];
            }
        }
        break;
        case Value::TYPE_ARG_INDEX:
        {
            if (-1 == index) {
                const StackInfo& info = m_StackInfos.Front();
                return info.m_ParamNum;
            }
            else {
                const StackInfo& info = m_StackInfos.Front();
                return info.m_Params[index];
            }
        }
        break;
        case Value::TYPE_LOCAL_INDEX:
        {
            if (-1 == index) {
                const StackInfo& info = m_StackInfos.Front();
                return info.m_Size;
            }
            else {
                const StackInfo& info = m_StackInfos.Front();
                int _index = info.m_Start + index;
                return m_StackValuePool[_index];
            }
        }
        break;
        }
        return Value::GetInvalidValueRef();
    }

    void Interpreter::PushStackInfo(Value* pParams, int paramNum, int stackSize, const ISyntaxComponent& definition)
    {
        if (FALSE == m_StackInfos.Full()) {
            StackInfo info;
            info.m_Start = 0;
            if (FALSE == m_StackInfos.Empty()) {
                const StackInfo& top = GetTopStackInfo();
                info.m_Start = top.m_Start + top.m_Size.GetInt();
            }
            info.m_Params = pParams;
            info.m_Size.SetInt(stackSize);
            info.m_ParamNum.SetInt(paramNum);
            info.m_pDefinition = &definition;
            m_StackInfos.PushFront(info);
        }
    }

    void Interpreter::PopStackInfo()
    {
        if (FALSE == m_StackInfos.Empty()) {
            m_StackInfos.PopFront();
        }
    }

    int Interpreter::IsStackEmpty()const
    {
        return m_StackInfos.Empty();
    }

    int Interpreter::IsStackFull()const
    {
        return m_StackInfos.Full();
    }

    const Interpreter::StackInfo& Interpreter::GetTopStackInfo()const
    {
        if (FALSE == m_StackInfos.Empty()) {
            return m_StackInfos.Front();
        }
        else {
            return StackInfo::GetInvalidStackInfoRef();
        }
    }

    int Interpreter::GetStackValueNum()const
    {
        if (FALSE == m_StackInfos.Empty()) {
            const StackInfo& info = m_StackInfos.Front();
            return info.m_Start + info.m_Size.GetInt();
        }
        else {
            return m_Options.GetStackValuePoolSize();
        }
    }

    int Interpreter::PushFunctionDefinition(FunctionData* pFunction)
    {
        if (FALSE == m_FunctionDefinitionStack.Full()) {
            m_FunctionDefinitionStack.PushFront(pFunction);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    int Interpreter::PopFunctionDefinition()
    {
        if (FALSE == m_FunctionDefinitionStack.Empty()) {
            m_FunctionDefinitionStack.PopFront();
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    FunctionData* Interpreter::GetCurFunctionDefinition()const
    {
        if (FALSE == m_FunctionDefinitionStack.Empty()) {
            return m_FunctionDefinitionStack.Front();
        }
        else {
            return 0;
        }
    }

    void Interpreter::AddStatement(ISyntaxComponent* p)
    {
        if (0 == p || 0 == m_Program)
            return;
        if (m_StatementNum < 0 || m_StatementNum >= m_Options.GetMaxProgramSize())
            return;
        m_Program[m_StatementNum] = p;
        ++m_StatementNum;
    }

    ValueData* Interpreter::AddNewValueComponent()
    {
        ValueData* p = new ValueData(*this);
        AddSyntaxComponent(p);
        return p;
    }

    FunctionData* Interpreter::AddNewFunctionComponent()
    {
        FunctionData* p = new FunctionData(*this);
        AddSyntaxComponent(p);
        return p;
    }

    StatementData* Interpreter::AddNewStatementComponent()
    {
        StatementData* p = new StatementData(*this);
        AddSyntaxComponent(p);
        return p;
    }

    RuntimeFunctionCall* Interpreter::AddNewRuntimeFunctionComponent()
    {
        RuntimeFunctionCall* p = new RuntimeFunctionCall(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Closure* Interpreter::AddNewClosureComponent()
    {
        Closure* p = new Closure(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Object* Interpreter::AddNewObjectComponent()
    {
        Object* p = new Object(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Struct* Interpreter::AddNewStructComponent()
    {
        Struct* p = new Struct(*this);
        AddRuntimeComponent(p);
        return p;
    }

    void Interpreter::AddSyntaxComponent(ISyntaxComponent* p)
    {
        if (m_SyntaxComponentNum >= m_Options.GetSyntaxComponentPoolSize() || 0 == m_SyntaxComponentPool)
            return;
        m_SyntaxComponentPool[m_SyntaxComponentNum] = p;
        ++m_SyntaxComponentNum;
    }

    void Interpreter::AddRuntimeComponent(RuntimeComponent* p)
    {
        if (m_RuntimeComponentNum >= m_Options.GetExpressionPoolSize() || 0 == m_RuntimeComponentPool)
            return;
        m_RuntimeComponentPool[m_RuntimeComponentNum] = p;
        ++m_RuntimeComponentNum;
    }

    char* Interpreter::AllocString(int len)
    {
        if (m_UnusedStringPtr + len - m_StringBuffer >= m_Options.GetStringBufferSize()) {
            return 0;
        }
        char* p = m_UnusedStringPtr;
        if (0 != p) {
            m_UnusedStringPtr[len] = 0;
            m_UnusedStringPtr += len + 1;
        }
        return p;
    }

    char* Interpreter::AllocString(const char* src)
    {
        if (0 == src)
            return 0;
        int len = (int)strlen(src);
        char* p = AllocString(len);
        if (0 != p) {
            strcpy(p, src);
        }
        return p;
    }

    Interpreter::StackInfo& Interpreter::StackInfo::GetInvalidStackInfoRef()
    {
        static StackInfo s_StackInfo;
        return s_StackInfo;
    }

    Interpreter::Interpreter(const InterpreterOptions& options) :m_Options(options), m_IsDebugInfoEnable(FALSE),
        m_pRunFlag(NULL),
        m_StringBuffer(NULL),
        m_UnusedStringPtr(NULL),
        m_SyntaxComponentPool(NULL),
        m_RuntimeComponentPool(NULL),
        m_Program(NULL),
        m_RuntimeProgram(NULL),
        m_PredefinedValue(NULL),
        m_ValuePool(NULL),
        m_StackValuePool(NULL),
        m_InterpreterValuePool(MAX_FUNCTION_LEVEL* MAX_STACK_LEVEL, MAX_FUNCTION_LEVEL* MAX_STACK_LEVEL),
        m_pNullSyntax(NULL),
        m_pNullFunction(NULL),
        m_ppNullFunction(NULL)
    {
        m_InnerFunctionApis.InitTable(m_Options.GetMaxInnerFunctionApiNum());
        m_InnerStatementApis.InitTable(m_Options.GetMaxInnerStatementApiNum());
        m_StatementApis.InitTable(m_Options.GetMaxStatementApiNum());

        m_ValueIndexes.InitTable(m_Options.GetValuePoolSize());

        m_PredefinedValueIndexes.InitTable(m_Options.GetExpressionPoolSize());
        m_PredefinedValue = new Value[m_Options.GetExpressionPoolSize()];
        m_PredefinedValueNum = 0;

        m_pNullSyntax = new NullSyntax(*this);
        m_pNullFunction = new FunctionData(*this);
        m_ppNullFunction = new FunctionData * [1];
        *m_ppNullFunction = m_pNullFunction;

        Init();
        InitInnerApis();
    }

    Interpreter::~Interpreter()
    {
        ReleaseInnerApis();
        Release();

        delete m_pNullSyntax;
        m_pNullSyntax = NULL;
        delete m_pNullFunction;
        m_pNullFunction = NULL;
        delete[] m_ppNullFunction;
        m_ppNullFunction = NULL;

        if (m_PredefinedValue) {
            delete[] m_PredefinedValue;
            m_PredefinedValue = NULL;
        }
    }

    void Interpreter::Reset()
    {
        Release();
        Init();
        m_InterpreterValuePool.Reset();
    }

    void Interpreter::Init()
    {
        m_StringBuffer = new char[m_Options.GetStringBufferSize()];
        m_UnusedStringPtr = m_StringBuffer;
        m_SyntaxComponentPool = new SyntaxComponentPtr[m_Options.GetSyntaxComponentPoolSize()];
        m_SyntaxComponentNum = 0;
        m_RuntimeComponentPool = new RuntimeComponentPtr[m_Options.GetExpressionPoolSize()];
        m_RuntimeComponentNum = 0;
        m_Program = new SyntaxComponentPtr[m_Options.GetMaxProgramSize()];
        m_StatementNum = 0;
        m_RuntimeProgram = new StatementApiPtr[m_Options.GetMaxProgramSize()];
        m_ValuePool = new Value[m_Options.GetValuePoolSize()];
        m_ValueNum = 0;
        m_StackValuePool = new Value[m_Options.GetStackValuePoolSize()];

        m_NextStatement = 0;

        m_ErrorAndStringBuffer.Reset(m_StringBuffer, m_UnusedStringPtr, m_Options.GetStringBufferSize());
    }

    void Interpreter::Release()
    {
        m_ValueIndexes.CleanUp();
        m_StackInfos.Clear();
        m_FunctionDefinitionStack.Clear();

        if (0 != m_StringBuffer) {
            delete[] m_StringBuffer;
            m_StringBuffer = 0;
            m_UnusedStringPtr = 0;
        }
        if (0 != m_SyntaxComponentPool) {
            for (int i = 0; i < m_SyntaxComponentNum; ++i) {
                if (0 != m_SyntaxComponentPool[i])
                    delete m_SyntaxComponentPool[i];
            }
            delete[] m_SyntaxComponentPool;
            m_SyntaxComponentNum = 0;
        }
        if (0 != m_RuntimeComponentPool) {
            for (int i = 0; i < m_RuntimeComponentNum; ++i) {
                if (0 != m_RuntimeComponentPool[i])
                    delete m_RuntimeComponentPool[i];
            }
            delete[] m_RuntimeComponentPool;
            m_RuntimeComponentNum = 0;
        }
        if (0 != m_Program) {
            delete[] m_Program;
            m_StatementNum = 0;
        }
        if (0 != m_RuntimeProgram) {
            delete[] m_RuntimeProgram;
        }
        if (0 != m_ValuePool) {
            delete[] m_ValuePool;
            m_ValueNum = 0;
        }
        if (0 != m_StackValuePool) {
            delete[] m_StackValuePool;
            m_StackValuePool = 0;
        }
    }

    void ErrorAndStringBuffer::ClearErrorInfo()
    {
        m_HasError = FALSE;
        m_ErrorNum = 0;
        memset(m_ErrorInfo, 0, sizeof(m_ErrorInfo));
    }

    void ErrorAndStringBuffer::AddError(const char* error)
    {
        char* p = NewErrorInfo();
        if (p)
            tsnprintf(p, MAX_ERROR_INFO_CAPACITY, "%s", error);
    }
}