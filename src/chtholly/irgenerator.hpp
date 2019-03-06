#pragma once

#include <map>
#include <functional>

#include "parsetree.hpp"
#include "instruction.hpp"
#include "stringconv.hpp"
#include "automata.hpp"
#include "functional.hpp"

namespace Chtholly
{
	template <typename StringView>
	struct BasicIRGenerator
	{
		using Sequence = std::vector<Instruction>;

		using SequenceRef = Sequence&;

		using Tree = BasicParseTree<StringView>;

		using UnitName = typename Tree::Unit::String;

		using Iter = typename Tree::Observer;
		
		using GenerateFunc = std::function<void (Iter, SequenceRef)>;

		using Map = std::map<UnitName, GenerateFunc>;

		static void Walk(Iter iter, SequenceRef seq)
		{
			return map.at(iter.value().name)(iter, seq);
		}

		static auto PushInstruction(const std::function<Instruction(StringView)>& toInstruction) 
		{
			return [=](Iter iter, SequenceRef seq) {
				seq.push_back(toInstruction(iter.value().value));
			};
		}

		static auto IterateChildren(const std::function<void(Iter, Iter, SequenceRef)>& iterateFunc)
		{
			return [=](Iter iter, SequenceRef seq) {
				iterateFunc(iter.childrenBegin(), iter.childrenEnd(), seq);
			};
		}
		
		inline static const Map map = {
			{"IntLiteral", PushInstruction(compose(
				Instruction::Literal::Int, Conv<StringView>::template To<Instruction::Value::Int>
			))},
			{"FloatLiteral", PushInstruction(compose(
				Instruction::Literal::Float,
				Conv<StringView>::template To<Instruction::Value::Float>
			))},
			{"StringLiteral", PushInstruction(compose(
				Instruction::Literal::String,
				Conv<Quoted<StringView>>::template To<Instruction::Value::String>
			))},
			{"NullLiteral", PushInstruction(constant(
				Instruction::Literal::Null()
			))},
			{"UndefinedLiteral", PushInstruction(constant(
				Instruction::Literal::Undef()
			))},
			{"TrueLiteral", PushInstruction(constant(
				Instruction::Literal::Bool(true)
			))},
			{"FalseLiteral", PushInstruction(constant(
				Instruction::Literal::Bool(false)
			))},
			{"Identifier", PushInstruction(compose(
				Instruction::Object::Use, constructor<Instruction::Value::String>
			))},
			{"Expression", sequence(IterateChildren([](Iter begin, Iter end, SequenceRef seq) {
				FiniteAutomaton<std::string, Iter>{"value", "error", {
					{"value", [&](Iter it) {
						seq.push_back(Instruction::Block::Begin());
						Walk(it, seq);
						return "sep";
					}},
					{"sep", [&](Iter it) {
						if (it.value().name == "Separator")
						{
							if (it.value().value == ";")
							{
								seq.push_back(Instruction::Block::End());
							}
							else if (it.value().value == ",")
							{
								seq.push_back(Instruction::Block::Drop());
							}

							return "value";
						}
						return "error";
					}}
				}}(begin, end); }),[](Iter iter, SequenceRef seq) {
					if ((--iter.childrenEnd()).value().name != "Separator")
					{
						seq.push_back(Instruction::Block::End());
					}
				}
			)},
		};

		static Sequence Generate(const Tree& tree)
		{
			Sequence seq;
			Walk(tree.observer().childrenBegin(), seq);
			return seq;
		}
	};

	using IRGenerator = BasicIRGenerator<std::string_view>;

}
