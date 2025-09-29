#ifndef __SourceCodeScript_H__
#define __SourceCodeScript_H__

#include "calc.h"

namespace FunctionScript
{
    class SourceCodeScript
    {
    public:
        void Parse(const char* buf);
        void Parse(IScriptSource& source);
    public:
        inline Interpreter& GetInterpreter() { return m_Interpreter; }
        inline const Interpreter& GetInterpreter()const { return m_Interpreter; }
    public:
        SourceCodeScript() {}
        SourceCodeScript(const InterpreterOptions& options) :m_Interpreter(options) {}
    private:
        Interpreter m_Interpreter;
    };
}

using namespace FunctionScript;

#endif //__SourceCodeScript_H__