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
		
		inline static const Map map = {
			{"IntLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Int(
					Conv<StringView>::template To<Instruction::Value::Int>(iter.value().value)
				));
			}},
			{"FloatLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Float(
					Conv<StringView>::template To<Instruction::Value::Float>(iter.value().value)
				));
			}},
			{"StringLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::String(
					Conv<Quoted<StringView>>::template To<Instruction::Value::String>(iter.value().value)
				));
			}},
			{"NullLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Null());
			}},
			{"UndefinedLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Undef());
			}},
			{"TrueLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Bool(true));
			}},
			{"FalseLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Bool(false));
			}},
			{"Identifier", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Object::Use(Instruction::Value::String{ iter.value().value }));
			}},
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
