#ifndef __Packet_H__
#define __Packet_H__

#include "Type.h"

#define MAX_SEND_PACKET_SIZE		4*1024+64
#define MAX_RECEIVE_PACKET_SIZE		256*1024+64

namespace PacketStreamUtility
{
	template<typename T>
	class impl_NotBaseType
	{};
	template<typename T>
	class impl_BaseType
	{};

	template<typename T>
	class TypeClassOf
	{
	public:
		typedef impl_NotBaseType<T> Classify;
	};

#define DEF_BASETYPE(X)	\
	template<>\
	class TypeClassOf<X>\
	{\
	public:\
		typedef impl_BaseType<X> Classify;\
	};

	//DEF_BASETYPE(unsigned long)
	//DEF_BASETYPE(long)
	DEF_BASETYPE(unsigned int)
	DEF_BASETYPE(int)
	DEF_BASETYPE(double)
	DEF_BASETYPE(float)
	DEF_BASETYPE(unsigned short)
	DEF_BASETYPE(short)		
	DEF_BASETYPE(unsigned char)	
	DEF_BASETYPE(char)
	DEF_BASETYPE(char*)
	DEF_BASETYPE(const char*)
	DEF_BASETYPE(unsigned long long)
	DEF_BASETYPE(long long)

	template<typename T>
	class TypeIsNotCharPtr
	{
	public:
		static const bool Value=true;
	};
	template<>
	class TypeIsNotCharPtr<char*>
	{
	public:
		static const bool Value=false;
	};
	template<>
	class TypeIsNotCharPtr<const char*>
	{
	public:
		static const bool Value=false;
	};
}

