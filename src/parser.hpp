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
#include <string_view>

#include "parserc.hpp"

namespace Chtholly
{
	using namespace std::literals;

	template <typename StringView>
	class BasicParser;

	template <>
	class BasicParser<ParserCombinator::Lang> : public ParserCombinator
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


		// IntLiteral = digit+
		inline static Process IntLiteral =
			Catch(+Match(isdigit), "IntLiteral");

		// FloatLiteral = IntLiteral '.' IntLiteral£¿ (('E'|'e') ('+'|'-')? IntLiteral)?
		inline static Process FloatLiteral =
			Catch(
				(
					+Match(isdigit),
					Match('.'),
					*Match(isdigit),
					~(
						Match({ 'E','e' }),
						~Match({ '+','-' }),
						+Match(isdigit)
					)
				),
				"FloatLiteral");

		// UnescapedCharacter = not ('"' | '\\')
		inline static Process UnescapedCharacter =
			Match([=](Char c)
			{
				if (c == '"' || c == '\\') return false;
				return true;
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
							Match(isalpha) | Match('_')
						),
						*(
							Match(isalnum) | Match('_')
						)
					), 
				"Identifier");

		// NullLiteral = "null"
		inline static Process NullLiteral =
			Catch(Match("null"), "NullLiteral");

		// UndefinedLiteral = "undef"
		inline static Process UndefinedLiteral =
			Catch(Match("undef"), "UndefinedLiteral");

		// TrueLiteral = "true"
		inline static Process TrueLiteral =
			Catch(Match("true"), "TrueLiteral");

		// FalseLiteral = "false"
		inline static Process FalseLiteral =
			Catch(Match("false"), "FalseLiteral");

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
			return *Match(isspace), lhs;
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
				Term(Match("var")),
				ChangeIn("VarDefineExpression"),
				Term(Identifier),
				ChangeOut()
			);

		// ConstDefineExpression = "const" Identifier
		inline static Process ConstDefineExpression =
			(
				Term(Match("const")),
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

		// FunctionExpression = PrimaryExpression (FunctionArgList | NullFunctionArg)*
		inline static Process FunctionExpression =
			(
				ChangeIn("FunctionExpression"),
				PrimaryExpression,
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
				BinaryOperator(FunctionExpression, Match("->")),
				ChangeOut(true)
			);

		// UnaryExpression = PointExpression | (('+' | '-') | "not") UnaryExpression
		inline static Process UnaryExpression =
			(
				ChangeIn("UnaryExpression"),
				(
					(
						Term(Match({ '+','-' })) |
						Term(Match("not"))
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
				BinaryOperator(AdditiveExpression, Match({ "<=", ">=", "<", ">" })),
				ChangeOut(true)
			);

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		inline static Process EqualityExpression =
			(
				ChangeIn("EqualityExpression"),
				BinaryOperator(RelationalExpression, Match({ "=="sv,"<>"sv })),
				ChangeOut(true)
			);

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		inline static Process LogicalAndExpression =
			(
				ChangeIn("LogicalAndExpression"),
				BinaryOperator(EqualityExpression, Match("and")),
				ChangeOut(true)
			);

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		inline static Process LogicalOrExpression =
			(
				ChangeIn("LogicalOrExpression"),
				BinaryOperator(LogicalAndExpression, Match("or")),
				ChangeOut(true)
			);

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		inline static Process AssignmentOperator =
			Match({ "=", "*=", "/=", "%=", "+=", "-=" });

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

		// ConditionExpression = PairForDictList | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		inline static Process ConditionExpression =
			(
				(
					Term(Match("if")),
					ChangeIn("ConditionExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					~(
						Term(Match("else")),
						SigleExpression
					),
					ChangeOut()
				) |
				PairForDictList
			);

		// ReturnExpression = ConditionExpression | "return" SigleExpression?
		inline static Process ReturnExpression =
			(
				(
					Term(Match("return")),
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
					Term(Match({ "break"sv,"continue"sv })),
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
					Term(Match("while")),
					ChangeIn("WhileLoopExpression"),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					ChangeOut()
				) |
				LoopControlExpression
			);

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		inline static Process DoWhileLoopExpression =
			(
				(
					Term(Match("do")),
					ChangeIn("DoWhileLoopExpression"),
					SigleExpression,
					Term(Match("while")),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					ChangeOut()
				) |
				WhileLoopExpression
			);

		// Cannot be inline static Process
		// SigleExpression = DoWhileLoopExpression
		static Info SigleExpression(Info info)
		{
			return DoWhileLoopExpression(info);
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