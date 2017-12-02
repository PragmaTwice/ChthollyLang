#pragma once

#include "parserc.hpp"
#include <cctype>
#include <iostream>

namespace Chtholly
{
	
	template <typename StringView>
	class BasicParser;

	template <>
	class BasicParser<ParserCombinator::Lang> : public ParserCombinator
	{

	public:

		BasicParser() = delete;

		/*
		// Space = '\t' | '\n' | '\v' | '\f' | '\r' | ' ' 
		static Lang Space(LangRef lang)
		{
			return Match(isspace)(lang);
		}

		// Digit = '0' ... '9'
		static Lang Digit(LangRef lang)
		{
			return Match(isdigit)(lang);
		}

		// UpperCaseLetter = 'A' ... 'Z'
		static Lang UpperCaseLetter(LangRef lang)
		{
			return Match(isupper)(lang);
		}

		// LowerCaseLetter = 'a' ... 'z'
		static Lang LowerCaseLetter(LangRef lang)
		{
			return Match(islower)(lang);
		}

		// Letter = UpperCaseLetter | LowerCaseLetter
		static Lang Letter(LangRef lang)
		{
			return Match(isalpha)(lang);
		}

		// Letter = UpperCaseLetter | LowerCaseLetter
		static Lang DigitOrLetter(LangRef lang)
		{
			return Match(isalnum)(lang);
		}
		*/

		/*
		static Process Catch(LangRef begin)
		{
			return [=](LangRef lang)
			{
				std::cout << "[" << Lang(begin.data(),begin.size()-lang.size()) << "]" << std::ends;
				return lang;
			};
		}
		*/

		// IntLiteral = digit+
		static Lang IntLiteral(LangRef lang)
		{
			return (+Match(isdigit))(lang);
		}

		// FloatLiteral = IntLiteral '.' IntLiteral£¿ (('E'|'e') ('+'|'-')? IntLiteral)?
		static Lang FloatLiteral(LangRef lang)
		{
			return (
				+Match(isdigit),
				Match('.'),
				~Process(+Match(isdigit)),
				~(
					Match({'E','e'}),
					~Match({ '+','-' }),
					+Match(isdigit)
					)
				)(lang);
		}

		// UnescapedCharacter = not ('"' | '\\')
		static Lang UnescapedCharacter(LangRef lang)
		{
			return Match([=](Char c)
			{
				if (c == '"' || c == '\\') return false;
				return true;
			})(lang);
		}

		// EscapedCharacter = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
		static Lang EscapedCharacter(LangRef lang)
		{
			return (
				Match('\\'),
				Match({ '"','\\','b','f','n','r','t','v' })
				)(lang);
		}

		// StringLiteral = '"' (EscapedCharacter|UnescapedCharacter)* '"'
		static Lang StringLiteral(LangRef lang)
		{
			return (
				Match('"'),
				*(
					Process(EscapedCharacter) | UnescapedCharacter
					),
				Match('"')
				)(lang);
		}

		// Identifier = (Letter | '_') (DigitOrLetter | '_')*
		static Lang Identifier(LangRef lang)
		{
			return (
				(
					Match(isalpha) | Match('_')
					),
				*(
					Match(isalnum) | Match('_')
					)
				)(lang);
		}

		// NullLiteral = "null"
		static Lang NullLiteral(LangRef lang)
		{
			return Match("null")(lang);
		}

		// UndefinedLiteral = "undef"
		static Lang UndefinedLiteral(LangRef lang)
		{
			return Match("undef")(lang);
		}

		// TrueLiteral = "true"
		static Lang TrueLiteral(LangRef lang)
		{
			return Match("true")(lang);
		}

		// FalseLiteral = "false"
		static Lang FalseLiteral(LangRef lang)
		{
			return Match("false")(lang);
		}

		// Literal = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
		static Lang Literal(LangRef lang)
		{
			return (
				Process(FloatLiteral) | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
				)(lang);
		}

		// Term(A) = Space* A
		static Process Term(ProcessRef lhs)
		{
			return *Match(isspace) , lhs;
		}

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Process(operatorMatcher), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		static Lang ExpressionList(LangRef lang)
		{
			return (
				Term(Match('(')),
				Expression,
				Term(Match(')'))
				)(lang);
		}

		// ArrayList = '[' (SigleExpression (',' SigleExpression)*)? ']'
		static Lang ArrayList(LangRef lang)
		{
			return (
				Term(Match('[')),
				~(
					Process(SigleExpression),
					*(
						Term(Match(',')),
						SigleExpression
						)
					),
				Term(Match(']'))
				)(lang);
		}

		// DictList = '{' (SigleExpression (',' SigleExpression)*)? '}'
		static Lang DictList(LangRef lang)
		{
			return (
				Term(Match('{')),
				~(
					Process(SigleExpression),
					*(
						Term(Match(',')),
						SigleExpression
						)
					),
				Term(Match('}'))
				)(lang);
		}

		// NullFunctionArg = '(' ')'
		static Lang NullFunctionArg(LangRef lang)
		{
			return (
				Term(Match('(')),
				Term(Match(')'))
				)(lang);
		}

