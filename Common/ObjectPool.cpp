#include	"ObjectPool.h"

/*
struct TestObjectData : public PoolAllocatedObjectBaseT<TestObjectData>
{
	int a;
	int b;
};

void CompileTest_ObjectPool(void)
{
	typedef ObjectPoolT<TestObjectData> DPool;
	DPool pool;
	pool.Init(50);
	TestObjectData* a1 = pool.NewObject();
	TestObjectData* a2 = pool.NewObject();
	TestObjectData* a3 = pool.NewObject();
	a1->Recycle();
	a2->Recycle();
	a3->Recycle();
}
*/




