#include "RuntimeBuilderData.h"
#include "calc.h"

#define BYTECODE_PRINTF	nullprintf

//--------------------------------------------------------------------------------------
FunctionScript::Value RuntimeBuilderData::TokenInfo::ToValue(void)const
{
	switch(mType)
	{
	case VARIABLE_TOKEN:
		{
			char* pStr=mString;
			if(0!=pStr)
			{
				Value val(pStr,Value::TYPE_VARIABLE_NAME);
				return val;
			}
		}
		break;
	case STRING_TOKEN:
		{
			char* pStr=mString;
			if(0!=pStr)
			{
				Value val(pStr,Value::TYPE_STRING);
				return val;
			}
		}
		break;
	case INT_TOKEN:
		{
			Value val(mInteger);
			return val;
		}
		break;
	case FLOAT_TOKEN:
		{
			Value val(mFloat);
			return val;
		}
		break;
	}
	return Value();
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::push(const TokenInfo& info)
{
	mTokenStack.PushBack(info);
}
RuntimeBuilderData::TokenInfo RuntimeBuilderData::pop(void)
{
	if(mTokenStack.Empty())
		return TokenInfo();
	TokenInfo info=mTokenStack.Back();
	mTokenStack.PopBack();
	return info;
}
int RuntimeBuilderData::isSemanticStackEmpty(void)const
{
	return mSemanticStack.Empty();
}
void RuntimeBuilderData::pushStatement(FunctionScript::Statement* p)
{
	mSemanticStack.PushBack(p);
}
FunctionScript::Statement* RuntimeBuilderData::popStatement(void)
{
	if(mSemanticStack.Empty())
	{
		return 0;
	}
	Statement* cdata=mSemanticStack.Back();
	mSemanticStack.PopBack();
	return cdata;
}
FunctionScript::Statement* RuntimeBuilderData::getCurStatement(void)
{
	if(mSemanticStack.Empty())
		return 0;
	Statement* topData=mSemanticStack.Back();
	return topData;
}
FunctionScript::Function*& RuntimeBuilderData::getLastFunctionRef(void)
{
	Statement* statement=getCurStatement();
	if(0!=statement)
		return statement->GetLastFunctionRef();
	else
		return Function::GetNullFunctionPtrRef();
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::genByteCode(unsigned char data)
{
	BYTECODE_PRINTF("[\\x%2.2X]",data);
	if(mByteCodeLength+sizeof(data)<=s_c_MaxByteCodeLength)
	{
		*reinterpret_cast<unsigned char*>(&mByteCode[mByteCodeLength])=data;
		mByteCode[mByteCodeLength]=Encode(mByteCode[mByteCodeLength],__SERVERVERSION__);
		mByteCodeLength+=sizeof(data);
	}
}
void RuntimeBuilderData::genByteCode(unsigned short data)
{
	BYTECODE_PRINTF("[\\x%2.2X\\x%2.2X]",(data & 0xff),((data & 0xff00)>>8));
	if(mByteCodeLength+sizeof(data)<=s_c_MaxByteCodeLength)
	{
		*reinterpret_cast<unsigned short*>(&mByteCode[mByteCodeLength])=data;
		for(int i=mByteCodeLength;i<static_cast<int>(mByteCodeLength+sizeof(data));++i)
		{
			mByteCode[i]=Encode(mByteCode[i],__SERVERVERSION__);
		}
		mByteCodeLength+=sizeof(data);
	}
}
void RuntimeBuilderData::genByteCode(unsigned int data)
{
	BYTECODE_PRINTF("[\\x%2.2X\\x%2.2X\\x%2.2X\\x%2.2X]",(data & 0xff),((data & 0xff00)>>8),((data & 0xff0000)>>16),((data & 0xff000000)>>24));
	if(mByteCodeLength+sizeof(data)<=s_c_MaxByteCodeLength)
	{
		*reinterpret_cast<unsigned int*>(&mByteCode[mByteCodeLength])=data;
		for(int i=mByteCodeLength;i<static_cast<int>(mByteCodeLength+sizeof(data));++i)
		{
			mByteCode[i]=Encode(mByteCode[i],__SERVERVERSION__);
		}
		mByteCodeLength+=sizeof(data);
	}
}
void RuntimeBuilderData::genByteCode(const char* data)
{
	if(NULL==data)
		return;
	BYTECODE_PRINTF("[%s]",data);
	size_t len=strlen(data)+1;
	if(mByteCodeLength+len<=s_c_MaxByteCodeLength)
	{
		for(int i=mByteCodeLength;i<static_cast<int>(mByteCodeLength+static_cast<int>(len));++i)
		{
			mByteCode[i]=Encode(data[i-mByteCodeLength],__SERVERVERSION__);
		}
		mByteCodeLength+=(int)len;
	}
}
//--------------------------------------------------------------------------------------
void RuntimeBuilderData::resetByteCode(void)
{
	mByteCodeLength=0;
	memset(mByteCode,0,sizeof(mByteCode));
}
void RuntimeBuilderData::setByteCode(const char* pByteCode,int len)
{
	if(NULL!=pByteCode && len<=s_c_MaxByteCodeLength)
	{
		memcpy(mByteCode,pByteCode,len);
		mByteCodeLength=len;
	}
}
const char* RuntimeBuilderData::getByteCode(int& len)const
{
	len=mByteCodeLength;return mByteCode;
}
//--------------------------------------------------------------------------------------
RuntimeBuilderData::RuntimeBuilderData(void)
{
	resetByteCode();
}