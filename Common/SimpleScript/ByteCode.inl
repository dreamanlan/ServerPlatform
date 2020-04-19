//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------

#define PRINT_FUNCTION_SCRIPT_DEBUG_INFO printf

namespace FunctionScript
{
    /*
     * 备忘：为什么采用约简的方式而不是延迟一次性构造
     * 1、已尝试过采用一个临时的结构比如SyntaxMaterial来收集语法解析过程中的数据，到语句完成时再构造语句
     * 2、临时的结构与最终语义数据结构上相似度很高，也需要表示递归结构并且要与现有语义数据关联，代码重复并且逻辑不够清晰。
     * 3、约简方式已经尽量重用语法解析中构造的实例，基本不会产生额外内存占用
     * 4、约简方式下最终内存占用与脚本复杂度线性相关，不用担心占用过多内存
     * 5、语义数据在定义上考虑了退化情形，除必须数据外已尽量不占用额外空间
     */
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::setExternScript(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 != p) {
            char* pStr = mThis->getLastToken();
            if (0 != pStr) {
                if (mInterpreter->IsDebugInfoEnable()) {
                    PRINT_FUNCTION_SCRIPT_DEBUG_INFO("script:%s\n", pStr);
                }

                p->SetExternScript(pStr);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markOperator(void)
    {
        if (!preconditionCheck())return;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildOperator(void)
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op:%s\n", tokenInfo.mString);
        }

        StatementData* pArg = mData.getCurStatement();
        if (0 == pArg)
            return;
        int handled = FALSE;
        if (0 == strcmp("=", tokenInfo.mString)) {
            //因为这里未出栈，不能先化简
            handled = wrapObjectMember(*pArg);
        }
        if (FALSE == handled) {
            mData.popStatement();
            ISyntaxComponent& argComp = simplifyStatement(*pArg);
            StatementData* pStatement = mInterpreter->AddNewStatementComponent();
            if (0 == pStatement)
                return;
            mData.pushStatement(pStatement);

            FunctionData* p = mInterpreter->AddNewFunctionComponent();
            if (0 != p) {
                CallData& call = p->GetCall();
                if (0 != tokenInfo.mString && tokenInfo.mString[0] == '`') {
                    call.SetParamClass(CallData::PARAM_CLASS_OPERATOR);

                    Value v = call.GetNameValue();
                    Value op(tokenInfo.mString + 1, Value::TYPE_IDENTIFIER);
                    op.SetLine(mThis->getLastLineNumber());
                    call.SetNameValue(op);
                }
                else {
                    call.SetParamClass(CallData::PARAM_CLASS_OPERATOR);

                    Value v = call.GetNameValue();
                    Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
                    op.SetLine(mThis->getLastLineNumber());
                    call.SetNameValue(op);
                }

                if (argComp.IsValid()) {
                    wrapObjectMember(argComp);
                    call.AddParam(&argComp);
                }

                pStatement->AddFunction(p);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildFirstTernaryOperator(void)
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op1/2:%d\n", tokenInfo.mInteger);
        }

        StatementData* pArg = mData.popStatement();
        if (0 == pArg)
            return;
        ISyntaxComponent& argComp = simplifyStatement(*pArg);
        StatementData* pStatement = mInterpreter->AddNewStatementComponent();
        if (0 == pStatement)
            return;
        mData.pushStatement(pStatement);

        FunctionData* p = mInterpreter->AddNewFunctionComponent();
        if (0 != p) {
            CallData& call = p->GetCall();
            call.SetParamClass(CallData::PARAM_CLASS_TERNARY_OPERATOR);
            p->SetExtentClass(FunctionData::EXTENT_CLASS_STATEMENT);

            Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
            op.SetLine(mThis->getLastLineNumber());
            call.SetNameValue(op);
            if (argComp.IsValid()) {
                wrapObjectMember(argComp);
                call.AddParam(&argComp);
            }

            pStatement->AddFunction(p);
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildSecondTernaryOperator(void)
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op2/2:%d\n", tokenInfo.mInteger);
        }

        StatementData* statement = mData.getCurStatement();
        if (0 != statement) {
            FunctionData* p = mInterpreter->AddNewFunctionComponent();
            if (0 != p) {
                CallData& call = p->GetCall();
                call.SetParamClass(CallData::PARAM_CLASS_TERNARY_OPERATOR);
                p->SetExtentClass(FunctionData::EXTENT_CLASS_STATEMENT);

                Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
                op.SetLine(mThis->getLastLineNumber());
                call.SetNameValue(op);

                statement->AddFunction(p);
            }
        }
    }
    //--------------------------------------------------------------------------------------	
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::beginStatement(void)
    {
        if (!preconditionCheck())return;
        StatementData* p = mInterpreter->AddNewStatementComponent();
        if (0 == p)
            return;
        mData.pushStatement(p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::endStatement(void)
    {
        if (!preconditionCheck())return;
        StatementData* statement = mData.popStatement();
        if (0 == statement || statement->GetFunctionNum() == 0)
            return;
        const char* id = statement->GetId();
        if (0 != id && strcmp(id, "@@delimiter") == 0 && statement->GetFunctionNum() == 1 && (statement->GetLastFunctionRef()->GetCall().GetParamNum() == 1 || statement->GetLastFunctionRef()->GetCall().GetParamNum() == 3) && !statement->GetLastFunctionRef()->GetCall().IsHighOrder()) {
            const CallData& call = statement->GetLastFunctionRef()->GetCall();
            const char* type = call.GetParamId(0);
            if (call.GetParamNum() == 3) {
                const char* begin = call.GetParamId(1);
                const char* end = call.GetParamId(2);
                if (strcmp(type, "string") == 0) {
                    mThis->setStringDelimiter(begin, end);
                }
                else if (strcmp(type, "script") == 0) {
                    mThis->setScriptDelimiter(begin, end);
                }
                else {
                    //invalid
                }
            }
            else {
                if (strcmp(type, "string") == 0) {
                    mThis->setStringDelimiter("", "");
                }
                else if (strcmp(type, "script") == 0) {
                    mThis->setScriptDelimiter("", "");
                }
                else {
                    //invalid
                }
            }
            return;
        }
        if (mData.isSemanticStackEmpty()) {
            if (!statement->IsValid()) {
                //_epsilon_表达式无语句语义
                return;
            }
            //化简只需要处理一级，参数与语句部分应该在添加到语句时已经处理了
            ISyntaxComponent& statementSyntax = simplifyStatement(*statement);
            //顶层元素结束
            wrapObjectMember(statementSyntax);
            mInterpreter->AddStatement(&statementSyntax);
            mThis->setCanFinish(TRUE);
        }
        else {
            //化简只需要处理一级，参数与语句部分应该在添加到语句时已经处理了
            ISyntaxComponent& statementSyntax = simplifyStatement(*statement);

            FunctionData* p = mData.getLastFunctionRef();
            if (0 != p) {
                CallData& call = p->GetCall();
                switch (p->GetExtentClass()) {
                case FunctionData::EXTENT_CLASS_NOTHING:
                {
                    if (call.GetParamClass() == CallData::PARAM_CLASS_OPERATOR && !statement->IsValid())
                        return;//操作符就不支持空参数了

                    //函数参数，允许空语句，用于表达默认状态(副作用是a()与a[]将总是会有一个空语句参数)。
                    if (statementSyntax.IsValid()) {
                        wrapObjectMember(statementSyntax);
                        call.AddParam(&statementSyntax);
                    }

                }
                break;
                case FunctionData::EXTENT_CLASS_STATEMENT:
                {
                    if (!statement->IsValid()) {
                        //_epsilon_表达式无语句语义
                        return;
                    }
                    //函数扩展语句部分
                    wrapObjectMember(statementSyntax);
                    p->AddStatement(&statementSyntax);
                }
                break;
                }
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::beginFunction(void)
    {
        if (!preconditionCheck())return;
        StatementData* statement = mData.getCurStatement();
        if (0 != statement) {
            FunctionData* newFunc = mInterpreter->AddNewFunctionComponent();
            if (0 != newFunc) {
                statement->AddFunction(newFunc);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::setFunctionId(void)
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            if (RuntimeBuilderData::STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%s\n", tokenInfo.mString);
            }
            else if (RuntimeBuilderData::FLOAT_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%f\n", tokenInfo.mFloat);
            }
            else if (RuntimeBuilderData::BOOL_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%d\n", tokenInfo.mBool);
            }
            else {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%d\n", tokenInfo.mInteger);
            }
        }

        FunctionData* p = mData.getLastFunctionRef();
        if (0 != p && !p->IsValid()) {
            Value val = tokenInfo.ToValue();
            if (FALSE == val.IsInvalid()) {
                val.SetLine(mThis->getLastLineNumber());
                p->GetCall().SetNameValue(val);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::setMemberId(void)
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            if (RuntimeBuilderData::STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%s\n", tokenInfo.mString);
            }
            else if (RuntimeBuilderData::FLOAT_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%f\n", tokenInfo.mFloat);
            }
            else if (RuntimeBuilderData::BOOL_TOKEN == tokenInfo.mType) {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%f\n", tokenInfo.mBool);
            }
            else {
                PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%d\n", tokenInfo.mInteger);
            }
        }

        FunctionData* p = mData.getLastFunctionRef();
        if (0 != p && !p->IsValid()) {
            Value val = tokenInfo.ToValue();
            if (FALSE == val.IsInvalid()) {
                //成员名需要转成字符串常量
                if (val.IsIdentifier() && val.GetString()) {
                    val.SetWeakRefString(val.GetString());
                }
                val.SetLine(mThis->getLastLineNumber());
                p->GetCall().SetNameValue(val);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::endFunction(void)
    {
        if (!preconditionCheck())return;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildHighOrderFunction(void)
    {
        if (!preconditionCheck())return;
        //高阶函数构造（当前函数返回一个函数）
        FunctionData*& p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData* newP = mInterpreter->AddNewFunctionComponent();
        if (0 != newP) {
            CallData& call = newP->GetCall();
            call.ClearParams();
            newP->ClearStatements();
            Value val(&p->GetCall());
            val.SetLine(p->GetLine());
            call.SetNameValue(val);
            p = newP;
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->GetCall().SetParamClass(CallData::PARAM_CLASS_PARENTHESIS);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->GetCall().SetParamClass(CallData::PARAM_CLASS_BRACKET);
        wrapObjectMemberInHighOrderFunction(p->GetCall());
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriod(void)
    {
        if (!preconditionCheck())return;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_PERIOD);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_PERIOD_PARENTHESIS);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodBracketParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_PERIOD_BRACKET);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodBraceParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_PERIOD_BRACE);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestion(void)
    {
        if (!preconditionCheck())return;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionPeriodParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_QUESTION_PERIOD);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_QUESTION_PARENTHESIS);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionBracketParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_QUESTION_BRACKET);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionBraceParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_QUESTION_BRACE);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointer(void)
    {
        if (!preconditionCheck())return;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointerParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_POINTER);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodStarParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_PERIOD_STAR);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionPeriodStarParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_QUESTION_PERIOD_STAR);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointerStarParam(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        CallData& call = p->GetCall();
        call.SetParamClass(CallData::PARAM_CLASS_POINTER_STAR);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markHaveStatement(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->SetExtentClass(FunctionData::EXTENT_CLASS_STATEMENT);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markHaveExternScript(void)
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->SetExtentClass(FunctionData::EXTENT_CLASS_EXTERN_SCRIPT);
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMember(ISyntaxComponent& comp)
    {
        int ret = FALSE;
        if (comp.GetSyntaxType() != ISyntaxComponent::TYPE_CALL)
            return ret;
        CallData& call = dynamic_cast<CallData&>(comp);
        if (0 != mInterpreter && call.IsValid() && call.HaveId()) {
            if (call.GetParamClass() == CallData::PARAM_CLASS_PERIOD ||
                call.GetParamClass() == CallData::PARAM_CLASS_BRACKET ||
                call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_BRACE ||
                call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_BRACKET ||
                call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_PARENTHESIS ||
                call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PERIOD ||
                call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PARENTHESIS ||
                call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_BRACKET ||
                call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_BRACE ||
                call.GetParamClass() == CallData::PARAM_CLASS_POINTER ||
                call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_STAR ||
                call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PERIOD_STAR ||
                call.GetParamClass() == CallData::PARAM_CLASS_POINTER_STAR) {
                //包装对象成员:（注意这里传进来的是obj.property，所以只需要转变成高阶形式即可）
                //	obj.property=val -> obj.property(val)
                //	val=obj.property -> val=obj.property()
                CallData* pNew = mInterpreter->AddNewCallComponent();
                if (0 != pNew) {
                    //构造一个新的obj.property
                    int paramClass = call.GetParamClass();
                    pNew->SetParamClass(paramClass);
                    pNew->SetNameValue(call.GetNameValue());
                    pNew->AddParam(call.GetParam(0));
                    //将旧的call变成高阶形式
                    Value newCall(pNew);
                    call.SetNameValue(newCall);
                    call.SetParamClass(CallData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                    call.ClearParams();
                    ret = TRUE;
                }
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMemberInHighOrderFunction(CallData& arg)
    {
        int ret = FALSE;
        if (0 != mInterpreter && arg.IsValid() && arg.IsHighOrder()) {
            //包装对象成员:
            //	obj.property. -> obj.property().
            const Value& hoCall = arg.GetNameValue();
            CallData* pCall = hoCall.GetCallData();
            if (NULL != pCall) {
                CallData& call = *pCall;
                if (call.GetParamClass() == CallData::PARAM_CLASS_PERIOD ||
                    call.GetParamClass() == CallData::PARAM_CLASS_BRACKET ||
                    call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_BRACE ||
                    call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_BRACKET ||
                    call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_PARENTHESIS ||
                    call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PERIOD ||
                    call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PARENTHESIS ||
                    call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_BRACKET ||
                    call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_BRACE ||
                    call.GetParamClass() == CallData::PARAM_CLASS_POINTER ||
                    call.GetParamClass() == CallData::PARAM_CLASS_PERIOD_STAR ||
                    call.GetParamClass() == CallData::PARAM_CLASS_QUESTION_PERIOD_STAR ||
                    call.GetParamClass() == CallData::PARAM_CLASS_POINTER_STAR) {
                    CallData* pNew = mInterpreter->AddNewCallComponent();
                    if (0 != pNew) {
                        int paramClass = call.GetParamClass();
                        pNew->SetNameValue(hoCall);
                        pNew->SetParamClass(CallData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                        Value newHoCall(pNew);
                        arg.SetNameValue(newHoCall);
                        ret = TRUE;
                    }
                }
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMember(StatementData& arg)
    {
        //这个函数用于无法先化简的情形
        int ret = FALSE;
        if (0 != mInterpreter && arg.IsValid() && 1 == arg.GetFunctionNum()) {
            FunctionData* p = arg.GetFunction(0);
            if (0 != p && p->HaveId()) {
                CallData& call = p->GetCall();
                ret = wrapObjectMember(call);
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(StatementData& data)const
    {
        int num = data.GetFunctionNum();
        //对语句进行化简（语法分析过程中为了方便，全部按完整StatementData来构造，这里化简为原来的类型：ValueData/CallData/FunctionData等，主要涉及参数与语句部分）
        if (num == 1) {
            //只有一个函数的语句退化为函数（再按函数进一步退化）。
            FunctionData& func = *data.GetFunction(0);
            return simplifyStatement(func);
        }
        return data;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(FunctionData& data)const
    {
        if (!data.HaveStatement() && !data.HaveExternScript()) {
            //没有语句部分的函数退化为函数调用（再按函数调用进一步退化）。
            CallData& call = data.GetCall();
            if (call.IsValid()) {
                return simplifyStatement(call);
            }
            else {
                //error
                return mInterpreter->GetNullSyntaxRef();
            }
        }
        return data;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(CallData& data)const
    {
        if (!data.HaveParam()) {
            //没有参数的调用退化为基本值数据
            if (data.IsHighOrder()) {
                //这种情况应该不会出现
                return data;
            }
            else {
                ValueData& name = data.GetName();
                return name;
            }
        }
        else if (NULL != data.GetId() && data.GetId()[0] == '-' && data.GetId()[1] == 0 && data.GetParamNum() == 1) {
            ISyntaxComponent& temp = *data.GetParam(0);
            if (temp.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                ValueData& nameOrVal = dynamic_cast<ValueData&>(temp);
                Value& val = nameOrVal.GetValue();
                char* pStr = val.GetString();
                if (0 != strchr(pStr, '.')) {
                    double v = atof(pStr);
                    val.SetDouble(-v);
                }
                else {
                    long long v = atoll(pStr);
                    val.SetInt64(-v);
                }
                return nameOrVal;
            }
            else {
                return data;
            }
        }
        else if (NULL != data.GetId() && data.GetId()[0] == '+' && data.GetId()[1] == 0 && data.GetParamNum() == 1) {
            ISyntaxComponent& temp = *data.GetParam(0);
            if (temp.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                ValueData& nameOrVal = dynamic_cast<ValueData&>(temp);
                Value& val = nameOrVal.GetValue();
                char* pStr = val.GetString();
                if (0 != strchr(pStr, '.')) {
                    double v = atof(pStr);
                    val.SetDouble(v);
                }
                else {
                    long long v = atoll(pStr);
                    val.SetInt64(v);
                }
                return nameOrVal;
            }
            else {
                return data;
            }
        }
        else {
            //有参数不会退化
            return data;
        }
    }
}
//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------