#pragma once

#include <map>
#include <functional>

namespace Chtholly 
{

	template <typename State, typename Iterator>
	struct FiniteAutomaton
	{
		using StateRef = const State&;

		using TransitionFunctionPerState = std::function<State(Iterator)>;

		using TransitionMap = std::map<State, TransitionFunctionPerState>;

		using EndPredicate = std::function<bool(StateRef)>;

		State state;
		EndPredicate endPredicate;
		TransitionMap map;

		FiniteAutomaton(StateRef beginState, const EndPredicate& inEndPredicate, const TransitionMap& inMap)
			: state(beginState), endPredicate(inEndPredicate), map(inMap)
		{
		}

		FiniteAutomaton(StateRef beginState, StateRef endState, const TransitionMap& inMap)
			: state(beginState), endPredicate([=](StateRef s) { return s == endState; }), map(inMap)
		{
		}

		FiniteAutomaton(StateRef beginState, const TransitionMap& inMap)
			: state(beginState), endPredicate([](StateRef) { return false; }), map(inMap)
		{
		}

		FiniteAutomaton(const FiniteAutomaton&) = default;

		FiniteAutomaton& operator=(const FiniteAutomaton&) = default;

		State operator()(Iterator begin, Iterator end)
		{
			Iterator iterator = begin;
			for (; !endPredicate(state) && iterator != end; ++iterator)
			{
				state = map.at(state)(iterator);
			}

			return state;
		}

		~FiniteAutomaton() = default;
	};

}