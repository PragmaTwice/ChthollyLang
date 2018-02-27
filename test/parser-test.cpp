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
	EXPECT_EQ(parseString("1"),ParseTree(Token("IntLiteral","1")));
	EXPECT_EQ(parseString("1234567890"), ParseTree(Token("IntLiteral", "1234567890")));
	EXPECT_EQ(parseString("78294385928147234687253"), ParseTree(Token("IntLiteral", "78294385928147234687253")));
}

TEST(Token, FloatLiteral)
{
	EXPECT_EQ(parseString("1."), ParseTree(Token("FloatLiteral", "1.")));
	EXPECT_EQ(parseString("01.e0222"), ParseTree(Token("FloatLiteral", "01.e0222")));
	EXPECT_EQ(parseString("2.345"), ParseTree(Token("FloatLiteral", "2.345")));
	EXPECT_EQ(parseString("6.78e-90"), ParseTree(Token("FloatLiteral", "6.78e-90")));
	EXPECT_EQ(parseString("234235.345323333333e+3435756756782"), ParseTree(Token("FloatLiteral", "234235.345323333333e+3435756756782")));
}

TEST(Token, StringLiteral)
{
	EXPECT_EQ(parseString(R"("hello")"), ParseTree(Token("StringLiteral", R"("hello")")));

	//EXPECT_EQ(parseString(R"("\thello\n\"")"), ParseTree(Token("StringLiteral", R"("\thello\n\"")")));
	EXPECT_EQ(parseString("\"\\thello\\n\\\"\""), ParseTree(Token("StringLiteral", "\"\\thello\\n\\\"\"")));

	auto tested_string_literal = R"("
		Dear user:
			Hello!
			Enjoy it.
	")";

	EXPECT_EQ(parseString(tested_string_literal), ParseTree(Token("StringLiteral", tested_string_literal)));
}

TEST(Token, IdentifierLiteral)
{
	EXPECT_EQ(parseString("null"), ParseTree(Token("NullLiteral", "null")));
	EXPECT_EQ(parseString("undef"), ParseTree(Token("UndefinedLiteral", "undef")));
	EXPECT_EQ(parseString("true"), ParseTree(Token("TrueLiteral", "true")));
	EXPECT_EQ(parseString("false"), ParseTree(Token("FalseLiteral", "false")));
}

TEST(Token, Identifier)
{
	EXPECT_EQ(parseString("a"), ParseTree(Token("Identifier", "a")));
	EXPECT_EQ(parseString("integer_container"), ParseTree(Token("Identifier", "integer_container")));
	EXPECT_EQ(parseString("_Static_assert"), ParseTree(Token("Identifier", "_Static_assert")));
	EXPECT_EQ(parseString("iHave100AppleForYou"), ParseTree(Token("Identifier", "iHave100AppleForYou")));
}

TEST(Expression, DefineExpression)
{
	EXPECT_EQ(parseString("var x"), ParseTree(
		Term("VarDefineExpression", 
			Term("ConstraintExperssion",
				Token("Identifier", "x")
			)
		)
	));
	EXPECT_EQ(parseString("const hello(0)"), ParseTree(
		Term("ConstDefineExpression", 
			Term("ConstraintExperssion",
				Token("Identifier", "hello")
			),
			Token("IntLiteral", "0")
		)
	));
	EXPECT_EQ(parseString("var y: int"), ParseTree(
		Term("VarDefineExpression", 
			Term("ConstraintExperssion",
				Token("Identifier", "y"), 
				Token("Identifier", "int")
			)
		)
	));
	EXPECT_EQ(parseString("var (x...,y:int,z):numbers(1.0;2.0;3;4;5.0)"), ParseTree(
		Term("VarDefineExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion",Token("Identifier","x"), Token("Separator", "...")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "y"), Token("Identifier", "int")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "z")),
				Token("Identifier", "numbers")
			),
			Term("Expression",
				Token("FloatLiteral", "1.0"),
				Token("Separator", ";"),
				Token("FloatLiteral", "2.0"),
				Token("Separator", ";"),
				Token("IntLiteral", "3"),
				Token("Separator", ";"),
				Token("IntLiteral", "4"),
				Token("Separator", ";"),
				Token("FloatLiteral", "5.0")
			)
		)
	));
}

