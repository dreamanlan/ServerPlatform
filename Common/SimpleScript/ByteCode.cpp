#include "ByteCode.h"
#include "calc.h"

class TestRuntimeBuilder : public RuntimeBuilderT<TestRuntimeBuilder>
{
	typedef RuntimeBuilderT<TestRuntimeBuilder> BaseType;
public:
	char* getLastToken(void) const{return "";}
	int getLastLineNumber(void) const{return 0;}
	void setCanFinish(int val){}
public:
	TestRuntimeBuilder(Interpreter& interpreter):BaseType(interpreter)
	{}
};

void CompileTest_ByteCode(void)
{
	Interpreter interpreter;
	TestRuntimeBuilder builder(interpreter);
	builder.beginFunction();
	builder.beginStatement();
	builder.buildFirstTernaryOperator();
	builder.buildHighOrderFunction();
	builder.buildOperator();
	builder.buildSecondTernaryOperator();
	builder.endFunction();
	builder.endStatement();
	builder.markBracketParam();
	builder.markHaveExternScript();
	builder.markHaveStatement();
	builder.markParenthesisParam();
	builder.markPeriodBraceParam();
	builder.markPeriodBracketParam();
	builder.markPeriodParam();
	builder.markPeriodParenthesisParam();
	builder.setMemberId();
	builder.setCanFinish(false);
	builder.setExternScript();
	builder.setFunctionId();
	builder.setMemberId();
}