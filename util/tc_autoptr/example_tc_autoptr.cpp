#include "tc_autoptr.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace tars;

class A: public TC_HandleBaseT<int>
{
	public:
	A(const string& name) { _name = name;}
	string test() { return _name; }

	~A() { cout << "release:" << _name << endl;}

private:
	string _name;
};

int main()
{
	A *a = new A("a");
	A *b = new A("b");
	cout << a << endl;
	cout << b << endl;

	cout << "TC_AutoPtr release begin" << endl;
	{
		TC_AutoPtr<A> spa(a);
		TC_AutoPtr<A> spb(b);

		spb = spa;

		cout << spa->getRef()  <<endl;
		cout << spb->getRef() << endl;

		cout << spa->test()  <<endl;
		cout << spb->test() << endl;
	}
	cout << "TC_AutoPtr release end" << endl;

	cout << a->test() << endl;
	cout << b->test() << endl;
	
/*	cout << "autoptr release begin" << endl;
	{
		std::auto_ptr<A> apa(a);
		std::auto_ptr<A> apb(b);

		apb = apa;

		cout << apa.get()  <<endl;
		cout << apb.get() << endl;
	}
	cout << "autoptr release end" << endl;

	cout << a->test() << endl;
	cout << b->test() << endl;*/




/*	cout << (a->get()) << endl;

	TC_AutoPtr<A> spa(a);
	TC_AutoPtr<A> spb(b);

	cout << "spa ref:" << spa->getRef() << endl;
	cout << "spb ref:" << spb->getRef() << endl;

	spa = spb;

	cout << "spa ref:" << spa->getRef() << endl;
	cout << "spb ref:" << spb->getRef() << endl;*/
}