//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------

#define PRINT_FUNCTION_SCRIPT_DEBUG_INFO printf

namespace FunctionScript
{
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::setExternScript(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
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

        Statement* pArg = mData.getCurStatement();
        if (0 == pArg)
            return;
        int handled = FALSE;
        if (0 == strcmp("=", tokenInfo.mString)) {
            ISyntaxComponent& argComp = simplifyStatement(*pArg);
            handled = wrapObjectMember(argComp);
        }
        if (FALSE == handled) {
            mData.popStatement();
            ISyntaxComponent& argComp = simplifyStatement(*pArg);
            Statement* pStatement = mInterpreter->AddNewStatementComponent();
            if (0 == pStatement)
                return;
            mData.pushStatement(pStatement);

            Function* p = mInterpreter->AddNewFunctionComponent();
            if (0 != p) {
            	Call& call = p->GetCall();
                if (0 != tokenInfo.mString && tokenInfo.mString[0] == '`') {
                    call.SetParamClass(Call::PARAM_CLASS_OPERATOR);

                    Value v = call.GetName();
                    Value op(tokenInfo.mString + 1, Value::TYPE_IDENTIFIER);
                    op.SetLine(mThis->getLastLineNumber());
                    call.SetName(op);
                }
                else {
                    call.SetParamClass(Call::PARAM_CLASS_OPERATOR);

                    Value v = call.GetName();
                    Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
                    op.SetLine(mThis->getLastLineNumber());
                    call.SetName(op);
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

        Statement* pArg = mData.popStatement();
        if (0 == pArg)
            return;
        ISyntaxComponent& argComp = simplifyStatement(*pArg);
        Statement* pStatement = mInterpreter->AddNewStatementComponent();
        if (0 == pStatement)
            return;
        mData.pushStatement(pStatement);

        Function* p = mInterpreter->AddNewFunctionComponent();
        if (0 != p) {
            Call& call = p->GetCall();
            call.SetParamClass(Call::PARAM_CLASS_TERNARY_OPERATOR);
            p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);

            Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
            op.SetLine(mThis->getLastLineNumber());
            call.SetName(op);
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

        Statement* statement = mData.getCurStatement();
        if (0 != statement) {
            Function* p = mInterpreter->AddNewFunctionComponent();
            if (0 != p) {
                Call& call = p->GetCall();
                call.SetParamClass(Call::PARAM_CLASS_TERNARY_OPERATOR);
                p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);

                Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
                op.SetLine(mThis->getLastLineNumber());
                call.SetName(op);

                statement->AddFunction(p);
            }
        }
    }
    //--------------------------------------------------------------------------------------	
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::beginStatement(void)
    {
        if (!preconditionCheck())return;
        Statement* p = mInterpreter->AddNewStatementComponent();
        if (0 == p)
            return;
        mData.pushStatement(p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::endStatement(void)
    {
        if (!preconditionCheck())return;
        Statement* statement = mData.popStatement();
        if (0 == statement || statement->GetFunctionNum() == 0)
            return;
        const char* id = statement->GetId();
        if (0 != id && strcmp(id, "@@delimiter") == 0 && statement->GetFunctionNum() == 1 && (statement->GetLastFunctionRef()->GetCall().GetParamNum() == 1 || statement->GetLastFunctionRef()->GetCall().GetParamNum() == 3) && !statement->GetLastFunctionRef()->GetCall().IsHighOrder()) {
            const Call& call = statement->GetLastFunctionRef()->GetCall();
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
            //顶层元素结束
            wrapObjectMember(*statement);
            mInterpreter->AddStatement(statement);
            mThis->setCanFinish(TRUE);
        }
        else {
            //化简只需要处理一级，参数与语句部分应该在添加到语句时已经处理了
            ISyntaxComponent& statementSyntax = simplifyStatement(*statement);

            Function* p = mData.getLastFunctionRef();
            if (0 != p) {
                Call& call = p->GetCall();
                switch (p->GetExtentClass()) {
                case Function::EXTENT_CLASS_NOTHING:
                {
                    if (call.GetParamClass() == Call::PARAM_CLASS_OPERATOR && !statement->IsValid())
                    	return;//操作符就不支持空参数了

                    //函数参数，允许空语句，用于表达默认状态(副作用是a()与a[]将总是会有一个空语句参数)。
                    if (statementSyntax.IsValid()) {
			wrapObjectMember(statementSyntax);
                        call.AddParam(&statementSyntax);
                    }

                }
                break;
                case Function::EXTENT_CLASS_STATEMENT:
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
        Statement* statement = mData.getCurStatement();
        if (0 != statement) {
            Function* newFunc = mInterpreter->AddNewFunctionComponent();
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

        Function* p = mData.getLastFunctionRef();
        if (0 != p && !p->IsValid()) {
            Value val = tokenInfo.ToValue();
            if (FALSE == val.IsInvalid()) {
                val.SetLine(mThis->getLastLineNumber());
                p->GetCall().SetName(val);
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

        Function* p = mData.getLastFunctionRef();
        if (0 != p && !p->IsValid()) {
            Value val = tokenInfo.ToValue();
            if (FALSE == val.IsInvalid()) {
                //成员名需要转成字符串常量
                if (val.IsIdentifier() && val.GetString()) {
                    val.SetWeakRefString(val.GetString());
                }
                val.SetLine(mThis->getLastLineNumber());
                p->GetCall().SetName(val);
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
        Function*& p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Function* newP = mInterpreter->AddNewFunctionComponent();
        if (0 != newP) {
            Call& call = newP->GetCall();
            call.ClearParams();
            newP->ClearStatements();
            Value val(&p->GetCall());
            val.SetLine(p->GetLine());
            call.SetName(val);
            p = newP;
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->GetCall().SetParamClass(Call::PARAM_CLASS_PARENTHESIS);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->GetCall().SetParamClass(Call::PARAM_CLASS_BRACKET);
        wrapObjectMemberInHighOrderFunction(*p);
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
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_PERIOD);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_PERIOD_PARENTHESIS);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodBracketParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_PERIOD_BRACKET);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodBraceParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_PERIOD_BRACE);
        wrapObjectMemberInHighOrderFunction(*p);
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
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_QUESTION_PERIOD);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionParenthesisParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_QUESTION_PARENTHESIS);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionBracketParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_QUESTION_BRACKET);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionBraceParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_QUESTION_BRACE);
        wrapObjectMemberInHighOrderFunction(*p);
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
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_POINTER);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodStarParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_PERIOD_STAR);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markQuestionPeriodStarParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_QUESTION_PERIOD_STAR);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointerStarParam(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        Call& call = p->GetCall();
        call.SetParamClass(Call::PARAM_CLASS_POINTER_STAR);
        wrapObjectMemberInHighOrderFunction(*p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markHaveStatement(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markHaveExternScript(void)
    {
        if (!preconditionCheck())return;
        Function* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        p->SetExtentClass(Function::EXTENT_CLASS_EXTERN_SCRIPT);
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMember(ISyntaxComponent& comp)
    {
        int ret = FALSE;
        if (comp.GetSyntaxType() != ISyntaxComponent::TYPE_STATEMENT)
            return ret;
        Statement& arg = *dynamic_cast<Statement*>(&comp);
        if (0 != mInterpreter && arg.IsValid() && 1 == arg.GetFunctionNum()) {
            Function* p = arg.GetFunction(0);
            if (0 != p && p->HaveId()) {
                Call& call = p->GetCall();
                if (call.GetParamClass() == Call::PARAM_CLASS_PERIOD ||
                    call.GetParamClass() == Call::PARAM_CLASS_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_BRACE ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_PARENTHESIS ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PERIOD ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PARENTHESIS ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_BRACE ||
                    call.GetParamClass() == Call::PARAM_CLASS_POINTER ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_STAR ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PERIOD_STAR ||
                    call.GetParamClass() == Call::PARAM_CLASS_POINTER_STAR) {
                    //包装对象成员:
                    //	obj.property=val -> obj.property(val)
                    //	val=obj.property -> val=obj.property()
                    Function*& pFunc = arg.GetLastFunctionRef();
                    Function* pNew = mInterpreter->AddNewFunctionComponent();
                    if (0 != pNew) {
                        int paramClass = pFunc->GetCall().GetParamClass();
                        Value func(pFunc);
                        pFunc = pNew;
                        pFunc->GetCall().SetName(func);
                        pFunc->GetCall().SetParamClass(Call::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                        ret = TRUE;
                    }
                }
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMemberInHighOrderFunction(ISyntaxComponent& comp)
    {
        int ret = FALSE;
        if (comp.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
            return ret;
        Function& arg = *dynamic_cast<Function*>(&comp);
        if (0 != mInterpreter && arg.IsValid() && arg.GetCall().GetName().IsSyntaxComponent() && NULL != arg.GetCall().GetName().GetSyntaxComponent()) {
            //包装对象成员:
            //	obj.property. -> obj.property().
            const Value& func = arg.GetCall().GetName();
            Function* pFunc = dynamic_cast<Function*>(func.GetSyntaxComponent());
            if (NULL != pFunc) {
                Call& call = pFunc->GetCall();
                if (call.GetParamClass() == Call::PARAM_CLASS_PERIOD ||
                    call.GetParamClass() == Call::PARAM_CLASS_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_BRACE ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_PARENTHESIS ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PERIOD ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PARENTHESIS ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_BRACKET ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_BRACE ||
                    call.GetParamClass() == Call::PARAM_CLASS_POINTER ||
                    call.GetParamClass() == Call::PARAM_CLASS_PERIOD_STAR ||
                    call.GetParamClass() == Call::PARAM_CLASS_QUESTION_PERIOD_STAR ||
                    call.GetParamClass() == Call::PARAM_CLASS_POINTER_STAR) {
                    Function* pNew = mInterpreter->AddNewFunctionComponent();
                    if (0 != pNew) {
                        int paramClass = call.GetParamClass();
                        call.SetName(func);
                        pNew->GetCall().SetParamClass(Call::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                        Value newFunc(pNew);
                        arg.GetCall().SetName(newFunc);
                        ret = TRUE;
                    }
                }
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(Statement& data)const
    {
        int num = data.GetFunctionNum();
        //对语句进行化简（语法分析过程中为了方便，全部按完整StatementData来构造，这里化简为原来的类型：ValueData/CallData/FunctionData等，主要涉及参数与语句部分）
        if (num == 1) {
            //只有一个函数的语句退化为函数（再按函数进一步退化）。
            Function& func = *data.GetFunction(0);
            return simplifyStatement(func);
        }
        return data;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(Function& data)const
    {
        if (!data.HaveStatement() && !data.HaveExternScript()) {
            //没有语句部分的函数退化为函数调用（再按函数调用进一步退化）。
            Call& call = data.GetCall();
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
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(Call& data)const
    {
        if (!data.HaveParam()) {
            //没有参数的调用退化为基本值数据
            if (data.IsHighOrder()) {
                //这种情况应该不会出现
                return data;
            }
            else {
                NameOrValue& name = data.GetNameOrValue();
                return name;
            }
        }
        else if (NULL != data.GetId() && data.GetId()[0] == '-' && data.GetParamNum() == 1) {
            ISyntaxComponent& temp = *data.GetParam(0);
            if (temp.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                NameOrValue& nameOrVal = dynamic_cast<NameOrValue&>(temp);
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
        else if (NULL != data.GetId() && data.GetId()[0] == '+' && data.GetParamNum() == 1) {
            ISyntaxComponent& temp = *data.GetParam(0);
            if (temp.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                NameOrValue& nameOrVal = dynamic_cast<NameOrValue&>(temp);
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