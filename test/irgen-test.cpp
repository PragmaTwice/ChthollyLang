#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly/irgenerator.hpp>

using namespace Chtholly;

using namespace std::string_literals;

std::string ToString(const Instruction& i)
{
	std::string out;
	out += std::to_string(i.opcode()) + "(";
	bool first = true;
	for(const auto& v : i.oprands())
	{
		if(!first) out += ",";
		else first = false;
		out += std::visit([](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, IRValue::Undef>)
				return "undef"s;
			else if constexpr (std::is_same_v<T, IRValue::Null>)
				return "null"s;
			else if constexpr (std::is_same_v<T, IRValue::Bool>)
				return arg ? "true"s : "false"s;
			else if constexpr (std::is_same_v<T, IRValue::Int>)
				return std::to_string(arg);
			else if constexpr (std::is_same_v<T, IRValueDef::Float>)
				return std::to_string(arg);
			else if constexpr (std::is_same_v<T, IRValueDef::String>)
				return Conv<Unquoted<std::string>>::To<std::string>(arg);
		}, static_cast<IRValue::Base>(v));
	}
	out += ")";

	return out;
}

std::ostream& operator<<(std::ostream& out, const Instruction& i)
{
	out << ToString(i);
	return out;
}

TEST(Token, IntLiteral)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("IntLiteral", "1"))), IRGenerator::Sequence{
		Instruction::Literal::Int(1)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("IntLiteral", "1234567890"))), IRGenerator::Sequence{
		Instruction::Literal::Int(1234567890)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("IntLiteral", "7829438592814723468"))), IRGenerator::Sequence{
		Instruction::Literal::Int(7829438592814723468ll)
	});
}

TEST(Token, FloatLiteral)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FloatLiteral", "1."))), IRGenerator::Sequence{
		Instruction::Literal::Float(1)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FloatLiteral", "01.e0222"))), IRGenerator::Sequence{
		Instruction::Literal::Float(01.e0222)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FloatLiteral", "2.345"))), IRGenerator::Sequence{
		Instruction::Literal::Float(2.345)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FloatLiteral", "6.78e-90"))), IRGenerator::Sequence{
		Instruction::Literal::Float(6.78e-90)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FloatLiteral", "234235.345323333333e+34"))), IRGenerator::Sequence{
		Instruction::Literal::Float(234235.345323333333e+34)
	});
}

TEST(Token, StringLiteral)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("StringLiteral", R"("hello")"))), IRGenerator::Sequence{
		Instruction::Literal::String("hello")
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("StringLiteral", R"("\thello\n\"")"))), IRGenerator::Sequence{
		Instruction::Literal::String("\thello\n\"")
	});
}