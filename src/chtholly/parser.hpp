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
		// SingleExpression = DoWhileLoopExpression
		static Info SingleExpressionImpl(Info info)
		{
			return PairExpression(info);
		};

		inline static const Process SingleExpression = Process(SingleExpressionImpl);

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
					~Term(Catch(Match({ ',',';' }), "Separator")),
					Change([](Modifier modi)
					{
						if (modi.childrenSize() < 2) return modi;
						
						auto back = --modi.childrenEnd();
						if(back.value().name == "Separator" && (--back).value().name == "Separator")
						{
							back.thisErase(back);
						}

						return modi;
					})
				);
		}

		// Expression = SingleExpression ((';'|',') SingleExpression)* (';'|',')?
		inline static const Process Expression = Process([](Info info) {
			return
				(
					ChangeIn("Expression"),
					MultiExpressionPackage(SingleExpression),
					ChangeOut(true)
				)(info);
		});

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Term(Catch(operatorMatcher,"BinaryOperator")), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		inline static const Process ExpressionList = Process([](Info info) {
			return
				(
					Term(Match('(')),
					Expression,
					Term(Match(')'))
				)(info);
		});

		// ArrayList = '[' (SingleExpression (',' SingleExpression)*)? ']'
		inline static const Process ArrayList = Process([](Info info) {
			return
				(
					Term(Match('[')),
					ChangeIn("ArrayList"),
					~(
						SingleExpression,
						*(
							Term(Match(',')),
							SingleExpression
							)
						),
					Term(Match(']')),
					Change(RemoveFailedBlankTerm),
					ChangeOut()
				)(info);
		});

		// DictList = '{' (SingleExpression (',' SingleExpression)*)? '}'
		inline static const Process DictList = Process([](Info info) {
			return
				(
					Term(Match('{')),
					ChangeIn("DictList"),
					~(
						SingleExpression,
						*(
							Term(Match(',')),
							SingleExpression
							)
						),
					Term(Match('}')),
					Change(RemoveFailedBlankTerm),
					ChangeOut()
				)(info);
		});

		// UndefExpression = '(' ')'
		inline static const Process UndefExpression = Process([](Info info) {
			return
				(
					Term(Match('(')),
					Term(Match(')')),
					ChangeIn("UndefExpression"),
					ChangeOut()
				)(info);
		});

		// List = UndefExpression | ExpressionList | ArrayList | DictList
		inline static const Process List = Process([](Info info) {
			return
				(
					UndefExpression |
					ExpressionList |
					ArrayList |
					DictList
				)(info);
		});

		// PrimaryExpression = Literal | Identifier | List
		inline static const Process PrimaryExpression = Process([](Info info) {
			return
				(
					Term(Literal) |
					Term(Identifier) |
					List
				)(info);
		});

		// ConstraintPartAtConstraintExperssion = ':' PrimaryExpression
		inline static const Process ConstraintPartAtConstraintExperssion = Process([](Info info) {
			return
				(
					Term(Match(':')),
					PrimaryExpression
				)(info);
		});

		// ConstraintPartAtConstraintExperssionAtPatternExperssion = ':' SingleExpression
		inline static const Process ConstraintPartAtConstraintExperssionAtPatternExperssion = Process([](Info info) {
			return
				(
					Term(Match(':')),
					SingleExpression
				)(info);
		});

		// ConstraintExperssion = Identifier ConstraintPartAtConstraintExperssion?
		inline static const Process ConstraintExperssion = Process([](Info info) {
			return
				(
					ChangeIn("ConstraintExperssion"),
					Term(Identifier),
					~ConstraintPartAtConstraintExperssion,
					ChangeOut()
				)(info);
		});

		// ConstraintExperssionAtPatternExperssion = Identifier "..."? ConstraintPartAtConstraintExperssionAtPatternExperssion?
		inline static const Process ConstraintExperssionAtPatternExperssion = Process([](Info info) {
			return
				(
					ChangeIn("ConstraintExperssionAtPatternExperssion"),
					(
						Term(Identifier),
						~Term(Catch(Match(GL("...")), "Separator"))
						),
					~ConstraintPartAtConstraintExperssionAtPatternExperssion,
					ChangeOut()
				)(info);
		});


		// PatternExperssion = '(' ( ConstraintExperssionAtPatternExperssion ((','|';') ConstraintExperssionAtPatternExperssion)* (','|';')? | Atom ) ')' ConstraintPartAtConstraintExperssion?
		inline static const Process PatternExperssion = Process([](Info info) {
			return
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
				)(info);
		});

		// VarDefineExpression = "var" (ConstraintExperssion | PatternExperssion) ~List
		inline static const Process VarDefineExpression = Process([](Info info) {
			return
				(
					Term(MatchKey(GL("var"))),
					ChangeIn("VarDefineExpression"),
					PatternExperssion | ConstraintExperssion,
					~List,
					ChangeOut()
				)(info);
		});
		
		// ConstDefineExpression = "const" (ConstraintExperssion | PatternExperssion) ~List
		inline static const Process ConstDefineExpression = Process([](Info info) {
			return
				(
					Term(MatchKey(GL( "const"))),
					ChangeIn("ConstDefineExpression"),
					PatternExperssion | ConstraintExperssion,
					~List,
					ChangeOut()
				)(info);
		});

		// DefineExpression = PrimaryExpression | VarDefineExpression | ConstDefineExpression
		inline static const Process DefineExpression = Process([](Info info) {
			return
				(
					VarDefineExpression |
					ConstDefineExpression |
					PrimaryExpression
				)(info);
		});

		// LambdaExperssion = DefineExpression | "fn" PatternExperssion SingleExpression
		inline static const Process LambdaExperssion = Process([](Info info) {
			return
				(
					(
						Term(MatchKey(GL("fn"))),
						ChangeIn("LambdaExpression"),
						PatternExperssion,
						SingleExpression,
						ChangeOut()
					) |
					DefineExpression
				)(info);
		});

		// ConditionExpression = LambdaExperssion | "if" '(' Expression ')' SingleExpression ("else" SingleExpression)?
		inline static const Process ConditionExpression = Process([](Info info) {
			return
				(
					(
						Term(MatchKey(GL( "if"))),
						ChangeIn("ConditionExpression"),
						ExpressionList,
						SingleExpression,
						~(
							Term(MatchKey(GL( "else"))),
							SingleExpression
						),
						ChangeOut()
					) |
					LambdaExperssion
				)(info);
		});

		// ReturnExpression = ConditionExpression | "return" SingleExpression?
		inline static const Process ReturnExpression = Process([](Info info) {
			return
				(
					(
						Term(MatchKey(GL( "return"))),
						ChangeIn("ReturnExpression"),
						~Process(SingleExpression),
						Change(RemoveFailedBlankTerm),
						ChangeOut()
					) |
					ConditionExpression
				)(info);
		});

		// BreakExpression = "break" SingleExpression?
		inline static const Process BreakExpression = Process([](Info info) {
			return
				(
					Term(MatchKey(GL("break"))),
					ChangeIn("BreakExpression"),
					~Process(SingleExpression),
					Change(RemoveFailedBlankTerm),
					ChangeOut()
				)(info);
		});

		// ContinueExpression = "continue" SingleExpression?
		inline static const Process ContinueExpression = Process([](Info info) {
			return
				(
					Term(MatchKey(GL("continue"))),
					ChangeIn("ContinueExpression"),
					~Process(SingleExpression),
					Change(RemoveFailedBlankTerm),
					ChangeOut()
				)(info);
		});

		// LoopControlExpression = ReturnExpression | (BreakExpression | ContinueExpression)
		inline static const Process LoopControlExpression = Process([](Info info) {
			return
				(
					BreakExpression		|
					ContinueExpression	|
					ReturnExpression
				)(info);
		});

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SingleExpression
		inline static const Process WhileLoopExpression = Process([](Info info) {
			return
				(
					(
						Term(MatchKey(GL( "while"))),
						ChangeIn("WhileLoopExpression"),
						ExpressionList,
						SingleExpression, 
						~(
							Term(MatchKey(GL( "else"))),
							SingleExpression
						),
						ChangeOut()
					) |
					LoopControlExpression
				)(info);
		});

		// DoWhileLoopExpression = WhileLoopExpression | "do" SingleExpression "while" '(' Expression ')'
		inline static const Process DoWhileLoopExpression = Process([](Info info) {
			return
				(
					(
						Term(MatchKey(GL( "do"))),
						ChangeIn("DoWhileLoopExpression"),
						SingleExpression,
						Term(MatchKey(GL( "while"))),
						ExpressionList,
						~(
							Term(MatchKey(GL( "else"))),
							SingleExpression
						),
						ChangeOut()
					) |
					WhileLoopExpression
				)(info);
		});

		// FunctionExpression = DoWhileLoopExpression List*
		inline static const Process FunctionExpression = Process([](Info info) {
			return
				(
					ChangeIn("FunctionExpression"),
					DoWhileLoopExpression,
					*List,
					ChangeOut(true)
				)(info);
		});

		// PointExpression = FunctionExpression | PointExpression '->' FunctionExpression
		inline static const Process PointExpression = Process([](Info info) {
			return
				(
					ChangeIn("PointExpression"),
					BinaryOperator(FunctionExpression, Match(GL( "->"))),
					ChangeOut(true)
				)(info);
		});

		// FoldExperssion = PointExpression "..."*
		inline static const Process FoldExpression = Process([](Info info) {
			return
				(
					ChangeIn("FoldExperssion"),
					PointExpression,
					~Term(Catch(Match(GL("...")),"Separator")),
					ChangeOut(true)
				)(info);
		});

		// UnaryExpression = FoldExperssion | (('+' | '-') | "not") UnaryExpression
		inline static const Process UnaryExpression = Process([](Info info) {
			return
				(
					ChangeIn("UnaryExpression"),
					*(
						Term(Catch(Match({ '+','-' }), "UnaryOperator")) |
						Term(Catch(MatchKey(GL("not")), "UnaryOperator"))
					),
					FoldExpression,
					ChangeOut(true)
				)(info);
		});

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		inline static const Process MultiplicativeExpression = Process([](Info info) {
			return
				(
					ChangeIn("MultiplicativeExpression"),
					BinaryOperator(UnaryExpression, Match({ '*','/','%' }) ^Match('=')),
					ChangeOut(true)
				)(info);
		});

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		inline static const Process AdditiveExpression = Process([](Info info) {
			return
				(
					ChangeIn("AdditiveExpression"),
					BinaryOperator(MultiplicativeExpression, Match({ '+','-' }) ^Match('=')),
					ChangeOut(true)
				)(info);
		});

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		inline static const Process RelationalExpression = Process([](Info info) {
			return
				(
					ChangeIn("RelationalExpression"),
					BinaryOperator(AdditiveExpression, Match({ GL("<="), GL(">="), GL(">") }) | (Match('<') ^ Match('>'))),
					ChangeOut(true)
				)(info);
		});

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		inline static const Process EqualityExpression = Process([](Info info) {
			return
				(
					ChangeIn("EqualityExpression"),
					BinaryOperator(RelationalExpression, Match({ GL("=="), GL("<>") })),
					ChangeOut(true)
				)(info);
		});

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		inline static const Process LogicalAndExpression = Process([](Info info) {
			return
				(
					ChangeIn("LogicalAndExpression"),
					BinaryOperator(EqualityExpression, MatchKey(GL( "and"))),
					ChangeOut(true)
				)(info);
		});

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		inline static const Process LogicalOrExpression = Process([](Info info) {
			return
				(
					ChangeIn("LogicalOrExpression"),
					BinaryOperator(LogicalAndExpression, MatchKey(GL( "or"))),
					ChangeOut(true)
				)(info);
		});

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		inline static const Process AssignmentOperator = Process([](Info info) {
			return
				Match({ GL("="), GL("*="), GL("/="), GL("%="), GL("+="), GL("-=") })(info);
		});

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SingleExpression
		inline static const Process AssignmentExpression = Process([](Info info) {
			return
				(
					ChangeIn("AssignmentExpression"),
					LogicalOrExpression, *(Term(Catch(AssignmentOperator,"BinaryOperator")), SingleExpression),
					ChangeOut(true)
				)(info);
		});

		// PairExperssion = AssignmentExpression | PairExperssion ':' SingleExpression
		inline static const Process PairExpression = Process([](Info info) {
			return
				(
					ChangeIn("PairExpression"),
					AssignmentExpression, *(Term(Match(':')), SingleExpression),
					ChangeOut(true)
				)(info);
		});

		#undef GL
	};

	using Parser = BasicParser<ParserCombinator::Lang>;

}

#undef G