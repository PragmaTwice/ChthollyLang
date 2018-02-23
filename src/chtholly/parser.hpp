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
#include <tuple>

#include "parserc.hpp"
#include "chartype.hpp"

#define G(StringView,stringLiteral) \
	std::get<StringView>(std::make_tuple(stringLiteral##sv, L##stringLiteral##sv))


namespace Chtholly
{
	using namespace std::literals;

	template <typename StringView>
	class BasicParser : public BasicParserCombinator<StringView>
	{
	protected:

		using Super = BasicParserCombinator<StringView>;

	public:

		BasicParser() = delete;

		// Types:

		using typename Super::Lang;
		using typename Super::LangRef;

		using typename Super::Tree;

		using typename Super::Modifier;
		using typename Super::ModifierRef;

		using typename Super::Info;
		using typename Super::InfoRef;

		using typename Super::Unit;

		using typename Super::Char;
		using typename Super::Iter;
		using typename Super::Size;

		using typename Super::Process;
		using typename Super::ProcessRef;

		template <typename ...Args>
		using Predicate = typename Super::template Predicate<Args...>;

		template <typename ...Args>
		using PredicateRef = const Predicate<Args...> &;

		template <typename T>
		using InitList = typename Super::template InitList<T>;

		template <typename T>
		using InitListRef = const InitList<T>&;

		using typename Super::ModifierProcess;
		using typename Super::ModifierProcessRef;

		using typename Super::ModifierChange;
		using typename Super::ModifierChangeRef;

		// Methods:

		using Super::Match;

		using Super::Atom;
		using Super::AnyChar;
		using Super::AnyCharUntil;

		using Super::Catch;

		using Super::Change;
		using Super::IntoTerm;
		using Super::ChangeIn;
		using Super::OutofTerm;
		using Super::OutofTermWithCuttingUnused;
		using Super::ChangeOut;
		

		inline static const ModifierChange RemoveFailedBlankTerm = [](Modifier modi)
		{
			if (modi.childrenSize() > 0)
			{
				auto removed = --modi.childrenEnd();

				auto i = removed;
				while (i.childrenSize() == 1)  i = i.childrenBegin();
				if (i.childrenSize() == 0 && i.value().type == Unit::Type::term && i.value().name == "FunctionExpression")
				{
					removed.thisErase(removed);
				}
			}

			return modi;
		};

		/*
		// Space = '\t' | '\n' | '\v' | '\f' | '\r' | ' '
		inline static const Process Space = Match(isspace);

		// Digit = '0' ... '9'
		inline static const Process Digit = Match(isdigit);

		// UpperCaseLetter = 'A' ... 'Z'
		inline static const Process UpperCaseLetter = Match(isupper);

		// LowerCaseLetter = 'a' ... 'z'
		inline static const Process LowerCaseLetter = Match(islower);

		// Letter = UpperCaseLetter | LowerCaseLetter
		inline static const Process Letter = Match(isalpha);

		// DigitOrLetter = Digit | Letter
		inline static const Process DigitOrLetter = Match(isalnum);
		*/

		#define GL(stringLiteral) G(Lang,stringLiteral)

		using CType = CharType<Char>;


		// MatchKey(A) = A ^ (DigitOrLetter | '_')
		static Process MatchKey(LangRef word)
		{
			return Match(word) ^ (Match(CType::isAlphaOrNum) | Match('_'));
		}

		static Process MatchKey(InitListRef<Lang> wordList)
		{
			return Match(wordList) ^ (Match(CType::isAlphaOrNum) | Match('_'));
		}

		// IntLiteral = Digit+
		inline static const Process IntLiteral =
			Catch(+Match(CType::isDigit), "IntLiteral");

		// FloatLiteral = Digit+ '.' Digit* (('E'|'e') ('+'|'-')? Digit+)?
		inline static const Process FloatLiteral =
			Catch(
				(
					+Match(CType::isDigit),
					Match('.'),
					*Match(CType::isDigit),
					~(
						Match({ 'E','e' }),
						~Match({ '+','-' }),
						+Match(CType::isDigit)
					)
				),
				"FloatLiteral");

		// UnescapedCharacter = not ('"' | '\\')
		inline static const Process UnescapedCharacter =
			Match([](Char c)
			{
				return c != '"' && c != '\\';
			});

		// EscapedCharacter = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
		inline static const Process EscapedCharacter =
			(
				Match('\\'),
				Match({ '"','\\','b','f','n','r','t','v' })
			);

		// StringLiteral = '"' (EscapedCharacter|UnescapedCharacter)* '"'
		inline static const Process StringLiteral =
			Catch(
				(
					Match('"'),
					*(
						EscapedCharacter | UnescapedCharacter
					),
					Match('"')
				), 
			"StringLiteral");

		// Identifier = (Letter | '_') (DigitOrLetter | '_')*
		inline static const Process Identifier =
			Catch(
				(
					(
						Match(CType::isAlpha) | Match('_')
					),
					*(
						Match(CType::isAlphaOrNum) | Match('_')
					)
				), 
			"Identifier");

		// NullLiteral = "null"
		inline static const Process NullLiteral =
			Catch(MatchKey(GL("null")), "NullLiteral");

		// UndefinedLiteral = "undef"
		inline static const Process UndefinedLiteral =
			Catch(MatchKey(GL( "undef")), "UndefinedLiteral");

		// TrueLiteral = "true"
		inline static const Process TrueLiteral =
			Catch(MatchKey(GL( "true")), "TrueLiteral");

		// FalseLiteral = "false"
		inline static const Process FalseLiteral =
			Catch(MatchKey(GL( "false")), "FalseLiteral");

		// Literal = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
		inline static const Process Literal =
			(
				FloatLiteral     |
				IntLiteral       |
				StringLiteral    |
				NullLiteral      |
				UndefinedLiteral |
				TrueLiteral      |
				FalseLiteral
			);

		// MultiLineComment = "/*" (not "*/")* "*/"
		inline static const Process MultiLineComment =
			(
				Match(GL("/*")),
				AnyCharUntil(Match(GL( "*/")))
			);

		// SingleLineComment = "//" (not '\n')*
		inline static const Process SingleLineComment =
			(
				Match(GL("//")),
				AnyCharUntil(Match('\n'))
			);

		// Comment = SingleLineComment | MultiLineComment
		inline static const Process Comment =
			(
				SingleLineComment | 
				MultiLineComment
			);

		// Term(A) = Space* A
		static Process Term(ProcessRef lhs)
		{
			return *(*Match(CType::isSpace), Comment), *Match(CType::isSpace), lhs;
		}

		// Cannot be inline static const Process
		// SigleExpression = DoWhileLoopExpression
		static Info SigleExpression(Info info)
		{
			return PairExperssion(info);
		}

		// Cannot be inline static const Process
		// Expression = SigleExpression ((';'|',') SigleExpression)* (';'|',')?
		static Info Expression(Info info)
		{
			return
				(
					ChangeIn("Expression"),
					MultiExpressionPackage(SigleExpression),
					ChangeOut(true)
				)(info);

		}

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Term(Catch(operatorMatcher,"BinaryOperator")), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		inline static const Process ExpressionList =
			(
				Term(Match('(')),
				Expression,
				Term(Match(')'))
			);

		// ArrayList = '[' (SigleExpression (',' SigleExpression)*)? ']'
		inline static const Process ArrayList =
			(
				Term(Match('[')),
				ChangeIn("ArrayList"),
				~(
					SigleExpression,
					*(
						Term(Match(',')),
						SigleExpression
					)
				),
				ChangeOut(),
				Term(Match(']'))
			);

		// DictList = '{' (SigleExpression (',' SigleExpression)*)? '}'
		inline static const Process DictList =
			(
				Term(Match('{')),
				ChangeIn("DictList"),
				~(
					SigleExpression,
					*(
						Term(Match(',')),
						SigleExpression
					)
				),
				ChangeOut(),
				Term(Match('}'))
			);

		// UndefExpression = '(' ')'
		inline static const Process UndefExpression =
			(
				Term(Match('(')),
				Term(Match(')')),
				ChangeIn("UndefExpression"),
				ChangeOut()
			);

		// List = UndefExpression | ExpressionList | ArrayList | DictList
		inline static const Process List =
			(
				UndefExpression|
				ExpressionList |
				ArrayList      |
				DictList
			);

		// PrimaryExpression = Literal | Identifier | List
		inline static const Process PrimaryExpression =
			(
				Term(Literal)	 |
				Term(Identifier) |
				List
			);

		// ConstraintPartAtConstraintExperssion = ':' PrimaryExpression
		inline static const Process ConstraintPartAtConstraintExperssion =
			(
				Term(Match(':')),
				PrimaryExpression
			);

		// ConstraintPartAtConstraintExperssionAtPatternExperssion = ':' SigleExpression
		inline static const Process ConstraintPartAtConstraintExperssionAtPatternExperssion =
			(
				Term(Match(':')),
				SigleExpression
			);

		// ConstraintExperssion = Identifier ConstraintPartAtConstraintExperssion?
		inline static const Process ConstraintExperssion =
			(
				ChangeIn("ConstraintExperssion"),
				Term(Identifier),
				~ConstraintPartAtConstraintExperssion,
				ChangeOut()
			);

		// ConstraintExperssionAtPatternExperssion = Identifier "..."? ConstraintPartAtConstraintExperssionAtPatternExperssion?
		inline static const Process ConstraintExperssionAtPatternExperssion =
			(
				ChangeIn("ConstraintExperssionAtPatternExperssion"),
				(
					Term(Identifier),
					~Term(Catch(Match(GL("...")), "Separator"))
				),
				~ConstraintPartAtConstraintExperssionAtPatternExperssion,
				ChangeOut()
			);

		// MultiExpressionPackage(exp) = exp ((';'|',') exp)* (';'|',')?
		static Process MultiExpressionPackage(ProcessRef exp)
		{
			return
				(
					exp,
					*(
						Term(Catch(Match({ ',',';' }), "Separator")),
						exp
					),
					Change(RemoveFailedBlankTerm),
					~Term(Catch(Match({ ',',';' }), "Separator"))
				);
		}

		// PatternExperssion = '(' ( ConstraintExperssionAtPatternExperssion ((','|';') ConstraintExperssionAtPatternExperssion)* (','|';')? | Atom ) ')' ConstraintPartAtConstraintExperssion?
		inline static const Process PatternExperssion =
			(
				Term(Match('(')),
				ChangeIn("PatternExperssion"),
				(
					Term(Match(')')) |
					(
						MultiExpressionPackage(ConstraintExperssionAtPatternExperssion),
						Term(Match(')')),
						~ConstraintPartAtConstraintExperssion
					)
				),
				ChangeOut()
			);

		// VarDefineExpression = "var" (ConstraintExperssion | PatternExperssion) ~List
		inline static const Process VarDefineExpression =
			(
				Term(MatchKey(GL( "var"))),
				ChangeIn("VarDefineExpression"),
				PatternExperssion | ConstraintExperssion,
				~List,
				ChangeOut()
			);
		
		// ConstDefineExpression = "const" (ConstraintExperssion | PatternExperssion) ~List
		inline static const Process ConstDefineExpression =
			(
				Term(MatchKey(GL( "const"))),
				ChangeIn("ConstDefineExpression"),
				PatternExperssion | ConstraintExperssion,
				~List,
				ChangeOut()
			);

		// DefineExpression = PrimaryExpression | VarDefineExpression | ConstDefineExpression
		inline static const Process DefineExpression =
			(
				VarDefineExpression |
				ConstDefineExpression |
				PrimaryExpression
			);

		// LambdaExperssion = DefineExpression | "fn" PatternExperssion SigleExpression
		inline static const Process LambdaExperssion =
			(
				(
					Term(MatchKey(GL("fn"))),
					ChangeIn("LambdaExpression"),
					PatternExperssion,
					SigleExpression,
					ChangeOut()
				) |
				DefineExpression
			);

		// ConditionExpression = LambdaExperssion | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		inline static const Process ConditionExpression =
			(
				(
					Term(MatchKey(GL( "if"))),
					ChangeIn("ConditionExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				LambdaExperssion
			);

		// ReturnExpression = ConditionExpression | "return" SigleExpression?
		inline static const Process ReturnExpression =
			(
				(
					Term(MatchKey(GL( "return"))),
					ChangeIn("ReturnExpression"),
					~Process(SigleExpression),
					Change(RemoveFailedBlankTerm),
					ChangeOut()
				) |
				ConditionExpression
			);

		// BreakExpression = "break" SigleExpression?
		inline static const Process BreakExpression =
			(
				Term(MatchKey(GL("break"))),
				ChangeIn("BreakExpression"),
				~Process(SigleExpression),
				Change(RemoveFailedBlankTerm),
				ChangeOut()
			);

		// ContinueExpression = "continue" SigleExpression?
		inline static const Process ContinueExpression =
			(
				Term(MatchKey(GL("continue"))),
				ChangeIn("ContinueExpression"),
				~Process(SigleExpression),
				Change(RemoveFailedBlankTerm),
				ChangeOut()
			);

		// LoopControlExpression = ReturnExpression | (BreakExpression | ContinueExpression)
		inline static const Process LoopControlExpression =
			(
				BreakExpression		|
				ContinueExpression	|
				ReturnExpression
			);

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SigleExpression
		inline static const Process WhileLoopExpression =
			(
				(
					Term(MatchKey(GL( "while"))),
					ChangeIn("WhileLoopExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression, 
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				LoopControlExpression
			);

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		inline static const Process DoWhileLoopExpression =
			(
				(
					Term(MatchKey(GL( "do"))),
					ChangeIn("DoWhileLoopExpression"),
					SigleExpression,
					Term(MatchKey(GL( "while"))),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					~(
						Term(MatchKey(GL( "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				WhileLoopExpression
			);

		// FunctionExpression = DoWhileLoopExpression List*
		inline static const Process FunctionExpression =
			(
				ChangeIn("FunctionExpression"),
				DoWhileLoopExpression,
				*List,
				ChangeOut(true)
			);

		// PointExpression = FunctionExpression | PointExpression '->' FunctionExpression
		inline static const Process PointExpression =
			(
				ChangeIn("PointExpression"),
				BinaryOperator(FunctionExpression, Match(GL( "->"))),
				ChangeOut(true)
			);

		// FoldExperssion = PointExpression "..."*
		inline static const Process FoldExperssion =
			(
				ChangeIn("FoldExperssion"),
				PointExpression,
				*Term(Catch(Match(GL("...")),"Separator")),
				ChangeOut(true)
			);

		// UnaryExpression = FoldExperssion | (('+' | '-') | "not") UnaryExpression
		inline static const Process UnaryExpression =
			(
				ChangeIn("UnaryExpression"),
				*(
					Term(Catch(Match({ '+','-' }), "UnaryOperator")) |
					Term(Catch(MatchKey(GL("not")), "UnaryOperator"))
				),
				FoldExperssion,
				ChangeOut(true)
			);

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		inline static const Process MultiplicativeExpression =
			(
				ChangeIn("MultiplicativeExpression"),
				BinaryOperator(UnaryExpression, Match({ '*','/','%' }) ^Match('=')),
				ChangeOut(true)
			);

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		inline static const Process AdditiveExpression =
			(
				ChangeIn("AdditiveExpression"),
				BinaryOperator(MultiplicativeExpression, Match({ '+','-' }) ^Match('=')),
				ChangeOut(true)
			);

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		inline static const Process RelationalExpression =
			(
				ChangeIn("RelationalExpression"),
				BinaryOperator(AdditiveExpression, Match({ GL("<="), GL(">="), GL("<"), GL(">") })),
				ChangeOut(true)
			);

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		inline static const Process EqualityExpression =
			(
				ChangeIn("EqualityExpression"),
				BinaryOperator(RelationalExpression, Match({ GL("=="), GL("<>") })),
				ChangeOut(true)
			);

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		inline static const Process LogicalAndExpression =
			(
				ChangeIn("LogicalAndExpression"),
				BinaryOperator(EqualityExpression, MatchKey(GL( "and"))),
				ChangeOut(true)
			);

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		inline static const Process LogicalOrExpression =
			(
				ChangeIn("LogicalOrExpression"),
				BinaryOperator(LogicalAndExpression, MatchKey(GL( "or"))),
				ChangeOut(true)
			);

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		inline static const Process AssignmentOperator =
			Match({ GL("="), GL("*="), GL("/="), GL("%="), GL("+="), GL("-=") });

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression
		inline static const Process AssignmentExpression =
			(
				ChangeIn("AssignmentExpression"),
				LogicalOrExpression, *(Term(Catch(AssignmentOperator,"BinaryOperator")), SigleExpression),
				ChangeOut(true)
			);

		// PairExperssion = AssignmentExpression | PairExperssion ':' SigleExpression
		inline static const Process PairExperssion =
			(
				ChangeIn("PairExperssion"),
				AssignmentExpression, *(Term(Match(':')), SigleExpression),
				ChangeOut(true)
			);

		#undef GL
	};

	using Parser = BasicParser<ParserCombinator::Lang>;

}

#undef G