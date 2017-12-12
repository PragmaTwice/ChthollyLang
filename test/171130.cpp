
#include "../src/parser.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace Chtholly;

ostream& operator<< (ostream& out, const ParseTree::Visitor& v)
{
	bool isTerm = v.value().type == ParseUnit::Type::term;
	if (isTerm) out << '(';

	if (isTerm)
	{
		out << v.value().name << ' ';
	}
	else
	{
		out << v.value().name << '[' << v.value().value << "] ";
	}

	for (auto i = v.childrenBegin(); i != v.childrenEnd(); ++i)
	{
		out << i;
	}

	if (isTerm) out << ')';

	return out;
}

int main()
{
	cout << R"(
 ____     __      __    __              ___    ___                __                                   
/\  _`\  /\ \    /\ \__/\ \            /\_ \  /\_ \              /\ \                                  
\ \ \/\_\\ \ \___\ \ ,_\ \ \___     ___\//\ \ \//\ \    __  __   \ \ \         __      ___      __     
 \ \ \/_/_\ \  _ `\ \ \/\ \  _ `\  / __`\\ \ \  \ \ \  /\ \/\ \   \ \ \  __  /'__`\  /' _ `\  /'_ `\   
  \ \ \L\ \\ \ \ \ \ \ \_\ \ \ \ \/\ \L\ \\_\ \_ \_\ \_\ \ \_\ \   \ \ \L\ \/\ \L\.\_/\ \/\ \/\ \L\ \  
   \ \____/ \ \_\ \_\ \__\\ \_\ \_\ \____//\____\/\____\\/`____ \   \ \____/\ \__/.\_\ \_\ \_\ \____ \ 
    \/___/   \/_/\/_/\/__/ \/_/\/_/\/___/ \/____/\/____/ `/___/> \   \/___/  \/__/\/_/\/_/\/_/\/___L\ \
                                                            /\___/                              /\____/
                                                            \/__/                               \_/__/ 
 
)";

	while (cin.good())
	{
		ParseTree tree(ParseUnit::Type::term, "root");
		auto modi = tree.modifier();


		string input,line;
		cout << "Input :" << endl;

		while (cin.good())
		{
			getline(cin, line);
			input += line + "\n";
		}
		cin.clear();

		auto beginTime = chrono::system_clock::now();
		auto result = Parser::Expression(Parser::MakeInfo(input, modi));
		auto endTime = chrono::system_clock::now();
		

		cout << "Parse tree : " << modi << endl;
		auto trimIter = find_if(result.first.rbegin(), result.first.rend(), [](auto&& elem) { return !isspace(elem); });
		result.first.remove_suffix(distance(result.first.rbegin(), trimIter));

		cout << "Time usage : " << std::chrono::duration<double>(endTime - beginTime).count() << "s" << endl;
		if(!result.first.empty()) cout << "Cannot resolve : " << result.first << endl;

		cout << endl;
	}
}
