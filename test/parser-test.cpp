#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly.h>

using namespace Chtholly;

ParseTree parseString(const std::string_view& input_string)
{
	ParseTree tree("root");
	Parser::Expression(Parser::MakeInfo(input_string, tree.modifier()));
	return tree;
}

TEST(Literal, Integer) {
	EXPECT_EQ(parseString("1"),ParseTree(ParseTree::Token("IntLiteral","1")));
	EXPECT_EQ(parseString("1234567890"), ParseTree(ParseTree::Token("IntLiteral", "1234567890")));
	EXPECT_EQ(parseString("78294385928147234687253"), ParseTree(ParseTree::Token("IntLiteral", "78294385928147234687253")));
}

TEST(Literal, Float)
{
	EXPECT_EQ(parseString("1."), ParseTree(ParseTree::Token("FloatLiteral", "1.")));
	EXPECT_EQ(parseString("2.345"), ParseTree(ParseTree::Token("FloatLiteral", "2.345")));
	EXPECT_EQ(parseString("6.78e-90"), ParseTree(ParseTree::Token("FloatLiteral", "6.78e-90")));
	EXPECT_EQ(parseString("234235.345323333333e+3435756756782"), ParseTree(ParseTree::Token("FloatLiteral", "234235.345323333333e+3435756756782")));
}

TEST(Literal, String)
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
