#include "ByteCode.h"
#include "calc.h"

class TestRuntimeBuilder : public RuntimeBuilderT<TestRuntimeBuilder>
{
    using BaseType = RuntimeBuilderT<TestRuntimeBuilder>;
public:
    char* getLastToken() const { return ""; }
    int getLastLineNumber() const { return 0; }
    void setCanFinish(int val) {}
    void setStringDelimiter(const char* begin, const char* end) {}
    void setScriptDelimiter(const char* begin, const char* end) {}
public:
    TestRuntimeBuilder(Interpreter& interpreter) :BaseType(interpreter)
    {}
};

void CompileTest_ByteCode()
{
    Interpreter interpreter;
    TestRuntimeBuilder builder(interpreter);
    builder.addFunction();
    builder.beginStatement();
    builder.buildFirstTernaryOperator();
    builder.buildHighOrderFunction();
    builder.buildOperator();
    builder.buildSecondTernaryOperator();
    builder.endStatement();
    builder.markBracketParam();
    builder.markStatement();
    builder.markExternScript();
    builder.markBracketColonParam();
    builder.markParenthesisColonParam();
    builder.markAngleBracketColonParam();
    builder.markBracePercentParam();
    builder.markBracketPercentParam();
    builder.markParenthesisPercentParam();
    builder.markAngleBracketPercentParam();
    builder.setExternScript();
    builder.markParenthesisParam();
    builder.markPeriodBraceParam();
    builder.markPeriodBracketParam();
    builder.markPeriodParam();
    builder.markPeriodParenthesisParam();
    builder.setMemberId();
    builder.setCanFinish(false);
    builder.setFunctionId();
    builder.setMemberId();
    builder.markQuestionPeriodParam();
    builder.markQuestionParenthesisParam();
    builder.markQuestionBracketParam();
    builder.markQuestionBraceParam();
    builder.markPointerParam();
    builder.markPeriodStarParam();
    builder.markQuestionPeriodStarParam();
    builder.markPointerStarParam();
}