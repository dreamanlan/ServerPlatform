#ifndef STATICCHECK_H
#define STATICCHECK_H

template<bool>
class CompileSuccess
{
};

template<>
class CompileSuccess<true>
{
public:
    CompileSuccess(...) {}; //���Խ����κβ����Ĺ��캯��
};

int CompileChecker(CompileSuccess<true> const&);

#define STATIC_CHECK(expr, msg)\
{\
	class CompileError_##msg {};\
	sizeof(CompileChecker(CompileSuccess<false!=(expr)>(CompileError_##msg())));\
}

#endif //STATICCHECK_H
