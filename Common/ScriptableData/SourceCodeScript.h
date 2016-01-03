#ifndef __SourceCodeScript_H__
#define __SourceCodeScript_H__

#include "ScriptableData.h"

namespace ScriptableData
{
  class ByteCodeGenerator
  {
    enum
    {
      GENERATOR_STRING_BUFFER_SIZE = 64 * 1024,
    };
  public:
    INT							Parse(const CHAR* buf, CHAR* pByteCode, INT codeBufferLen);
    INT							Parse(IScriptSource& source, CHAR* pByteCode, INT codeBufferLen);
  public:
    ErrorAndStringBuffer&		GetErrorAndStringBuffer(void){ return m_ErrorAndStringBuffer; }
    const ErrorAndStringBuffer&	GetErrorAndStringBuffer(void)const{ return m_ErrorAndStringBuffer; }
  public:
    ByteCodeGenerator(void) :m_UnusedStringPtr(m_StringBuffer)
    {
      m_ErrorAndStringBuffer.Reset(m_StringBuffer, m_UnusedStringPtr, GENERATOR_STRING_BUFFER_SIZE);
    }
  private:
    CHAR*						m_UnusedStringPtr;
  private:
    ErrorAndStringBuffer		m_ErrorAndStringBuffer;
    CHAR						m_StringBuffer[GENERATOR_STRING_BUFFER_SIZE];
  };

  void ParseText(const char* buf, ScriptableDataFile& file);
  void ParseText(IScriptSource& source, ScriptableDataFile& file);
}

using namespace ScriptableData;

#endif //__SourceCodeScript_H__