TEST(Expression, LambdaExpression)
{
	EXPECT_EQ(parseString("fn(x) x"), ParseTree(
		Term("LambdaExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x"))
			),
			Token("Identifier", "x")
		)
	));

	EXPECT_EQ(parseString("fn(x,y) if(x>y) x else y"), ParseTree(
		Term("LambdaExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "y"))
			),
			Term("ConditionExpression",
				Term("RelationalExpression",
					Token("Identifier", "x"),
					Token("BinaryOperator", ">"),
					Token("Identifier", "y")
				),
				Token("Identifier", "x"),
				Token("Identifier", "y")
			)
		)
	));

	EXPECT_EQ(parseString("fn(x,y,z):number x*x+y*y+z*z"), ParseTree(
		Term("LambdaExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "y")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "z")),
				Token("Identifier", "number")
			),
			Term("AdditiveExpression",
				Term("MultiplicativeExpression",
					Token("Identifier", "x"),
					Token("BinaryOperator", "*"),
					Token("Identifier", "x")
				),
				Token("BinaryOperator", "+"),
				Term("MultiplicativeExpression",
					Token("Identifier", "y"),
					Token("BinaryOperator", "*"),
					Token("Identifier", "y")
				),
				Token("BinaryOperator", "+"),
				Term("MultiplicativeExpression",
					Token("Identifier", "z"),
					Token("BinaryOperator", "*"),
					Token("Identifier", "z")
				)
			)
		)
	));

	EXPECT_EQ(parseString("fn(x,y) (var temp=x, x=y; y=temp)"), ParseTree(
		Term("LambdaExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "y"))
			),
			Term("Expression",
				Term("AssignmentExpression",
					Term("VarDefineExpression",
						Term("ConstraintExperssion",
							Token("Identifier", "temp")
						)
					),
					Token("BinaryOperator","="),
					Token("Identifier", "x")
				),
				Token("Separator", ","),
				Term("AssignmentExpression",
					Token("Identifier", "x"),
					Token("BinaryOperator", "="),
					Token("Identifier", "y")
				),
				Token("Separator", ";"),
				Term("AssignmentExpression",
					Token("Identifier", "y"),
					Token("BinaryOperator", "="),
					Token("Identifier", "temp")
				)
			)
		)
	));
}

TEST(Expression, ConditionExpression)
{
	EXPECT_EQ(parseString("if(cond) true_branch else false_branch"), ParseTree(
		Term("ConditionExpression",
			Token("Identifier", "cond"),
			Token("Identifier", "true_branch"),
			Token("Identifier", "false_branch")
		)
	));

	EXPECT_EQ(parseString("if(1 > 2) 1 else 2"), ParseTree(
		Term("ConditionExpression",
			Term("RelationalExpression",
				Token("IntLiteral", "1"),
				Token("BinaryOperator", ">"),
				Token("IntLiteral", "2")
			),
			Token("IntLiteral","1"),
			Token("IntLiteral","2")
		)
	));

	EXPECT_EQ(parseString("if (isNumber(x)) x+1"), ParseTree(
		Term("ConditionExpression",
			Term("FunctionExpression",
				Token("Identifier", "isNumber"),
				Token("Identifier", "x")
			),
			Term("AdditiveExpression",
				Token("Identifier", "x"),
				Token("BinaryOperator", "+"),
				Token("IntLiteral", "1")
			)
		)
	));

	EXPECT_EQ(parseString("if(true) (x;y;z)"), ParseTree(
		Term("ConditionExpression",
			Token("TrueLiteral", "true"),
			Term("Expression",
				Token("Identifier", "x"),
				Token("Separator", ";"),
				Token("Identifier", "y"),
				Token("Separator", ";"),
				Token("Identifier", "z")
			)
		)
	));
}

TEST(Expression, ReturnExpression)
{
	EXPECT_EQ(parseString("return"), ParseTree(
		Term("ReturnExpression")
	));
	EXPECT_EQ(parseString("return a+b"), ParseTree(
		Term("ReturnExpression",
			Term("AdditiveExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "b")
			)
		)
	));
}

