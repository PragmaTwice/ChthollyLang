#pragma once

#include <string>

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
		static T To(StringView);

		template <>
		static std::int64_t To(StringView v)
		{
			return std::stoll(std::basic_string<typename StringView::value_type>{ v });
		}

		template <>
		static std::double_t To(StringView v)
		{
			return std::stold(std::basic_string<typename StringView::value_type>{ v });
		}

	};

	template <typename String>
	struct Conv <String, std::enable_if_t<is_instance_of_v<String, std::basic_string>, int>>
	{
		template <typename T>
		static T To(const String& v)
		{
			return Conv<std::basic_string_view<typename String::value_type>>::To<T>(v);
		}
	};

	template <>
	struct Conv<std::int64_t>
	{
		template <typename T>
		static T To(std::int64_t v);

		template <>
		static std::string To(std::int64_t v)
		{
			return std::to_string(v);
		}

		template <>
		static std::wstring To(std::int64_t v)
		{
			return std::to_wstring(v);
		}
	};

	template <>
	struct Conv<std::double_t>
	{
		template <typename T>
		static T To(std::double_t v);

		template <>
		static std::string To(std::double_t v)
		{
			return std::to_string(v);
		}

		template <>
		static std::wstring To(std::double_t v)
		{
			return std::to_wstring(v);
		}
	};

	template <typename T>
	struct Quoted;

	template <typename String>
	struct Conv <Quoted<String>>
	{
		template <typename T>
		static T To(const String&);

		template <>
		static std::basic_string<typename String::value_type> To(const String& in)
		{
			std::basic_string<typename String::value_type> out;
			enum { start, escaped, normal, end } status = start;
			for(auto c : in)
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
	};

	template <typename T>
	struct Unquoted;

	template <typename String>
	struct Conv <Unquoted<String>>
	{
		template <typename T>
		static T To(const String&);

		template <>
		static std::basic_string<typename String::value_type> To(const String& in)
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
	};

}