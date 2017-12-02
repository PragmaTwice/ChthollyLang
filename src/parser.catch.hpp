#pragma once

#include <cctype>
#include <iostream>

#include "parserc.catch.hpp"

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
		static Info Space(Info info)
		{
		return Match(isspace)(info);
		}

		// Digit = '0' ... '9'
		static Info Digit(Info info)
		{
		return Match(isdigit)(info);
		}

		// UpperCaseLetter = 'A' ... 'Z'
		static Info UpperCaseLetter(Info info)
		{
		return Match(isupper)(info);
		}

		// LowerCaseLetter = 'a' ... 'z'
		static Info LowerCaseLetter(Info info)
		{
		return Match(islower)(info);
		}

		// Letter = UpperCaseLetter | LowerCaseLetter
		static Info Letter(Info info)
		{
		return Match(isalpha)(info);
		}

		// Letter = UpperCaseLetter | LowerCaseLetter
		static Info DigitOrLetter(Info info)
		{
		return Match(isalnum)(info);
		}
		*/

		using ModifierProcess = std::function<Modifier(Modifier,LangRef)>;
		using ModifierProcessRef = const ModifierProcess &;

		static Process Catch(ProcessRef pro, ModifierProcessRef mod)
		{
			return [=](Info info)
			{
				if (Info i = pro(info); pro.IsNotEqual(i, info))
				{
					return Info(i.first,mod(i.second,Lang(info.first.data(),info.first.size() - i.first.size())));
				}

				return info;
			};
		}

		static Process Catch(ProcessRef pro, const ParseUnit::String& tokenName)
		{
			return Catch(pro, [=](Modifier modi, LangRef lang)
			{
				modi.childrenPushBack(ParseUnit::Type::token, tokenName, lang);
				return modi;
			});
		}

		using ModifierChange = std::function<Modifier(Modifier)>;
		using ModifierChangeRef = const ModifierChange &;

		static Process Change(ModifierChangeRef mod)
		{
			return [=](Info info)
			{
				return Info(info.first, mod(info.second));
			};
		}

		static Process ChangeIn(const ParseUnit::String& termName)
		{
			return ~Change([=](Modifier modi)
			{
				modi.childrenPushBack(ParseUnit::Type::term, termName);
				return --modi.childrenEnd();
			});
		}
		static Process ChangeOut(bool cutUnusedUnit = false)
		{
			return ~Change([=](Modifier modi)
			{
				if (cutUnusedUnit)
				{
					if (modi.childrenSize() == 1)
					{
						auto cutted = modi.childrenBegin().thisMoveTo(modi);
						modi.thisErase(modi);
						return cutted.parent();
					}
				}
				return modi.parent();
			});
		}

		// IntLiteral = digit+
		static Info IntLiteral(Info info)
		{
			return (Catch(+Match(isdigit), "IntLiteral"))(info);
		}

		// FloatLiteral = IntLiteral '.' IntLiteral£¿ (('E'|'e') ('+'|'-')? IntLiteral)?
		static Info FloatLiteral(Info info)
		{
			return (
				Catch(
				(
					+Match(isdigit),
					Match('.'),
					~Process(+Match(isdigit)),
					~(
						Match({ 'E','e' }),
						~Match({ '+','-' }),
						+Match(isdigit)
						)
					), "FloatLiteral")
				)(info);
		}

		// UnescapedCharacter = not ('"' | '\\')
		static Info UnescapedCharacter(Info info)
		{
			return Match([=](Char c)
			{
				if (c == '"' || c == '\\') return false;
				return true;
			})(info);
		}

		// EscapedCharacter = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
		static Info EscapedCharacter(Info info)
		{
			return (
				Match('\\'),
				Match({ '"','\\','b','f','n','r','t','v' })
				)(info);
		}

		// StringLiteral = '"' (EscapedCharacter|UnescapedCharacter)* '"'
		static Info StringLiteral(Info info)
		{
			return (
				Catch((
					Match('"'),
					*(
						Process(EscapedCharacter) | UnescapedCharacter
						),
					Match('"')
					), "StringLiteral")
				)(info);
		}

		// Identifier = (Letter | '_') (DigitOrLetter | '_')*
		static Info Identifier(Info info)
		{
			return (
				Catch((
					(
						Match(isalpha) | Match('_')
						),
					*(
						Match(isalnum) | Match('_')
						)
					), "Identifier")
				)(info);
		}

		// NullLiteral = "null"
		static Info NullLiteral(Info info)
		{
			return Catch(Match("null"), "NullLiteral")(info);
		}

		// UndefinedLiteral = "undef"
		static Info UndefinedLiteral(Info info)
		{
			return Catch(Match("undef"), "UndefinedLiteral")(info);
		}

		// TrueLiteral = "true"
		static Info TrueLiteral(Info info)
		{
			return Catch(Match("true"), "TrueLiteral")(info);
		}

		// FalseLiteral = "false"
		static Info FalseLiteral(Info info)
		{
			return Catch(Match("false"), "FalseLiteral")(info);
		}

		// Literal = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
		static Info Literal(Info info)
		{
			return (
				Process(FloatLiteral) | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral
				)(info);
		}

		// Term(A) = Space* A
		static Process Term(ProcessRef lhs)
		{
			return *Match(isspace), lhs;
		}

		// BinaryOperator(A,B) = A (B A)*
		static Process BinaryOperator(ProcessRef operatorUponIt, ProcessRef operatorMatcher)
		{
			return operatorUponIt, *(Process(operatorMatcher), operatorUponIt);
		}

		// ExpressionList = '(' Expression ')'
		static Info ExpressionList(Info info)
		{
			return (
				Term(Match('(')),
				Expression,
				Term(Match(')'))
				)(info);
		}

		// ArrayList = '[' (SigleExpression (',' SigleExpression)*)? ']'
		static Info ArrayList(Info info)
		{
			return (
				Term(Match('[')),
				ChangeIn("ArrayList"),
				~(
					Process(SigleExpression),
					*(
						Term(Match(',')),
						SigleExpression
						)
					),
				ChangeOut(),
				Term(Match(']'))
				)(info);
		}

		// DictList = '{' (SigleExpression (',' SigleExpression)*)? '}'
		static Info DictList(Info info)
		{
			return (
				Term(Match('{')),
				ChangeIn("DictList"),
				~(
					Process(SigleExpression),
					*(
						Term(Match(',')),
						SigleExpression
						)
					),
				ChangeOut(),
				Term(Match('}'))
				)(info);
		}

		// NullFunctionArg = '(' ')'
		static Info NullFunctionArg(Info info)
		{
			return (
				Term(Match('(')),
				Term(Match(')')),
				ChangeIn("NullFunctionArg"),
				ChangeOut()
				)(info);
		}

		// FunctionArgList = ExpressionList | ArrayList | DictList
		static Info FunctionArgList(Info info)
		{
			return (
				Process(ExpressionList) |
				ArrayList |
				DictList
				)(info);
		}

		// VarDefineExpression = "var" Identifier
		static Info VarDefineExpression(Info info)
		{
			return (
				Term(Match("var")),
				ChangeIn("VarDefineExpression"),
				Term(Identifier),
				ChangeOut()
				)(info);
		}

		// ConstDefineExpression = "const" Identifier
		static Info ConstDefineExpression(Info info)
		{
			return (
				Term(Match("const")),
				ChangeIn("ConstDefineExpression"),
				Term(Identifier),
				ChangeOut()
				)(info);
		}

		// PrimaryExpression = Identifier | Literal | FunctionArgList | VarDefineExpression | ConstDefineExpression
		static Info PrimaryExpression(Info info)
		{
			return (
				Term(Literal) |
				VarDefineExpression |
				ConstDefineExpression |
				Term(Identifier) |
				FunctionArgList
				)(info);
		}

		// FunctionExpression = PrimaryExpression (FunctionArgList | NullFunctionArg)*
		static Info FunctionExpression(Info info)
		{
			return (
				ChangeIn("FunctionExpression"),
				PrimaryExpression,
				*(
					Process(NullFunctionArg) |
					FunctionArgList
					),
				ChangeOut(true)
				)(info);
		}

		// PointExpression = FunctionExpression | PointExpression '->' PrimaryExpression
		static Info PointExpression(Info info)
		{
			return (
				ChangeIn("PointExpression"),
				BinaryOperator(FunctionExpression, Term(Match("->"))),
				ChangeOut(true)
				)(info);
		}

		// UnaryExpression = PointExpression | (('+' | '-') | "not") UnaryExpression
		static Info UnaryExpression(Info info)
		{
			return (
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
				)(info);
		}

		// MultiplicativeExpression = UnaryExpression | MultiplicativeExpression ('*'|'/'|'%') UnaryExpression
		static Info MultiplicativeExpression(Info info)
		{
			return (
				ChangeIn("MultiplicativeExpression"),
				BinaryOperator(UnaryExpression, Term(Match({ '*','/','%' }))),
				ChangeOut(true)
				)(info);
		}

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		static Info AdditiveExpression(Info info)
		{
			return (
				ChangeIn("AdditiveExpression"),
				BinaryOperator(MultiplicativeExpression, Term(Match({ '+','-' }))),
				ChangeOut(true)
				)(info);
		}

		// RelationalExpression = AdditiveExpression | RelationalExpression ("<="|">="|'<'|'>') AdditiveExpression
		static Info RelationalExpression(Info info)
		{
			return (
				ChangeIn("RelationalExpression"),
				BinaryOperator(AdditiveExpression, Term(Match({ "<=", ">=", "<", ">" }))),
				ChangeOut(true)
				)(info);
		}

		// EqualityExpression = RelationalExpression | EqualityExpression ("=="|"<>") RelationalExpression
		static Info EqualityExpression(Info info)
		{
			return (
				ChangeIn("EqualityExpression"),
				BinaryOperator(RelationalExpression, Term(Match({ "==","<>" }))),
				ChangeOut(true)
				)(info);
		}

		// LogicalAndExpression = EqualityExpression | LogicalAndExpression "and" EqualityExpression
		static Info LogicalAndExpression(Info info)
		{
			return (
				ChangeIn("LogicalAndExpression"),
				BinaryOperator(EqualityExpression, Term(Match("and"))),
				ChangeOut(true)
				)(info);
		}

		// LogicalOrExpression = LogicalAndExpression | LogicalOrExpression "or" LogicalAndExpression
		static Info LogicalOrExpression(Info info)
		{
			return (
				ChangeIn("LogicalOrExpression"),
				BinaryOperator(LogicalAndExpression, Term(Match("or"))),
				ChangeOut(true)
				)(info);
		}

		// AssignmentOperator = '=' | '*=' | '/=' | '%=' | '+=' | '-='
		static Info AssignmentOperator(Info info)
		{
			return Term(Match({ "=", "*=", "/=", "%=", "+=", "-=" }))(info);
		}

		// AssignmentExpression = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression
		static Info AssignmentExpression(Info info)
		{
			return (
				ChangeIn("AssignmentExpression"),
				LogicalOrExpression, *(Process(AssignmentOperator), SigleExpression),
				ChangeOut(true)
				)(info);
		}

		// PairExperssion = AssignmentExpression | PairExperssion ':' SigleExpression
		static Info PairForDictList(Info info)
		{
			return (
				ChangeIn("PairForDictList"),
				AssignmentExpression, *(Term(Match(':')), SigleExpression),
				ChangeOut(true)
				)(info);
		}

		// ConditionExpression = AssignmentExpression | "if" '(' Expression ')' SigleExpression ("else" SigleExpression)?
		static Info ConditionExpression(Info info)
		{
			return (
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
				)(info);
		}

		// LoopControlExpression = ConditionExpression | ("break"|"continue") SigleExpression?
		static Info LoopControlExpression(Info info)
		{
			return (
				(
					Term(Match({ "break","continue" })),
					ChangeIn("LoopControlExpression"),
					~Process(SigleExpression),
					ChangeOut()
					) |
				ConditionExpression
				)(info);
		}

		// WhileLoopExpression = LoopControlExpression | "while" '(' Expression ')' SigleExpression
		static Info WhileLoopExpression(Info info)
		{
			return (
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
				)(info);
		}

		// DoWhileLoopExpression = WhileLoopExpression | "do" SigleExpression "while" '(' Expression ')'
		static Info DoWhileLoopExpression(Info info)
		{
			return (
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
				)(info);
		}

		// SigleExpression = DoWhileLoopExpression
		static Info SigleExpression(Info info)
		{
			return (
				DoWhileLoopExpression
				)(info);
		}

		// Expression = SigleExpression ((';'|',') SigleExpression)* ('.'|';')?
		static Info Expression(Info info)
		{
			return (
				ChangeIn("Expression"),
				SigleExpression,
				*(
					Term(Match({ ',',';' })),
					SigleExpression
					),
				~Term(Match({ '.' , ';' })),
				ChangeOut(true)
				)(info);
		}
	};

	using Parser = BasicParser<std::string_view>;

}