#include <iostream>
#include "../src/parser.hpp"

using namespace std;

int main()
{
	char input[2048];

	while (cin.getline(input, 2048))
	{
		
		Chtholly::Parser parser(input);

		cout << parser.StringLiteral(parser.Begin()) - parser.Begin()
			<< endl;
	}
}