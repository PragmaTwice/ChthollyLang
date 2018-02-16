#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly.h>

using namespace Chtholly;

std::string ToString(const ParseTree::Observer& v)
{
	std::string out;

	const auto isTerm = v.value().type == ParseUnit::Type::term;

	if (isTerm) out += '(';

	if (isTerm)
	{
		out += v.value().name + ' ';
	}
	else
	{
		out += v.value().name + '[' + std::string(v.value().value) + "] ";
	}

	for (auto i = v.childrenBegin(); i != v.childrenEnd(); ++i)
	{
		out += ToString(i);
	}

	if (isTerm) out += ')';

	return out;
}

std::ostream& operator<<(std::ostream& out, const ParseTree& tree)
{
	out << ToString(tree.observer());
	return out;
}

ParseTree parseString(const std::string_view& input_string)
{
	ParseTree tree;
	Parser::Expression(Parser::MakeInfo(input_string, tree.modifier()));
	return tree;
}

TEST(Token, IntegerLiteral) {
	EXPECT_EQ(parseString("1"),ParseTree(ParseTree::Token("IntLiteral","1")));
	EXPECT_EQ(parseString("1234567890"), ParseTree(ParseTree::Token("IntLiteral", "1234567890")));
	EXPECT_EQ(parseString("78294385928147234687253"), ParseTree(ParseTree::Token("IntLiteral", "78294385928147234687253")));
}

TEST(Token, FloatLiteral)
{
	EXPECT_EQ(parseString("1."), ParseTree(ParseTree::Token("FloatLiteral", "1.")));
	EXPECT_EQ(parseString("01.e0222"), ParseTree(ParseTree::Token("FloatLiteral", "01.e0222")));
	EXPECT_EQ(parseString("2.345"), ParseTree(ParseTree::Token("FloatLiteral", "2.345")));
	EXPECT_EQ(parseString("6.78e-90"), ParseTree(ParseTree::Token("FloatLiteral", "6.78e-90")));
	EXPECT_EQ(parseString("234235.345323333333e+3435756756782"), ParseTree(ParseTree::Token("FloatLiteral", "234235.345323333333e+3435756756782")));
}

TEST(Token, StringLiteral)
{
	EXPECT_EQ(parseString(R"("hello")"), ParseTree(ParseTree::Token("StringLiteral", R"("hello")")));

	//EXPECT_EQ(parseString(R"("\thello\n\"")"), ParseTree(ParseTree::Token("StringLiteral", R"("\thello\n\"")")));
	EXPECT_EQ(parseString("\"\\thello\\n\\\"\""), ParseTree(ParseTree::Token("StringLiteral", "\"\\thello\\n\\\"\"")));

	auto tested_string_literal = R"("
		Dear user:
			Hello!
			Enjoy it.
	")";

	EXPECT_EQ(parseString(tested_string_literal), ParseTree(ParseTree::Token("StringLiteral", tested_string_literal)));
}

TEST(Token, IdentifierLiteral)
{
	EXPECT_EQ(parseString("null"), ParseTree(ParseTree::Token("NullLiteral", "null")));
	EXPECT_EQ(parseString("undef"), ParseTree(ParseTree::Token("UndefinedLiteral", "undef")));
	EXPECT_EQ(parseString("true"), ParseTree(ParseTree::Token("TrueLiteral", "true")));
	EXPECT_EQ(parseString("false"), ParseTree(ParseTree::Token("FalseLiteral", "false")));
}

TEST(Token, Identifier)
{
	EXPECT_EQ(parseString("a"), ParseTree(ParseTree::Token("Identifier", "a")));
	EXPECT_EQ(parseString("integer_container"), ParseTree(ParseTree::Token("Identifier", "integer_container")));
	EXPECT_EQ(parseString("_Static_assert"), ParseTree(ParseTree::Token("Identifier", "_Static_assert")));
	EXPECT_EQ(parseString("iHave100AppleForYou"), ParseTree(ParseTree::Token("Identifier", "iHave100AppleForYou")));
}

TEST(Expression, DefineExpression)
{
	EXPECT_EQ(parseString("var x"), ParseTree(ParseTree::Term("VarDefineExpression", ParseTree::Token("Identifier", "x"))));
	EXPECT_EQ(parseString("const hello"), ParseTree(ParseTree::Term("ConstDefineExpression", ParseTree::Token("Identifier", "hello"))));
	EXPECT_EQ(parseString("var y: int"), ParseTree(ParseTree::Term("VarDefineExpression", ParseTree::Token("Identifier", "y"), ParseTree::Token("Identifier", "int"))));
}