TEST(Expression, BreakExpression)
{
	EXPECT_EQ(parseString("break"), ParseTree(
		Term("BreakExpression")
	));
	EXPECT_EQ(parseString("break (1;a)"), ParseTree(
		Term("BreakExpression",
			Term("Expression",
				Token("IntLiteral", "1"),
				Token("Separator", ";"),
				Token("Identifier", "a")
			)
		)
	));
}

TEST(Expression, ContinueExpression)
{
	EXPECT_EQ(parseString("continue"), ParseTree(
		Term("ContinueExpression")
	));
	EXPECT_EQ(parseString("continue a"), ParseTree(
		Term("ContinueExpression",
			Token("Identifier", "a")
		)
	));
}

TEST(Expression, WhileLoopExpression)
{
	EXPECT_EQ(parseString("while(cond) something else otherthing"), ParseTree(
		Term("WhileLoopExpression",
			Token("Identifier", "cond"),
			Token("Identifier", "something"),
			Token("Identifier", "otherthing")
		)
	));

	EXPECT_EQ(parseString("while(true) dosomething()"), ParseTree(
		Term("WhileLoopExpression",
			Token("TrueLiteral", "true"),
			Term("FunctionExpression",
				Token("Identifier", "dosomething"),
				Term("UndefExpression")
			)
		)
	));

	EXPECT_EQ(parseString("var i(0),var sum(0) += while(i<10) i+=1"), ParseTree(
		Term("Expression",
			Term("VarDefineExpression",
				Term("ConstraintExperssion",
					Token("Identifier", "i")
				),
				Token("IntLiteral", "0")
			),
			Token("Separator", ","),
			Term("AssignmentExpression",
				Term("VarDefineExpression",
					Term("ConstraintExperssion",
						Token("Identifier", "sum")
					),
					Token("IntLiteral", "0")
				),
				Token("BinaryOperator", "+="),
				Term("WhileLoopExpression",
					Term("RelationalExpression",
						Token("Identifier", "i"),
						Token("BinaryOperator", "<"),
						Token("IntLiteral", "10")
					),
					Term("AssignmentExpression",
						Token("Identifier", "i"),
						Token("BinaryOperator", "+="),
						Token("IntLiteral", "1")
					)
				)
			)
		)
	));

	EXPECT_EQ(parseString("var i(0),while(len(array) > i and array->i <> 233) (i+=1,) else i"), ParseTree(
		Term("Expression",
			Term("VarDefineExpression",
				Term("ConstraintExperssion",
					Token("Identifier", "i")
				),
				Token("IntLiteral", "0")
			),
			Token("Separator", ","),
			Term("WhileLoopExpression",
				Term("LogicalAndExpression",
					Term("RelationalExpression",
						Term("FunctionExpression",
							Token("Identifier", "len"),
							Token("Identifier", "array")
						),
						Token("BinaryOperator", ">"),
						Token("Identifier", "i")
					),
					Token("BinaryOperator", "and"),
					Term("EqualityExpression",
						Term("PointExpression",
							Token("Identifier", "array"),
							Token("BinaryOperator", "->"),
							Token("Identifier", "i")
						),
						Token("BinaryOperator", "<>"),
						Token("IntLiteral", "233")
					)
				),
				Term("Expression",
					Term("AssignmentExpression",
						Token("Identifier", "i"),
						Token("BinaryOperator", "+="),
						Token("IntLiteral", "1")
					),
					Token("Separator", ",")
				),
				Token("Identifier", "i")
			)
		)
	));
}

TEST(Expression, DoWhileLoopExpression)
{
	EXPECT_EQ(parseString("do something while(somecondition) else otherthing"), ParseTree(
		Term("DoWhileLoopExpression",
			Token("Identifier", "something"),
			Token("Identifier", "somecondition"),
			Token("Identifier", "otherthing")
		)
	));

	EXPECT_EQ(parseString("do i = randomNumber() while(not exists(array;i))"), ParseTree(
		Term("DoWhileLoopExpression",
			Term("AssignmentExpression",
				Token("Identifier", "i"),
				Token("BinaryOperator", "="),
				Term("FunctionExpression",
					Token("Identifier", "randomNumber"),
					Term("UndefExpression")
				)
			),
			Term("UnaryExpression",
				Token("UnaryOperator","not"),
				Term("FunctionExpression",
					Token("Identifier", "exists"),
					Term("Expression",
						Token("Identifier", "array"),
						Token("Separator", ";"),
						Token("Identifier", "i")
					)
				)
			)
		)
	));
}

