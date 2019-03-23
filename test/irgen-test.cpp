#define GTEST_HAS_STD_TUPLE_ 1
#define GTEST_HAS_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <chtholly/irgenerator.hpp>

using namespace Chtholly;

using namespace std::string_literals;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

std::string OpcodeToString(std::size_t code)
{
#define OpcodeStrPair(func) { Opcode<&Instruction::func>(), #func }
	return std::map<std::size_t, std::string> {
		OpcodeStrPair(None), 
		OpcodeStrPair(Block::Begin), OpcodeStrPair(Block::NamedBegin),
		OpcodeStrPair(Block::Drop), OpcodeStrPair(Block::End),
		OpcodeStrPair(Function::Begin), OpcodeStrPair(Function::End), OpcodeStrPair(Function::Call),
		OpcodeStrPair(List::Push), OpcodeStrPair(List::Pop),
		OpcodeStrPair(Control::Jump), OpcodeStrPair(Control::JumpIf),
		OpcodeStrPair(Control::JumpIfElse), OpcodeStrPair(Control::Mark),
		OpcodeStrPair(Object::Begin), OpcodeStrPair(Object::End), 
		OpcodeStrPair(Object::EndWithInit), OpcodeStrPair(Object::AttachTo),
		OpcodeStrPair(Object::Var), OpcodeStrPair(Object::VarWithConstraint),
		OpcodeStrPair(Object::VarPack), OpcodeStrPair(Object::VarPackWithConstraint),
		OpcodeStrPair(Object::Const), OpcodeStrPair(Object::ConstWithConstraint),
		OpcodeStrPair(Object::ConstPack), OpcodeStrPair(Object::ConstPackWithConstraint),
		OpcodeStrPair(Object::Use),
		OpcodeStrPair(Literal::Int), OpcodeStrPair(Literal::Float),
		OpcodeStrPair(Literal::String), OpcodeStrPair(Literal::Bool),
		OpcodeStrPair(Literal::Null), OpcodeStrPair(Literal::Undef),
	}.at(code);
#undef OpcodeStrPair
}

std::string ToString(const Instruction& i)
{
	std::string out;
	out += OpcodeToString(i.opcode());
	if (!i.oprands().empty())
	{
		out += "(";
		bool first = true;
		for (const auto& v : i.oprands())
		{
			if (!first) out += ",";
			else first = false;
			out += std::visit(overloaded{
				[](IRValue::Undef arg) { return "undef"s; },
				[](IRValue::Null arg) { return "null"s; },
				[](IRValue::Bool arg) { return arg ? "true"s : "false"s; },
				[](IRValue::Int arg) { return std::to_string(arg); },
				[](IRValue::Float arg) { return std::to_string(arg); },
				[](IRValue::String arg) { return Conv<Unquoted<IRValue::String>>::To<std::string>(arg); },
				}, static_cast<IRValue::Base>(v));
		}
		out += ")";
	}

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

TEST(Token, IdentifierLiteral)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("NullLiteral", "null"))), IRGenerator::Sequence{
		Instruction::Literal::Null()
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("UndefinedLiteral", "undef"))), IRGenerator::Sequence{
		Instruction::Literal::Undef()
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("TrueLiteral", "true"))), IRGenerator::Sequence{
		Instruction::Literal::Bool(true)
	});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("FalseLiteral", "false"))), IRGenerator::Sequence{
		Instruction::Literal::Bool(false)
	});
}

TEST(Token, Identifier)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("Identifier", "a"))), IRGenerator::Sequence{
		Instruction::Object::Use("a")
		});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("Identifier", "integer_container"))), IRGenerator::Sequence{
		Instruction::Object::Use("integer_container")
		});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("Identifier", "_Static_assert"))), IRGenerator::Sequence{
		Instruction::Object::Use("_Static_assert")
		});
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Token("Identifier", "iHave100AppleForYou"))), IRGenerator::Sequence{
		Instruction::Object::Use("iHave100AppleForYou")
		});
}

