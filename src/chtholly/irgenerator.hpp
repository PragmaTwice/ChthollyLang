/*
* Copyright 2019 PragmaTwice
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*		http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

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
		struct State
		{
			enum class ObjectProp { Invalid, Var, Const };

			static auto ObjectPropMap(ObjectProp prop, bool hasConstraint = false, bool isPack = false)
			{
				switch(prop)
				{
				case ObjectProp::Const:
					if (hasConstraint)
					{
						if (isPack) return Instruction::Object::ConstPackWithConstraint;
						return Instruction::Object::ConstWithConstraint;
					}
					if (isPack) return Instruction::Object::ConstPack;
					return Instruction::Object::Const;
					
				case ObjectProp::Var:
					if (hasConstraint)
					{
						if (isPack) return Instruction::Object::VarPackWithConstraint;
						return Instruction::Object::VarWithConstraint;
					}
					if (isPack) return Instruction::Object::VarPack;
					return Instruction::Object::Var;
				}
			}

			ObjectProp objectProp = ObjectProp::Invalid;
		};

		using StateRef = State&;

		using Sequence = std::vector<Instruction>;

		using SequenceRef = Sequence&;

		using Tree = BasicParseTree<StringView>;

		using UnitName = typename Tree::Unit::String;

		using Iter = typename Tree::Observer;
		
		using GenerateFunc = std::function<void (Iter, SequenceRef, StateRef)>;

		using Map = std::map<UnitName, GenerateFunc>;

		static void Walk(Iter iter, SequenceRef seq, StateRef state)
		{
			return map.at(iter.value().name)(iter, seq, state);
		}

		static auto PushInstruction(const std::function<Instruction(StringView)>& toInstruction) 
		{
			return [=](Iter iter, SequenceRef seq, StateRef) {
				seq.push_back(toInstruction(iter.value().value));
			};
		}
		
		static auto PushInstructionIf(const std::function<Instruction(StringView)>& toInstruction, const std::function<bool(Iter)>& predicate)
		{
			return [=](Iter iter, SequenceRef seq, StateRef) {
				if (predicate(iter))
				{
					seq.push_back(toInstruction(iter.value().value));
				}
			};
		}

		static auto IterateChildren(const std::function<void(Iter, Iter, SequenceRef, StateRef)>& iterateFunc)
		{
			return [=](Iter iter, SequenceRef seq, StateRef state) {
				iterateFunc(iter.childrenBegin(), iter.childrenEnd(), seq, state);
			};
		}

		static auto IterateChildrenForAll(const GenerateFunc& iterateFunc)
		{
			return IterateChildren([=](Iter begin, Iter end, SequenceRef seq, StateRef state) {
				for(Iter i = begin; i != end; ++i)
				{
					iterateFunc(i, seq, state);
				}
			});
		}

		template <typename F>
		static auto IterateChildrenInAutomaton(F&& iterateFunc, 
			std::enable_if_t<is_instance_of_v<std::invoke_result_t<decltype(iterateFunc), SequenceRef, StateRef>, FiniteAutomaton>, int> = 0)
		{
			return IterateChildren([=](Iter begin, Iter end, SequenceRef seq, StateRef state) {
				iterateFunc(seq, state)(begin, end);
			});
		}

		static auto ChangeState(const std::function<void(Iter, StateRef)>& func)
		{
			return [=](Iter iter, SequenceRef, StateRef state) {
				func(iter, state);
			};
		}

		static auto ChangeStateConst(const std::function<void(StateRef)>& func)
		{
			return ChangeState([=](Iter, StateRef state) {
				func(state);
			});
		}

		template <typename T>
		static auto SetStateProp(T State::* member, const T& value)
		{
			return ChangeStateConst([=](StateRef state) {
				std::mem_fn(member)(state) = value;
			});
		}

		static auto MultiExpressionPackage(const GenerateFunc& iterateFunc)
		{
			return sequence(IterateChildrenInAutomaton([=](SequenceRef seq, StateRef state) {
				return FiniteAutomaton<std::string, Iter> {"value", "error", {
					{"value", [&](Iter it) {
						seq.push_back(Instruction::Block::Begin());
						iterateFunc(it, seq, state);
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
			));
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
			{"Expression", MultiExpressionPackage(Walk)
			},
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
			))},
			{"VarDefineExpression", sequence(
				PushInstruction(constant(Instruction::Object::Begin())),
				SetStateProp(&State::objectProp, State::ObjectProp::Var),
				IterateChildrenForAll(Walk),
				SetStateProp(&State::objectProp, State::ObjectProp::Invalid),
				PushInstruction(constant(Instruction::Object::End()))
			)},
			{"ConstDefineExpression", sequence(
				PushInstruction(constant(Instruction::Object::Begin())),
				SetStateProp(&State::objectProp, State::ObjectProp::Const),
				IterateChildrenForAll(Walk),
				SetStateProp(&State::objectProp, State::ObjectProp::Invalid),
				PushInstruction(constant(Instruction::Object::End()))
			)},
			{"PatternExpression", MultiExpressionPackage(Walk)
			},
			{"ConstraintExpression", [](Iter iter, SequenceRef seq, StateRef state) {
				auto identifier = iter.childrenBegin();
				auto constraint = identifier.thisNext();
				bool hasConstraint = constraint != iter.childrenEnd();
				if (hasConstraint)
				{
					Walk(constraint, seq, state);
				}
				seq.push_back(
					State::ObjectPropMap(state.objectProp, hasConstraint)(Instruction::Value::String{ identifier.value().value })
				);
			}},
			{"ConstraintExpressionAtPatternExpression", [](Iter iter, SequenceRef seq, StateRef state) {
				auto identifier = iter.childrenBegin();
				auto constraint = identifier.thisNext();
				auto separator = iter.childrenEnd();

				if(constraint != iter.childrenEnd() && constraint.value().name == "Separator")
				{
					separator = constraint;
					constraint = separator.thisNext();
				}

				bool hasConstraint = constraint != iter.childrenEnd();
				bool hasSeparator = separator != iter.childrenEnd();

				if (hasConstraint)
				{
					Walk(constraint, seq, state);
				}
				seq.push_back(
					State::ObjectPropMap(state.objectProp, hasConstraint, hasSeparator)(Instruction::Value::String{ identifier.value().value })
				);
			}}
		};
		
		static Sequence Generate(const Tree& tree)
		{
			Sequence seq;
			State state;
			Walk(tree.observer().childrenBegin(), seq, state);
			return seq;
		}
	};

	using IRGenerator = BasicIRGenerator<std::string_view>;

}