TEST(Expression, FunctionExpression)
{
	EXPECT_EQ(parseString("dosomething()"), ParseTree(
		Term("FunctionExpression",
			Token("Identifier", "dosomething"),
			Term("UndefExpression")
		)
	));

	EXPECT_EQ(parseString("f(x)"), ParseTree(
		Term("FunctionExpression",
			Token("Identifier", "f"),
			Token("Identifier", "x")
		)
	));

	EXPECT_EQ(parseString("sin(2*PI)"), ParseTree(
		Term("FunctionExpression",
			Token("Identifier", "sin"),
			Term("MultiplicativeExpression",
				Token("IntLiteral", "2"),
				Token("BinaryOperator", "*"),
				Token("Identifier", "PI")
			)
		)
	));

	EXPECT_EQ(parseString("sort(fn(x,y) x<y)(begin(array);end(array))"), ParseTree(
		Term("FunctionExpression",
			Token("Identifier", "sort"),
			Term("LambdaExpression",
				Term("PatternExperssion",
					Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x")),
					Token("Separator", ","),
					Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "y"))
				),
				Term("RelationalExpression",
					Token("Identifier", "x"),
					Token("BinaryOperator", "<"),
					Token("Identifier", "y")
				)
			),
			Term("Expression",
				Term("FunctionExpression",
					Token("Identifier", "begin"),
					Token("Identifier", "array")
				),
				Token("Separator", ";"),
				Term("FunctionExpression",
					Token("Identifier", "end"),
					Token("Identifier", "array")
				)
			)
		)
	));
}

TEST(Expression, PointExpression)
{
	EXPECT_EQ(parseString("array -> n"), ParseTree(
		Term("PointExpression",
			Token("Identifier", "array"),
			Token("BinaryOperator", "->"),
			Token("Identifier", "n")
		)
	));

	EXPECT_EQ(parseString(R"(map -> "elem1")"), ParseTree(
		Term("PointExpression",
			Token("Identifier", "map"),
			Token("BinaryOperator", "->"),
			Token("StringLiteral", R"("elem1")")
		)
	));

	EXPECT_EQ(parseString("matrix->0->2"), ParseTree(
		Term("PointExpression",
			Token("Identifier", "matrix"),
			Token("BinaryOperator", "->"),
			Token("IntLiteral", "0"),
			Token("BinaryOperator", "->"),
			Token("IntLiteral", "2")
		)
	));

	EXPECT_EQ(parseString("array -> n"), ParseTree(
		Term("PointExpression",
			Token("Identifier", "array"),
			Token("BinaryOperator", "->"),
			Token("Identifier", "n")
		)
	));

	EXPECT_EQ(parseString("array -> find(array;1)"), ParseTree(
		Term("PointExpression",
			Token("Identifier", "array"),
			Token("BinaryOperator", "->"),
			Term("FunctionExpression",
				Token("Identifier", "find"),
				Term("Expression",
					Token("Identifier", "array"),
					Token("Separator", ";"),
					Token("IntLiteral", "1")
				)
			)
		)
	));
}

TEST(Expression, FoldExpression)
{
	EXPECT_EQ(parseString("args..."), ParseTree(
		Term("FoldExperssion",
			Token("Identifier", "args"),
			Token("Separator","...")
		)
	));

	EXPECT_EQ(parseString("(args + 1)..."), ParseTree(
		Term("FoldExperssion",
			Term("AdditiveExpression",
				Token("Identifier", "args"),
				Token("BinaryOperator", "+"),
				Token("IntLiteral", "1")
			),
			Token("Separator", "...")
		)
	));

	EXPECT_EQ(parseString("fn(f,x...) f(x...)"), ParseTree(
		Term("LambdaExpression",
			Term("PatternExperssion",
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "f")),
				Token("Separator", ","),
				Term("ConstraintExperssionAtPatternExperssion", Token("Identifier", "x"), Token("Separator", "..."))
			),
			Term("FunctionExpression",
				Token("Identifier", "f"),
				Term("FoldExperssion",
					Token("Identifier", "x"),
					Token("Separator", "...")
				)
			)
		)
	));
}

