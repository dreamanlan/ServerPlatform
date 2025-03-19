//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------

#define PRINT_FUNCTION_SCRIPT_DEBUG_INFO printf

namespace FunctionScript
{
    /*
    * Memo: Why use reduction instead of delayed one-time construction
    * 1. We have tried to use a temporary structure such as SyntaxMaterial to collect data during the syntax parsing process, and then construct the statement when the statement is completed.
    * 2. The temporary structure is very similar to the final semantic data structure. It also needs to represent the recursive structure and be associated with the existing semantic data. The code is repeated and the logic is not clear enough.
    * 3. The reduction method has tried its best to reuse the instances constructed in the grammar parsing, and basically does not cause additional memory usage.
    * 4. In the reduction mode, the final memory usage is linearly related to the script complexity, so there is no need to worry about taking up too much memory.
    * 5. The definition of semantic data takes degradation situations into consideration and tries not to occupy additional space except for necessary data.
    */
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::setExternScript()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 != p) {
            char* pStr = mThis->getLastToken();
            if (0 != pStr) {
                if (mInterpreter->IsDebugInfoEnable()) {
                    PRINT_FUNCTION_SCRIPT_DEBUG_INFO("script:%s\n", pStr);
                }

                ValueData* pVal = mInterpreter->AddNewValueComponent();
                pVal->GetValue().SetIdentifier(pStr);
                p->AddParam(pVal);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildOperator()
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
            //Because the stack has not been popped here, we cannot simplify it first.
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
                FunctionData& call = *p;
                if (0 != tokenInfo.mString && tokenInfo.mString[0] == '`') {
                    call.SetParamClass(FunctionData::PARAM_CLASS_OPERATOR);

                    Value v = call.GetNameValue();
                    Value op(tokenInfo.mString + 1, Value::TYPE_IDENTIFIER);
                    op.SetLine(mThis->getLastLineNumber());
                    call.SetNameValue(op);
                }
                else {
                    call.SetParamClass(FunctionData::PARAM_CLASS_OPERATOR);

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
        void RuntimeBuilderT<RealTypeT>::buildFirstTernaryOperator()
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
        pushPairType(FunctionData::PAIR_TYPE_QUESTION_COLON);

        FunctionData* p = mInterpreter->AddNewFunctionComponent();
        if (0 != p) {
            //The ternary operator is expressed as op1(cond)(true_val)op2(false_val)
            FunctionData* lowerOrderFunction = mInterpreter->AddNewFunctionComponent();
            p->GetNameValue().SetFunction(lowerOrderFunction);
            p->SetParamClass(FunctionData::PARAM_CLASS_TERNARY_OPERATOR);
            lowerOrderFunction->SetParamClass(FunctionData::PARAM_CLASS_PARENTHESIS);

            Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
            op.SetLine(mThis->getLastLineNumber());
            lowerOrderFunction->SetNameValue(op);
            if (argComp.IsValid()) {
                wrapObjectMember(argComp);
                lowerOrderFunction->AddParam(&argComp);
            }

            pStatement->AddFunction(p);
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildSecondTernaryOperator()
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op2/2:%d\n", tokenInfo.mInteger);
        }

        StatementData* statement = mData.getCurStatement();
        if (0 != statement) {
            popPairType();

            FunctionData* p = mInterpreter->AddNewFunctionComponent();
            if (0 != p) {
                FunctionData& call = *p;
                call.SetParamClass(FunctionData::PARAM_CLASS_TERNARY_OPERATOR);

                Value op(tokenInfo.mString, Value::TYPE_IDENTIFIER);
                op.SetLine(mThis->getLastLineNumber());
                call.SetNameValue(op);

                statement->AddFunction(p);
            }
        }
    }
    //--------------------------------------------------------------------------------------	
    static inline int calcSeparator(const char* tok)
    {
        if (nullptr == tok)
            return ISyntaxComponent::SEPARATOR_NOTHING;
        else if (tok[0] == ',' && tok[1] == '\0')
            return ISyntaxComponent::SEPARATOR_COMMA;
        else if (tok[0] == ';' && tok[1] == '\0')
            return ISyntaxComponent::SEPARATOR_SEMICOLON;
        else
            return ISyntaxComponent::SEPARATOR_NOTHING;
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markSeparator()
    {
        if (!preconditionCheck())return;

        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op2/2:%d\n", tokenInfo.mInteger);
        }

        if (mData.isSemanticStackEmpty()) {
            int stmNum = mInterpreter->GetStatementNum();
            if (stmNum > 0) {
                auto* pStm = mInterpreter->GetStatement(stmNum - 1);
                if (0 != pStm) {
                    pStm->SetSeparator(calcSeparator(tokenInfo.mString));
                }
            }
        }
        else {
            FunctionData* p = mData.getLastFunctionRef();
            if (0 != p) {
                int paramNum = p->GetParamNum();
                if (paramNum > 0) {
                    auto* pParam = p->GetParam(paramNum - 1);
                    if (nullptr != pParam) {
                        pParam->SetSeparator(calcSeparator(tokenInfo.mString));
                    }
                }
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::beginStatement()
    {
        if (!preconditionCheck())return;
        StatementData* p = mInterpreter->AddNewStatementComponent();
        if (0 == p)
            return;
        mData.pushStatement(p);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::endStatement()
    {
        if (!preconditionCheck())return;
        StatementData* statement = mData.popStatement();
        if (0 == statement || statement->GetFunctionNum() == 0)
            return;

        const char* id = statement->GetId();
        if (0 != id && strcmp(id, "@@delimiter") == 0 && statement->GetFunctionNum() == 1 && (statement->GetLastFunctionRef()->GetParamNum() == 1 || statement->GetLastFunctionRef()->GetParamNum() == 3) && !statement->GetLastFunctionRef()->IsHighOrder()) {
            const FunctionData& call = *statement->GetLastFunctionRef();
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
                //_epsilon_expression has no statement semantics
                return;
            }
            //Simplification only needs to be processed at one level, and the parameters and statement parts should have been processed when they are added to the statement.
            ISyntaxComponent& statementSyntax = simplifyStatement(*statement);
            //End of top-level element
            wrapObjectMember(statementSyntax);
            mInterpreter->AddStatement(&statementSyntax);
            mThis->setCanFinish(TRUE);
        }
        else {
            //Simplification only needs to be processed at one level, and the parameters and statement parts should have been processed when they are added to the statement.
            ISyntaxComponent& statementSyntax = simplifyStatement(*statement);

            FunctionData* p = mData.getLastFunctionRef();
            if (0 != p) {
                FunctionData& call = *p;
                if (call.HaveParam()) {
                    if ((call.GetParamClass() == FunctionData::PARAM_CLASS_OPERATOR
                        || call.GetParamClass() == FunctionData::PARAM_CLASS_QUESTION_NULLABLE_OPERATOR
                        || call.GetParamClass() == FunctionData::PARAM_CLASS_EXCLAMATION_NULLABLE_OPERATOR
                        || call.GetParamClass() == FunctionData::PARAM_CLASS_TERNARY_OPERATOR
                        ) && !statement->IsValid())
                        return;//The operator does not support empty parameters.

                    //Function parameters, allowing empty statements, are used to express the default state (a side effect is that a() and a[] will always have an empty statement parameter).
                    if (statementSyntax.IsValid()) {
                        wrapObjectMember(statementSyntax);
                        call.AddParam(&statementSyntax);

                        if (call.GetParamClass() == FunctionData::PARAM_CLASS_PERIOD && ISyntaxComponent::TYPE_VALUE==statementSyntax.GetSyntaxType()) {
                            //Member names need to be converted into string constants
                            auto&& vd = static_cast<ValueData&>(statementSyntax);
                            auto&& val = vd.GetValue();
                            if (val.IsIdentifier() && val.GetString()) {
                                val.SetWeakRefString(val.GetString());
                            }
                        }
                    }

                }
                else if (call.HaveStatement()) {
                    if (!statement->IsValid()) {
                        //_epsilon_expression has no statement semantics
                        return;
                    }
                    //Function expansion statement part
                    wrapObjectMember(statementSyntax);
                    call.AddParam(&statementSyntax);
                }
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::addFunction()
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
        void RuntimeBuilderT<RealTypeT>::setFunctionId()
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            if (RuntimeBuilderData::STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::DOLLAR_STRING_TOKEN == tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN == tokenInfo.mType) {
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
                p->SetNameValue(val);
            }
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildNullableOperator()
    {
        if (!preconditionCheck())return;
        RuntimeBuilderData::TokenInfo tokenInfo = mData.pop();
        if (TRUE != tokenInfo.IsValid())return;

        if (mInterpreter->IsDebugInfoEnable()) {
            PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op:%s\n", tokenInfo.mString);
        }

        //Higher-order function construction (the current function returns a function)
        FunctionData*& p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        if (tokenInfo.mString[0] == '?')
            p->SetParamClass(FunctionData::PARAM_CLASS_QUESTION_NULLABLE_OPERATOR);
        else
            p->SetParamClass(FunctionData::PARAM_CLASS_EXCLAMATION_NULLABLE_OPERATOR);

        FunctionData* newP = mInterpreter->AddNewFunctionComponent();
        if (0 != newP) {
            Value val(p);
            val.SetLine(p->GetLine());
            newP->SetNameValue(val);
            p = newP;
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::buildHighOrderFunction()
    {
        if (!preconditionCheck())return;
        //Higher-order function construction (the current function returns a function)
        FunctionData*& p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData* newP = mInterpreter->AddNewFunctionComponent();
        if (0 != newP) {
            Value val(p);
            val.SetLine(p->GetLine());
            newP->SetNameValue(val);
            p = newP;
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;

        call.SetParamClass(FunctionData::PARAM_CLASS_PARENTHESIS);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_PARENTHESIS, tag);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;

        call.SetParamClass(FunctionData::PARAM_CLASS_BRACKET);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_BRACKET, tag);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_PERIOD);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markStatement()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_STATEMENT);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_BRACE, tag);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markStatementEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markExternScript()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_EXTERN_SCRIPT);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketColonParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_BRACKET_COLON);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_BRACKET_COLON, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketColonParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisColonParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_PARENTHESIS_COLON);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_PARENTHESIS_COLON, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisColonParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markAngleBracketColonParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_ANGLE_BRACKET_COLON);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_ANGLE_BRACKET_COLON, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markAngleBracketColonParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracePercentParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_BRACE_PERCENT);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_BRACE_PERCENT, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracePercentParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketPercentParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_BRACKET_PERCENT);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_BRACKET_PERCENT, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markBracketPercentParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisPercentParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_PARENTHESIS_PERCENT);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_PARENTHESIS_PERCENT, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markParenthesisPercentParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markAngleBracketPercentParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_ANGLE_BRACKET_PERCENT);
        auto&& pID = call.GetId();
        uint32_t tag = 0;
        if (pID) {
            auto&& tags = mInterpreter->NameTagsRef();
            auto&& tagIt = tags.find(pID);
            if (tagIt != tags.end()) {
                tag = tagIt->second;
            }
        }
        pushPairType(FunctionData::PAIR_TYPE_ANGLE_BRACKET_PERCENT, tag);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markAngleBracketPercentParamEnd()
    {
        popPairType();
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markColonColonParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_COLON_COLON);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointerParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_POINTER);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPeriodStarParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_PERIOD_STAR);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::markPointerStarParam()
    {
        if (!preconditionCheck())return;
        FunctionData* p = mData.getLastFunctionRef();
        if (0 == p)
            return;
        FunctionData& call = *p;
        call.SetParamClass(FunctionData::PARAM_CLASS_POINTER_STAR);
        wrapObjectMemberInHighOrderFunction(call);
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMember(ISyntaxComponent& comp)
    {
        int ret = FALSE;
        if (comp.GetSyntaxType() != ISyntaxComponent::TYPE_FUNCTION)
            return ret;
        FunctionData& call = dynamic_cast<FunctionData&>(comp);
        if (0 != mInterpreter && call.IsValid() && call.HaveId()) {
            if (call.GetParamClass() == FunctionData::PARAM_CLASS_PERIOD ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_PARENTHESIS_COLON ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET_COLON ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_ANGLE_BRACKET_COLON ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_PARENTHESIS_PERCENT ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET_PERCENT ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_BRACE_PERCENT ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_ANGLE_BRACKET_PERCENT ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_COLON_COLON ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_POINTER ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_PERIOD_STAR ||
                call.GetParamClass() == FunctionData::PARAM_CLASS_POINTER_STAR) {
                //Packaging object members: (Note that obj.property is passed in here, so it only needs to be converted into a higher-order form)
                //	obj.property=val -> obj.property(val)
                //	val=obj.property -> val=obj.property()
                FunctionData* pNew = mInterpreter->AddNewFunctionComponent();
                if (0 != pNew) {
                    //Construct a new obj.property
                    int paramClass = call.GetParamClass();
                    pNew->SetParamClass(paramClass);
                    pNew->SetNameValue(call.GetNameValue());
                    pNew->AddParam(call.GetParam(0));
                    //Convert old calls into higher-order forms
                    Value newCall(pNew);
                    call.SetNameValue(newCall);
                    call.SetParamClass(FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                    call.ClearParams();
                    ret = TRUE;
                }
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::wrapObjectMemberInHighOrderFunction(FunctionData& arg)
    {
        int ret = FALSE;
        if (0 != mInterpreter && arg.IsValid() && arg.IsHighOrder()) {
            //Wrapper object members:
            //	obj.property. -> obj.property().
            const Value& loCall = arg.GetNameValue();
            FunctionData* pCall = loCall.GetFunction();
            if (NULL != pCall) {
                FunctionData& call = *pCall;
                if (call.GetParamClass() == FunctionData::PARAM_CLASS_PERIOD ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_PARENTHESIS_COLON ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET_COLON ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_ANGLE_BRACKET_COLON ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_PARENTHESIS_PERCENT ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_BRACKET_PERCENT ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_BRACE_PERCENT ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_ANGLE_BRACKET_PERCENT ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_COLON_COLON ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_POINTER ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_PERIOD_STAR ||
                    call.GetParamClass() == FunctionData::PARAM_CLASS_POINTER_STAR) {
                    FunctionData* pNew = mInterpreter->AddNewFunctionComponent();
                    if (0 != pNew) {
                        int paramClass = call.GetParamClass();
                        pNew->SetNameValue(loCall);
                        pNew->SetParamClass(FunctionData::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK | paramClass);
                        Value newLoCall(pNew);
                        arg.SetNameValue(newLoCall);
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
        //This function is used when it is not possible to simplify first
        int ret = FALSE;
        if (0 != mInterpreter && arg.IsValid() && 1 == arg.GetFunctionNum()) {
            FunctionData* p = arg.GetFunction(0);
            if (0 != p && p->HaveId()) {
                FunctionData& call = *p;
                ret = wrapObjectMember(call);
            }
        }
        return ret;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(StatementData& data)const
    {
        int num = data.GetFunctionNum();
        //Simplify the statements (for convenience during the syntax analysis process,
        // all are constructed according to the complete StatementData. Here they are
        // simplified to the original types: ValueData/FunctionData/FunctionData, etc.
        // mainly involving parameters and statement parts)
        if (num == 1) {
            //A statement with only one function degenerates into a function (and then further degenerates by function).
            FunctionData& func = *data.GetFunction(0);
            return simplifyStatement(func);
        }
        return data;
    }
    template<class RealTypeT> inline
        ISyntaxComponent& RuntimeBuilderT<RealTypeT>::simplifyStatement(FunctionData& data)const
    {
        if (!data.HaveParamOrStatement()) {
            //Calls without parameters degenerate to basic value data
            if (data.IsHighOrder()) {
                //This should not happen
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
                switch (val.GetType()) {
                case Value::TYPE_FLOAT:
                    val.SetFloat(-val.GetFloat());
                    return nameOrVal;
                case Value::TYPE_DOUBLE:
                    val.SetDouble(-val.GetDouble());
                    return nameOrVal;
                case Value::TYPE_INT:
                    val.SetInt(-val.GetInt());
                    return nameOrVal;
                case Value::TYPE_INT64:
                    val.SetInt64(-val.GetInt64());
                    return nameOrVal;
                default:
                    return data;
                }
            }
            else {
                return data;
            }
        }
        else if (NULL != data.GetId() && data.GetId()[0] == '+' && data.GetId()[1] == 0 && data.GetParamNum() == 1) {
            ISyntaxComponent& temp = *data.GetParam(0);
            if (temp.GetSyntaxType() == ISyntaxComponent::TYPE_VALUE) {
                ValueData& nameOrVal = dynamic_cast<ValueData&>(temp);
                //keep value.
                return nameOrVal;
            }
            else {
                return data;
            }
        }
        else {
            //There are parameters that will not degrade
            return data;
        }
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::peekPairTypeStack(uint32_t& tag)const
    {
        uint32_t v =  mData.peekPairType();
        tag = (v >> 8);
        return static_cast<int>(v & 0xff);
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::getPairTypeStackSize()const
    {
        return mData.getPairTypeStack().Size();
    }
    template<class RealTypeT> inline
        int RuntimeBuilderT<RealTypeT>::peekPairTypeStack(int ix, uint32_t& tag)const
    {
        if (ix >= 0 && ix < getPairTypeStackSize()) {
            auto&& stack = mData.getPairTypeStack();
            int id = stack.BackID();
            for (int i = 0; i < ix; ++i) {
                id = stack.PrevID(id);
            }
            uint32_t v = stack[id];
            tag = (v >> 8);
            return static_cast<int>(v & 0xff);
        }
        else {
            tag = 0;
            return FunctionData::PAIR_TYPE_NONE;
        }
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::pushPairType(int type, uint32_t tag)
    {
        uint32_t v = (tag << 8) + static_cast<uint32_t>(type & 0xff);
        mData.pushPairType(v);
    }
    template<class RealTypeT> inline
        void RuntimeBuilderT<RealTypeT>::popPairType()
    {
        mData.popPairType();
    }
}
//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------