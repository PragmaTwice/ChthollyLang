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
		enum class Function : unsigned
		{
			Start = 0x20,
			Call
		};
		enum class List : unsigned
		{
			Push = 0x30,
			Pop
		};
		enum class Control : unsigned
		{
			Jump = 0x40,			// 1
			JumpIf,					// 2
			Mark					// 1
		};
		enum class Object : unsigned
		{
			Var = 0x50,				// 1
			VarWithConstraint,		// 1
			VarPack,				// 1
			VarPackWithConstraint,	// 1
			Const,					// 1
			ConstWithConstraint,	// 1
			ConstPack,				// 1
			ConstPackWithConstraint,// 1
			Use,					// 1
			Init
		};
		enum class Pattern : unsigned
		{
			Begin = 0x60,
			End
		};
		enum class Literal : unsigned
		{
			Int = 0x70,				// 1
			Float,					// 1
			String,					// 1
			Null,
			Undef,
			True,
			False
		};

		Opcode() : value(None) {}

		Opcode(Block inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Function inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(List inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Control inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Object inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Pattern inValue) : value(static_cast<unsigned>(inValue)) {}
		Opcode(Literal inValue) : value(static_cast<unsigned>(inValue)) {}

		Opcode(const Opcode& opcode) = default;

		Opcode& operator= (const Opcode& opcode) = default;

		bool operator== (const Opcode& opcode) const
		{
			return unsigned{ *this } == unsigned{ opcode };
		}

		bool operator!= (const Opcode& opcode) const
		{
			return !(*this == opcode);
		}
		
		explicit operator unsigned() const
		{
			return value;
		}

		~Opcode() = default;

	private:
		unsigned value;
	};

}
