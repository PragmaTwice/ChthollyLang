#pragma once

#include <string_view>
#include <functional>
#include <algorithm>

namespace Chtholly
{

	template <typename StringView>
	class BasicParserCombinator
	{

	public /*type*/:

		using Lang = StringView;
		using LangRef = const Lang &;

		using Char = typename StringView::value_type;
		using Iter = typename StringView::const_iterator;
		using Size = typename StringView::size_type;

		using BasicProcess = Lang(LangRef);
		using BasicProcessWrapper = std::function<BasicProcess>;

		struct Process : public BasicProcessWrapper
		{
		private:
			bool isOptional;

		public:
			Process(const BasicProcessWrapper& pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}
			Process(BasicProcess* const pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}

			bool IsOptional() const { return isOptional; }

			bool IsNotEqual(LangRef lhs, LangRef rhs) const { return isOptional ? true : (lhs != rhs); }

			void SetOptional(const bool _isOptional = true) { isOptional = _isOptional; }

		};
		using ProcessRef = const Process &;

		template <typename ...Args>
		using Predicate = std::function<bool(Args...)>;

		template <typename ...Args>
		using PredicateRef = const Predicate<Args...> &;

		template <typename T>
		using InitList = std::initializer_list<T>;

	public /*method*/:

		BasicParserCombinator() = delete;

		static Lang MoveOn(LangRef lang, const Size step = 1)
		{
			return Lang(lang.data() + step, std::max(signed(lang.size() - step), 0));
		}

		static Lang MoveOn(LangRef lang, Iter iter)
		{
			return Lang(lang.data() + (iter - lang.cbegin()), std::max(signed(lang.size() - (iter - lang.cbegin())), 0));
		}

		// Match a character with a predicate
		static Process Match(PredicateRef<Char> predicate)
		{
			return [=](LangRef lang)
			{
				if (lang.empty()) return lang;

				if (predicate(lang.front())) return MoveOn(lang);

				return lang;
			};
		}

		// Match a character with a character list
		static Process Match(InitList<Char> initList)
		{
			return [=](LangRef lang)
			{
				if (lang.empty()) return lang;

				Iter i = lang.cbegin();
				for (auto&& elem : initList)
				{
					if (elem == *i) return MoveOn(lang);
				}

				return lang;
			};
		}

		// Match a character with a character
		static Process Match(Char character)
		{
			return [=](LangRef lang)
			{
				if (lang.empty()) return lang;

				if (character == lang.front()) return MoveOn(lang);

				return lang;
			};
		}

		// Match a character string with a string list
		static Process Match(InitList<Lang> initList)
		{
			return [=](LangRef lang)
			{
				for (auto&& elem : initList)
				{
					Iter i = elem.cbegin(), j = lang.cbegin();
					while (i != elem.cend() && j != lang.cend() && *i == *j) ++i, ++j;
					if (i == elem.cend()) return MoveOn(lang, j);
				}

				return lang;
			};
		}

		// Match a character string with a string
		static Process Match(Lang string)
		{
			return [=](LangRef lang)
			{
				Iter i = string.cbegin(), j = lang.cbegin();
				while (i != string.cend() && j != lang.cend() && *i == *j) ++i, ++j;
				if (i == string.cend()) return MoveOn(lang, j);

				return lang;
			};
		}

		// operator,(A,B) = A B
		friend Process operator,(ProcessRef lhs, ProcessRef rhs)
		{
			return [=](LangRef lang)
			{
				if (Lang i = lhs(lang); lhs.IsNotEqual(i, lang))
					if (Lang j = rhs(i); rhs.IsNotEqual(j, i))
						return j;

				return lang;
			};
		}

		// operator|(A,B) = A | B
		friend Process operator|(ProcessRef lhs, ProcessRef rhs)
		{
			return [=](LangRef lang)
			{
				if (Lang i = lhs(lang); lhs.IsNotEqual(i, lang))
					return i;

				if (Lang i = rhs(lang); rhs.IsNotEqual(i, lang))
					return i;

				return lang;
			};
		}

		// operator~(A) = A?
		friend Process operator~(ProcessRef lhs)
		{
			return Process(lhs, true);
		}

		// operator+(A) = A A*
		friend Process operator+(ProcessRef lhs)
		{
			return [=](LangRef lang)
			{
				if (Lang i = lhs(lang); lhs.IsNotEqual(i, lang))
					return (+lhs)(i);

				return lang;
			};
		}

		// operator*(A) = A*
		friend Process operator*(ProcessRef lhs)
		{
			return ~+ lhs;
		}


	};

	using ParserCombinator = BasicParserCombinator<std::string_view>;

}