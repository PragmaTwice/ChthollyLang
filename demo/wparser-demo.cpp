/*
* Copyright 2017 PragmaTwice
*/

#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include "chtholly.hpp"

using namespace std;
using namespace Chtholly;

wstring ToWString(const string& str)
{
	wstring result;
	for(auto&& elem : str)
	{
		result += elem;
	}

	return result;
}

wstring ToWString(const BasicParseTree<wstring_view>::Observer& v)
{
	wstring out;

	const auto isTerm = v.value().type == BasicParseUnit<wstring_view>::Type::term;

	if (isTerm) out += L'(';

	if (isTerm)
	{
		out += ToWString(v.value().name) + L' ';
	}
	else
	{
		out += ToWString(v.value().name) + L'[' + wstring(v.value().value) + L"] ";
	}

	for (auto i = v.childrenBegin(); i != v.childrenEnd(); ++i)
	{
		out += ToWString(i);
	}

	if (isTerm) out += ')';

	return out;
}

void parseInputAndLog(const BasicParser<wstring_view>::Info& info)
{
	const auto beginTime = chrono::system_clock::now();
	auto result = BasicParser<wstring_view>::Expression(info);
	const auto endTime = chrono::system_clock::now();

	
	wcout << L"Parse tree : " << ToWString(info.second) << endl;
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
		BasicParseTree<wstring_view> tree;
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
