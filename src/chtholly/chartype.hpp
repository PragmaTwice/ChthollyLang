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

		inline static auto isAlphaOrNum		= static_cast<Type>(std::isalnum);
		inline static auto isAlpha			= static_cast<Type>(std::isalpha);
		inline static auto isLowercaseAlpha	= static_cast<Type>(std::islower);
		inline static auto isUppercaseAlpha	= static_cast<Type>(std::isupper);
		inline static auto isDigit			= static_cast<Type>(std::isdigit);
		inline static auto isHexDigit		= static_cast<Type>(std::isxdigit);
		inline static auto isControl		= static_cast<Type>(std::iscntrl);
		inline static auto isGraphic		= static_cast<Type>(std::isgraph);
		inline static auto isSpace			= static_cast<Type>(std::isspace);
		inline static auto isBlank			= static_cast<Type>(std::isblank);
		inline static auto isPrintable		= static_cast<Type>(std::isprint);
		inline static auto isPunctuation	= static_cast<Type>(std::ispunct);
	};

	template <>
	struct CharType<wchar_t>
	{
		CharType() = delete;

		using Type = int(*)(std::wint_t);

		inline static auto isAlphaOrNum		= static_cast<Type>(std::iswalnum);
		inline static auto isAlpha			= static_cast<Type>(std::iswalpha);
		inline static auto isLowercaseAlpha = static_cast<Type>(std::iswlower);
		inline static auto isUppercaseAlpha = static_cast<Type>(std::iswupper);
		inline static auto isDigit			= static_cast<Type>(std::iswdigit);
		inline static auto isHexDigit		= static_cast<Type>(std::iswxdigit);
		inline static auto isControl		= static_cast<Type>(std::iswcntrl);
		inline static auto isGraphic		= static_cast<Type>(std::iswgraph);
		inline static auto isSpace			= static_cast<Type>(std::iswspace);
		inline static auto isBlank			= static_cast<Type>(std::iswblank);
		inline static auto isPrintable		= static_cast<Type>(std::iswprint);
		inline static auto isPunctuation	= static_cast<Type>(std::iswpunct);
	};
}