TEST(Expression, ExpressionOfLiteral)
{
	const auto& seq = IRGenerator::Sequence{
		Instruction::Block::Begin(),
		Instruction::Literal::Float(1.0),
		Instruction::Block::End(),
		Instruction::Block::Begin(),
		Instruction::Literal::Float(2.0),
		Instruction::Block::Drop(),
		Instruction::Block::Begin(),
		Instruction::Literal::Null(),
		Instruction::Block::End(),
		Instruction::Block::Begin(),
		Instruction::Literal::Int(4),
		Instruction::Block::Drop(),
		Instruction::Block::Begin(),
		Instruction::Literal::Float(5.0),
		Instruction::Block::End()
	};

	EXPECT_EQ(IRGenerator::Generate(ParseTree(Term("Expression",
		Token("FloatLiteral", "1.0"),
		Token("Separator", ";"),
		Token("FloatLiteral", "2.0"),
		Token("Separator", ","),
		Token("NullLiteral", "null"),
		Token("Separator", ";"),
		Token("IntLiteral", "4"),
		Token("Separator", ","),
		Token("FloatLiteral", "5.0")
	))), seq);

	const auto& seq_2 = IRGenerator::Sequence{
		Instruction::Block::Begin(),
		Instruction::Literal::Float(1.0),
		Instruction::Block::End(),
		Instruction::Block::Begin(),
		Instruction::Literal::Float(2.0),
		Instruction::Block::Drop(),
		Instruction::Block::Begin(),
		Instruction::Literal::Null(),
		Instruction::Block::End(),
		Instruction::Block::Begin(),
		Instruction::Block::Begin(),
		Instruction::Literal::Bool(false),
		Instruction::Block::End(),
		Instruction::Block::Begin(),
		Instruction::Literal::Int(666),
		Instruction::Block::End(),
		Instruction::Block::Drop(),
		Instruction::Block::Begin(),
		Instruction::Literal::Float(5.0),
		Instruction::Block::End()
	};
	
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Term("Expression",
		Token("FloatLiteral", "1.0"),
		Token("Separator", ";"),
		Token("FloatLiteral", "2.0"),
		Token("Separator", ","),
		Token("NullLiteral", "null"),
		Token("Separator", ";"),
		Term("Expression",
			Token("FalseLiteral", "false"),
			Token("Separator", ";"),
			Token("IntLiteral", "666"),
			Token("Separator", ";")
		),
		Token("Separator", ","),
		Token("FloatLiteral", "5.0")
	))), seq_2);
}

TEST(Expression, ArrayListOfLiteral)
{
	const auto& seq = IRGenerator::Sequence{
		Instruction::Block::Begin(),
		Instruction::Object::Use("array.literal"),
		Instruction::Literal::Float(1.0),
		Instruction::Literal::Float(2.0),
		Instruction::Literal::Null(),
		Instruction::Literal::Int(4),
		Instruction::Literal::Float(5.0),
		Instruction::Function::Call()
	};

	EXPECT_EQ(IRGenerator::Generate(ParseTree(Term("ArrayList",
		Token("FloatLiteral", "1.0"),
		Token("FloatLiteral", "2.0"),
		Token("NullLiteral", "null"),
		Token("IntLiteral", "4"),
		Token("FloatLiteral", "5.0")
	))), seq);

	const auto& seq_2 = IRGenerator::Sequence{
	Instruction::Block::Begin(),
	Instruction::Object::Use("array.literal"),
	Instruction::Literal::Float(1.0),
	Instruction::Literal::Float(2.0),
	Instruction::Literal::Null(),
	Instruction::Block::Begin(),
	Instruction::Literal::Float(1.0),
	Instruction::Block::End(),
	Instruction::Block::Begin(),
	Instruction::Literal::Float(2.0),
	Instruction::Block::Drop(),
	Instruction::Literal::Float(5.0),
	Instruction::Function::Call()
	};

	EXPECT_EQ(IRGenerator::Generate(ParseTree(Term("ArrayList",
		Token("FloatLiteral", "1.0"),
		Token("FloatLiteral", "2.0"),
		Token("NullLiteral", "null"),
		Term("Expression",
			Token("FloatLiteral", "1.0"),
			Token("Separator", ";"),
			Token("FloatLiteral", "2.0"),
			Token("Separator", ",")
		),
		Token("FloatLiteral", "5.0")
	))), seq_2);
}

TEST(Expression, UndefExpression)
{
	EXPECT_EQ(IRGenerator::Generate(ParseTree(Term("UndefExpression"))), IRGenerator::Sequence{
		Instruction::Literal::Undef() 
	});
}

