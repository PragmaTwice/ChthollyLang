/*
* Copyright 2017 PragmaTwice
*/

#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <parser.hpp>

using namespace std;
using namespace Chtholly;

wostream& operator<< (wostream& out, string str)
{
	for (auto elem : str) out << elem;
	return out;
}

wostream& operator<< (wostream& out, const BasicParseTree<wstring_view>::Visitor& v)
{
	const auto isTerm = v.value().type == BasicParseUnit<wstring_view>::Type::term;

	if (isTerm) out << '(';

	if (isTerm)
	{
		out << v.value().name << ' ';
	}
	else
	{
		out << v.value().name << '[' << v.value().value << L"] ";
	}

	for (auto i = v.childrenBegin(); i != v.childrenEnd(); ++i)
	{
		out << i;
	}

	if (isTerm) out << ')';

	return out;
}

void parseInputAndLog(const BasicParser<wstring_view>::Info& info)
{
	const auto beginTime = chrono::system_clock::now();
	auto result = BasicParser<wstring_view>::Expression(info);
	const auto endTime = chrono::system_clock::now();

	
	wcout << L"Parse tree : " << info.second << endl;
	auto trimIter = find_if(result.first.rbegin(), result.first.rend(), [](auto&& elem) { return !iswspace(elem); });
	result.first.remove_suffix(distance(result.first.rbegin(), trimIter));
	
	wcout << L"Time usage : " << std::chrono::duration<double>(endTime - beginTime).count() << L"s" << endl;
	if (!result.first.empty()) wcout << L"Cannot resolve : " << result.first << endl;

	wcout << endl;
}

int main()
{
	wcout << LR"(
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

	while (true)
	{
		BasicParseTree<wstring_view> tree(BasicParseUnit<wstring_view>::Type::term, "root");
		auto modi = tree.modifier();

		wstring input, line;
		wcout << L"Input :" << endl;

		while (wcin.good())
		{
			getline(wcin, line);
			input += line + L"\n";
		}
		wcin.clear();

		static const auto exitCommand = L"exit"sv;
		if (wstring_view{ input.data(), exitCommand.size() } == exitCommand)
			break;

		parseInputAndLog(BasicParser<wstring_view>::MakeInfo(input, modi));
	}
}
