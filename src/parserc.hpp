#pragma once

#include <string_view>
#include <functional>
#include <algorithm>

#include "parsetree.hpp"

namespace Chtholly
{

	template <typename StringView>
	class BasicParserCombinator
	{

	public /*type*/:

		using Lang = StringView;
		using LangRef = const Lang &;

		using Modifier = typename BasicParseTree<StringView>::Modifier;
		using ModifierRef = const Modifier &;

		using Info = std::pair<Lang, Modifier>;
		using InfoRef = const Info &;

		using Char = typename StringView::value_type;
		using Iter = typename StringView::const_iterator;
		using Size = typename StringView::size_type;

		using BasicProcess = Info(Info);
		using BasicProcessWrapper = std::function<BasicProcess>;


		struct Process : public BasicProcessWrapper
		{
		private:
			bool isOptional;

		public:
			Process(const BasicProcessWrapper& pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}
			Process(BasicProcess* const pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}

			bool IsOptional() const { return isOptional; }

			bool IsNotEqual(InfoRef lhs, InfoRef rhs) const { return isOptional ? true : (lhs.first != rhs.first); }

			void SetOptional(const bool _isOptional = true) { isOptional = _isOptional; }

		};
		using ProcessRef = const Process &;

		template <typename ...Args>
		using Predicate = std::function<bool(Args...)>;

		template <typename ...Args>
		using PredicateRef = const Predicate<Args...> &;

		template <typename T>
		using InitList = std::vector<T>;

		template <typename T>
		using InitListRef = const InitList<T>&;

	public /*method*/:

		BasicParserCombinator() = delete;

		static auto MakeInfo(LangRef lang, ModifierRef modi)
		{
			return Info(lang, modi);
		}

		static Info MoveOn(InfoRef info, const Size step = 1)
		{
			return Info(Lang(info.first.data() + step, std::max(signed(info.first.size() - step), 0)),info.second);
		}

		static Info MoveOn(InfoRef info, Iter iter)
		{
			return Info(Lang(info.first.data() + (iter - info.first.cbegin()), std::max(signed(info.first.size() - (iter - info.first.cbegin())), 0)), info.second);
		}

		// Match a character with a predicate
		static Process Match(PredicateRef<Char> predicate)
		{
			return [=](Info info)
			{
				if (info.first.empty()) return info;

				if (predicate(info.first.front())) return MoveOn(info);

				return info;
			};
		}

		// Match a character with a character list
		static Process Match(InitListRef<Char> initList)
		{
			return [=](Info info)
			{
				if (info.first.empty()) return info;

				Iter i = info.first.cbegin();
				for (auto&& elem : initList)
				{
					if (elem == *i) return MoveOn(info);
				}

				return info;
			};
		}

		// Match a character with a character
		static Process Match(Char character)
		{
			return [=](Info info)
			{
				if (info.first.empty()) return info;

				if (character == info.first.front()) return MoveOn(info);

				return info;
			};
		}

		// Match a character string with a string list
		static Process Match(InitListRef<Lang> initList)
		{
			return [=](Info info)
			{
				for (auto&& elem : initList)
				{
					auto i = elem.cbegin();
					auto j = info.first.cbegin();
					while (i != elem.cend() && j != info.first.cend() && *i == *j) ++i, ++j;
					if (i == elem.cend()) return MoveOn(info, j);
				}

				return info;
			};
		}

		// Match a character string with a string
		static Process Match(Lang string)
		{
			return [=](Info info)
			{
				Iter i = string.cbegin(), j = info.first.cbegin();
				while (i != string.cend() && j != info.first.cend() && *i == *j) ++i, ++j;
				if (i == string.cend()) return MoveOn(info, j);

				return info;
			};
		}

		// operator,(A,B) = A B
		friend Process operator,(ProcessRef lhs, ProcessRef rhs)
		{
			return [=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					if (Info j = rhs(i); rhs.IsNotEqual(j, i))
						return j;

				return info;
			};
		}

		// operator|(A,B) = A | B
		friend Process operator|(ProcessRef lhs, ProcessRef rhs)
		{
			return [=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					return i;

				if (Info i = rhs(info); rhs.IsNotEqual(i, info))
					return i;

				return info;
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
			return [=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					return (+lhs)(i);

				return info;
			};
		}

		// operator*(A) = A*
		friend Process operator*(ProcessRef lhs)
		{
			return ~+ lhs;
		}

		// operator^(A,B) = A (not followed by B)
		friend Process operator^(ProcessRef pro, ProcessRef except)
		{
			return [=](Info info)
			{
				if (Info i = pro(info); pro.IsNotEqual(i, info))
					if (Info j = except(i); !except.IsNotEqual(j, i))
						return i;

				return info;
			};
		}


	};

	using ParserCombinator = BasicParserCombinator<std::string_view>;

}