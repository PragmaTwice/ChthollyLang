/*
* Copyright 2017 PragmaTwice
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

#include <string_view>
#include <string>
#include <list>
#include <type_traits>

namespace Chtholly
{
	template<typename T>
	T UnusedConstruct(std::in_place_type_t<T>)
	{
		return {};
	}

	template <typename inValueType>
	class BasicTree
	{
	public:

		using ValueType = inValueType;

	protected:

		class NodeWrapper;

		struct Node
		{
			using Container = std::list<Node>;

			using ConstIterator = typename Container::const_iterator;
			using Iterator = typename Container::iterator;

			using Size = typename Container::size_type;

			ValueType value;

			Iterator parent;
			Container children;

			template <typename ...T, std::enable_if_t<std::is_constructible_v<ValueType, T&&...>,int> = 0>
			Node(T&& ...args) 
				: value(std::forward<T>(args)...) {}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<ValueType, T&&...>, int> = 0>
			Node(Iterator in_parent, T&& ...args)
				: value(std::forward<T>(args)...), parent(in_parent) {}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<ValueType, T&&...>, int> = 0>
			Node(const Container& in_children, T&& ...args) 
				: value(std::forward<T>(args)...), children(in_children) {}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<ValueType, T&&...>, int> = 0>
			Node(Iterator in_parent, const Container& in_children, T&& ...args)
				: value(std::forward<T>(args)...), parent(in_parent), children(in_children) {}

			Node(const Node& src)
				: value(src.value), parent(src.parent), children(src.children) {}

			Node(Iterator in_parent, const Node& src)
				: value(src.value), parent(in_parent), children(src.children) {}

			Node(const Container& in_container, const Node& src)
				: value(src.value), parent(src.parent), children(in_container) {}

			Node(Iterator in_iterator, const Container& in_container, const Node& src)
				: value(src.value), parent(in_iterator), children(in_container) {}

			Node(const NodeWrapper& wrapper)
				: Node((const Node&)wrapper) {}

			Node& operator=(const Node& src)
			{
				value = src.value;
				parent = src.parent;
				children = src.children;

				return *this;
			}

			Node& operator=(Node&& src) noexcept
			{
				value = std::move(src.value);
				parent = std::move(src.parent);
				children = std::move(src.children);

				return *this;
			}

			bool operator==(const Node& other) const
			{
				return value == other.value && children == other.children;
			}

			bool operator!=(const Node& other) const
			{
				return !(*this == other);
			}

		};


		typename Node::Container root;

	public:

		struct NodeWrapper : private Node
		{
			using Node::Node;
		};

		class Visitor;

		class Observer
		{
		protected:

			using Iterator = typename Node::ConstIterator;

			Iterator nodeIter;

			Observer(const Iterator& src) : nodeIter(src) {}

			Observer& operator=(const Iterator& src)
			{
				return nodeIter = src;
			}

			const Node& operator*() const
			{
				return *nodeIter;
			}

			friend class BasicTree<ValueType>;

		public:

			Observer() {}
			Observer(const Observer& src) : nodeIter(src.nodeIter) {}
			Observer(const Visitor& src) : nodeIter(src.nodeIter) {}


			Observer& operator=(const Observer& src)
			{
				return nodeIter = src.nodeIter;
			}
			bool operator==(const Observer& other) const
			{
				return nodeIter == other.nodeIter;
			}
			bool operator!=(const Observer& other) const
			{
				return nodeIter != other.nodeIter;
			}

			Observer& operator++()
			{
				++nodeIter;
				return *this;
			}
			Observer& operator--()
			{
				--nodeIter;
				return *this;
			}

			bool childrenEmpty() const
			{
				return nodeIter->children.empty();
			}

			typename Node::Size thisSize() const
			{
				return Observer(nodeIter->parent).childrenSize();
			}
			typename Node::Size childrenSize() const
			{
				return nodeIter->children.size();
			}

			Observer parent() const
			{
				return nodeIter->parent;
			}

			Observer thisBegin() const
			{
				return Observer(nodeIter->parent).childrenBegin();
			}
			Observer thisEnd() const
			{
				return Observer(nodeIter->parent).childrenEnd();
			}
			Observer thisNext() const
			{
				auto tempIter = nodeIter;
				return ++tempIter;
			}
			Observer thisPrev() const
			{
				auto tempIter = nodeIter;
				return --tempIter;
			}

			Observer childrenBegin() const
			{
				return nodeIter->children.begin();
			}
			Observer childrenEnd() const
			{
				return nodeIter->children.end();
			}

			const ValueType& value() const
			{
				return nodeIter->value;
			}

			const ValueType& childrenFrontValue() const
			{
				return nodeIter->children.front().value;
			}
			
			const ValueType& childrenBackValue() const
			{
				return nodeIter->children.back().value;
			}

			~Observer() {}
		};

		class Visitor
		{
		protected:

			using Iterator = typename Node::Iterator;

			Iterator nodeIter;

			Visitor(const Iterator& src) : nodeIter(src) {}

			Visitor& operator=(const Iterator& src)
			{
				return nodeIter = src;
			}
			
			const Node& operator*() const
			{
				return *nodeIter;
			}

			friend class BasicTree<ValueType>;

		public:

			Visitor() {}
			Visitor(const Visitor& src) : nodeIter(src.nodeIter) {}

			Visitor& operator=(const Visitor& src)
			{
				return nodeIter = src.nodeIter;
			}
			bool operator==(const Visitor& other) const
			{
				return nodeIter == other.nodeIter;
			}
			bool operator!=(const Visitor& other) const
			{
				return nodeIter != other.nodeIter;
			}

			Visitor& operator++()
			{
				++nodeIter;
				return *this;
			}
			Visitor& operator--()
			{
				--nodeIter;
				return *this;
			}

			bool childrenEmpty() const
			{
				return nodeIter->children.empty();
			}

			typename Node::Size thisSize() const
			{
				return Visitor(nodeIter->parent).childrenSize();
			}
			typename Node::Size childrenSize() const
			{
				return nodeIter->children.size();
			}

			Visitor parent() const
			{
				return nodeIter->parent;
			}

			Visitor thisBegin() const
			{
				return Visitor(nodeIter->parent).childrenBegin();
			}
			Visitor thisEnd() const
			{
				return Visitor(nodeIter->parent).childrenEnd();
			}
			Visitor thisNext() const
			{
				auto tempIter = nodeIter;
				return ++tempIter;
			}
			Visitor thisPrev() const
			{
				auto tempIter = nodeIter;
				return --tempIter;
			}

			Visitor childrenBegin() const
			{
				return nodeIter->children.begin();
			}
			Visitor childrenEnd() const
			{
				return nodeIter->children.end();
			}

			const ValueType& value() const
			{
				return nodeIter->value;
			}
			ValueType& value()
			{
				return nodeIter->value;
			}

			ValueType& childrenFrontValue()
			{
				return nodeIter->children.front().value;
			}
			const ValueType& childrenFrontValue() const
			{
				return nodeIter->children.front().value;
			}
			ValueType& childrenBackValue()
			{
				return nodeIter->children.back().value;
			}
			const ValueType& childrenBackValue() const
			{
				return nodeIter->children.back().value;
			}

			~Visitor() {}
		};

		class Modifier : public Visitor
		{
		protected:

			Modifier(const Iterator& src) : Visitor(src) {}

			void setParent(const Visitor& parent)
			{
				nodeIter->parent = parent.nodeIter;
			}

			const Node& operator*() const
			{
				return *nodeIter;
			}

			Node& operator*()
			{
				return *nodeIter;
			}

			explicit Modifier(const Visitor& src)
				: Visitor(src.nodeIter) {}

			Modifier& operator=(const Visitor& src)
			{
				nodeIter = src.nodeIter;
				return *this;
			}

			friend class BasicTree<ValueType>;

		public:

			Modifier(){}

			Modifier(const Modifier& src) 
				: Visitor(src.nodeIter) {}

			Modifier& operator=(const Modifier& src)
			{
				nodeIter = src.nodeIter;
				return *this;
			}

			Modifier& operator++()
			{
				++nodeIter;
				return *this;
			}
			Modifier& operator--()
			{
				--nodeIter;
				return *this;
			}

			Modifier parent() const
			{
				return nodeIter->parent;
			}

			Modifier thisBegin() const
			{
				return Modifier(nodeIter->parent).childrenBegin();
			}
			Modifier thisEnd() const
			{
				return Modifier(nodeIter->parent).childrenEnd();
			}
			Modifier thisNext() const
			{
				return Modifier{ Visitor::thisNext() };
			}
			Modifier thisPrev() const
			{
				return Modifier{ Visitor::thisPrev() };
			}

			Modifier childrenBegin() const
			{
				return Modifier{ Visitor::childrenBegin() };
			}
			Modifier childrenEnd() const
			{
				return Modifier{ Visitor::childrenEnd() };
			}

			bool operator==(const Modifier& other) const
			{
				return nodeIter == other.nodeIter;
			}
			bool operator!=(const Modifier& other) const
			{
				return nodeIter != other.nodeIter;
			}

			void childrenClear()
			{
				nodeIter->children.clear();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void childrenPushFront(T&& ...inValue)
			{
				nodeIter->children.emplace_front(nodeIter, std::forward<T>(inValue)...);
				BasicTree::FixParent({ childrenBegin() });
			}

			void childrenPopFront()
			{
				nodeIter->children.pop_front();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void childrenPushBack(T&& ...inValue)
			{
				nodeIter->children.emplace_back(nodeIter, std::forward<T>(inValue)...);
				BasicTree::FixParent({ --childrenEnd() });
			}

			void childrenPopBack()
			{
				nodeIter->children.pop_back();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void childrenResize(typename Node::Size size, T&& ...inValue)
			{
				nodeIter->children.resize(size, Node{ nodeIter, std::forward<T>(inValue)... });
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			Modifier childrenInsert(const Modifier& pos, T&& ...inValue)
			{
				auto result = nodeIter->children.emplace(pos.nodeIter, nodeIter, std::forward<T>(inValue)...);
				BasicTree::FixParent({ result });
				return result;
			}

			Modifier childrenErase(const Modifier& pos)
			{
				return nodeIter->children.erase(pos.nodeIter);
			}
			Modifier childrenErase(const Modifier& begin, const Modifier& end)
			{
				return nodeIter->children.erase(begin.nodeIter, end.nodeIter);
			}


			void thisClear()
			{
				Modifier(nodeIter->parent).childrenClear();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void thisPushFront(T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenPushFront(std::forward<T>(inValue)...);
			}

			void thisPopFront()
			{
				Modifier(nodeIter->parent).childrenPopFront();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void thisPushBack(T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenPushBack(std::forward<T>(inValue)...);
			}

			void thisPopBack()
			{
				Modifier(nodeIter->parent).childrenPopBack();
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			void thisResize(typename Node::Size size, T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenResize(size, std::forward<T>(inValue)...);
			}

			template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
			Modifier thisInsert(const Modifier& pos, T&& ...inValue)
			{
				return Modifier(nodeIter->parent).childrenInsert(pos, std::forward<T>(inValue)...);
			}

			Modifier thisErase(const Modifier& pos)
			{
				return Modifier(nodeIter->parent).childrenErase(pos);
			}
			Modifier thisErase(const Modifier& begin, const Modifier& end)
			{
				return Modifier(nodeIter->parent).childrenErase(begin,end);
			}

			Modifier thisCopyTo(Modifier modi)
			{
				return modi.thisInsert(modi, nodeIter->children, nodeIter->value);
			}

			Modifier thisMoveTo(Modifier modi)
			{
				auto newModi = thisCopyTo(modi);
				thisErase(*this);

				return newModi;
			}

			~Modifier() {}
		};

		template <typename ...T, std::enable_if_t<std::is_constructible_v<Node, T&&...>, int> = 0>
		BasicTree(T&& ...inValue) : root{ { UnusedConstruct(std::in_place_type<ValueType>) } }
		{
			Modifier(root.begin()).childrenPushBack(std::forward<T>(inValue)...);
		}

		BasicTree(const BasicTree& src) : root(src.root)
		{
			FixParent(root.begin());
		}

		BasicTree& operator=(const BasicTree& src)
		{
			root = src.root;
			FixParent(root.begin());

			return *this;
		}

		BasicTree& operator=(BasicTree&& src) noexcept
		{
			root = std::move(src.root);
			return *this;
		}

		Observer observer() const
		{
			return root.front().children.begin();
		}

		Visitor visitor()
		{
			return root.front().children.begin();
		}

		Modifier modifier()
		{
			return root.front().children.begin();
		}

		static bool CheckParent(const Observer& vis)
		{
			for (auto child = vis.childrenBegin(); child != vis.childrenEnd(); ++child)
			{
				if (child.parent() != vis) return false;
				if (!CheckParent(child)) return false;
			}

			return true;
		}

		static void FixParent(Modifier modi)
		{
			for (auto child = modi.childrenBegin(); child != modi.childrenEnd(); ++child)
			{
				child.setParent(modi);
				FixParent(child);
			}
		}

		static bool Equals(Observer lhs, Observer rhs)
		{
			return *lhs == *rhs;
		}

		bool operator==(const BasicTree& other) const
		{
			return Equals(observer(), other.observer());
		}

		bool checkParent() const
		{
			return CheckParent(observer());
		}

		void fixParent()
		{
			FixParent(modifier());
		}

		~BasicTree() {}
	};

	template <typename inStringView>
	class BasicParseUnit
	{

	public:

		using StringView = inStringView;
		using String = std::string;
		
		enum class Type { token = 1, term };

	public:


		Type type;
		String name;
		StringView value;

		template <typename ...T, std::enable_if_t<std::is_constructible_v<StringView, T&&...>, int> = 0>
		BasicParseUnit(Type inType, const String& inName, T&& ... inValue) : type(inType), name(inName), value(inValue...) {}

		BasicParseUnit(const BasicParseUnit& src) : type(src.type), name(src.name), value(src.value) {}

		BasicParseUnit& operator=(const BasicParseUnit& src)
		{
			type = src.type;
			name = src.name;
			value = src.value;
			
			return *this;
		}

		bool operator== (const BasicParseUnit& other) const
		{
			return type == other.type && name == other.name && value == other.value;
		}
		bool operator!= (const BasicParseUnit& other) const
		{
			return !(*this == other);
		}

		~BasicParseUnit() {}
	};

	template<typename ValueType>
	BasicParseUnit<ValueType> UnusedConstruct(std::in_place_type_t<BasicParseUnit<ValueType>>)
	{
		return { BasicParseUnit<ValueType>::Type::term, "unused" };
	}


	using ParseUnit = BasicParseUnit<std::string_view>;

	template <typename StringView>
	class BasicParseTree : public BasicTree<BasicParseUnit<StringView>>
	{
	public:

		using Unit		= BasicParseUnit<StringView>;
		using UnitName	= typename Unit::String;
		using UnitValue	= typename Unit::StringView;
		using UnitType	= typename Unit::Type;

		template <typename... Nodes>
		BasicParseTree(const UnitName& rootName, Nodes&& ...nodes) 
			: BasicTree(Node::Container{ std::forward<Nodes>(nodes)... }, UnitType::term, rootName)
		{
			static_assert(std::conjunction_v<std::is_same<std::remove_reference_t<Nodes>, NodeWrapper>...>, "BasicParseTree::BasicParseTree: invalid arguments type");
		}

		template <typename... Nodes, std::enable_if_t<std::conjunction_v<std::negation<std::is_constructible<UnitName, Nodes>>...>,int> = 0>
		BasicParseTree(Nodes&& ...nodes)
			: BasicParseTree("root", std::forward<Nodes>(nodes)...)
		{
			static_assert(std::conjunction_v<std::is_same<std::remove_reference_t<Nodes>, NodeWrapper>...>, "BasicParseTree::BasicParseTree: invalid arguments type");
		}

		static NodeWrapper Token(const UnitName& name, const UnitValue& value)
		{
			return {UnitType::token, name, value};
		}

		template <typename... Nodes>
		static NodeWrapper Term(const UnitName& name, Nodes&& ...nodes)
		{
			static_assert(std::conjunction_v<std::is_same<std::remove_reference_t<Nodes>, NodeWrapper>...>, "BasicParseTree::Term: invalid arguments type");
			
			return { Node::Container{ std::forward<Nodes>(nodes)... }, UnitType::term, name };
		}

	};

	using ParseTree = BasicParseTree<std::string_view>;

	inline auto Token = ParseTree::Token;

	template <typename ...Args>
	decltype(auto) Term(Args&& ...args)
	{
		return ParseTree::Term(std::forward<Args>(args)...);
	}
}
