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
		
		static auto PushInstructionIf(const std::function<Instruction(StringView)>& toInstruction, const std::function<bool(Iter)>& predicate)
		{
			return [=](Iter iter, SequenceRef seq) {
				if (predicate(iter))
				{
					seq.push_back(toInstruction(iter.value().value));
				}
			};
		}

		static auto IterateChildren(const std::function<void(Iter, Iter, SequenceRef)>& iterateFunc)
		{
			return [=](Iter iter, SequenceRef seq) {
				iterateFunc(iter.childrenBegin(), iter.childrenEnd(), seq);
			};
		}

		static auto IterateChildrenForAll(const std::function<void(Iter, SequenceRef)>& iterateFunc)
		{
			return IterateChildren([=](Iter begin, Iter end, SequenceRef seq) {
				for(Iter i = begin; i != end; ++i)
				{
					iterateFunc(i, seq);
				}
			});
		}

		template <typename F>
		static auto IterateChildrenInAutomaton(F&& iterateFunc, 
			std::enable_if_t<is_instance_of_v<std::invoke_result_t<decltype(iterateFunc), SequenceRef>, FiniteAutomaton>, int> = 0)
		{
			return IterateChildren([=](Iter begin, Iter end, SequenceRef seq) {
				iterateFunc(seq)(begin, end);
			});
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
			{"Expression", sequence(IterateChildrenInAutomaton([](SequenceRef seq) {
					return FiniteAutomaton<std::string, Iter> {"value", "error", {
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
					}}; 
			}), PushInstructionIf(
					constant(Instruction::Block::End()), 
					[](Iter iter) {
						return (--iter.childrenEnd()).value().name != "Separator";
					}
				)
			)},
			{"ArrayList", sequence(
				PushInstruction(constant(Instruction::Block::Begin())),
				PushInstruction(constant(Instruction::Object::Use("array.literal"))),
				IterateChildrenForAll(Walk),
				PushInstruction(constant(Instruction::Function::Call()))
			)},
			{"DictList", sequence(
				PushInstruction(constant(Instruction::Block::Begin())),
				PushInstruction(constant(Instruction::Object::Use("dict.literal"))),
				IterateChildrenForAll(Walk),
				PushInstruction(constant(Instruction::Function::Call()))
			)},
			{"UndefExpression", PushInstruction(constant(
				Instruction::Literal::Undef()
			))}
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