TEST(Expression, DefineExpression)
{
	const auto& seq = IRGenerator::Sequence{
			Instruction::Object::Begin(),
			Instruction::Object::Var("a"),
			Instruction::Object::End()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("VarDefineExpression",
			Term("ConstraintExpression",
				Token("Identifier", "a")
			)
		))), seq
	);

	const auto& seq_2 = IRGenerator::Sequence{
			Instruction::Object::Begin(),
			Instruction::Object::Use("int"),
			Instruction::Object::VarWithConstraint("number"),
			Instruction::Object::End()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("VarDefineExpression",
			Term("ConstraintExpression",
				Token("Identifier", "number"),
				Token("Identifier", "int")
			)
		))), seq_2
	);

	const auto& seq_3 = IRGenerator::Sequence{
			Instruction::Object::Begin(),
			Instruction::Block::Begin(),
			Instruction::Object::Var("a"),
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::Use("float"),
			Instruction::Object::VarWithConstraint("b"),
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::VarPack("c"),
			Instruction::Block::End(),
			Instruction::Object::End()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("VarDefineExpression",
			Term("PatternExpression",
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "a")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "b"),
					Token("Identifier", "float")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "c"),
					Token("Separator", "...")
				)
			)
		))), seq_3
	);

	const auto& seq_4 = IRGenerator::Sequence{
			Instruction::Object::Begin(),
			Instruction::Block::Begin(),
			Instruction::Object::Const("a"),
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::Use("float"),
			Instruction::Object::ConstWithConstraint("b"),
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::ConstPack("c"),
			Instruction::Block::End(),
			Instruction::Object::End()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("ConstDefineExpression",
			Term("PatternExpression",
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "a")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "b"),
					Token("Identifier", "float")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "c"),
					Token("Separator", "...")
				)
			)
		))), seq_4
	);

	const auto& seq_5 = IRGenerator::Sequence{
			Instruction::Object::Begin(),
			Instruction::Object::Var("a"),
			Instruction::Literal::Int(1),
			Instruction::Object::EndWithInit()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("VarDefineExpression",
			Term("ConstraintExpression",
				Token("Identifier", "a")
			),
			Token("IntLiteral", "1")
		))), seq_5
	);

	const auto& seq_6 = IRGenerator::Sequence{
			Instruction::Object::Begin(), 
			Instruction::Block::Begin(), 
			Instruction::Object::Var("a"), 
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::Use("float"), 
			Instruction::Object::VarWithConstraint("b"), 
			Instruction::Block::Drop(),
			Instruction::Block::Begin(),
			Instruction::Object::VarPack("c"), 
			Instruction::Block::End(),
			Instruction::Block::Begin(),
			Instruction::Literal::Int(1), 
			Instruction::Block::End(),
			Instruction::Block::Begin(),
			Instruction::Literal::Float(2.33), 
			Instruction::Block::End(),
			Instruction::Block::Begin(),
			Instruction::Literal::Float(3.14), 
			Instruction::Block::End(),
			Instruction::Block::Begin(),
			Instruction::Literal::Bool(true), 
			Instruction::Block::End(),
			Instruction::Block::Begin(),
			Instruction::Literal::Null(),
			Instruction::Block::End(),
			Instruction::Object::EndWithInit()
	};
	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("VarDefineExpression",
			Term("PatternExpression",
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "a")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "b"),
					Token("Identifier", "float")
				),
				Token("Separator", ","),
				Term("ConstraintExpressionAtPatternExpression",
					Token("Identifier", "c"),
					Token("Separator", "...")
				)
			),
			Term("Expression",
				Token("IntLiteral", "1"),
				Token("Separator", ";"),
				Token("FloatLiteral", "2.33"),
				Token("Separator", ";"),
				Token("FloatLiteral", "3.14"),
				Token("Separator", ";"),
				Token("TrueLiteral", "true"),
				Token("Separator", ";"),
				Token("NullLiteral", "null")
			)
		))), seq_6
	);
}

TEST(Expression, LambdaExpression)
{
	const auto& seq_1 = IRGenerator::Sequence{
			Instruction::Function::Begin(),
			Instruction::Block::Begin(),
			Instruction::Object::Var("x"),
			Instruction::Block::End(),
			Instruction::Object::Use("x"),
			Instruction::Function::End()
	};

	EXPECT_EQ(IRGenerator::Generate(ParseTree(
		Term("LambdaExpression",
			Term("PatternExpression",
				Term("ConstraintExpressionAtPatternExpression", Token("Identifier", "x"))
			),
			Token("Identifier", "x")
		)
	)), seq_1);
}