		// FunctionArgList = ExpressionList | ArrayList | DictList
		static Lang FunctionArgList(LangRef lang)
		{
			return (
				Process(ExpressionList) |
				ArrayList |
				DictList
				)(lang);
		}

		// VarDefineExpression = "var" Identifier
		static Lang VarDefineExpression(LangRef lang)
		{
			return (
				Term(Match("var")),
				Term(Identifier)
				)(lang);
		}

		// PrimaryExpression = Identifier | Literal | FunctionArgList
		static Lang PrimaryExpression(LangRef lang)
		{
			return (
				Term(Literal) |
				VarDefineExpression |
				Term(Identifier) |
				FunctionArgList
				)(lang);
		}

		// FunctionExpression = PrimaryExpression (FunctionArgList | NullFunctionArg)*
		static Lang FunctionExpression(LangRef lang)
		{
			return (
				PrimaryExpression,
				*(
					Process(NullFunctionArg) |
					FunctionArgList
					)
				)(lang);
		}

		// PointExpression = FunctionExpression | PointExpression '->' PrimaryExpression
		static Lang PointExpression(LangRef lang)
		{
			return (
				BinaryOperator(FunctionExpression,Term(Match("->")))
				)(lang);
		}

		// UnaryExpression = PointExpression | (('+' | '-') | "not") UnaryExpression
		static Lang UnaryExpression(LangRef lang)
		{
			return (
				(
					(
						Term(Match({ '+','-' })) |
						Term(Match("not"))
						),
					UnaryExpression
					) |
				PointExpression
				)(lang);
		}

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		static Lang MultiplicativeExpression(LangRef lang)
		{
			return (
				BinaryOperator(UnaryExpression, Term(Match({ '*','/','%' })))
				)(lang);
		}
		
		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		static Lang AdditiveExpression(LangRef lang)
		{
			return (
				BinaryOperator(MultiplicativeExpression, Term(Match({ '+','-' })))
				)(lang);
		}

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		static Lang RelationalExpression(LangRef lang)
		{
			return (
				BinaryOperator(AdditiveExpression, Term(Match({ "<=", ">=", "<", ">" })))
				)(lang);
		}
		
		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		static Lang EqualityExpression(LangRef lang)
		{
			return (
				BinaryOperator(RelationalExpression, Term(Match({ "==","<>" })))
				)(lang);
		}

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		static Lang LogicalAndExpression(LangRef lang)
		{
			return (
				BinaryOperator(EqualityExpression, Term(Match("and")))
				)(lang);
		}

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		static Lang LogicalOrExpression(LangRef lang)
		{
			return (
				BinaryOperator(LogicalAndExpression, Term(Match("or")))
				)(lang);
		}

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		static Lang AssignmentOperator(LangRef lang)
		{
			return Term(Match({ "=", "*=", "/=", "%=", "+=", "-=" }))(lang);
		}

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression
		static Lang AssignmentExpression(LangRef lang)
		{
			return (
				LogicalOrExpression, *(Process(AssignmentOperator), SigleExpression)
				)(lang);
		}

		// PairExperssion = AssignmentExpression | PairExperssion ':' SigleExpression
		static Lang PairForDictList(LangRef lang)
		{
			return (
				AssignmentExpression, *(Term(Match(':')), SigleExpression)
				)(lang);
		}

		// ConditionExpression = AssignmentExpression | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		static Lang ConditionExpression(LangRef lang)
		{
			return (
				(	
					Term(Match("if")),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression,
					~(
						Term(Match("else")),
						SigleExpression
						)
					) |
				PairForDictList
				)(lang);
		}

		// LoopControlExpression = ConditionExpression | ("break"|"continue") SigleExpression?
		static Lang LoopControlExpression(LangRef lang)
		{
			return (
				(
					Term(Match({ "break","continue" })),
					~Process(SigleExpression)
					) |
				ConditionExpression
				)(lang);
		}

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SigleExpression
		static Lang WhileLoopExpression(LangRef lang)
		{
			return (
				(
					Term(Match("while")),
					Term(Match('(')),
					Expression,
					Term(Match(')')),
					SigleExpression
					) |
				LoopControlExpression
				)(lang);
		}

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		static Lang DoWhileLoopExpression(LangRef lang)
		{
			return (
				(
					Term(Match("do")),
					SigleExpression,
					Term(Match("while")),
					Term(Match('(')),
					Expression,
					Term(Match(')'))
					) |
				WhileLoopExpression
				)(lang);
		}

		// SigleExpression = DoWhileLoopExpression
		static Lang SigleExpression(LangRef lang)
		{
			return (
				DoWhileLoopExpression
				)(lang);
		}

		// Expression = SigleExpression ((';'|',') SigleExpression)* ('.'|';')?
		static Lang Expression(LangRef lang)
		{
			return (
				SigleExpression,
				*(
					Term(Match({ ',',';' })),
					SigleExpression
					),
				~Term(Match({ '.' , ';' }))
				)(lang);
		}
	};

	using Parser = BasicParser<std::string_view>;

}