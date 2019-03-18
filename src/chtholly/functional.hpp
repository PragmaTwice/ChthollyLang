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

#include <type_traits>

namespace Chtholly 
{

	template <typename T>
	constexpr auto identity(T&& v) noexcept
	{
		return v;
	}

	template <typename T>
	constexpr auto constant(T&& v) noexcept
	{
		return [=](auto&&...) { return v; };
	}

	namespace curry_details
	{
		template <typename, typename = std::void_t<>>
		struct needs_unapply : std::true_type {};

		template <typename T>
		struct needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type {};

		template <typename T>
		constexpr auto needs_unapply_v = needs_unapply<T>::value;
	}

	template <typename F>
	constexpr auto curry(F&& f)
	{
		if constexpr (curry_details::needs_unapply_v<decltype(f)>)
		{
			return [=](auto&& x) {
				return curry(
					[=](auto&&...xs) -> decltype(f(x, xs...)) {
					return f(x, xs...);
				}
				);
			};
		}
		else return f();
	}

	template <typename F>
	constexpr auto uncurry(F&& f)
	{
		return [=](auto&& x, auto&& ...xs) {
			if constexpr (sizeof...(xs) > 0) {
				return uncurry(f(x))(xs...);
			}
			else return f(x);
		};
	}

	template <typename T, typename ...U>
	constexpr auto compose(T&& f1, U&& ...fn)
	{
		if constexpr (sizeof...(U) > 0)
		{
			return [=](auto&& v) {
				return f1(compose(fn...)(v));
			};
		}
		else return f1;
	}

	template <typename T, typename ...U>
	constexpr auto pipe(T&& f1, U&& ...fn)
	{
		if constexpr (sizeof...(U) > 0)
		{
			return [=](auto&& v) {
				return pipe(fn...)(f1(v));
			};
		}
		else return f1;
	}

	template <typename T>
	inline constexpr auto constructor = [](auto&& ...v) {
		return T{ v... };
	};

	template <typename ...T>
	constexpr auto sequence(T&& ...fs)
	{
		return [=](auto&& ...v) {
			return (fs(v...), ...);
		};
	}

	namespace foldl_details
	{
		template <typename F, typename T, typename... U>
		constexpr auto foldl_impl(F&& f, T&& v1, U&& ...vn)
		{
			if constexpr (sizeof...(vn) > 0)
			{
				return f(v1, foldl_impl(f, vn...));
			}
			else return v1;
		}
	}

	template <typename F>
	constexpr auto foldl(F&& f)
	{
		return [=](auto&& ...xs) {
			return foldl_details::foldl_impl(f, xs...);
		};
	}

	template <typename F, typename... V>
	constexpr auto invoke(F&& f, V&& ...v)
	{
		return f(v...);
	}

}
