#ifndef __ByteCodeScript_H__
#define __ByteCodeScript_H__

#include "calc.h"

namespace FunctionScript
{
    class ByteCodeScript
    {
    public:
        void Parse(const char* buf, int size);
    public:
        inline Interpreter& GetInterpreter() { return m_Interpreter; }
        inline const Interpreter& GetInterpreter()const { return m_Interpreter; }
    public:
        ByteCodeScript() {}
        ByteCodeScript(const InterpreterOptions& options) :m_Interpreter(options) {}
    private:
        Interpreter m_Interpreter;
    };
}

using namespace FunctionScript;

#endif //__ByteCodeScript_H__