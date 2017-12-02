#include "../src/parser.hpp"
#include <iostream>
#include <string>
#include <chrono>

using namespace std;
using namespace Chtholly;

int main()
{

	auto p = Parser::Expression;

	while (cin.good())
	{
		string s;
		getline(cin, s);

		auto&& beginTime = chrono::system_clock::now();
		auto&& result = p(s);
		auto&& endTime = chrono::system_clock::now();

		cout << "(" << chrono::duration<double>(endTime - beginTime).count() << "s)" << endl;
		cout << result << endl;

	}

	cin.get();
}