class BinaryStream
{
public:
	BinaryStream(unsigned int bufferSize):m_pBuffer(NULL),m_BufferSize(0),m_Cursor(0),m_Size(0)
	{
		m_pBuffer=new char[bufferSize];
		if(m_pBuffer)
		{
			m_BufferSize=bufferSize;
		}
	}
	virtual ~BinaryStream(void)
	{
		if(m_pBuffer)
		{
			delete[] m_pBuffer;
			m_pBuffer=NULL;
		}
		m_BufferSize=0;
		m_Cursor=0;
		m_Size=0;
	}
public:
	inline int											GetStreamSize(void)const
	{
		return m_Size;
	}
	inline void*										GetStreamData(void) const
	{
		return m_pBuffer;
	}
	inline int											SetStreamData(const void* pData,int size)
	{
		int ret=FALSE;
		if(NULL!=pData && 0<=size)
		{
			CleanUp();
			ret=AddStream(pData,size);
		}
		return ret;
	}
	inline int											GetCursorPos() const { return m_Cursor;}
public:
	void ResetCursor(void) const
	{
		m_Cursor=0;
	}
	int Skip(int nSize) const
	{
		if ( m_Cursor + nSize <= 0 || m_Cursor + nSize > m_Size )
		{
			return FALSE;
		}
		else
		{
			m_Cursor += nSize;
			return TRUE;
		}
	}
	void CleanUp(void)
	{
		m_Cursor=0;
		m_Size=0;
	}
public:
	template<typename T>
	inline void										   Add(T data)
	{
		_Add(data,typename PacketStreamUtility::TypeClassOf<T>::Classify());
	}
	template<typename T>
	inline T										   Get(void) const
	{
		STATIC_CHECK(PacketStreamUtility::TypeIsNotCharPtr<T>::Value,ERROR_Please_Use_2_Params_Get_For_String_Get);
		T res=T();
		_Get(res,typename PacketStreamUtility::TypeClassOf<T>::Classify());
		return res;
	}
	template<typename T>
	inline void										   Get(T& data) const
	{
		STATIC_CHECK(PacketStreamUtility::TypeIsNotCharPtr<T>::Value,ERROR_Please_Use_2_Params_Get_For_String_Get);
		_Get(data,typename PacketStreamUtility::TypeClassOf<T>::Classify());
	}
	inline void											Add(const char* str,int sizeByteNum)
	{
		if(str)
		{
			int size									= (int)strlen( str );
			if ( m_Size+sizeByteNum+size <= m_BufferSize )
			{
				switch(sizeByteNum)
				{
				case 1:
					Add((unsigned char)size);
					break;
				case 2:
					Add((unsigned short)size);
					break;
				case 4:
					Add((int)size);
					break;
				default:
					Add((unsigned short)size);
					break;
				}
				AddStream( (void*)str, size );
			}
		}
	}
	inline int											Get(char* str, int nSize) const
	{
		return Get(str,nSize,2);
	}
	inline int											Get(char* str, int nSize, int sizeByteNum) const
	{
		if(str)
		{
			int realSize = 0;
			int size = 0;
			switch(sizeByteNum)
			{
			case 1:
				size=Get<unsigned char>();
				break;
			case 2:
				size=Get<unsigned short>();
				break;
			case 4:
				size=Get<int>();
				break;
			default:
				size=Get<unsigned short>();
				break;
			}
			if(size <= nSize)
			{
				if(TRUE==GetStream( (void*)str, size ))
				{
					str[size]=0;
				}
			}
			realSize=size;
			return realSize;
		}
		else
		{
			return 0;
		}
	}
	inline int GetBlock(void* pBuffer, int nSize) const
	{
		int ret=FALSE;
		if(NULL!=pBuffer && 0<=nSize)
		{
			ret=GetStream(pBuffer, nSize);
		}
		return ret;
	}
	inline int	AddBlock(void const* pBuffer, int nSize)
	{
		int ret=FALSE;
		if(NULL!=pBuffer && 0<=nSize)
		{
			ret=AddStream(pBuffer, nSize);
		}
		return ret;
	}
private:
	template<typename T>
	inline void											_Add(T data,PacketStreamUtility::impl_NotBaseType<T>)
	{
		data.Write(*this);
	}
	template<typename T>
	inline void											_Get(T& data,PacketStreamUtility::impl_NotBaseType<T>) const
	{
		data.Read(*this);
	}
	template<typename T>
	inline void											_Add(T data,PacketStreamUtility::impl_BaseType<T>)
	{
		AddStream( (void*)( &data ), sizeof( data ) );
	}
	template<typename T>
	inline void											_Get(T& res,PacketStreamUtility::impl_BaseType<T>) const
	{
		GetStream( (void*)( &res ), sizeof( res ) );
	}
protected:
	int AddStream( const void* szStream, int nSize )
	{
		int ret=FALSE;
		if(NULL!=szStream && NULL!=m_pBuffer && m_Size + nSize > 0 && m_Size+nSize<m_BufferSize)
		{
			memcpy( &( m_pBuffer[m_Size] ), szStream, nSize );
			m_Size += nSize;
			ret=TRUE;
		}
		return ret;
	}
	int GetStream( void* szStream, int nSize ) const
	{
		int ret=FALSE;
		if(NULL!=szStream && NULL!=m_pBuffer && m_Cursor + nSize > 0 && m_Cursor + nSize <= m_Size )
		{
			memcpy( (void*)szStream, &( m_pBuffer[m_Cursor] ), nSize );
			m_Cursor = m_Cursor + nSize;
			ret=TRUE;
		}
		return ret;
	}
protected:
	char*			m_pBuffer;
	unsigned int			m_BufferSize;
	unsigned int			m_Size;
	mutable unsigned int	m_Cursor;
};
template<>
inline void BinaryStream::_Add<const char*>(const char* str,PacketStreamUtility::impl_BaseType<const char*>)
{
	Add(str,2);
}
template<>
inline void BinaryStream::_Add<char*>(char* str,PacketStreamUtility::impl_BaseType<char*>)
{
	_Add((const char*)str,PacketStreamUtility::impl_BaseType<const char*>());
}

#endif //__Packet_H__