TEST(Expression, UnaryExpression)
{
	EXPECT_EQ(parseString("+a"), ParseTree(
		Term("UnaryExpression",
			Token("UnaryOperator", "+"),
			Token("Identifier", "a")
		)
	));

	EXPECT_EQ(parseString("not b"), ParseTree(
		Term("UnaryExpression",
			Token("UnaryOperator", "not"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("+a + +c"), ParseTree(
		Term("AdditiveExpression",
			Term("UnaryExpression",
				Token("UnaryOperator", "+"),
				Token("Identifier", "a")
			),
			Token("BinaryOperator", "+"),
			Term("UnaryExpression",
				Token("UnaryOperator", "+"),
				Token("Identifier", "c")
			)
		)
	));

	EXPECT_EQ(parseString("not not b"), ParseTree(
		Term("UnaryExpression",
			Token("UnaryOperator", "not"),
			Token("UnaryOperator", "not"),
			Token("Identifier", "b")
		)
	));
}

TEST(Expression, MultiplicativeExpression)
{
	EXPECT_EQ(parseString("a*b"), ParseTree(
		Term("MultiplicativeExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "*"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a/b"), ParseTree(
		Term("MultiplicativeExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "/"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a%b"), ParseTree(
		Term("MultiplicativeExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "%"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a*b*c/d*e/f"), ParseTree(
		Term("MultiplicativeExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "*"),
			Token("Identifier", "b"),
			Token("BinaryOperator", "*"),
			Token("Identifier", "c"),
			Token("BinaryOperator", "/"),
			Token("Identifier", "d"),
			Token("BinaryOperator", "*"),
			Token("Identifier", "e"),
			Token("BinaryOperator", "/"),
			Token("Identifier", "f")
		)
	));

	EXPECT_EQ(parseString("a*b+c/d"), ParseTree(
		Term("AdditiveExpression",
			Term("MultiplicativeExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "*"),
				Token("Identifier", "b")
			),
			Token("BinaryOperator", "+"),
			Term("MultiplicativeExpression",
				Token("Identifier", "c"),
				Token("BinaryOperator", "/"),
				Token("Identifier", "d")
			)
		)
	));
}

TEST(Expression, AdditiveExpression)
{
	EXPECT_EQ(parseString("a+b"), ParseTree(
		Term("AdditiveExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "+"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a-b"), ParseTree(
		Term("AdditiveExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "-"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a+b-c+d-e"), ParseTree(
		Term("AdditiveExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "+"),
			Token("Identifier", "b"),
			Token("BinaryOperator", "-"),
			Token("Identifier", "c"),
			Token("BinaryOperator", "+"),
			Token("Identifier", "d"),
			Token("BinaryOperator", "-"),
			Token("Identifier", "e")
		)
	));

	EXPECT_EQ(parseString("a-b>c+d-e"), ParseTree(
		Term("RelationalExpression",
			Term("AdditiveExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "-"),
				Token("Identifier", "b")
			),
			Token("BinaryOperator", ">"),
			Term("AdditiveExpression",
				Token("Identifier", "c"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "d"),
				Token("BinaryOperator", "-"),
				Token("Identifier", "e")
			)
		)
	));
}

TEST(Expression, RelationalExpression)
{
	EXPECT_EQ(parseString("a<=b"), ParseTree(
		Term("RelationalExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "<="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a>=b"), ParseTree(
		Term("RelationalExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", ">="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a<b"), ParseTree(
		Term("RelationalExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "<"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a>b"), ParseTree(
		Term("RelationalExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", ">"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a<b and b<=c"), ParseTree(
		Term("LogicalAndExpression",
			Term("RelationalExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "<"),
				Token("Identifier", "b")
			),
			Token("BinaryOperator", "and"),
			Term("RelationalExpression",
				Token("Identifier", "b"),
				Token("BinaryOperator", "<="),
				Token("Identifier", "c")
			)
		)
	));
}

TEST(Expression, EqualityExpression)
{
	EXPECT_EQ(parseString("a==b"), ParseTree(
		Term("EqualityExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "=="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a<>b"), ParseTree(
		Term("EqualityExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "<>"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a==b or a<>c"), ParseTree(
		Term("LogicalOrExpression",
			Term("EqualityExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "=="),
				Token("Identifier", "b")
			),
			Token("BinaryOperator", "or"),
			Term("EqualityExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "<>"),
				Token("Identifier", "c")
			)
		)
	));
}

TEST(Expression, LogicalAndExpression)
{
	EXPECT_EQ(parseString("a and b"), ParseTree(
		Term("LogicalAndExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "and"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a and b or c and d"), ParseTree(
		Term("LogicalOrExpression",
			Term("LogicalAndExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "and"),
				Token("Identifier", "b")
			),
			Token("BinaryOperator", "or"),
			Term("LogicalAndExpression",
				Token("Identifier", "c"),
				Token("BinaryOperator", "and"),
				Token("Identifier", "d")
			)
		)
	));
}

TEST(Expression, LogicalOrExpression)
{
	EXPECT_EQ(parseString("a or b"), ParseTree(
		Term("LogicalOrExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "or"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a or b or c and d and e"), ParseTree(
		Term("LogicalOrExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "or"),
			Token("Identifier", "b"),
			Token("BinaryOperator", "or"),
			Term("LogicalAndExpression",
				Token("Identifier", "c"),
				Token("BinaryOperator", "and"),
				Token("Identifier", "d"),
				Token("BinaryOperator", "and"),
				Token("Identifier", "e")
			)
		)
	));
}

TEST(Expression, AssignmentExpression)
{
	EXPECT_EQ(parseString("a = b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a *= b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "*="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a /= b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "/="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a %= b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "%="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a += b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "+="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a -= b"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "-="),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a = b + c"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "="),
			Term("AdditiveExpression",
				Token("Identifier", "b"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "c")
			)
		)
	));

	EXPECT_EQ(parseString("a = b /= c += d"), ParseTree(
		Term("AssignmentExpression",
			Token("Identifier", "a"),
			Token("BinaryOperator", "="),
			Term("AssignmentExpression",
				Token("Identifier", "b"),
				Token("BinaryOperator", "/="),
				Term("AssignmentExpression",
					Token("Identifier", "c"),
					Token("BinaryOperator", "+="),
					Token("Identifier", "d")
				)
			)
		)
	));
}

TEST(Expression, PairExpression)
{
	EXPECT_EQ(parseString("a : b"), ParseTree(
		Term("PairExpression",
			Token("Identifier", "a"),
			Token("Identifier", "b")
		)
	));

	EXPECT_EQ(parseString("a + c : b + c"), ParseTree(
		Term("PairExpression",
			Term("AdditiveExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "c")
			),
			Term("AdditiveExpression",
				Token("Identifier", "b"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "c")
			)
		)
	));

	EXPECT_EQ(parseString("a : b : c : d"), ParseTree(
		Term("PairExpression",
			Token("Identifier", "a"),
			Term("PairExpression",
				Token("Identifier", "b"),
				Term("PairExpression",
					Token("Identifier", "c"),
					Token("Identifier", "d")
				)
			)
		)
	));
}

TEST(Expression, UndefExpression)
{
	EXPECT_EQ(parseString("()"), ParseTree(
		Term("UndefExpression")
	));

	EXPECT_EQ(parseString("((()))"), ParseTree(
		Term("UndefExpression")
	));

	EXPECT_EQ(parseString("() + 1"), ParseTree(
		Term("AdditiveExpression",
			Term("UndefExpression"),
			Token("BinaryOperator", "+"),
			Token("IntLiteral", "1")
		)
	));
}

TEST(Expression, ExpressionList)
{
	EXPECT_EQ(parseString("(((a)))"), ParseTree(
		Token("Identifier", "a")
	));

	EXPECT_EQ(parseString("(a,b,c;d,e;f;g)"), ParseTree(
		Term("Expression",
			Token("Identifier", "a"),
			Token("Separator", ","),
			Token("Identifier", "b"),
			Token("Separator", ","),
			Token("Identifier", "c"),
			Token("Separator", ";"),
			Token("Identifier", "d"),
			Token("Separator", ","),
			Token("Identifier", "e"),
			Token("Separator", ";"),
			Token("Identifier", "f"),
			Token("Separator", ";"),
			Token("Identifier", "g")
		)
	));

	EXPECT_EQ(parseString("(a+b, var c = 2, d*e, if(true) (f,g;),)"), ParseTree(
		Term("Expression",
			Term("AdditiveExpression",
				Token("Identifier", "a"),
				Token("BinaryOperator", "+"),
				Token("Identifier", "b")
			),
			Token("Separator", ","),
			Term("AssignmentExpression",
				Term("VarDefineExpression",
					Term("ConstraintExperssion",
						Token("Identifier", "c")
					)
				),
				Token("BinaryOperator", "="),
				Token("IntLiteral", "2")
			),
			Token("Separator", ","),
			Term("MultiplicativeExpression",
				Token("Identifier", "d"),
				Token("BinaryOperator", "*"),
				Token("Identifier", "e")
			),
			Token("Separator", ","),
			Term("ConditionExpression",
				Token("TrueLiteral", "true"),
				Term("Expression",
					Token("Identifier", "f"),
					Token("Separator", ","),
					Token("Identifier", "g"),
					Token("Separator", ";")
				)
			),
			Token("Separator", ",")
		)
	));
}

TEST(Expression, ArrayList)
{
	EXPECT_EQ(parseString("[]"), ParseTree(
		Term("ArrayList")
	));

	EXPECT_EQ(parseString("[a,b,c,d,e,f,g]"), ParseTree(
		Term("ArrayList",
			Token("Identifier", "a"),
			Token("Identifier", "b"),
			Token("Identifier", "c"),
			Token("Identifier", "d"),
			Token("Identifier", "e"),
			Token("Identifier", "f"),
			Token("Identifier", "g")
		)
	));

	EXPECT_EQ(parseString("[0, while((var i(0) +=1) < 10) i]"), ParseTree(
		Term("ArrayList",
			Token("IntLiteral", "0"),
			Term("WhileLoopExpression",
				Term("RelationalExpression",
					Term("AssignmentExpression",
						Term("VarDefineExpression",
							Term("ConstraintExperssion",
								Token("Identifier", "i")
							),
							Token("IntLiteral", "0")
						),
						Token("BinaryOperator", "+="),
						Token("IntLiteral", "1")
					),
					Token("BinaryOperator", "<"),
					Token("IntLiteral", "10")
				),
				Token("Identifier", "i")
			)
		)
	));
}

TEST(Expression, DictList)
{
	EXPECT_EQ(parseString("{}"), ParseTree(
		Term("DictList")
	));

	EXPECT_EQ(parseString("{a,b,c,d,e,f,g}"), ParseTree(
		Term("DictList",
			Token("Identifier", "a"),
			Token("Identifier", "b"),
			Token("Identifier", "c"),
			Token("Identifier", "d"),
			Token("Identifier", "e"),
			Token("Identifier", "f"),
			Token("Identifier", "g")
		)
	));

	EXPECT_EQ(parseString("{0 : 1, while((var i(0) +=1) < 10) i:i+1}"), ParseTree(
		Term("DictList",
			Term("PairExpression",
				Token("IntLiteral", "0"),
				Token("IntLiteral", "1")
			),
			Term("WhileLoopExpression",
				Term("RelationalExpression",
					Term("AssignmentExpression",
						Term("VarDefineExpression",
							Term("ConstraintExperssion",
								Token("Identifier", "i")
							),
							Token("IntLiteral", "0")
						),
						Token("BinaryOperator", "+="),
						Token("IntLiteral", "1")
					),
					Token("BinaryOperator", "<"),
					Token("IntLiteral", "10")
				),
				Term("PairExpression",
					Token("Identifier", "i"),
					Term("AdditiveExpression",
						Token("Identifier", "i"),
						Token("BinaryOperator", "+"),
						Token("IntLiteral", "1")
					)
				)
			)
		)
	));
}
