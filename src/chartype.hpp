/*
* Copyright 2017 PragmaTwice
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*		http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include <cctype>
#include <cwctype>

namespace Chtholly
{
	template <typename Char>
	struct CharType;

	template <>
	struct CharType<char>
	{
		CharType() = delete;

		using Type = int(*)(int);

		inline static Type isAlphaOrNum		= std::isalnum;
		inline static Type isAlpha			= std::isalpha;
		inline static Type isLowercaseAlpha	= std::islower;
		inline static Type isUppercaseAlpha	= std::isupper;
		inline static Type isDigit			= std::isdigit;
		inline static Type isHexDigit		= std::isxdigit;
		inline static Type isControl		= std::iscntrl;
		inline static Type isGraphic		= std::isgraph;
		inline static Type isSpace			= std::isspace;
		inline static Type isBlank			= std::isblank;
		inline static Type isPrintable		= std::isprint;
		inline static Type isPunctuation	= std::ispunct;
	};

	template <>
	struct CharType<wchar_t>
	{
		CharType() = delete;

		using Type = int(*)(std::wint_t);

		inline static Type isAlphaOrNum		= std::iswalnum;
		inline static Type isAlpha			= std::iswalpha;
		inline static Type isLowercaseAlpha = std::iswlower;
		inline static Type isUppercaseAlpha = std::iswupper;
		inline static Type isDigit			= std::iswdigit;
		inline static Type isHexDigit		= std::iswxdigit;
		inline static Type isControl		= std::iswcntrl;
		inline static Type isGraphic		= std::iswgraph;
		inline static Type isSpace			= std::iswspace;
		inline static Type isBlank			= std::iswblank;
		inline static Type isPrintable		= std::iswprint;
		inline static Type isPunctuation	= std::iswpunct;
	};
}