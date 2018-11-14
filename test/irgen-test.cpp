#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly/irgenerator.hpp>

using namespace Chtholly;

TEST(TEMP, _1)
{
	IRGenerator::Sequence seq;
	IRGenerator::Goto(ParseTree(Token("IntLiteral", "1")).observer().childrenBegin(), seq);
	EXPECT_EQ(seq.size(), 1);
}
