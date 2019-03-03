#pragma once

#include <map>
#include <functional>

#include "parsetree.hpp"
#include "instruction.hpp"
#include "stringconv.hpp"

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

		static auto PushInstruction(const std::function<Instruction(StringView)>& toInstruction) {
			return [=](Iter iter, SequenceRef seq) {
				seq.push_back(toInstruction(iter.value().value));
			};
		}
		
		inline static const Map map = {
			{"IntLiteral", PushInstruction([](StringView v){
				return Instruction::Literal::Int(
					Conv<StringView>::template To<Instruction::Value::Int>(v)
				);
			})},
			{"FloatLiteral", PushInstruction([](StringView v) {
				return Instruction::Literal::Float(
					Conv<StringView>::template To<Instruction::Value::Float>(v)
				);
			})},
			{"StringLiteral", PushInstruction([](StringView v) {
				return Instruction::Literal::String(
					Conv<Quoted<StringView>>::template To<Instruction::Value::String>(v)
				);
			})},
			{"NullLiteral", PushInstruction([](StringView) {
				return Instruction::Literal::Null();
			})},
			{"UndefinedLiteral", PushInstruction([](StringView) {
				return Instruction::Literal::Undef();
			})},
			{"TrueLiteral", PushInstruction([](StringView) {
				return Instruction::Literal::Bool(true);
			})},
			{"FalseLiteral", PushInstruction([](StringView) {
				return Instruction::Literal::Bool(false);
			})},
			{"Identifier", PushInstruction([](StringView v) {
				return Instruction::Object::Use(Instruction::Value::String{ v });
			})},
			{"Expression", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Block::Begin());
				for(auto i = iter.childrenBegin(); i != iter.childrenEnd(); ++i)
				{
					if (i.value().name == "Separator")
					{
						if(i.value().value == ";")
						{
							seq.push_back(Instruction::Block::End());
						}
						else if(i.value().value == ",")
						{
							seq.push_back(Instruction::Block::Drop());
						}
						if (i != --iter.childrenEnd())
						{
							seq.push_back(Instruction::Block::Begin());
						}
					}
					else
					{
						Walk(i, seq);
					}
				}
				if((--iter.childrenEnd()).value().name != "Separator")
				{
					seq.push_back(Instruction::Block::End());
				}
			}},
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
