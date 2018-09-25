#pragma once

#include <vector>
#include <variant>

#include "parsetree.hpp"

namespace Chtholly
{
	struct Opcode
	{
		enum : unsigned
		{
			None = 0
		};
		enum class Block : unsigned
		{
			Begin = 0x10,
			Drop,
			End
		};
		enum class Lambda : unsigned
		{
			Begin = 0x20,
			End
		};
		enum class List : unsigned
		{
			Push = 0x30,
			Call
		};
		enum class Control : unsigned
		{
			Jump = 0x40,
			JumpIf,
			Mark
		};
		enum class Object : unsigned
		{
			Var = 0x50,
			VarWithInit,
			VarWithConstraint,
			VarWithInitAndConstraint,
			Const,
			ConstWithInit,
			ConstWithConstraint,
			ConstWithInitAndConstraint,
			Use
		};
		enum class Pattern : unsigned
		{
			Var = 0x60,
			VarWithConstraint,
			Vars,
			VarsWithConstraint,
			Const,
			ConstWithConstraint,
			Consts,
			ConstsWithConstraint
		};
		enum class Literal : unsigned
		{
			Int = 0x70,
			Float,
			String,
			Null,
			Undef,
			True,
			False
		};

		Opcode() : value(None) {}

		Opcode(Block inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Lambda inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(List inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Control inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Object inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Pattern inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Literal inValue) : value(static_cast<unsigned>(inValue)) {}

	private:
		unsigned value;
	};

}
