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
					Conv<StringView>::To<Instruction::Value::Int>(iter.value().value)
				));
			}},
			{"FloatLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::Float(
					Conv<StringView>::To<Instruction::Value::Float>(iter.value().value)
				));
			}},
			{"StringLiteral", [](Iter iter, SequenceRef seq) {
				seq.push_back(Instruction::Literal::String(
					Conv<Quoted<StringView>>::To<Instruction::Value::String>(iter.value().value)
				));
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
