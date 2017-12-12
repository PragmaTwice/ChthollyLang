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
		static Process ChangeOut(const bool cutUnusedUnit = false)
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
				BinaryOperator(UnaryExpression, Match({ '*','/','%' })),
				ChangeOut(true)
			);

		// AdditiveExpression = MultiplicativeExpression | AdditiveExpression ('+'|'-') MultiplicativeExpression
		inline static Process AdditiveExpression =
			(
				ChangeIn("AdditiveExpression"),
				BinaryOperator(MultiplicativeExpression, Match({ '+','-' })),
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