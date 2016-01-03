//-----------------------------------------------------------------------------
// ×îµ×²ãµÄÁ´±í
//-----------------------------------------------------------------------------
#include	"Chain.h"
#include	"ChainNodePool.h"

struct TestChainData
{
	int a;
	int b;
};

typedef NodeRecyclerForPoolT<TestChainData> Recycler;

void CompileTest_Chain(void)
{
	CChain<TestChainData,Recycler> chain;
	CChainNode<TestChainData,Recycler> item1;
	CChainNode<TestChainData,Recycler> item2;
	CChainNode<TestChainData,Recycler> item3;
	CChainNode<TestChainData,Recycler> item4;
	chain.AddLast(CChainNode<TestChainData,Recycler>::Iterator(&item1));
	chain.AddLast(CChainNode<TestChainData,Recycler>::Iterator(&item2));
	chain.AddFirst(CChainNode<TestChainData,Recycler>::Iterator(&item3));
	chain.AddFirst(CChainNode<TestChainData,Recycler>::Iterator(&item4));

	chain.Remove(chain.GetIterator(1));
	int ix=chain.IndexOf(CChainNode<TestChainData,Recycler>::Iterator(&item3));
	int r=(CChainNode<TestChainData,Recycler>::Iterator(&item3)==chain.GetIterator(ix));
	r;

	typedef CChainNodePoolT<TestChainData,Recycler> DPool;
	typedef CChainNode<TestChainData,Recycler> DNode;
	DPool pool;
	pool.Init(50);
	DNode::Iterator a1 = pool.NewNode();
	DNode::Iterator a2 = pool.NewNode();
	DNode::Iterator a3 = pool.NewNode();
	a1.Recycle();
	a2.Recycle();
	a3.Recycle();
}