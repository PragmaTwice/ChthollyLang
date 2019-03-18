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