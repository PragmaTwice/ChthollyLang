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

		inline static auto isAlphaOrNum		= std::isalnum;
		inline static auto isAlpha			= std::isalpha;
		inline static auto isLowercaseAlpha	= std::islower;
		inline static auto isUppercaseAlpha	= std::isupper;
		inline static auto isDigit			= std::isdigit;
		inline static auto isHexDigit		= std::isxdigit;
		inline static auto isControl		= std::iscntrl;
		inline static auto isGraphic		= std::isgraph;
		inline static auto isSpace			= std::isspace;
		inline static auto isBlank			= std::isblank;
		inline static auto isPrintable		= std::isprint;
		inline static auto isPunctuation	= std::ispunct;
	};

	template <>
	struct CharType<wchar_t>
	{
		CharType() = delete;

		inline static auto isAlphaOrNum		= std::iswalnum;
		inline static auto isAlpha			= std::iswalpha;
		inline static auto isLowercaseAlpha = std::iswlower;
		inline static auto isUppercaseAlpha = std::iswupper;
		inline static auto isDigit			= std::iswdigit;
		inline static auto isHexDigit		= std::iswxdigit;
		inline static auto isControl		= std::iswcntrl;
		inline static auto isGraphic		= std::iswgraph;
		inline static auto isSpace			= std::iswspace;
		inline static auto isBlank			= std::iswblank;
		inline static auto isPrintable		= std::iswprint;
		inline static auto isPunctuation	= std::iswpunct;
	};
}
