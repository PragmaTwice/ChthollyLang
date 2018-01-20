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
#include <type_traits>

#include "parserc.hpp"
#include "chartype.hpp"

#define G(charType,stringLiteral) \
	std::get<std::basic_string_view<charType>>(std::make_tuple(stringLiteral##sv, L##stringLiteral##sv))


namespace Chtholly
{
	using namespace std::literals;

	template <typename StringView>
	class BasicParser : public BasicParserCombinator<StringView>
	{

	public:

		BasicParser() = delete;

		/*
		// Space = '\t' | '\n' | '\v' | '\f' | '\r' | ' '
		inline static Process Space = Match(isspace);

		// Digit = '0' ... '9'
		inline static Process Digit = Match(isdigit);

		// UpperCaseLetter = 'A' ... 'Z'
		inline static Process UpperCaseLetter = Match(isupper);

		// LowerCaseLetter = 'a' ... 'z'
		inline static Process LowerCaseLetter = Match(islower);

		// Letter = UpperCaseLetter | LowerCaseLetter
		inline static Process Letter = Match(isalpha);

		// DigitOrLetter = Digit | Letter
		inline static Process DigitOrLetter = Match(isalnum);
		*/

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

		// IntLiteral = digit+
		inline static Process IntLiteral =
			Catch(+Match(CType::isDigit), "IntLiteral");

		// FloatLiteral = IntLiteral '.' IntLiteral£¿ (('E'|'e') ('+'|'-')? IntLiteral)?
		inline static Process FloatLiteral =
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
		inline static Process UnescapedCharacter =
			Match([=](Char c)
			{
				return c != '"' && c != '\\';
			});

		// EscapedCharacter = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
		inline static Process EscapedCharacter =
			(
				Match('\\'),
				Match({ '"','\\','b','f','n','r','t','v' })
			);

		// StringLiteral = '"' (EscapedCharacter|UnescapedCharacter)* '"'
		inline static Process StringLiteral =
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
		inline static Process Identifier =
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
		inline static Process NullLiteral =
			Catch(MatchKey(G(Char,"null")), "NullLiteral");

		// UndefinedLiteral = "undef"
		inline static Process UndefinedLiteral =
			Catch(MatchKey(G(Char, "undef")), "UndefinedLiteral");

		// TrueLiteral = "true"
		inline static Process TrueLiteral =
			Catch(MatchKey(G(Char, "true")), "TrueLiteral");

		// FalseLiteral = "false"
		inline static Process FalseLiteral =
			Catch(MatchKey(G(Char, "false")), "FalseLiteral");

		// Literal = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
		inline static Process Literal =
			(
				FloatLiteral     |
				IntLiteral       |
				StringLiteral    |
				NullLiteral      |
				UndefinedLiteral |
				TrueLiteral      |
				FalseLiteral
			);

		// Term(A) = Space* A
		static Process Term(ProcessRef lhs)
		{
			return *Match(CType::isSpace), lhs;
		}

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Term(Catch(operatorMatcher,"BinaryOperator")), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		inline static Process ExpressionList =
			(
				Term(Match('(')),
				Expression,
				Term(Match(')'))
			);

		// ArrayList = '[' (SigleExpression (',' SigleExpression)*)? ']'
		inline static Process ArrayList =
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
		inline static Process DictList =
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

		// NullFunctionArg = '(' ')'
		inline static Process NullFunctionArg =
			(
				Term(Match('(')),
				Term(Match(')')),
				ChangeIn("NullFunctionArg"),
				ChangeOut()
			);

		// FunctionArgList = ExpressionList | ArrayList | DictList
		inline static Process FunctionArgList =
			(
				ExpressionList |
				ArrayList      |
				DictList
			);

		// VarDefineExpression = "var" Identifier
		inline static Process VarDefineExpression =
			(
				Term(MatchKey(G(Char, "var"))),
				ChangeIn("VarDefineExpression"),
				Term(Identifier),
				ChangeOut()
			);

		// ConstDefineExpression = "const" Identifier
		inline static Process ConstDefineExpression =
			(
				Term(MatchKey(G(Char, "const"))),
				ChangeIn("ConstDefineExpression"),
				Term(Identifier),
				ChangeOut()
			);

		// PrimaryExpression = Identifier | Literal | FunctionArgList | VarDefineExpression | ConstDefineExpression
		inline static Process PrimaryExpression =
			(
				Term(Literal)         |
				VarDefineExpression   |
				ConstDefineExpression |
				Term(Identifier)      |
				FunctionArgList
			);

		// ConditionExpression = PairForDictList | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		inline static Process ConditionExpression =
			(
				(
					Term(MatchKey(G(Char, "if"))),
					ChangeIn("ConditionExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					~(
						Term(MatchKey(G(Char, "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				PrimaryExpression
			);

		// ReturnExpression = ConditionExpression | "return" SigleExpression?
		inline static Process ReturnExpression =
			(
				(
					Term(MatchKey(G(Char, "return"))),
					ChangeIn("ReturnExpression"),
					~Process(SigleExpression),
					ChangeOut()
				) |
				ConditionExpression
			);

		// LoopControlExpression = ReturnExpression | ("break"|"continue") SigleExpression?
		inline static Process LoopControlExpression =
			(
				(
					Term(MatchKey({ G(Char, "break"), G(Char,"continue") })),
					ChangeIn("LoopControlExpression"),
					~Process(SigleExpression),
					ChangeOut()
				) |
				ReturnExpression
			);

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SigleExpression
		inline static Process WhileLoopExpression =
			(
				(
					Term(MatchKey(G(Char, "while"))),
					ChangeIn("WhileLoopExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression, 
					~(
						Term(MatchKey(G(Char, "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				LoopControlExpression
			);

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		inline static Process DoWhileLoopExpression =
			(
				(
					Term(MatchKey(G(Char, "do"))),
					ChangeIn("DoWhileLoopExpression"),
					SigleExpression,
					Term(MatchKey(G(Char, "while"))),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					~(
						Term(MatchKey(G(Char, "else"))),
						SigleExpression
					),
					ChangeOut()
				) |
				WhileLoopExpression
			);

		// FunctionExpression = PrimaryExpression (FunctionArgList | NullFunctionArg)*
		inline static Process FunctionExpression =
			(
				ChangeIn("FunctionExpression"),
				DoWhileLoopExpression,
				*(
					NullFunctionArg |
					FunctionArgList
				),
				ChangeOut(true)
			);

		// PointExpression = FunctionExpression | PointExpression '->' PrimaryExpression
		inline static Process PointExpression =
			(
				ChangeIn("PointExpression"),
				BinaryOperator(FunctionExpression, Match(G(Char, "->"))),
				ChangeOut(true)
			);

		// UnaryExpression = PointExpression | (('+' | '-') | "not") UnaryExpression
		inline static Process UnaryExpression =
			(
				ChangeIn("UnaryExpression"),
				(
					(
						Term(Match({ '+','-' })) |
						Term(MatchKey(G(Char, "not")))
					),
					UnaryExpression
				) |
				PointExpression,
				ChangeOut(true)
			);

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		inline static Process MultiplicativeExpression =
			(
				ChangeIn("MultiplicativeExpression"),
				BinaryOperator(UnaryExpression, Match({ '*','/','%' }) ^Match('=')),
				ChangeOut(true)
			);

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		inline static Process AdditiveExpression =
			(
				ChangeIn("AdditiveExpression"),
				BinaryOperator(MultiplicativeExpression, Match({ '+','-' }) ^Match('=')),
				ChangeOut(true)
			);

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		inline static Process RelationalExpression =
			(
				ChangeIn("RelationalExpression"),
				BinaryOperator(AdditiveExpression, Match({ G(Char,"<="), G(Char,">="), G(Char,"<"), G(Char,">") })),
				ChangeOut(true)
			);

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		inline static Process EqualityExpression =
			(
				ChangeIn("EqualityExpression"),
				BinaryOperator(RelationalExpression, Match({ G(Char,"=="),G(Char,"<>") })),
				ChangeOut(true)
			);

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		inline static Process LogicalAndExpression =
			(
				ChangeIn("LogicalAndExpression"),
				BinaryOperator(EqualityExpression, MatchKey(G(Char, "and"))),
				ChangeOut(true)
			);

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		inline static Process LogicalOrExpression =
			(
				ChangeIn("LogicalOrExpression"),
				BinaryOperator(LogicalAndExpression, MatchKey(G(Char, "or"))),
				ChangeOut(true)
			);

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		inline static Process AssignmentOperator =
			Match({ G(Char,"="), G(Char,"*="), G(Char,"/="), G(Char,"%="), G(Char,"+="), G(Char,"-=") });

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression
		inline static Process AssignmentExpression =
			(
				ChangeIn("AssignmentExpression"),
				LogicalOrExpression, *(Term(Catch(AssignmentOperator,"BinaryOperator")), SigleExpression),
				ChangeOut(true)
			);

		// PairExperssion = AssignmentExpression | PairExperssion ':' SigleExpression
		inline static Process PairForDictList =
			(
				ChangeIn("PairForDictList"),
				AssignmentExpression, *(Term(Match(':')), SigleExpression),
				ChangeOut(true)
			);

		// Cannot be inline static Process
		// SigleExpression = DoWhileLoopExpression
		static Info SigleExpression(Info info)
		{
			return PairForDictList(info);
		}

		// Cannot be inline static Process
		// Expression = SigleExpression ((';'|',') SigleExpression)* ('.'|';')?
		static Info Expression(Info info)
		{
			return (
				ChangeIn("Expression"),
				SigleExpression,
				*(
					Term(Catch(Match({ ',',';' }),"Separator")),
					SigleExpression
					),
				~Term(Catch(Match({ '.' , ';' }), "Separator")),
				ChangeOut(true)
				)(info);
		}
	};

	using Parser = BasicParser<ParserCombinator::Lang>;

}

#undef G