#ifndef __SourceCodeScript_H__
#define __SourceCodeScript_H__

#include "calc.h"

namespace FunctionScript
{
    class ByteCodeGenerator
    {
        enum
        {
            GENERATOR_STRING_BUFFER_SIZE = 64 * 1024,
        };
    public:
        int Parse(const char* buf, char* pByteCode, int codeBufferLen);
        int Parse(IScriptSource& source, char* pByteCode, int codeBufferLen);
    public:
        ErrorAndStringBuffer& GetErrorAndStringBuffer() { return m_ErrorAndStringBuffer; }
        const ErrorAndStringBuffer& GetErrorAndStringBuffer()const { return m_ErrorAndStringBuffer; }
    public:
        ByteCodeGenerator() :m_UnusedStringPtr(m_StringBuffer)
        {
            m_ErrorAndStringBuffer.Reset(m_StringBuffer, m_UnusedStringPtr, GENERATOR_STRING_BUFFER_SIZE);
        }
    private:
        char* m_UnusedStringPtr;
    private:
        ErrorAndStringBuffer m_ErrorAndStringBuffer;
        char m_StringBuffer[GENERATOR_STRING_BUFFER_SIZE];
    };

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