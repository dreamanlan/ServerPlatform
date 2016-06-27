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
            } else if (p.IsVariableName()) {
                p = m_Interpreter->GetValue(p.GetString());
            }
        }
    }

    void RuntimeComponent::SetVariableValue(const Value& p, const Value& val)const
    {
        if (NULL != m_Interpreter) {
            if (p.IsIndex() || p.IsArgIndex() || p.IsLocalIndex()) {
                m_Interpreter->SetValue(p.GetType(), p.GetInt(), val);
            } else if (p.IsVariableName()) {
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
                } else {
                    return t;
                }
            } else if (p.IsVariableName())
                return m_Interpreter->GetValue(p.GetString());
            else
                return p;
        } else {
            return Value::GetInvalidValueRef();
        }
    }
    //------------------------------------------------------------------------------------------------------
    namespace InnerApi
    {
        class ExprApi : public ExpressionApi
        {
        public:
            virtual ExecuteResultEnum Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
            {
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
                        if (pParams[0].IsIndex() && (pParams[1].IsArgIndex() || pParams[1].IsLocalIndex())) {
                            SetVariableValue(pParams[0], val);
                        } else {
                            SetVariableValue(pParams[0], pParams[1]);
                        }
                        if (NULL != pRetValue) {
                            *pRetValue = val;
                        }
                    } else {
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
                        } else {
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
                                } else {
                                    int len = (int)strlen(pStr1) + (int)strlen(pStr2);
                                    val.AllocString(len);
                                    tsnprintf(val.GetString(), len + 1, "%s%s", pStr1, pStr2);
                                }
                            } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                                val.SetFloat(val.ToFloat() + pParams[ix].ToFloat());
                            } else if (val.IsInt() && pParams[ix].IsInt()) {
                                val.SetInt(val.ToInt() + pParams[ix].ToInt());
                            } else {
                                val.SetInvalid();
                                break;
                            }
                        }
                    } else {
                        if (val.IsString() || val.IsFloat() || val.IsInt()) {
                        } else {
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
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    if (num > 1) {
                        for (int ix = 1; ix < num; ++ix) {
                            if (val.IsFloat() || pParams[ix].IsFloat()) {
                                val.SetFloat(val.ToFloat() - pParams[ix].ToFloat());
                            } else if (val.IsInt() && pParams[ix].IsInt()) {
                                val.SetInt(val.ToInt() - pParams[ix].ToInt());
                            } else {
                                val.SetInvalid();
                                break;
                            }
                        }
                    } else {
                        if (val.IsFloat()) {
                            val.SetFloat(-val.GetFloat());
                        } else if (val.IsInt()) {
                            val.SetInt(-val.GetInt());
                        } else {
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsFloat() || pParams[ix].IsFloat()) {
                            val.SetFloat(val.ToFloat()*pParams[ix].ToFloat());
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt()*pParams[ix].ToInt());
                        } else {
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsFloat() || pParams[ix].IsFloat()) {
                            val.SetFloat(val.ToFloat() / pParams[ix].ToFloat());
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt() / pParams[ix].ToInt());
                        } else {
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    Value val = pParams[0];
                    for (int ix = 1; ix < num; ++ix) {
                        if (val.IsInt() && pParams[ix].IsInt()) {
                            val.SetInt(val.ToInt() % pParams[ix].ToInt());
                        } else {
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta >= -0.000001f && delta <= 0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() == pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次不等，后面的就不用判断了
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta<-0.000001f || delta>0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() != pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次相等，后面的就不用判断了
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta < -0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() < pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次不小于，后面的就不用判断了
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta > 0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() > pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次不大于，后面的就不用判断了
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta <= 0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() <= pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次不小于等于，后面的就不用判断了
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
                        } else if (val.IsFloat() || pParams[ix].IsFloat()) {
                            float delta = val.ToFloat() - pParams[ix].ToFloat();
                            pRetValue->SetInt((delta >= -0.000001f) ? 1 : 0);
                        } else if (val.IsInt() && pParams[ix].IsInt()) {
                            pRetValue->SetInt((val.ToInt() >= pParams[ix].ToInt()) ? 1 : 0);
                        } else {
                            pRetValue->SetInt(0);
                        }
                        //只要有一次不大于等于，后面的就不用判断了
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetInt(1);
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (pParams[ix].GetInt() == 0) {
                                //有一个为0，后面就不用再计算了
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    pRetValue->SetInt(0);
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (pParams[ix].GetInt() == 1) {
                                //有一个为1，后面就不用再计算了
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
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                for (int ix = 0; ix < num; ++ix) {
                    int writeback = FALSE;
                    Value val = pParams[ix];
                    if (val.IsVariableName() || val.IsIndex() || val.IsArgIndex() || val.IsLocalIndex()) {
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
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                for (int ix = 0; ix < num; ++ix) {
                    int writeback = FALSE;
                    Value val = pParams[ix];
                    if (val.IsVariableName() || val.IsIndex() || val.IsArgIndex() || val.IsLocalIndex()) {
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    int val = 0xffffffff;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (val == 0) {
                                //运算中间结果为0，后面就不用再计算了
                                break;
                            } else {
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
                if (num < 2 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != pRetValue) {
                    int val = 0;
                    for (int ix = 0; ix < num; ++ix) {
                        if (pParams[ix].IsInt()) {
                            if (val == (int)0xffffffff) {
                                //运算中间结果为0xffffffff，后面就不用再计算了
                                break;
                            } else {
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
                if (1 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    const char* p = pParams[0].GetString();
                    if (0 != p) {
                        int len = (int)strlen(p);
                        pRetValue->SetInt(len);
                    } else {
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
                    } else {
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
                        } else {
                            pRetValue->SetWeakRefString("");
                        }
                    } else {
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
                    } else {
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
                    } else {
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
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int ix = (int)strcspn(pSrc, pDest);
                        pRetValue->SetInt(ix);
                    } else {
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
                if (2 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString() && pParams[1].IsString()) {
                    const char* pSrc = pParams[0].GetString();
                    const char* pDest = pParams[1].GetString();
                    if (0 != pSrc && 0 != pDest && 0 != pDest[0]) {
                        int ix = (int)strspn(pSrc, pDest);
                        pRetValue->SetInt(ix);
                    } else {
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
                        } else {
                            pRetValue->SetWeakRefString("");
                        }
                    } else {
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
                        } else {
                            pRetValue->SetWeakRefString("");
                        }
                    } else {
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
                                } else if (pParams[2].IsString()) {
                                    const char* p = pParams[2].GetString();
                                    if (0 != p && 0 != p[0]) {
                                        pSrc[index] = p[0];
                                    }
                                }
                            }
                            pRetValue->SetInt(c);
                        } else {
                            pRetValue->SetInt(0);
                        }
                    } else {
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
                                    } else if (pParams[ix].IsString()) {
                                        const char* pN = pParams[ix].GetString();
                                        if (0 != pN && 0 != pN[0]) {
                                            p[ix] = pN[0];
                                        }
                                    } else {
                                        p[ix] = ' ';
                                    }
                                }
                            } else {
                                pRetValue->SetWeakRefString("");
                            }
                        } else {
                            pRetValue->SetWeakRefString("");
                        }
                    } else {
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
                if (3 != num || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && 0 != pRetValue && pParams[0].IsString()) {
                    char* pSrc = pParams[0].GetString();
                    char s = 0;
                    if (pParams[1].IsInt()) {
                        s = (char)pParams[1].GetInt();
                    } else if (pParams[1].IsString()) {
                        const char* pTmp = pParams[1].GetString();
                        if (pTmp && pTmp[0]) {
                            s = pTmp[0];
                        }
                    }
                    char d = 0;
                    if (pParams[2].IsInt()) {
                        d = (char)pParams[2].GetInt();
                    } else if (pParams[2].IsString()) {
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
                    } else {
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
                if (0 != m_Interpreter && 0 != pRetValue) {
                    Object* pObject = m_Interpreter->AddNewObjectComponent();
                    if (0 != pObject) {
                        pRetValue->SetExpression(pObject);
                    } else {
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
                if (num < 1 || 0 == pParams)
                    return EXECUTE_RESULT_NORMAL;
                ReplaceVariableWithValue(pParams, num);
                if (0 != m_Interpreter && pParams[0].IsExpression() && 0 != pRetValue) {
                    ExpressionApi* p = pParams[0].GetExpression();
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
                            pRetValue->SetExpression(pObject);
                        } else {
                            pRetValue->SetInt(0);
                        }
                    } else {
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
                if (0 != m_Interpreter && 0 != pRetValue && 0 != pParams) {
                    ReplaceVariableWithValue(pParams, num);
                    if (1 == num && pParams[0].IsInt()) {
                        *pRetValue = m_Interpreter->GetValue(Value::TYPE_ARG_INDEX, pParams[0].GetInt());
                    } else if (2 == num && pParams[0].IsInt()) {
                        m_Interpreter->SetValue(Value::TYPE_ARG_INDEX, pParams[0].GetInt(), pParams[1]);
                        *pRetValue = pParams[1];
                    } else {
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
        AutoFunctionDefinitionStackOperation(Interpreter& interpreter, Function* pFunction) :m_Interpreter(&interpreter), m_Function(pFunction), m_NeedPop(FALSE)
        {
            if (NULL != m_Function && NULL != m_Interpreter) {
                m_NeedPop = m_Interpreter->PushFunctionDefinition(m_Function);
            }
        }
        ~AutoFunctionDefinitionStackOperation(void)
        {
            if (NULL != m_Function && NULL != m_Interpreter && TRUE == m_NeedPop) {
                m_Interpreter->PopFunctionDefinition();
            }
        }
    private:
        Interpreter*    m_Interpreter;
        Function*       m_Function;
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
                if (m_Exp1.IsStatement() && m_Exp1.GetStatement()) {
                    m_Exp1.GetStatement()->Execute(NULL);
                }
                for (; m_Interpreter->IsRunFlagEnable();) {
                    if (!m_Exp2.IsInvalid()) {
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (m_Exp2.IsStatement() && m_Exp2.GetStatement()) {
                            m_Exp2.GetStatement()->Execute(&val);
                        } else {
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
                    if (m_Exp3.IsStatement() && m_Exp3.GetStatement()) {
                        m_Exp3.GetStatement()->Execute(0);
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
            virtual ~ForStatement(void)
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
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc || pFunc->GetParamNum() != 3)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                statement.PrepareRuntimeObjectWithFunctions();
                Function* pFunc = statement.GetFunction(0);
                if (NULL != pFunc && pFunc->GetParamNum() == 3) {
                    ForStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<ForStatement>();
                    if (NULL != pApi) {
                        Statement* p1 = pFunc->GetParam(0);
                        Statement* p2 = pFunc->GetParam(1);
                        Statement* p3 = pFunc->GetParam(2);
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
                return NULL;
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
                if (m_Exp.IsStatement() && m_Exp.GetStatement()) {
                    m_Exp.GetStatement()->Execute(&val);
                } else {
                    val = m_Exp;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (NULL != m_pIf)
                        return m_pIf->Execute(pRetValue);
                } else {
                    int num = m_ElseIfNum;
                    for (int ix = 0; ix < num; ++ix) {
                        AutoInterpreterValuePoolValueOperation op2(m_Interpreter->GetInnerValuePool());
                        Value& val2 = op2.Get();
                        if (m_pElseIfExp[ix].IsStatement() && NULL != m_pElseIfExp[ix].GetStatement()) {
                            m_pElseIfExp[ix].GetStatement()->Execute(&val2);
                        } else {
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
            virtual ~IfElseStatement(void)
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
            virtual int IsMatch(const Statement& statement)const
            {
                Function* pFunc0 = statement.GetFunction(0);
                if (0 == pFunc0 || pFunc0->GetParamNum() != 1)
                    return FALSE;
                if (statement.GetFunctionNum() == 1)
                    return TRUE;
                int num = statement.GetFunctionNum();
                for (int ix = 1; ix < num; ++ix) {
                    Function* pFunc1 = statement.GetFunction(ix);
                    if (0 == pFunc1)
                        return FALSE;
                    const Value& funcName1 = pFunc1->GetName();
                    if (funcName1.IsVariableName()) {
                        const char* pName = funcName1.GetString();
                        if (0 != pName) {
                            if (ix < num - 1 && 0 != strcmp("elseif", pName))
                                return FALSE;
                            if (ix == num - 1 && 0 != strcmp("elseif", pName) && 0 != strcmp("else", pName))
                                return FALSE;
                            if (0 == strcmp("elseif", pName) && pFunc1->GetParamNum() != 1)
                                return FALSE;
                        } else {
                            return FALSE;
                        }
                    } else {
                        return FALSE;
                    }
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                IfElseStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<IfElseStatement>();
                if (NULL != pApi) {
                    Value name;
                    int num = statement.GetFunctionNum();
                    if (num > 1) {
                        Function* pLastFunc = statement.GetFunction(num - 1);
                        if (NULL != pLastFunc) {
                            name = pLastFunc->GetName();
                        }
                    }
                    statement.PrepareRuntimeObjectWithFunctions();
                    Function* pFunc = statement.GetFunction(0);
                    if (NULL != pFunc && pFunc->GetParamNum() == 1) {
                        Statement* p = pFunc->GetParam(0);
                        if (NULL != p)
                            pApi->m_Exp = p->GetRuntimeObject();
                        pApi->m_pIf = pFunc->GetRuntimeFunctionBody();
                    }
                    if (num > 1) {
                        Function* pLastFunc = statement.GetFunction(num - 1);
                        if (NULL != pLastFunc) {
                            if (name.IsVariableName() && NULL != name.GetString()) {
                                const char* pName = name.GetString();
                                if (0 == strcmp(pName, "else")) {
                                    pApi->m_pElse = pLastFunc->GetRuntimeFunctionBody();
                                    pApi->m_ElseIfNum = num - 2;
                                } else {
                                    pApi->m_ElseIfNum = num - 1;
                                }
                                if (pApi->m_ElseIfNum > 0) {
                                    pApi->m_pElseIfExp = new Value[pApi->m_ElseIfNum];
                                    pApi->m_pElseIf = new RuntimeStatementBlock*[pApi->m_ElseIfNum];
                                    for (int ix = 0; ix < pApi->m_ElseIfNum; ++ix) {
                                        Function* pElseIfFunc = statement.GetFunction(ix + 1);
                                        if (NULL != pElseIfFunc && NULL != pElseIfFunc->GetParam(0)) {
                                            Statement* pExpStatement = pElseIfFunc->GetParam(0);
                                            pApi->m_pElseIfExp[ix] = pExpStatement->GetRuntimeObject();
                                            pApi->m_pElseIf[ix] = pElseIfFunc->GetRuntimeFunctionBody();
                                        } else {
                                            pApi->m_pElseIf[ix] = NULL;
                                        }
                                    }
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
                if (m_Exp.IsStatement() && m_Exp.GetStatement()) {
                    m_Exp.GetStatement()->Execute(&val);
                } else {
                    val = m_Exp;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (m_True.IsStatement() && m_True.GetStatement()) {
                        m_True.GetStatement()->Execute(pRetValue);
                    } else {
                        if (0 != pRetValue) {
                            *pRetValue = m_True;
                            ReplaceVariableWithValue(*pRetValue);
                        }
                    }
                } else {
                    if (m_False.IsStatement() && m_False.GetStatement()) {
                        m_False.GetStatement()->Execute(pRetValue);
                    } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                Function* pFunc0 = statement.GetFunction(0);
                if (0 == pFunc0 || pFunc0->GetParamNum() != 1)
                    return FALSE;
                if (statement.GetFunctionNum() == 1)
                    return TRUE;
                if (statement.GetFunctionNum() != 2)
                    return FALSE;
                Function* pFunc1 = statement.GetFunction(1);
                if (0 == pFunc1)
                    return FALSE;
                const Value& funcName1 = pFunc1->GetName();
                if (funcName1.IsVariableName()) {
                    if (0 == funcName1.GetString() || 0 != strcmp(funcName1.GetString(), ":"))
                        return FALSE;
                } else {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                CondExpStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<CondExpStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    Function* pFunc = statement.GetFunction(0);
                    if (NULL != pFunc && pFunc->GetParamNum() == 1) {
                        Statement* p = pFunc->GetParam(0);
                        if (NULL != p)
                            pApi->m_Exp = p->GetRuntimeObject();
                        Statement* pTrue = pFunc->GetStatement(0);
                        if (NULL != pTrue)
                            pApi->m_True = pTrue->GetRuntimeObject();
                    }
                    Function* pFunc2 = statement.GetFunction(1);
                    if (NULL != pFunc2) {
                        Statement* pFalse = pFunc2->GetStatement(0);
                        if (NULL != pFalse)
                            pApi->m_False = pFalse->GetRuntimeObject();
                    }
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
                if (m_If.IsStatement() && NULL != m_If.GetStatement())
                    m_If.GetStatement()->Execute(&val);
                else {
                    val = m_If;
                    ReplaceVariableWithValue(val);
                }
                if (0 != val.GetInt()) {
                    if (m_Goto.IsStatement() && NULL != m_Goto.GetStatement()) {
                        m_Goto.GetStatement()->Execute(pRetValue);
                    }
                    return EXECUTE_RESULT_GOTO;
                } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 2)
                    return FALSE;
                Function* pFunc0 = statement.GetFunction(0);
                if (0 == pFunc0 || pFunc0->GetParamNum() != 1)
                    return FALSE;
                Function* pFunc1 = statement.GetFunction(1);
                if (0 == pFunc1 || pFunc1->GetParamNum() != 1)
                    return FALSE;
                const Value& funcName1 = pFunc1->GetName();
                if (funcName1.IsVariableName()) {
                    if (0 == funcName1.GetString() || 0 != strcmp(funcName1.GetString(), "goto"))
                        return FALSE;
                } else {
                    return FALSE;
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                IfGotoStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<IfGotoStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    Function* pFunc0 = statement.GetFunction(0);
                    Function* pFunc1 = statement.GetFunction(1);
                    if (NULL != pFunc0 && NULL != pFunc0->GetParam(0) && NULL != pFunc1 && NULL != pFunc1->GetParam(0)) {
                        Statement* p0 = pFunc0->GetParam(0);
                        Statement* p1 = pFunc1->GetParam(0);
                        pApi->m_If = p0->GetRuntimeObject();
                        pApi->m_Goto = p1->GetRuntimeObject();
                    }
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
                if (m_Goto.IsStatement() && NULL != m_Goto.GetStatement()) {
                    m_Goto.GetStatement()->Execute(pRetValue);
                } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc || pFunc->GetParamNum() != 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                GotoStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<GotoStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    Function* pFunc = statement.GetFunction(0);
                    if (NULL != pFunc && NULL != pFunc->GetParam(0)) {
                        Statement* p = pFunc->GetParam(0);
                        pApi->m_Goto = p->GetRuntimeObject();
                    }
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
                if (m_Return.IsStatement() && NULL != m_Return.GetStatement()) {
                    m_Return.GetStatement()->Execute(pRetValue);
                } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                int fnum = statement.GetFunctionNum();
                if (fnum != 1 && fnum != 2)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc || pFunc->GetParamNum() > 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                ReturnStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<ReturnStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    Function* pFunc1 = statement.GetFunction(0);
                    Function* pFunc2 = statement.GetFunction(1);
                    if (NULL != pFunc1) {
                        if (pFunc1->GetParamNum() == 1 && pFunc1->HaveParam() && NULL != pFunc1->GetParam(0)) {
                            Statement* pStatement = pFunc1->GetParam(0);
                            pApi->m_Return = pStatement->GetRuntimeObject();
                        } else if (NULL != pFunc2) {
                            pApi->m_Return = pFunc2->GetRuntimeFunctionHead();
                        }
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
                return EXECUTE_RESULT_BREAK;
            }
        public:
            explicit BreakStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        };
        class BreakStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc || pFunc->GetParamNum() > 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                if (NULL == m_pApi) {
                    m_pApi = statement.GetInterpreter().AddNewStatementApiComponent<BreakStatement>();
                }
                return m_pApi;
            }
        public:
            BreakStatementFactory(void) :m_pApi(NULL) {}
            virtual ~BreakStatementFactory(void)
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
                return EXECUTE_RESULT_CONTINUE;
            }
        public:
            explicit ContinueStatement(Interpreter& interpreter) :StatementApi(interpreter) {}
        };
        class ContinueStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc || pFunc->GetParamNum() > 1)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                if (NULL == m_pApi) {
                    m_pApi = statement.GetInterpreter().AddNewStatementApiComponent<ContinueStatement>();
                }
                return m_pApi;
            }
        public:
            ContinueStatementFactory(void) :m_pApi(NULL) {}
            virtual ~ContinueStatementFactory(void)
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
                //函数定义生成一个Closure实例。
                Statement& statement = *m_pDefine;
                int num = statement.GetFunctionNum();
                if (0 == m_Interpreter || 1 != num && 2 != num)
                    return EXECUTE_RESULT_NORMAL;
                Function* func0 = statement.GetFunction(0);
                if (0 == func0 || 1 < func0->GetParamNum())
                    return EXECUTE_RESULT_NORMAL;
                Closure* pClosure = m_Interpreter->AddNewClosureComponent();
                if (0 != pClosure) {
                    pClosure->SetDefinitionRef(m_pArguments, m_ArgumentNum, statement);
                    if (func0->HaveParam()) {
                        Statement* pName = func0->GetParam(0);
                        if (NULL != pName) {
                            Value nameVal = pName->GetRuntimeObject();
                            AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                            Value& val = op.Get();
                            if (nameVal.IsStatement() && NULL != nameVal.GetStatement()) {
                                nameVal.GetStatement()->Execute(&val);
                            } else {
                                val = nameVal;
                            }
                            SetVariableValue(val, Value(pClosure));
                        }
                    }
                    if (NULL != pRetValue) {
                        pRetValue->SetExpression(pClosure);
                    }
                }

                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit FunctionStatement(Interpreter& interpreter) :StatementApi(interpreter), m_pArguments(NULL), m_ArgumentNum(0), m_pDefine(NULL) {}
            virtual ~FunctionStatement(void)
            {
                if (0 != m_pArguments) {
                    delete[] m_pArguments;
                    m_pArguments = 0;
                }
            }
        private:
            Value* m_pArguments;
            int m_ArgumentNum;
            Statement* m_pDefine;
        };
        class FunctionStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const Statement& statement)const
            {
                //函数定义语法：function(name)args(@a,@b,@c){};其中name与args(@a,@b,@c)可选
                int num = statement.GetFunctionNum();
                if (1 != num && 2 != num)
                    return FALSE;
                Function* pFunc0 = statement.GetFunction(0);
                if (0 == pFunc0 || 1 < pFunc0->GetParamNum())
                    return FALSE;
                if (1 == num) {
                    if (FALSE == pFunc0->HaveStatement()) {
                        return FALSE;
                    }
                } else {
                    Function* pFunc1 = statement.GetFunction(1);
                    if (0 != pFunc1) {
                        if (TRUE == pFunc0->HaveStatement() || FALSE == pFunc1->HaveStatement())
                            return FALSE;
                        const Value& funcName1 = pFunc1->GetName();
                        if (funcName1.IsVariableName()) {
                            if (0 == funcName1.GetString() || 0 != strcmp(funcName1.GetString(), "args"))
                                return FALSE;
                        } else {
                            return FALSE;
                        }
                    }
                }
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                FunctionStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<FunctionStatement>();
                if (NULL != pApi) {
                    Function* pFunc = 0;
                    int num = statement.GetFunctionNum();
                    if (num > 0) {
                        pFunc = statement.GetFunction(num - 1);
                    }
                    AutoFunctionDefinitionStackOperation autoDefStack(pFunc->GetInterpreter(), pFunc);
                    statement.PrepareRuntimeObjectWithFunctions();
                    pApi->m_pDefine = &statement;

                    if (num > 1 && 0 != pFunc && pFunc->GetParamNum() > 0) {
                        pApi->m_ArgumentNum = pFunc->GetParamNum();
                        pApi->m_pArguments = new Value[pFunc->GetParamNum()];
                        for (int i = 0; i < pFunc->GetParamNum(); ++i) {
                            Statement* pStatement = pFunc->GetParam(i);
                            if (0 != pStatement && pStatement->GetFunctionNum() > 0) {
                                Function* pFunc = pStatement->GetFunction(0);
                                if (0 != pFunc) {
                                    pApi->m_pArguments[i] = pFunc->GetName();
                                }
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
                    pRetValue->SetExpression(pObject);
                    for (int i = 0; i < m_Count; ++i) {
                        Value value = m_pValues[i];
                        AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                        Value& val = op.Get();
                        if (value.IsStatement() && NULL != value.GetStatement()) {
                            value.GetStatement()->Execute(&val);
                        } else {
                            val = value;
                            ReplaceVariableWithValue(val);
                        }
                        AutoInterpreterValuePoolValueOperation ret(m_Interpreter->GetInnerValuePool());
                        Value& retVal = ret.Get();
                        Value key(i);
                        m_Interpreter->CallMember(*pObject, key, TRUE, Function::PARAM_CLASS_OPERATOR, &val, 1, &retVal);
                    }
                } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc)
                    return FALSE;
                if (!pFunc->HaveParam() || pFunc->HaveStatement())
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                LiteralArrayStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LiteralArrayStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    if (statement.GetFunctionNum() == 1) {
                        Function* pFunc = statement.GetFunction(0);
                        if (0 != pFunc) {
                            if (pFunc->HaveParam() && !pFunc->HaveStatement()) {
                                pApi->m_Count = pFunc->GetParamNum();
                                pApi->m_pValues = new Value[pApi->m_Count];
                                if (0 != pApi->m_pValues) {
                                    for (int i = 0; i < pFunc->GetParamNum(); ++i) {
                                        Statement* p = pFunc->GetParam(i);
                                        if (0 == p)
                                            continue;
                                        pApi->m_pValues[i] = p->GetRuntimeObject();
                                    }
                                }
                            }
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
                    pRetValue->SetExpression(pObject);
                    for (int i = 0; i < m_Count; ++i) {
                        Value key = m_pKeys[i];
                        Value value = m_pValues[i];
                        if (!key.IsInvalid()) {
                            AutoInterpreterValuePoolValueOperation op1(m_Interpreter->GetInnerValuePool());
                            AutoInterpreterValuePoolValueOperation op2(m_Interpreter->GetInnerValuePool());
                            Value& val1 = op1.Get();
                            Value& val2 = op2.Get();
                            if (key.IsStatement() && NULL != key.GetStatement()) {
                                key.GetStatement()->Execute(&val1);
                            } else {
                                val1 = key;
                                ReplaceVariableWithValue(val1);
                            }
                            if (value.IsStatement() && NULL != value.GetStatement()) {
                                value.GetStatement()->Execute(&val2);
                            } else {
                                val2 = value;
                                ReplaceVariableWithValue(val2);
                            }
                            AutoInterpreterValuePoolValueOperation ret(m_Interpreter->GetInnerValuePool());
                            Value& retVal = ret.Get();
                            m_Interpreter->CallMember(*pObject, val1, TRUE, Function::PARAM_CLASS_OPERATOR, &val2, 1, &retVal);
                        }
                    }
                } else {
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
            virtual int IsMatch(const Statement& statement)const
            {
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc = statement.GetFunction(0);
                if (0 == pFunc)
                    return FALSE;
                if (pFunc->HaveParam() && pFunc->HaveStatement() || !pFunc->HaveParam() && !pFunc->HaveStatement())
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                LiteralObjectStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<LiteralObjectStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    if (statement.GetFunctionNum() == 1) {
                        Function* pFunc = statement.GetFunction(0);
                        if (0 != pFunc) {
                            if (pFunc->HaveParam() && !pFunc->HaveStatement()) {
                                pApi->m_Count = pFunc->GetParamNum();
                                pApi->m_pKeys = new Value[pApi->m_Count];
                                pApi->m_pValues = new Value[pApi->m_Count];
                                if (0 != pApi->m_pKeys && 0 != pApi->m_pValues) {
                                    for (int i = 0; i < pFunc->GetParamNum(); ++i) {
                                        Statement* p = pFunc->GetParam(i);
                                        if (0 == p)
                                            continue;
                                        if (p->GetFunctionNum() != 1)
                                            continue;
                                        Function* pPair = p->GetFunction(0);
                                        if (0 == pPair)
                                            continue;
                                        if (pPair->GetParamNum() != 2 || pPair->HaveStatement() || pPair->HaveExternScript())
                                            continue;
                                        Statement* p1 = pPair->GetParam(0);
                                        Statement* p2 = pPair->GetParam(1);
                                        if (p1 && p2) {
                                            pApi->m_pKeys[i] = p1->GetRuntimeObject();
                                            pApi->m_pValues[i] = p2->GetRuntimeObject();
                                        }
                                    }
                                }
                            } else if (!pFunc->HaveParam() && pFunc->HaveStatement()) {
                                pApi->m_Count = pFunc->GetStatementNum();
                                pApi->m_pKeys = new Value[pApi->m_Count];
                                pApi->m_pValues = new Value[pApi->m_Count];
                                if (0 != pApi->m_pKeys && 0 != pApi->m_pValues) {
                                    for (int i = 0; i < pFunc->GetStatementNum(); ++i) {
                                        Statement* p = pFunc->GetStatement(i);
                                        if (0 == p)
                                            continue;
                                        if (p->GetFunctionNum() != 1)
                                            continue;
                                        Function* pPair = p->GetFunction(0);
                                        if (0 == pPair)
                                            continue;
                                        if (pPair->GetParamNum() != 2 || pPair->HaveStatement() || pPair->HaveExternScript())
                                            continue;
                                        Statement* p1 = pPair->GetParam(0);
                                        Statement* p2 = pPair->GetParam(1);
                                        if (p1 && p2) {
                                            pApi->m_pKeys[i] = p1->GetRuntimeObject();
                                            pApi->m_pValues[i] = p2->GetRuntimeObject();
                                        }
                                    }
                                }
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
                Statement& statement = *m_pDefine;
                if (0 == m_Interpreter || statement.GetFunctionNum() != 1)
                    return EXECUTE_RESULT_NORMAL;
                Function* func0 = statement.GetFunction(0);
                if (0 == func0)
                    return EXECUTE_RESULT_NORMAL;
                int pnum = func0->GetParamNum();
                if (1 < pnum)
                    return EXECUTE_RESULT_NORMAL;
                Struct* pStruct = m_Interpreter->AddNewStructComponent();
                if (0 != pStruct) {
                    pStruct->SetDefinitionRef(statement);
                    if (func0->HaveParam()) {
                        Statement* pName = func0->GetParam(0);
                        if (NULL != pName) {
                            Value nameVal = pName->GetRuntimeObject();
                            AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                            Value& val = op.Get();
                            if (nameVal.IsStatement() && NULL != nameVal.GetStatement()) {
                                nameVal.GetStatement()->Execute(&val);
                            } else {
                                val = nameVal;
                            }
                            SetVariableValue(val, Value(pStruct));
                        }
                    }
                    if (NULL != pRetValue) {
                        pRetValue->SetExpression(pStruct);
                    }
                }

                return EXECUTE_RESULT_NORMAL;
            }
        public:
            explicit StructStatement(Interpreter& interpreter) :StatementApi(interpreter), m_pDefine(NULL) {}
        private:
            Statement* m_pDefine;
        };
        class StructStatementFactory : public StatementApiFactory
        {
        public:
            virtual int IsMatch(const Statement& statement)const
            {
                //结构定义语法：struct(name){member1(size,num);member2(size,num);...};其中，size可以是数或char、short、int、ptr
                if (statement.GetFunctionNum() != 1)
                    return FALSE;
                Function* pFunc0 = statement.GetFunction(0);
                if (0 == pFunc0)
                    return FALSE;
                return TRUE;
            }
            virtual StatementApi* PrepareRuntimeObject(Statement& statement)const
            {
                StructStatement* pApi = statement.GetInterpreter().AddNewStatementApiComponent<StructStatement>();
                if (NULL != pApi) {
                    statement.PrepareRuntimeObjectWithFunctions();
                    pApi->m_pDefine = &statement;
                }
                return pApi;
            }
        };
    }
    //------------------------------------------------------------------------------------------------------
    void Interpreter::InitInnerApis(void)
    {
        using namespace InnerApi;
        //
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

    void Interpreter::ReleaseInnerApis(void)
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
        for (int id = list.FrontID(); list.IsValidID(id); id = list.NextID(id)) {
            StatementApiFactory* pApi = list[id];
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
        AutoStackInfoStackOperation(Interpreter& interpreter, Value* pParams, int paramNum, int stackSize, const Statement& definition) :m_Interpreter(&interpreter)
        {
            if (NULL != m_Interpreter) {
                m_Interpreter->PushStackInfo(pParams, paramNum, stackSize, definition);
            }
        }
        ~AutoStackInfoStackOperation(void)
        {
            if (NULL != m_Interpreter) {
                m_Interpreter->PopStackInfo();
            }
        }
    private:
        Interpreter* m_Interpreter;
    };
    //------------------------------------------------------------------------------------------------------
    void Closure::SetDefinitionRef(const Value* pArguments, int argumentNum, const Statement& statement)
    {
        m_pArguments = pArguments;
        m_ArgumentNum = argumentNum;
        m_pDefinition = &statement;
        int num = statement.GetFunctionNum();
        if (num > 0) {
            Function* pFunc = statement.GetFunction(num - 1);
            if (NULL != pFunc) {
                m_StackSize = pFunc->CalculateStackSize();
                m_Statements = pFunc->GetRuntimeFunctionBody();
            }
        }
    }
    ExecuteResultEnum Closure::Execute(int paramClass, Value* pParams, int num, Value* pRetValue)
    {
        //Closure实现自定义函数的调用。
        if (NULL != m_Statements && NULL != m_Interpreter) {
            ReplaceVariableWithValue(pParams, num);
            AutoStackInfoStackOperation op(*m_Interpreter, pParams, num, m_StackSize, *m_pDefinition);
            //处理参数列表
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
                //_epsilon_句法副作用的特殊处理
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

    ObjectBase::~ObjectBase(void)
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
                    } else {
                        int index = _index - m_InnerMemberNum;
                        if (index >= 0 && index < m_MemberNum && NULL != m_MemberInfos) {
                            switch (num) {
                            case 2:
                            {
                                if ((Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (0 != pRetValue) {
                                        *pRetValue = info.m_Value;
                                    }
                                } else//obj.method()
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (info.m_Value.IsExpression() && 0 != info.m_Value.GetExpression()) {
                                        ExpressionApi& s_Exp = *(info.m_Value.GetExpression());
                                        Value params[] = { Value(this) };
                                        s_Exp.Execute(paramClass, params, 1, pRetValue);
                                    }
                                }
                            }
                                break;
                            case 3:
                            {
                                if ((Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property=val
                                {
                                    MemberInfo& info = m_MemberInfos[index];
                                    info.m_Value = pParams[2];
                                    if (0 != pRetValue) {
                                        *pRetValue = pParams[2];
                                    }
                                } else//obj.method(p1)
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (info.m_Value.IsExpression() && 0 != info.m_Value.GetExpression()) {
                                        ExpressionApi& s_Exp = *(info.m_Value.GetExpression());
                                        Value params[] = { Value(this), pParams[2] };
                                        s_Exp.Execute(paramClass, params, 2, pRetValue);
                                    }
                                }
                            }
                                break;
                            default://obj.method(p1,p2,...)
                            {
                                const MemberInfo& info = m_MemberInfos[index];
                                if (info.m_Value.IsExpression() && 0 != info.m_Value.GetExpression()) {
                                    ExpressionApi& s_Exp = *(info.m_Value.GetExpression());
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
                        } else if (index >= m_MemberNum) {
                            //新增成员只允许通过obj.property=val方式
                            if (3 == num && (Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK) {
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
            } else {
                if (1 == num) {
                    if ((pParams[0].IsString() || pParams[0].IsVariableName()) && 0 != pParams[0].GetString()) {
                        if (0 != pRetValue) {
                            const char* pName = pParams[0].GetString();
                            int index = GetMemberIndex(pName);
                            if (index >= 0 && index < m_InnerMemberNum && NULL != m_InnerMemberAccessors) {
                                pRetValue->SetExpression(m_InnerMemberAccessors[index]);
                            } else if (index >= m_InnerMemberNum && index < m_InnerMemberNum + m_MemberNum && NULL != m_Accessors) {
                                pRetValue->SetExpression(m_Accessors[index - m_InnerMemberNum]);
                            } else {
                                m_TempMemberInfo.m_Name = pName;
                                m_TempAccessor.SetMemberIndex(m_MemberNum + m_InnerMemberNum);
                                pRetValue->SetExpression(&m_TempAccessor);
                            }
                        }
                    } else if (pParams[0].IsInt())//直接给数值的，按脚本定义成员解释
                    {
                        if (0 != pRetValue) {
                            int index = pParams[0].GetInt();
                            if (index >= 0 && index < m_MemberNum && NULL != m_Accessors) {
                                pRetValue->SetExpression(m_Accessors[index]);
                            } else {
                                m_TempMemberInfo.m_Name = "";
                                m_TempAccessor.SetMemberIndex(index + m_InnerMemberNum);
                                pRetValue->SetExpression(&m_TempAccessor);
                            }
                        }
                    }
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void ObjectBase::Resize(void)
    {
        if (0 != m_Interpreter) {
            int capacity = m_Capacity + MEMBER_INFO_CAPACITY_DELTA_SIZE;
            MemberInfo* pInfos = new MemberInfo[capacity];
            if (0 != pInfos) {
                NameIndexMap* pNameIndexMap = new NameIndexMap();
                if (0 != pNameIndexMap) {
                    MemberAccessorPtr* pAccessors = new MemberAccessorPtr[capacity];
                    if (0 != pAccessors) {
                        memset(pAccessors, 0, sizeof(MemberAccessorPtr)*capacity);
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
                    } else {
                        delete pNameIndexMap;
                        delete[] pInfos;
                    }
                } else {
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

    void ObjectBase::ResetTemp(void)
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
                        } else {
                            //重复名字，不可能走到这！
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

    Object::~Object(void)
    {

    }

    Struct::~Struct(void)
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
                                    pRetValue->SetExpression(pNew);
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
                    } else {
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
                                if ((Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK & paramClass) == Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK)//obj.property=val
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (0 != m_Addr && pParams[2].IsInt() && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                        void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset);
                                        int val = pParams[2].GetInt();
                                        memcpy(pAddr, &val, info.m_Size);
                                        pRetValue->SetInt(val);
                                    }
                                } else//obj.property(index)
                                {
                                    const MemberInfo& info = m_MemberInfos[index];
                                    if (pParams[2].IsInt() && info.m_Size >= 0 && info.m_Size <= 4 && 0 != pRetValue) {
                                        int argIndex = pParams[2].GetInt();
                                        if (0 != m_Addr && argIndex >= 0 && argIndex < info.m_Num) {
                                            void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset + info.m_Size*argIndex);
                                            int val = 0;
                                            memcpy(&val, pAddr, info.m_Size);
                                            pRetValue->SetInt(val);
                                        } else if (argIndex == INNER_ARG_INDEX_OFFSET) {
                                            pRetValue->SetInt(info.m_Offset);
                                        } else if (argIndex == INNER_ARG_INDEX_SIZE) {
                                            pRetValue->SetInt(info.m_Size);
                                        } else if (argIndex == INNER_ARG_INDEX_NUM) {
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
                                        void* pAddr = ReinterpretCast<void*>::From(m_Addr + info.m_Offset + info.m_Size*argIndex);
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
            } else {
                if (1 == num) {
                    int index = -1;
                    if ((pParams[0].IsString() || pParams[0].IsVariableName()) && 0 != pParams[0].GetString()) {
                        const char* pName = pParams[0].GetString();
                        index = GetMemberIndex(pName);
                    } else if (pParams[0].IsInt())//数值解释成脚本定义成员
                    {
                        index = pParams[0].GetInt() + INNER_MEMBER_INDEX_NUM;
                    }
                    if (index >= 0 && index < m_MemberNum + INNER_MEMBER_INDEX_NUM && 0 != pRetValue && NULL != m_Accessors) {
                        pRetValue->SetExpression(m_Accessors[index]);
                    }
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void Struct::SetDefinitionRef(const Statement& statement)
    {
        m_pDefinition = &statement;
        //先不支持嵌套结构
        //todo:分析定义，生成结构布局数据，初始化m_Accessors
        Function* pFunc = statement.GetFunction(0);
        if (0 != pFunc && 0 != m_Interpreter) {
            int memberNum = pFunc->GetStatementNum();
            m_MemberInfos = new MemberInfo[memberNum];
            if (0 != m_MemberInfos) {
                m_Accessors = new MemberAccessorPtr[memberNum + INNER_MEMBER_INDEX_NUM];
                if (0 != m_Accessors) {
                    m_MemberNum = memberNum;

                    memset(m_Accessors, 0, sizeof(MemberAccessorPtr)*(m_MemberNum + INNER_MEMBER_INDEX_NUM));
                    m_Accessors[INNER_MEMBER_INDEX_ATTACH] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_ATTACH);
                    m_Accessors[INNER_MEMBER_INDEX_CLONE] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_CLONE);
                    m_Accessors[INNER_MEMBER_INDEX_ADDR] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_ADDR);
                    m_Accessors[INNER_MEMBER_INDEX_SIZE] = new MemberAccessor(*m_Interpreter, *this, INNER_MEMBER_INDEX_SIZE);
                    m_Size = 0;
                    for (int ix = 0; ix < m_MemberNum; ++ix) {
                        Statement* pStatement = pFunc->GetStatement(ix);
                        if (0 != pStatement) {
                            Function* pMember = pStatement->GetFunction(0);
                            if (0 != pMember) {

                                const Value& name = GetVariableValue(pMember->GetName());
                                Statement* pParam0 = pMember->GetParam(0);
                                Statement* pParam1 = pMember->GetParam(1);
                                m_Accessors[ix + INNER_MEMBER_INDEX_NUM] = new MemberAccessor(*m_Interpreter, *this, ix + INNER_MEMBER_INDEX_NUM);

                                int size = 4;
                                int num = 1;
                                if (0 != pParam0) {
                                    Value param0 = pParam0->GetRuntimeObject();
                                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                                    Value& val = op.Get();
                                    if (param0.IsStatement() && NULL != param0.GetStatement()) {
                                        param0.GetStatement()->Execute(&val);
                                    } else {
                                        val = param0;
                                        ReplaceVariableWithValue(val);
                                    }
                                    if (val.IsInt())
                                        size = val.GetInt();
                                    else if (val.IsVariableName() && 0 != val.GetString()) {
                                        const char* pName = val.GetString();
                                        if (0 == strcmp("char", pName)) {
                                            size = 1;
                                        } else if (0 == strcmp("short", pName)) {
                                            size = 2;
                                        } else if (0 == strcmp("int", pName)) {
                                            size = 4;
                                        } else if (0 == strcmp("ptr", pName)) {
                                            size = 4;
                                        }
                                    }
                                }
                                if (0 != pParam1) {
                                    Value param1 = pParam1->GetRuntimeObject();
                                    AutoInterpreterValuePoolValueOperation op(m_Interpreter->GetInnerValuePool());
                                    Value& val = op.Get();
                                    if (param1.IsStatement() && NULL != param1.GetStatement()) {
                                        param1.GetStatement()->Execute(&val);
                                    } else {
                                        val = param1;
                                        ReplaceVariableWithValue(val);
                                    }
                                    if (val.IsInt())
                                        num = val.GetInt();
                                }
                                m_MemberInfos[ix].m_Offset = m_Size;
                                m_MemberInfos[ix].m_Size = size;
                                m_MemberInfos[ix].m_Num = num;
                                if (name.IsVariableName()) {
                                    m_MemberInfos[ix].m_Name = name.GetString();
                                }
                                m_Size += size*num;
                            }
                        }
                    }
                } else {
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

    Struct* Struct::Clone(void) const
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
    Function::Function(Interpreter& interpreter) :
        SyntaxComponent(interpreter),
        m_RuntimeFunctionCall(0),
        m_RuntimeStatementBlock(0),
        m_RuntimeObjectPrepared(FALSE),
        m_LocalNum(0),
        m_LocalSpace(0),
        m_Params(0),
        m_ParamNum(0),
        m_ParamSpace(0),
        m_Statements(0),
        m_StatementNum(0),
        m_StatementSpace(0),
        m_ParamClass(PARAM_CLASS_NOTHING),
        m_ExtentClass(EXTENT_CLASS_NOTHING),
        m_ExternScript(0),
        m_pInnerValuePool(0)
    {
        const InterpreterOptions& options = interpreter.GetOptions();
        m_MaxStatementNum = options.GetMaxStatementNum();
        m_MaxLocalNum = options.GetMaxLocalNum();
        m_pInnerValuePool = &interpreter.GetInnerValuePool();
    }

    Function::~Function(void)
    {
        if (NULL != m_RuntimeStatementBlock) {
            delete m_RuntimeStatementBlock;
            m_RuntimeStatementBlock = NULL;
        }

        ReleaseParams();
        ReleaseStatements();
        ClearLocalIndexes();
    }

    void Function::PrepareParams(void)
    {
        if (NULL == m_Params && TRUE == HaveParam()) {
            m_Params = new StatementPtr[DELTA_FUNCTION_PARAM];
            if (m_Params) {
                m_ParamSpace = DELTA_FUNCTION_PARAM;
            }
        } else if (HaveParam() && m_ParamNum >= m_ParamSpace) {
            int newSpace = m_ParamSpace + DELTA_FUNCTION_PARAM;
            if (newSpace <= MAX_FUNCTION_PARAM_NUM) {
                StatementPtr* pNew = new StatementPtr[newSpace];
                if (pNew) {
                    memcpy(pNew, m_Params, m_ParamNum*sizeof(StatementPtr));
                    memset(pNew + m_ParamNum, 0, DELTA_FUNCTION_PARAM*sizeof(StatementPtr));
                    delete[] m_Params;
                    m_Params = pNew;
                    m_ParamSpace = newSpace;
                }
            }
        }
    }

    void Function::ReleaseParams(void)
    {
        if (NULL != m_Params) {
            delete[] m_Params;
            m_Params = NULL;
        }
    }

    void Function::PrepareStatements(void)
    {
        if (NULL == m_Statements && TRUE == HaveStatement()) {
            m_Statements = new StatementPtr[DELTA_FUNCTION_STATEMENT];
            if (m_Statements) {
                m_StatementSpace = DELTA_FUNCTION_STATEMENT;
            }
        } else if (HaveStatement() && m_StatementNum >= m_StatementSpace) {
            int newSpace = m_StatementSpace + DELTA_FUNCTION_STATEMENT;
            if (newSpace <= m_MaxStatementNum) {
                StatementPtr* pNew = new StatementPtr[newSpace];
                if (pNew) {
                    memcpy(pNew, m_Statements, m_StatementNum*sizeof(StatementPtr));
                    memset(pNew + m_StatementNum, 0, DELTA_FUNCTION_STATEMENT*sizeof(StatementPtr));
                    delete[] m_Statements;
                    m_Statements = pNew;
                    m_StatementSpace = newSpace;
                }
            }
        }
    }

    void Function::ReleaseStatements(void)
    {
        if (NULL != m_Statements) {
            delete[] m_Statements;
            m_Statements = NULL;
        }
    }

    void Function::PrepareLocalIndexes(void)
    {
        if (FALSE == m_LocalIndexes.IsInited()) {
            //m_LocalIndexes.InitTable(m_MaxLocalNum);
            //m_LocalSpace=m_MaxLocalNum;
            m_LocalIndexes.InitTable(DELTA_FUNCTION_LOCAL);
            m_LocalSpace = DELTA_FUNCTION_LOCAL;
        } else if (m_LocalNum >= m_LocalSpace) {
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
                        } else {
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

    void Function::ClearLocalIndexes(void)
    {
        if (TRUE == m_LocalIndexes.IsInited()) {
            m_LocalIndexes.CleanUp();
        }
    }

    int Function::AllocLocalIndex(const char* id)
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

    int Function::GetLocalIndex(const char* id)const
    {
        int index = -1;
        if (0 != id && TRUE == m_LocalIndexes.IsInited()) {
            index = m_LocalIndexes.Get(id);
        }
        return index;
    }

    const char* Function::GetLocalName(int index)const
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

    int	Function::CalculateStackSize(void)const
    {
        return m_LocalIndexes.GetNum();
    }

    void Function::PrepareRuntimeObject(void)
    {
        if (NULL == m_Interpreter)
            return;
        if (m_RuntimeObjectPrepared)
            return;
        //处理函数名
        if (m_Name.IsVariableName()) {
            char* pStr = m_Name.GetString();
            if (NULL != pStr) {
                if (pStr[0] == '$')//参数
                {
                    if (pStr[1] == '$') {
                        m_Name.SetArgIndex(-1);
                    } else {
                        int index = atoi(pStr + 1);
                        if (index >= 0 && index < MAX_FUNCTION_PARAM_NUM) {
                            m_Name.SetArgIndex(index);
                        }
                    }
                } else if (pStr[0] == '@')//局部变量
                {
                    if (pStr[1] == '@') {
                        m_Name.SetLocalIndex(-1);
                    } else {
                        Function* pFuncDef = m_Interpreter->GetCurFunctionDefinition();
                        if (0 != pFuncDef) {
                            int index = pFuncDef->GetLocalIndex(pStr);
                            if (index < 0) {
                                index = pFuncDef->AllocLocalIndex(pStr);
                            }
                            if (index >= 0 && index < m_MaxLocalNum) {
                                m_Name.SetLocalIndex(index);
                            }
                        }
                    }
                } else {
                    //函数、全局变量与预定义变量
                    if (0 == strcmp(pStr, "this")) {
                        //this <=> $0
                        m_Name.SetArgIndex(0);
                    } else {
                        ExpressionApi* p = m_Interpreter->FindFunctionApi(pStr);
                        //普通变量
                        AutoInterpreterValuePoolValueOperation op(*m_pInnerValuePool);
                        Value& val = op.Get();
                        if (0 != p) {
                            //内部api名称不作变量处理，直接计算值（相当于常量），运行时会少一次计算变量值的间接处理
                            val.SetExpression(p);
                            m_Name = val;
                        } else {
                            val = m_Name;
                            if (m_Interpreter->GetValueIndex(pStr) == -1 || 0 != p) {
                                m_Interpreter->SetValue(pStr, val);
                            }
                            m_Name.SetIndex(m_Interpreter->GetValueIndex(pStr));
                        }
                    }
                }
            }
        } else if (m_Name.IsFunction() && 0 != m_Name.GetFunction()) {
            m_Name.GetFunction()->PrepareRuntimeObject();
        }
        m_RuntimeObjectPrepared = TRUE;
        //参数
        if (0 != m_Params) {
            for (int ix = 0; ix < m_ParamNum; ++ix) {
                if (0 != m_Params[ix]) {
                    m_Params[ix]->PrepareRuntimeObject();
                }
            }
        }
        //语句
        if (0 != m_Statements) {
            for (int ix = 0; ix < m_StatementNum; ++ix) {
                if (0 != m_Statements[ix]) {
                    m_Statements[ix]->PrepareRuntimeObject();
                }
            }
            if (m_StatementNum > 0) {
                m_RuntimeStatementBlock = new RuntimeStatementBlock(*m_Interpreter, *this);
            }
        }
        if (HaveParam()) {
            //最后生成复合运行时对象，与语句不同，函数（除非退化成Value的情形，此时由外层函数去组合成RuntimeFunction）总要生成一个RuntimeFunction，从语义上讲Function代表的其实也是一种StatementApi，而不是ExpressionApi
            RuntimeFunctionCall* pRuntimeFunction = m_Interpreter->AddNewRuntimeFunctionComponent();
            if (NULL != pRuntimeFunction) {
                pRuntimeFunction->Init(*this);
                m_RuntimeFunctionCall.SetStatement(pRuntimeFunction);
            } else {
                m_RuntimeFunctionCall.SetInvalid();
            }
        } else {
            m_RuntimeFunctionCall = m_Name;
        }
    }

    const Value& Function::GetRuntimeFunctionHead(void)const
    {
        return m_RuntimeFunctionCall;
    }

    Statement::Statement(Interpreter& interpreter) :
        SyntaxComponent(interpreter),
        m_RuntimeObject(0),
        m_RuntimeObjectPrepared(FALSE),
        m_Functions(0),
        m_FunctionNum(0),
        m_FunctionSpace(0),
        m_Line(-1)
    {
        const InterpreterOptions& options = interpreter.GetOptions();
        m_MaxFunctionNum = options.GetMaxFunctionDimensionNum();
    }

    void Statement::PrepareFunctions(void)
    {
        if (NULL == m_Functions) {
            m_Functions = new Function*[DELTA_STATEMENT_FUNCTION];
            if (m_Functions) {
                m_FunctionSpace = DELTA_STATEMENT_FUNCTION;
            }
        } else if (m_FunctionNum >= m_FunctionSpace) {
            int newSpace = m_FunctionSpace + DELTA_STATEMENT_FUNCTION;
            if (newSpace <= m_MaxFunctionNum) {
                Function** pNew = new Function*[newSpace];
                if (pNew) {
                    memcpy(pNew, m_Functions, m_FunctionNum*sizeof(Function*));
                    memset(pNew + m_FunctionNum, 0, DELTA_STATEMENT_FUNCTION*sizeof(Function*));
                    delete[] m_Functions;
                    m_Functions = pNew;
                    m_FunctionSpace = newSpace;
                }
            }
        }
    }

    void Statement::ReleaseFunctions(void)
    {
        if (NULL != m_Functions) {
            delete[] m_Functions;
            m_Functions = NULL;
        }
    }

    void Statement::PrepareRuntimeObject(void)
    {
        if (NULL == m_Interpreter)
            return;
        if (m_RuntimeObjectPrepared)
            return;
        StatementApiFactory* pApiFactory = m_Interpreter->FindStatementApi(*this);
        m_RuntimeObjectPrepared = TRUE;
        if (0 != pApiFactory) {
            //让API生成复合运行时对象。
            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
            m_RuntimeObject.SetStatement(pApi);
        } else {
            //object literal检测
            if (GetFunctionNum() == 1) {
                Function* pFunc = GetFunction(0);
                if (0 != pFunc && !pFunc->HaveName()) {
                    if (pFunc->HaveParam() && !pFunc->HaveStatement() && pFunc->GetParamClass() == Function::PARAM_CLASS_BRACKET) {
                        pApiFactory = m_Interpreter->GetLiteralArrayApi();
                        if (0 != pApiFactory) {
                            //让API生成复合运行时对象。
                            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
                            m_RuntimeObject.SetStatement(pApi);
                        }
                        return;
                    } else if (!pFunc->HaveParam() && pFunc->HaveStatement()) {
                        pApiFactory = m_Interpreter->GetLiteralObjectApi();
                        if (0 != pApiFactory) {
                            //让API生成复合运行时对象。
                            StatementApi* pApi = pApiFactory->PrepareRuntimeObject(*this);
                            m_RuntimeObject.SetStatement(pApi);
                        }
                        return;
                    }
                }
            }
            //先让各个函数准备运行时对象
            PrepareRuntimeObjectWithFunctions();
            //再生成复合运行时对象,此时组成语句的各函数已经递归完成运行时对象构造，复合运行时对象只需要组合本语句层面的运行时对象即可。
            bool isStatement = false;
            int num = GetFunctionNum();
            if (0 == num) {
                m_RuntimeObject.SetInvalid();
            } else if (1 == num) {
                //退化为函数或一个普通值
                Function* pFunc = GetFunction(0);
                if (NULL != pFunc) {
                    if (pFunc->HaveStatement()) {
                        isStatement = true;
                    } else {
                        m_RuntimeObject = pFunc->GetRuntimeFunctionHead();
                    }
                } else {
                    m_RuntimeObject.SetInvalid();
                }
            } else {
                isStatement = true;
            }
            if (isStatement) {
                RuntimeStatement* pRuntimeStatement = m_Interpreter->AddNewRuntimeStatementComponent();
                if (NULL != pRuntimeStatement) {
                    pRuntimeStatement->PrepareRuntimeObject(*this);
                    m_RuntimeObject.SetStatement(pRuntimeStatement);
                } else {
                    m_RuntimeObject.SetInvalid();
                }
            }
        }
    }

    void Statement::PrepareRuntimeObjectWithFunctions(void)
    {
        if (NULL == m_Functions)
            return;
        for (int ix = 0; ix < m_FunctionNum; ++ix) {
            Function* pFunction = m_Functions[ix];
            if (0 != pFunction) {
                pFunction->PrepareRuntimeObject();
            }
        }
    }

    const Value& Statement::GetRuntimeObject(void)const
    {
        return m_RuntimeObject;
    }

    RuntimeStatementBlock::RuntimeStatementBlock(Interpreter& interpreter, Function& func) :m_Interpreter(&interpreter), m_StatementNum(0), m_Statements(NULL), m_pInnerValuePool(NULL)
    {
        m_pInnerValuePool = &interpreter.GetInnerValuePool();

        m_StatementNum = func.GetStatementNum();
        if (m_StatementNum > 0) {
            m_Statements = new StatementApi*[m_StatementNum];
            for (int ix = 0; ix < m_StatementNum; ++ix) {
                Statement* pStatement = func.GetStatement(ix);
                if (NULL != pStatement) {
                    Value val = pStatement->GetRuntimeObject();
                    if (val.IsStatement())
                        m_Statements[ix] = val.GetStatement();
                    else
                        m_Statements[ix] = NULL;
                } else {
                    m_Statements[ix] = NULL;
                }
            }
        }
    }
    RuntimeStatementBlock::~RuntimeStatementBlock(void)
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
                } else if (EXECUTE_RESULT_GOTO == ret && 0 != val.GetInt()) {
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

    RuntimeFunctionCall::~RuntimeFunctionCall(void)
    {
        if (NULL != m_Params) {
            delete[] m_Params;
        }
        m_ParamNum = 0;
    }

    void RuntimeFunctionCall::Init(Function& func)
    {
        m_Name = func.GetName();
        if (m_Name.IsFunction()) {
            Function* pFunc = m_Name.GetFunction();
            if (NULL != pFunc) {
                m_Name = pFunc->GetRuntimeFunctionHead();
            }
        }
        m_ParamClass = func.GetParamClass();
        m_ParamNum = func.GetParamNum();
        if (m_ParamNum > 0) {
            m_Params = new Value[m_ParamNum];
            for (int ix = 0; ix < m_ParamNum; ++ix) {
                Statement* pParam = func.GetParam(ix);
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
        //这里有一个约定，所有参数都由处理函数自己计算参数值，传递的是原始参数信息，这样便于实现赋值或out参数特性
        //复制参数信息
        if (0 != m_Params) {
            for (int ix = 0; ix < m_ParamNum && m_Interpreter->IsRunFlagEnable(); ++ix) {
                params[ix] = m_Params[ix];
                Value& param = params[ix];
                if (param.IsStatement() && NULL != param.GetStatement()) {
                    //参数是语句的需要执行语句后传递其返回值
                    ExecuteResultEnum ret = param.GetStatement()->Execute(&param);
                    if (EXECUTE_RESULT_NORMAL != ret) {
#ifndef _GAMECLIENT_
                        printf("Error break/continue/return !\n");
                        m_Interpreter->AddError("Error break/continue/return !\n");
#endif
                    }
                }
            }
            if (!m_Interpreter->IsRunFlagEnable()) {
                return EXECUTE_RESULT_NORMAL;
            }
        }
        if (m_Name.IsStatement()) {
            StatementApi* pApi = m_Name.GetStatement();
            if (0 != pApi) {
                AutoInterpreterValuePoolValueOperation op(*m_pInnerValuePool);
                Value& val = op.Get();
                ExecuteResultEnum ret = pApi->Execute(&val);
                if (EXECUTE_RESULT_NORMAL != ret) {
#ifndef _GAMECLIENT_
                    printf("Error break/continue/return !\n");
                    m_Interpreter->AddError("Error break/continue/return !\n");
#endif
                }
                ExpressionApi* pExp = val.GetExpression();
                if (val.IsExpression() && 0 != pExp) {
                    /*if(0!=pRetValue)
                    pRetValue->SetInvalid();*/
                    pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
                }
            }
        } else if (m_Name.IsExpression()) {
            ExpressionApi* pExp = m_Name.GetExpression();
            if (0 != pExp) {
                /*if(0!=pRetValue)
                pRetValue->SetInvalid();*/
                pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
            }
        } else {
            if (m_Name.IsInvalid()) {
                //逗号表达式实现（现在我们自己就是API的实现，有对参数求值的职责）
                ReplaceVariableWithValue(params, m_ParamNum);
                //逗号表达式返回最后一个参数的值
                if (0 != pRetValue)
                    *pRetValue = params[m_ParamNum - 1];
            } else {
                Value val = m_Name;
                //变量引用函数的函数调用
                ReplaceVariableWithValue(val);
                ExpressionApi* pExp = val.GetExpression();
                if (val.IsExpression() && 0 != pExp) {
                    pExp->Execute(m_ParamClass, params, m_ParamNum, pRetValue);
                } else {
                    //函数名引用的不是函数，忽略函数名，当作逗号表达式处理，返回最后一个参数的值
                    //逗号表达式实现（现在我们自己就是API的实现，有对参数求值的职责）
                    ReplaceVariableWithValue(params, m_ParamNum);
                    //逗号表达式返回最后一个参数的值
                    if (0 != pRetValue)
                        *pRetValue = params[m_ParamNum - 1];
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    RuntimeStatement::RuntimeStatement(Interpreter& interpreter) :StatementApi(interpreter), m_FunctionNum(0), m_FunctionHeads(NULL), m_FunctionBodies(NULL)
    {
    }

    RuntimeStatement::~RuntimeStatement(void)
    {
        if (NULL != m_FunctionHeads) {
            delete[] m_FunctionHeads;
            m_FunctionHeads = NULL;
        }
        if (NULL != m_FunctionBodies) {
            delete[] m_FunctionBodies;
            m_FunctionBodies = NULL;
        }
        m_FunctionNum = 0;
    }

    void RuntimeStatement::PrepareRuntimeObject(Statement& statement)
    {
        m_FunctionNum = statement.GetFunctionNum();
        if (m_FunctionNum > 0) {
            m_FunctionHeads = new StatementApi*[m_FunctionNum];
            m_FunctionBodies = new RuntimeStatementBlock*[m_FunctionNum];
            for (int ix = 0; ix < m_FunctionNum; ++ix) {
                Function* pFunc = statement.GetFunction(ix);
                if (NULL != pFunc) {
                    Value val = pFunc->GetRuntimeFunctionHead();
                    if (val.IsStatement())
                        m_FunctionHeads[ix] = val.GetStatement();
                    else
                        m_FunctionHeads[ix] = NULL;
                    m_FunctionBodies[ix] = pFunc->GetRuntimeFunctionBody();
                } else {
                    m_FunctionHeads[ix] = NULL;
                    m_FunctionBodies[ix] = NULL;
                }
            }
        }
    }

    ExecuteResultEnum RuntimeStatement::Execute(Value* pRetValue)const
    {
        if (NULL == m_Interpreter || NULL == m_FunctionHeads || NULL == m_FunctionBodies)
            return EXECUTE_RESULT_NORMAL;
        //按顺序执行函数部分与扩展语句部分
        for (int ix = 0; ix < m_FunctionNum && m_Interpreter->IsRunFlagEnable(); ++ix) {
            StatementApi* pFunction = m_FunctionHeads[ix];
            if (0 != pFunction) {
                ExecuteResultEnum ret = pFunction->Execute(pRetValue);
                if (EXECUTE_RESULT_NORMAL != ret)
                    return ret;
            }
            RuntimeStatementBlock* pStatements = m_FunctionBodies[ix];
            if (0 != pStatements) {
                ExecuteResultEnum ret = pStatements->Execute(pRetValue);
                if (EXECUTE_RESULT_NORMAL != ret)
                    return ret;
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    void Interpreter::PrepareRuntimeObject(void)
    {
        if (0 == m_Program)
            return;
        if (!HasError()) {
            for (int ix = 0; ix < m_StatementNum; ++ix) {
                Statement* pStatement = m_Program[ix];
                if (0 != pStatement) {
                    pStatement->PrepareRuntimeObject();
                    Value val = pStatement->GetRuntimeObject();
                    if (val.IsStatement()) {
                        m_RuntimeProgram[ix] = val.GetStatement();
                    } else {
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
                } else if (EXECUTE_RESULT_GOTO == ret && 0 != val.GetInt()) {
                    m_NextStatement += val.GetInt() - 1;
                    continue;
                }
            }
        }
        return EXECUTE_RESULT_NORMAL;
    }

    ExecuteResultEnum Interpreter::CallMember(ExpressionApi& obj, const Value& member, int isProperty, int paramClass, Value* pParams, int paramNum, Value* pRetValue)
    {
        //对象成员访问分2步完成，第一步用成员名或索引作参数，获得成员访问对象。第二步用成员访问对象操作对象成员，完成实际操作。
        //解析器在wrapObjectMember/wrapObjectMemberInHighOrderFunction方法里将这2步组合成二阶函数调用。
        Value param = member;
        AutoInterpreterValuePoolValueOperation op(m_InterpreterValuePool);
        Value& memberAccessor = op.Get();
        ExecuteResultEnum r = obj.Execute(Function::PARAM_CLASS_PERIOD, &param, 1, &memberAccessor);
        if (memberAccessor.IsExpression() && 0 != memberAccessor.GetExpression()) {
            ExpressionApi& mAccessor = *(memberAccessor.GetExpression());
            int mask = 0;
            if (TRUE == isProperty)
                mask = Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK;
            return mAccessor.Execute(paramClass | mask, pParams, paramNum, pRetValue);
        } else {
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
        for (int id = list.FrontID(); list.IsValidID(id); id = list.NextID(id)) {
            StatementApiFactory* pApi = list[id];
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

    StatementApiFactory* Interpreter::GetLiteralArrayApi(void)const
    {
        const StatementApiFactoryList& innerList = m_InnerStatementApis.Get("literalarray");
        if (innerList.Size() == 1) {
            return innerList.Front();
        } else {
            return 0;
        }
    }

    StatementApiFactory* Interpreter::GetLiteralObjectApi(void)const
    {
        const StatementApiFactoryList& innerList = m_InnerStatementApis.Get("literalobject");
        if (innerList.Size() == 1) {
            return innerList.Front();
        } else {
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

    StatementApiFactory* Interpreter::FindStatementApi(const Statement& statement)const
    {
        if (statement.GetFunctionNum() < 1)
            return 0;
        Function* pFunc = statement.GetFunction(0);
        if (0 == pFunc || !pFunc->HaveName()) {
            return 0;
        }
        Value funcName = pFunc->GetName();
        if (funcName.IsInvalid() || !funcName.IsVariableName() || 0 == funcName.GetString()) {
            return 0;
        }
        const char* id = funcName.GetString();
        StatementApiFactory* p = 0;
        const StatementApiFactoryList& list = m_StatementApis.Get(id);
        if (list.Size() > 0) {
            for (int id = list.FrontID(); list.IsValidID(id); id = list.NextID(id)) {
                StatementApiFactory* pApi = list[id];
                if (0 != pApi) {
                    if (pApi->IsMatch(statement)) {
                        p = pApi;
                        break;
                    }
                }
            }
        } else {
            const StatementApiFactoryList& innerList = m_InnerStatementApis.Get(id);
            if (innerList.Size() > 0) {
                for (int id = innerList.FrontID(); innerList.IsValidID(id); id = innerList.NextID(id)) {
                    StatementApiFactory* pApi = innerList[id];
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
            } else {
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
            } else if (index < sizeof(g_ArgNames) / sizeof(const char*)) {
                return g_ArgNames[index];
            }
        }
            break;
        case Value::TYPE_LOCAL_INDEX:
        {
            if (index < 0) {
                return "@@";
            } else if (index < m_Options.GetMaxLocalNum() && FALSE == m_StackInfos.Empty()) {
                const StackInfo& info = m_StackInfos.Front();
                const Statement* pDefinition = info.m_pDefinition;
                if (NULL != pDefinition && index < info.m_Size.GetInt()) {
                    Function* pFunc = pDefinition->GetFunction(1);
                    if (NULL != pFunc) {
                        return pFunc->GetLocalName(index);
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
            } else {
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
            } else {
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
            } else {
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
            } else {
                const StackInfo& info = m_StackInfos.Front();
                int _index = info.m_Start + index;
                return m_StackValuePool[_index];
            }
        }
            break;
        }
        return Value::GetInvalidValueRef();
    }

    void Interpreter::PushStackInfo(Value* pParams, int paramNum, int stackSize, const Statement& definition)
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

    void Interpreter::PopStackInfo(void)
    {
        if (FALSE == m_StackInfos.Empty()) {
            m_StackInfos.PopFront();
        }
    }

    int Interpreter::IsStackEmpty(void)const
    {
        return m_StackInfos.Empty();
    }

    int Interpreter::IsStackFull(void)const
    {
        return m_StackInfos.Full();
    }

    const Interpreter::StackInfo& Interpreter::GetTopStackInfo(void)const
    {
        if (FALSE == m_StackInfos.Empty()) {
            return m_StackInfos.Front();
        } else {
            return StackInfo::GetInvalidStackInfoRef();
        }
    }

    int Interpreter::GetStackValueNum(void)const
    {
        if (FALSE == m_StackInfos.Empty()) {
            const StackInfo& info = m_StackInfos.Front();
            return info.m_Start + info.m_Size.GetInt();
        } else {
            return m_Options.GetStackValuePoolSize();
        }
    }

    int Interpreter::PushFunctionDefinition(Function* pFunction)
    {
        if (FALSE == m_FunctionDefinitionStack.Full()) {
            m_FunctionDefinitionStack.PushFront(pFunction);
            return TRUE;
        } else {
            return FALSE;
        }
    }

    int Interpreter::PopFunctionDefinition(void)
    {
        if (FALSE == m_FunctionDefinitionStack.Empty()) {
            m_FunctionDefinitionStack.PopFront();
            return TRUE;
        } else {
            return FALSE;
        }
    }

    Function* Interpreter::GetCurFunctionDefinition(void)const
    {
        if (FALSE == m_FunctionDefinitionStack.Empty()) {
            return m_FunctionDefinitionStack.Front();
        } else {
            return 0;
        }
    }

    void Interpreter::AddStatement(Statement* p)
    {
        if (0 == p || 0 == m_Program)
            return;
        if (m_StatementNum < 0 || m_StatementNum >= m_Options.GetMaxProgramSize())
            return;
        m_Program[m_StatementNum] = p;
        ++m_StatementNum;
    }

    Function* Interpreter::AddNewFunctionComponent(void)
    {
        Function* p = new Function(*this);
        AddSyntaxComponent(p);
        return p;
    }

    Statement* Interpreter::AddNewStatementComponent(void)
    {
        Statement* p = new Statement(*this);
        AddSyntaxComponent(p);
        return p;
    }

    RuntimeFunctionCall* Interpreter::AddNewRuntimeFunctionComponent(void)
    {
        RuntimeFunctionCall* p = new RuntimeFunctionCall(*this);
        AddRuntimeComponent(p);
        return p;
    }

    RuntimeStatement* Interpreter::AddNewRuntimeStatementComponent(void)
    {
        RuntimeStatement* p = new RuntimeStatement(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Closure* Interpreter::AddNewClosureComponent(void)
    {
        Closure* p = new Closure(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Object* Interpreter::AddNewObjectComponent(void)
    {
        Object* p = new Object(*this);
        AddRuntimeComponent(p);
        return p;
    }

    Struct* Interpreter::AddNewStructComponent(void)
    {
        Struct* p = new Struct(*this);
        AddRuntimeComponent(p);
        return p;
    }

    void Interpreter::AddSyntaxComponent(SyntaxComponent* p)
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

    Interpreter::StackInfo& Interpreter::StackInfo::GetInvalidStackInfoRef(void)
    {
        static StackInfo s_StackInfo;
        return s_StackInfo;
    }

    Interpreter::Interpreter(void) :m_IsDebugInfoEnable(FALSE),
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
        m_InterpreterValuePool(MAX_FUNCTION_LEVEL*MAX_STACK_LEVEL, MAX_FUNCTION_LEVEL*MAX_STACK_LEVEL)
    {
        m_InnerFunctionApis.InitTable(m_Options.GetMaxInnerFunctionApiNum());
        m_InnerStatementApis.InitTable(m_Options.GetMaxInnerStatementApiNum());
        m_StatementApis.InitTable(m_Options.GetMaxStatementApiNum());

        m_ValueIndexes.InitTable(m_Options.GetValuePoolSize());

        m_PredefinedValueIndexes.InitTable(m_Options.GetExpressionPoolSize());
        m_PredefinedValue = new Value[m_Options.GetExpressionPoolSize()];
        m_PredefinedValueNum = 0;

        Init();
        InitInnerApis();
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
        m_InterpreterValuePool(MAX_FUNCTION_LEVEL*MAX_STACK_LEVEL, MAX_FUNCTION_LEVEL*MAX_STACK_LEVEL)
    {
        m_InnerFunctionApis.InitTable(m_Options.GetMaxInnerFunctionApiNum());
        m_InnerStatementApis.InitTable(m_Options.GetMaxInnerStatementApiNum());
        m_StatementApis.InitTable(m_Options.GetMaxStatementApiNum());

        m_ValueIndexes.InitTable(m_Options.GetValuePoolSize());

        m_PredefinedValueIndexes.InitTable(m_Options.GetExpressionPoolSize());
        m_PredefinedValue = new Value[m_Options.GetExpressionPoolSize()];
        m_PredefinedValueNum = 0;

        Init();
        InitInnerApis();
    }

    Interpreter::~Interpreter(void)
    {
        ReleaseInnerApis();
        Release();

        if (m_PredefinedValue) {
            delete[] m_PredefinedValue;
            m_PredefinedValue = NULL;
        }
    }

    void Interpreter::Reset(void)
    {
        Release();
        Init();
        m_InterpreterValuePool.Reset();
    }

    void Interpreter::Init(void)
    {
        m_StringBuffer = new char[m_Options.GetStringBufferSize()];
        m_UnusedStringPtr = m_StringBuffer;
        m_SyntaxComponentPool = new SyntaxComponentPtr[m_Options.GetSyntaxComponentPoolSize()];
        m_SyntaxComponentNum = 0;
        m_RuntimeComponentPool = new RuntimeComponentPtr[m_Options.GetExpressionPoolSize()];
        m_RuntimeComponentNum = 0;
        m_Program = new StatementPtr[m_Options.GetMaxProgramSize()];
        m_StatementNum = 0;
        m_RuntimeProgram = new StatementApiPtr[m_Options.GetMaxProgramSize()];
        m_ValuePool = new Value[m_Options.GetValuePoolSize()];
        m_ValueNum = 0;
        m_StackValuePool = new Value[m_Options.GetStackValuePoolSize()];

        m_NextStatement = 0;

        m_ErrorAndStringBuffer.Reset(m_StringBuffer, m_UnusedStringPtr, m_Options.GetStringBufferSize());
    }

    void Interpreter::Release(void)
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

    void ErrorAndStringBuffer::ClearErrorInfo(void)
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