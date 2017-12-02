#pragma once

#include <string_view>
#include <string>
#include <list>

namespace Chtholly
{
	template <typename inValueType>
	class BasicTree
	{
		using ValueType = inValueType;

		struct Node
		{
			using Container = std::list<Node>;
			using Iterator = typename Container::iterator;
			using Size = typename Container::size_type;

			ValueType value;

			Iterator parent;
			Container children;


			template <typename ...T>
			Node(T&& ...args) 
				: value(std::forward<T>(args)...) {}

			template <typename ...T>
			Node(Iterator in_parent, T&& ...args)
				: value(std::forward<T>(args)...), parent(in_parent) {}

			template <typename ...T>
			Node(Container in_children, T&& ...args) 
				: value(std::forward<T>(args)...), children(in_children) {}

			template <typename ...T>
			Node(Iterator in_parent, Container in_children, T&& ...args)
				: value(std::forward<T>(args)...), parent(in_parent), children(in_children) {}

			Node(Node& src)
				: value(src.value), parent(src.parent), children(src.children) {}
			Node(const Node& src)
				: value(src.value), parent(src.parent), children(src.children) {}
			Node(Node&& src) 
				: value(std::move(src.value)), parent(std::move(src.parent)), children(std::move(src.children)) {}

			Node& operator=(const Node& src)
			{
				value = src.value;
				parent = src.parent;
				children = src.children;

				return *this;
			}

			Node& operator=(Node&& src)
			{
				value = std::move(src.value);
				parent = std::move(src.parent);
				children = std::move(src.children);

				return *this;
			}
		};

		typename Node::Container root;

	public:

		class Visitor
		{
		protected:

			typename Node::Iterator nodeIter;

			Visitor(const typename Node::Iterator& src) : nodeIter(src) {}

			Visitor& operator=(const typename Node::Iterator& src)
			{
				return nodeIter = src;
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

			Modifier(const typename Node::Iterator& src) : Visitor(src) {}

			friend class BasicTree<ValueType>;

		public:

			Modifier() {}
			explicit Modifier(const Visitor& src) : Visitor(src.nodeIter) {}
			Modifier(const Modifier& src) : Visitor(src.nodeIter) {}

			Modifier& operator=(const Modifier& src)
			{
				nodeIter = src.nodeIter;
				return *this;
			}
			Modifier& operator=(const Visitor& src)
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

			template <typename ...T>
			void childrenPushFront(T&& ...inValue)
			{
				nodeIter->children.emplace_front(nodeIter, std::forward<T>(inValue)...);
			}

			void childrenPopFront()
			{
				nodeIter->children.pop_front();
			}

			template <typename ...T>
			void childrenPushBack(T&& ...inValue)
			{
				nodeIter->children.emplace_back(nodeIter, std::forward<T>(inValue)...);
			}

			void childrenPopBack()
			{
				nodeIter->children.pop_back();
			}

			template <typename ...T>
			void childrenResize(typename Node::Size size, T&& ...inValue)
			{
				nodeIter->children.resize(size, Node{ nodeIter, std::forward<T>(inValue)... });
			}

			template <typename ...T>
			Modifier childrenInsert(const Modifier& pos, T&& ...inValue)
			{
				return nodeIter->children.emplace(pos.nodeIter, nodeIter, std::forward<T>(inValue)...);
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

			template <typename ...T>
			void thisPushFront(T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenPushFront(std::forward<T>(inValue)...);
			}

			void thisPopFront()
			{
				Modifier(nodeIter->parent).childrenPopFront();
			}

			template <typename ...T>
			void thisPushBack(T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenPushBack(std::forward<T>(inValue)...);
			}

			void thisPopBack()
			{
				Modifier(nodeIter->parent).childrenPopBack();
			}

			template <typename ...T>
			void thisResize(typename Node::Size size, T&& ...inValue)
			{
				Modifier(nodeIter->parent).childrenResize(size, std::forward<T>(inValue)...);
			}

			template <typename ...T>
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

		template <typename ...T>
		BasicTree(T&& ...inValue) : root{ { std::forward<T>(inValue)... } }
		{
			root.front().children.emplace_back(root.begin(), std::forward<T>(inValue)...);
		}

		BasicTree(BasicTree& src) : root(src.root) {}
		BasicTree(const BasicTree& src) : root(src.root) {}
		BasicTree(BasicTree&& src) : root(std::move(src.root)) {}

		BasicTree& operator=(BasicTree& src)
		{
			root = src.root;
			return *this;
		}

		BasicTree& operator=(BasicTree&& src)
		{
			root = std::move(src.root);
			return *this;
		}

		Visitor visitor()
		{
			return root.front().children.begin();
		}

		Modifier modifier()
		{
			return root.front().children.begin();
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

		template <typename ...T>
		BasicParseUnit(Type inType, const String& inName, T&& ... inValue) : type(inType), name(inName), value(inValue...) {}

		BasicParseUnit(const BasicParseUnit& src) : type(src.type), name(src.name), value(src.value) {}

		BasicParseUnit& operator=(const BasicParseUnit& src)
		{
			type = src.type;
			name = src.name;
			value = src.value;
			return *this;
		}

		~BasicParseUnit() {}
	};

	using ParseUnit = BasicParseUnit<std::string_view>;

	template <typename StringView>
	using BasicParseTree = BasicTree<BasicParseUnit<StringView>>;

	using ParseTree = BasicParseTree<std::string_view>;

}
