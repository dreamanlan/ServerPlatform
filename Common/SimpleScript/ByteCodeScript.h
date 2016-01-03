#ifndef __ByteCodeScript_H__
#define __ByteCodeScript_H__

#include "calc.h"

namespace FunctionScript
{
	class ByteCodeScript
	{
	public:
		void						Parse(const char* buf,int size);
	public:
		inline Interpreter&			GetInterpreter(void){return m_Interpreter;}
		inline const Interpreter&	GetInterpreter(void)const{return m_Interpreter;}
	public:
		ByteCodeScript(void){}
		ByteCodeScript(const InterpreterOptions& options):m_Interpreter(options){}
	private:
		Interpreter					m_Interpreter;
	};
}

using namespace FunctionScript;

#endif //__ByteCodeScript_H__