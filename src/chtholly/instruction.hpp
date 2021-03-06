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

#include <utility>
#include <variant>
#include <vector>
#include <string>
#include <cmath>

namespace Chtholly
{
	template <auto V>
	struct LiteralValueIdentity
	{
		constexpr static auto value = V;
	};

	template <typename T>
	struct AlwaysEq
	{
		friend bool operator==(const T&, const T&)
		{
			return true;
		}
	};

	struct IRValueDef
	{
		struct Undef : AlwaysEq<Undef> {};
		using Null = std::nullptr_t;
		using Bool = bool;
		using Int = std::int64_t;
		using Float = std::double_t;
		using String = std::string;
	};

	template <
		typename String_ = IRValueDef::String,
		typename Float_ = IRValueDef::Float,
		typename Int_ = IRValueDef::Int,
		typename Bool_ = IRValueDef::Bool,
		typename Null_ = IRValueDef::Null,
		typename Undef_ = IRValueDef::Undef
	>
		struct BasicIRValue : std::variant<Undef_, Null_, Bool_, Int_, Float_, String_>
	{
		using Base = std::variant<Undef_, Null_, Bool_, Int_, Float_, String_>;
		using std::variant<Undef_, Null_, Bool_, Int_, Float_, String_>::variant;

		using Undef = Undef_;
		using Null = Null_;
		using Bool = Bool_;
		using Int = Int_;
		using Float = Float_;
		using String = String_;
	};

	using IRValue = BasicIRValue<>;

	template <auto Generator
		, std::enable_if_t<
		std::is_function_v<std::remove_pointer_t<decltype(Generator)>>
		, int> = 0
	>
		static constexpr std::size_t Opcode()
	{
		return typeid(LiteralValueIdentity<Generator>).hash_code();
	}

	template<typename V>
	struct BasicInstruction
	{

		using Value = V;

	private:

		std::size_t _opcode;
		std::vector<Value> _oprands;

		BasicInstruction(std::size_t hash)
			: _opcode(hash)
		{}

		BasicInstruction(std::size_t hash, std::vector<Value> value)
			: _opcode(hash), _oprands(std::move(value))
		{}

	public:

		BasicInstruction() = delete;

		~BasicInstruction() = default;

		BasicInstruction(const BasicInstruction&) = default;

		BasicInstruction& operator=(const BasicInstruction&) = default;

		bool operator==(const BasicInstruction& other) const
		{
			return _opcode == other._opcode && _oprands == other._oprands;
		}

		std::size_t opcode() const
		{
			return _opcode;
		}

		const std::vector<IRValue>& oprands() const
		{
			return _oprands;
		}

		static BasicInstruction None() { return { Opcode<None>() }; }

		struct Block
		{
			static BasicInstruction Begin() { return { Opcode<Begin>() }; }
			static BasicInstruction NamedBegin(typename Value::String blockName)
			{
				return { Opcode<NamedBegin>(), { std::move(blockName) } };
			}

			static BasicInstruction Drop() { return { Opcode<Drop>() }; }
			static BasicInstruction End() { return { Opcode<End>() }; }
		};

		struct Function
		{
			static BasicInstruction Begin() { return { Opcode<Begin>() }; }
			static BasicInstruction End() { return { Opcode<End>() }; }

			static BasicInstruction Call() { return { Opcode<Call>() }; }
		};

		struct List
		{
			static BasicInstruction Push() { return { Opcode<Push>() }; }
			static BasicInstruction Pop() { return { Opcode<Pop>() }; }
		};

		struct Control
		{
			static BasicInstruction Jump(typename Value::String tag)
			{
				return { Opcode<Jump>(), { tag } };
			}
			static BasicInstruction JumpIf(typename Value::String tag)
			{
				return { Opcode<JumpIf>(), { tag } };
			}
			static BasicInstruction JumpIfElse(typename Value::String tagIf, typename Value::String tagElse)
			{
				return { Opcode<JumpIfElse>(), { tagIf, tagElse } };
			}
			static BasicInstruction Mark(typename Value::String tag)
			{
				return { Opcode<Mark>(), { tag } };
			}
		};

		struct Object
		{
			static BasicInstruction Begin() { return { Opcode<Begin>() }; }
			static BasicInstruction End() { return { Opcode<End>() }; }
			static BasicInstruction EndWithInit() { return { Opcode<EndWithInit>() }; }
			static BasicInstruction AttachTo(typename Value::String blockName)
			{
				return { Opcode<AttachTo>(), { blockName } };
			}
			// StartPattern (temporarily ignore)
			static BasicInstruction Var(typename Value::String varName)
			{
				return { Opcode<Var>(), { varName } };
			}
			static BasicInstruction VarWithConstraint(typename Value::String varName)
			{
				return { Opcode<VarWithConstraint>(), { varName } };
			}
			static BasicInstruction VarPack(typename Value::String varName)
			{
				return { Opcode<VarPack>(), { varName } };
			}
			static BasicInstruction VarPackWithConstraint(typename Value::String varName)
			{
				return { Opcode<VarPackWithConstraint>(), { varName } };
			}
			static BasicInstruction Const(typename Value::String varName)
			{
				return { Opcode<Const>(), { varName } };
			}
			static BasicInstruction ConstWithConstraint(typename Value::String varName)
			{
				return { Opcode<ConstWithConstraint>(), { varName } };
			}
			static BasicInstruction ConstPack(typename Value::String varName)
			{
				return { Opcode<ConstPack>(), { varName } };
			}
			static BasicInstruction ConstPackWithConstraint(typename Value::String varName)
			{
				return { Opcode<ConstPackWithConstraint>(), { varName } };
			}
			static BasicInstruction Use(typename Value::String varName)
			{
				return { Opcode<Use>(), { varName } };
			}
		};
		struct Literal
		{
			static BasicInstruction Int(typename Value::Int value)
			{
				return { Opcode<Int>(), { value } };
			}
			static BasicInstruction Float(typename Value::Float value)
			{
				return { Opcode<Float>(), { value } };
			}
			static BasicInstruction String(typename Value::String value)
			{
				return { Opcode<String>(), { value } };
			}
			static BasicInstruction Bool(typename Value::Bool value)
			{
				return { Opcode<Bool>(), { value } };
			}
			static BasicInstruction Null() { return { Opcode<Null>() }; }
			static BasicInstruction Undef() { return { Opcode<Undef>() }; }

		};
	};

	using Instruction = BasicInstruction<IRValue>;

}
