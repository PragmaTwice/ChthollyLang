/*
* Copyright 2019 PragmaTwice
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

#include <string>
#include <type_traits>

namespace Chtholly
{
	template <typename T, template <typename...> typename U>
	struct is_instance_of : std::false_type {};

	template <template <typename...> typename U, typename... T>
	struct is_instance_of <U<T...>, U> : std::true_type {};

	template <typename T, template <typename...> typename U>
	constexpr auto is_instance_of_v = is_instance_of<T, U>::value;

	template <typename T, typename Enable = void>
	struct Conv;

	template <typename StringView>
	struct Conv <StringView, std::enable_if_t<is_instance_of_v<StringView, std::basic_string_view>>>
	{
		template <typename T>
		static T To(StringView v)
		{
			if constexpr (std::is_same_v<T, std::int64_t>)
			{
				return std::stoll(std::basic_string<typename StringView::value_type>{ v });
			}
			else if constexpr (std::is_same_v<T, std::double_t>)
			{
				return std::stold(std::basic_string<typename StringView::value_type>{ v });
			}
			else static_assert("Conv::To invalid type");
		}

	};

	template <typename String>
	struct Conv <String, std::enable_if_t<is_instance_of_v<String, std::basic_string>, int>>
	{
		template <typename T>
		static T To(const String& v)
		{
			return Conv<std::basic_string_view<typename String::value_type>>::template To<T>(v);
		}
	};

	template <>
	struct Conv<std::int64_t>
	{
		template <typename T>
		static T To(std::int64_t v)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<T, std::wstring>)
			{
				return std::to_wstring(v);
			}
			else static_assert("Conv::To invalid type");
		}
	};

	template <>
	struct Conv<std::double_t>
	{
		template <typename T>
		static T To(std::double_t v)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<T, std::wstring>)
			{
				return std::to_wstring(v);
			}
			else static_assert("Conv::To invalid type");
		}
	};

	template <typename T>
	struct Quoted;

	template <typename String>
	struct Conv <Quoted<String>>
	{
		template <typename T>
		static T To(const String& in)
		{
			if constexpr (std::is_same_v<T, std::basic_string<typename String::value_type>>)
			{
				std::basic_string<typename String::value_type> out;
				enum { start, escaped, normal, end } status = start;
				for (auto c : in)
				{
					switch (status)
					{
					case start:
						if (c != '"') status = end;
						else status = normal;
						break;
					case escaped:
						switch (c)
						{
						case '"':  out += '"';  break;
						case '\\': out += '\\'; break;
						case 'b':  out += '\b'; break;
						case 'f':  out += '\f'; break;
						case 'n':  out += '\n'; break;
						case 'r':  out += '\r'; break;
						case 't':  out += '\t'; break;
						case 'v':  out += '\v'; break;
						default:;
						}
						status = normal;
						break;
					case normal:
						if (c == '\\') status = escaped;
						else if (c == '"') status = end;
						else out += c;
						break;
					default:
						;
					}
					if (status == end) break;
				}

				return out;
			}
			else static_assert("Conv::To invalid type");
		}
	};

	template <typename T>
	struct Unquoted;

	template <typename String>
	struct Conv <Unquoted<String>>
	{
		template <typename T>
		static T To(const String& in)
		{
			if constexpr (std::is_same_v<T, std::basic_string<typename String::value_type>>)
			{
				std::basic_string<typename String::value_type> out;
				out += '"';
				for (auto c : in) {
					switch (c) {
					case '"':  out += "\\\"";    break;
					case '\\': out += "\\\\";    break;
					case '\b': out += "\\b";     break;
					case '\f': out += "\\f";     break;
					case '\n': out += "\\n";     break;
					case '\r': out += "\\r";     break;
					case '\t': out += "\\t";     break;
					case '\v': out += "\\v";     break;
					default:   out += c;
					}
				}
				out += '"';
				return out;
			}
			else static_assert("Conv::To invalid type");
		}
	};

}