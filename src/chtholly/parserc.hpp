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

		using Tree = BasicParseTree<StringView>;

		using Modifier = typename Tree::Modifier;
		using ModifierRef = const Modifier &;

		using Info = std::pair<Lang, Modifier>;
		using InfoRef = const Info &;

		using Unit = BasicParseUnit<StringView>;

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
			//explicit Process(BasicProcess* const pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}
			explicit Process(const BasicProcessWrapper& pro, const bool _isOptional = false) : BasicProcessWrapper(pro), isOptional(_isOptional) {}

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
			return Process([=](Info info)
			{
				if (info.first.empty()) return info;

				if (predicate(info.first.front())) return MoveOn(info);

				return info;
			});
		}

		// Match a character with a character list
		static Process Match(InitListRef<Char> initList)
		{
			return Process([=](Info info)
			{
				if (info.first.empty()) return info;

				Iter i = info.first.cbegin();
				for (auto&& elem : initList)
				{
					if (elem == *i) return MoveOn(info);
				}

				return info;
			});
		}

		// Match a character with a character
		static Process Match(Char character)
		{
			return Process([=](Info info)
			{
				if (info.first.empty()) return info;

				if (character == info.first.front()) return MoveOn(info);

				return info;
			});
		}

		// Match a character string with a string list
		static Process Match(InitListRef<Lang> initList)
		{
			return Process([=](Info info)
			{
				for (auto&& elem : initList)
				{
					auto i = elem.cbegin();
					auto j = info.first.cbegin();
					while (i != elem.cend() && j != info.first.cend() && *i == *j) ++i, ++j;
					if (i == elem.cend()) return MoveOn(info, j);
				}

				return info;
			});
		}

		// Match a character string with a string
		static Process Match(Lang string)
		{
			return Process([=](Info info)
			{
				Iter i = string.cbegin(), j = info.first.cbegin();
				while (i != string.cend() && j != info.first.cend() && *i == *j) ++i, ++j;
				if (i == string.cend()) return MoveOn(info, j);

				return info;
			});
		}

		// Atom process
		inline static const Process Atom = Process(BasicProcessWrapper([](Info info) { return info; }), true);

		// Match any character
		inline static const Process AnyChar = Match([](Char) { return true; });

		// Match any characters until matching lhs
		static Process AnyCharUntil(ProcessRef lhs)
		{
			return 
				(	
					lhs |
					(
						*(AnyChar ^ lhs), AnyChar, lhs
					)
				);
		}

		// operator,(A,B) = A B
		friend Process operator,(ProcessRef lhs, ProcessRef rhs)
		{
			return Process([=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					if (Info j = rhs(i); rhs.IsNotEqual(j, i))
						return j;

				return info;
			}, lhs.IsOptional() && rhs.IsOptional());
		}

		// operator|(A,B) = A | B
		friend Process operator|(ProcessRef lhs, ProcessRef rhs)
		{
			return Process([=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					return i;

				if (Info i = rhs(info); rhs.IsNotEqual(i, info))
					return i;

				return info;
			}, lhs.IsOptional() && rhs.IsOptional());
		}

		// operator~(A) = A?
		friend Process operator~(ProcessRef lhs)
		{
			return Process(lhs, !lhs.IsOptional());
		}

		// operator+(A) = A A*
		friend Process operator+(ProcessRef lhs)
		{
			return Process([=](Info info)
			{
				if (Info i = lhs(info); lhs.IsNotEqual(i, info))
					return (+lhs)(i);

				return info;
			});
		}

		// operator*(A) = A*
		friend Process operator*(ProcessRef lhs)
		{
			return ~+ lhs;
		}

		// operator^(A,B) = A (not followed by B)
		friend Process operator^(ProcessRef pro, ProcessRef except)
		{
			return Process([=](Info info)
			{
				if (Info i = pro(info); pro.IsNotEqual(i, info))
					if (Info j = except(i); !except.IsNotEqual(j, i))
						return i;

				return info;
			}, pro.IsOptional());
		}


		using ModifierProcess = std::function<Modifier(Modifier, LangRef)>;
		using ModifierProcessRef = const ModifierProcess &;

		// Catch Info
		static Process Catch(ProcessRef pro, ModifierProcessRef mod)
		{
			return Process([=](Info info)
			{
				if (Info i = pro(info); pro.IsNotEqual(i, info))
				{
					return Info(i.first, mod(i.second, Lang(info.first.data(), info.first.size() - i.first.size())));
				}

				return info;
			});
		}

		// Catch Info(token) then push it to the parseTree
		static Process Catch(ProcessRef pro, const typename Unit::String& tokenName)
		{
			return Catch(pro, [=](Modifier modi, LangRef lang)
			{
				modi.childrenPushBack(Unit::Type::token, tokenName, lang);
				return modi;
			});
		}


		using ModifierChange = std::function<Modifier(Modifier)>;
		using ModifierChangeRef = const ModifierChange &;

		// Change modifier in Info
		static Process Change(ModifierChangeRef mod)
		{
			return Process([=](Info info)
			{
				return Info(info.first, mod(info.second));
			},true);
		}

		static ModifierChange IntoTerm(const typename Unit::String& termName)
		{
			return [=](Modifier modi)
			{
				modi.childrenPushBack(Unit::Type::term, termName);
				return --modi.childrenEnd();
			};
		}

		// Enter a term
		static Process ChangeIn(const typename Unit::String& termName)
		{
			return Change(IntoTerm(termName));
		}

		inline static const ModifierChange OutofTerm = [](Modifier modi)
		{
			return modi.parent();
		};

		static ModifierChange OutofTermWithCuttingUnused(Size minAllowedChildSize)
		{
			return [=](Modifier modi)
			{
				if(minAllowedChildSize > modi.childrenSize())
				{
					if (modi.childrenSize() != 0)
					{
						for (auto moved = modi.childrenBegin(); moved != modi.childrenEnd(); ++moved)
						{
							moved.thisCopyTo(modi);
						}
					}

					auto parent = modi.parent();
					modi.thisErase(modi);
					return parent;
				}

				return OutofTerm(modi);
			};
		}

		// Leave a term
		static Process ChangeOut(const bool cutUnusedUnit = false)
		{
			return Change(OutofTermWithCuttingUnused(cutUnusedUnit? 2:0));
		}
	};

	using ParserCombinator = BasicParserCombinator<std::string_view>;

}