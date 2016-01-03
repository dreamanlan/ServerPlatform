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
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0!=p)
		{
			char* pStr=mThis->getLastToken();
			if(0!=pStr)
			{
				if(mInterpreter->IsDebugInfoEnable())
				{
					PRINT_FUNCTION_SCRIPT_DEBUG_INFO("script:%s\n",pStr);
				}

				p->SetExternScript(pStr);
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::buildOperator(void)
	{
		if(!preconditionCheck())return;
		RuntimeBuilderData::TokenInfo tokenInfo=mData.pop();
		if(TRUE!=tokenInfo.IsValid())return;

		if(mInterpreter->IsDebugInfoEnable())
		{
			PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op:%s\n",tokenInfo.mString);
		}

		Statement* pArg=mData.getCurStatement();
		if(0==pArg)
			return;
		int handled=FALSE;
    if(0==strcmp("=",tokenInfo.mString))
		{
			handled=wrapObjectMember(*pArg);
		}
		if(FALSE==handled)
		{
			mData.popStatement();
			Statement* pStatement=mInterpreter->AddNewStatementComponent();
			if(0==pStatement)
				return;
			pStatement->SetLine(mThis->getLastLineNumber());
			mData.pushStatement(pStatement);

			Function* p=mInterpreter->AddNewFunctionComponent();
			if(0!=p)
			{
				p->SetParamClass(Function::PARAM_CLASS_OPERATOR);

				Value v=p->GetName();
				Value op(tokenInfo.mString,Value::TYPE_VARIABLE_NAME);
				p->SetName(op);
				if(pArg->IsValid())
				{
					wrapObjectMember(*pArg);
					p->AddParam(pArg);
				}

				pStatement->AddFunction(p);
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::buildFirstTernaryOperator(void)
	{
		if(!preconditionCheck())return;
		RuntimeBuilderData::TokenInfo tokenInfo=mData.pop();
		if(TRUE!=tokenInfo.IsValid())return;

		if(mInterpreter->IsDebugInfoEnable())
		{
			PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op1/2:%d\n",tokenInfo.mInteger);
		}

		Statement* pArg=mData.popStatement();
		if(0==pArg)
			return;
		Statement* pStatement=mInterpreter->AddNewStatementComponent();
		if(0==pStatement)
			return;
		pStatement->SetLine(mThis->getLastLineNumber());
		mData.pushStatement(pStatement);

		Function* p=mInterpreter->AddNewFunctionComponent();
		if(0!=p)
		{
			p->SetParamClass(Function::PARAM_CLASS_TERNARY_OPERATOR);
			p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);

      Value op(tokenInfo.mString,Value::TYPE_VARIABLE_NAME);
			p->SetName(op);
			wrapObjectMember(*pArg);
			p->AddParam(pArg);

			pStatement->AddFunction(p);
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::buildSecondTernaryOperator(void)
	{
		if(!preconditionCheck())return;
		RuntimeBuilderData::TokenInfo tokenInfo=mData.pop();
		if(TRUE!=tokenInfo.IsValid())return;

		if(mInterpreter->IsDebugInfoEnable())
		{
			PRINT_FUNCTION_SCRIPT_DEBUG_INFO("op2/2:%d\n",tokenInfo.mInteger);
		}

		Statement* statement=mData.getCurStatement();
		if(0!=statement)
		{
			Function* p=mInterpreter->AddNewFunctionComponent();
			if(0!=p)
			{
				p->SetParamClass(Function::PARAM_CLASS_TERNARY_OPERATOR);
				p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);

        Value op(tokenInfo.mString,Value::TYPE_VARIABLE_NAME);
				p->SetName(op);

				statement->AddFunction(p);
			}
		}
	}
	//--------------------------------------------------------------------------------------	
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::beginStatement(void)
	{
		if(!preconditionCheck())return;
		Statement* p=mInterpreter->AddNewStatementComponent();
		if(0==p)
			return;
		p->SetLine(mThis->getLastLineNumber());
		mData.pushStatement(p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::endStatement(void)
	{
		if(!preconditionCheck())return;
		Statement* statement=mData.popStatement();
		if(0==statement)
			return;
		if(mData.isSemanticStackEmpty())
		{
			if(!statement->IsValid())
			{
				//_epsilon_表达式无语句语义
				return;
			}
			//顶层元素结束
			wrapObjectMember(*statement);
			mInterpreter->AddStatement(statement);
			mThis->setCanFinish(TRUE);
		}
		else
		{
			Function* p=mData.getLastFunctionRef();
			if(0!=p)
			{
				switch(p->GetExtentClass())
				{
				case Function::EXTENT_CLASS_NOTHING:
					{
						if(p->GetParamClass()==Function::PARAM_CLASS_OPERATOR && !statement->IsValid())
							return;//操作符就不支持空参数了

						//函数参数，允许空语句，用于表达默认状态(副作用是a()与a[]将总是会有一个空语句参数)。
						wrapObjectMember(*statement);
						p->AddParam(statement);
					}
					break;
				case Function::EXTENT_CLASS_STATEMENT:
					{
						if(!statement->IsValid())
						{
							//_epsilon_表达式无语句语义
							return;
						}
						//函数扩展语句部分
						wrapObjectMember(*statement);
						p->AddStatement(statement);
					}
					break;
				}
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::beginFunction(void)
	{
		if(!preconditionCheck())return;
		Statement* statement=mData.getCurStatement();
		if(0!=statement)
		{
			Function* newFunc=mInterpreter->AddNewFunctionComponent();
			if(0!=newFunc)
			{
				statement->AddFunction(newFunc);
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::setFunctionId(void)
	{
		if(!preconditionCheck())return;
		RuntimeBuilderData::TokenInfo tokenInfo=mData.pop();
		if(TRUE!=tokenInfo.IsValid())return;

		if(mInterpreter->IsDebugInfoEnable())
		{
			if(RuntimeBuilderData::STRING_TOKEN==tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN==tokenInfo.mType)
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%s\n",tokenInfo.mString);
			}
			else if(RuntimeBuilderData::FLOAT_TOKEN==tokenInfo.mType)
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%f\n",tokenInfo.mFloat);
			}
			else
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("id:%d\n",tokenInfo.mInteger);
			}
		}

		Function* p=mData.getLastFunctionRef();
		if(0!=p && !p->IsValid())
		{
			Value val=tokenInfo.ToValue();
			if(FALSE==val.IsInvalid())
			{
				p->SetName(val);
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::setMemberId(void)
	{
		if(!preconditionCheck())return;
		RuntimeBuilderData::TokenInfo tokenInfo=mData.pop();
		if(TRUE!=tokenInfo.IsValid())return;

		if(mInterpreter->IsDebugInfoEnable())
		{
			if(RuntimeBuilderData::STRING_TOKEN==tokenInfo.mType || RuntimeBuilderData::VARIABLE_TOKEN==tokenInfo.mType)
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%s\n",tokenInfo.mString);
			}
			else if(RuntimeBuilderData::FLOAT_TOKEN==tokenInfo.mType)
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%f\n",tokenInfo.mFloat);
			}
			else
			{
				PRINT_FUNCTION_SCRIPT_DEBUG_INFO("member:%d\n",tokenInfo.mInteger);
			}
		}

		Function* p=mData.getLastFunctionRef();
		if(0!=p && !p->IsValid())
		{
			Value val=tokenInfo.ToValue();
			if(FALSE==val.IsInvalid())
			{
				//成员名需要转成字符串常量
				if(val.IsVariableName() && val.GetString())
				{
					val.SetWeakRefString(val.GetString());
				}
				p->SetName(val);
			}
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::endFunction(void)
	{
		if(!preconditionCheck())return;
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::buildHighOrderFunction(void)
	{
		if(!preconditionCheck())return;
		//高阶函数构造（当前函数返回一个函数）
		Function*& p=mData.getLastFunctionRef();
		if(0==p)
			return;
		Function* newP=mInterpreter->AddNewFunctionComponent();
		if(0!=newP)
		{
			newP->ClearParams();
			newP->ClearStatements();
			Value val(p);
			newP->SetName(val);
			p=newP;
		}
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markParenthesisParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_PARENTHESIS);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markBracketParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_BRACKET);
		wrapObjectMemberInHighOrderFunction(*p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markPeriodParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_PERIOD);
		wrapObjectMemberInHighOrderFunction(*p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markPeriodParenthesisParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_PERIOD_PARENTHESIS);
		wrapObjectMemberInHighOrderFunction(*p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markPeriodBracketParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_PERIOD_BRACKET);
		wrapObjectMemberInHighOrderFunction(*p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markPeriodBraceParam(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetParamClass(Function::PARAM_CLASS_PERIOD_BRACE);
		wrapObjectMemberInHighOrderFunction(*p);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markHaveStatement(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetExtentClass(Function::EXTENT_CLASS_STATEMENT);
	}
	template<class RealTypeT> inline 
		void RuntimeBuilderT<RealTypeT>::markHaveExternScript(void)
	{
		if(!preconditionCheck())return;
		Function* p=mData.getLastFunctionRef();
		if(0==p)
			return;
		p->SetExtentClass(Function::EXTENT_CLASS_EXTERN_SCRIPT);
	}
	template<class RealTypeT> inline 
		int RuntimeBuilderT<RealTypeT>::wrapObjectMember(Statement& arg)
	{
		int ret=FALSE;
		if(0!=mInterpreter && arg.IsValid() && 1==arg.GetFunctionNum())
		{
			Function* p=arg.GetFunction(0);
			if(0!=p && p->HaveName())
			{
				if(p->GetParamClass()==Function::PARAM_CLASS_PERIOD || 
					p->GetParamClass()==Function::PARAM_CLASS_BRACKET || 
					p->GetParamClass()==Function::PARAM_CLASS_PERIOD_BRACE || 
					p->GetParamClass()==Function::PARAM_CLASS_PERIOD_BRACKET || 
					p->GetParamClass()==Function::PARAM_CLASS_PERIOD_PARENTHESIS)
				{
					//包装对象成员:
					//	obj.property=val -> obj.property(val)
					//	val=obj.property -> val=obj.property()
					Function*& pFunc=arg.GetLastFunctionRef();
					Function* pNew=mInterpreter->AddNewFunctionComponent();
					if(0!=pNew)
					{
						int paramClass=pFunc->GetParamClass();
						Value func(pFunc);
						pFunc=pNew;
						pFunc->SetName(func);
						pFunc->SetParamClass(Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK|paramClass);
						ret=TRUE;
					}
				}
			}
		}
		return ret;
	}	
	template<class RealTypeT> inline 
		int RuntimeBuilderT<RealTypeT>::wrapObjectMemberInHighOrderFunction(Function& arg)
	{
		int ret=FALSE;
		if(0!=mInterpreter && arg.IsValid() && arg.GetName().IsFunction() && NULL!=arg.GetName().GetFunction())
		{
			//包装对象成员:
			//	obj.property. -> obj.property().
			const Value& func=arg.GetName();
			Function* pFunc=func.GetFunction();
			if(NULL!=pFunc)
			{
				if(pFunc->GetParamClass()==Function::PARAM_CLASS_PERIOD || 
					pFunc->GetParamClass()==Function::PARAM_CLASS_BRACKET || 
					pFunc->GetParamClass()==Function::PARAM_CLASS_PERIOD_BRACE || 
					pFunc->GetParamClass()==Function::PARAM_CLASS_PERIOD_BRACKET || 
					pFunc->GetParamClass()==Function::PARAM_CLASS_PERIOD_PARENTHESIS)
				{
					Function* pNew=mInterpreter->AddNewFunctionComponent();
					if(0!=pNew)
					{
						int paramClass=pFunc->GetParamClass();
						pNew->SetName(func);
						pNew->SetParamClass(Function::PARAM_CLASS_WRAP_OBJECT_MEMBER_MASK|paramClass);
						Value newFunc(pNew);
						arg.SetName(newFunc);
						ret=TRUE;
					}
				}
			}
		}
		return ret;
	}
}
//--------------------------------------------------------------------------------------
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//--------------------------------------------------------------------------------------