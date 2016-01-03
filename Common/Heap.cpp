#include "Heap.h"

void CompileTest_Heap(void)
{
	HeapT<int> heap(25);
	heap.Push(1);
	heap.Push(3);
	heap.Push(2);
	heap.Push(4);
	heap.Push(6);
	heap.Push(7);
	heap.Push(3);
	int val=heap.Root();
	heap.Pop();
	val=heap.Root();

	int a[]={3,1,3,4,6,8,7,9};
	heap.Build(a,8);
	HeapT<int> heap2=heap;
	HeapT<int> heap3;
	heap3=heap;
	val=heap.Root();
	heap.Pop();
	val=heap.Root();
}