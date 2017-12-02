#include "../src/parsetree.hpp"
#include <iostream>

using namespace std;
using namespace Chtholly;

BasicTree<int> create()
{
	BasicTree<int> a;
	auto m = a.modifier();

	m.childrenPushBack(2);
	m.childrenPushBack(5);
	m.childrenPushBack(4);
	m.childrenPushBack(2);

	auto mc = m.childrenBegin();
	mc.childrenPushBack(3);
	mc.childrenPushBack(5);
	++mc;
	mc.childrenPushBack(9);

	return a;
}

void visit(BasicTree<int>::Visitor i)
{
	cout << "(" << i.value() << ends;
	for (auto j = i.childrenBegin(); j != i.childrenEnd(); ++j)
	{
		visit(j);
	}
	cout << ")";
}

int main()
{
	visit(create().visitor());
	cin.get();
}