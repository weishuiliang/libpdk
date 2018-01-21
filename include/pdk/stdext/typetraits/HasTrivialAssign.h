// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2018/01/21.

//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef PDK_STDEXT_HAS_TRIVIAL_ASSIGN_H
#define PDK_STDEXT_HAS_TRIVIAL_ASSIGN_H

#include <cstddef> // size_t
#include <type_traits>

namespace pdk {
namespace stdext {

template <typename T>
struct HasTrivialAssign : std::integral_constant<bool,
      std::is_pod<T>::value && !std::is_const<T>::value && !std::is_volatile<T>::value
      >
{};

template <>
struct HasTrivialAssign<void> : std::false_type
{};

template <>
struct HasTrivialAssign<void const> : public std::false_type
{};

template <>
struct HasTrivialAssign<void const volatile> : public std::false_type
{};

template <>
struct HasTrivialAssign<void volatile> : public std::false_type
{};

template<typename T>
struct HasTrivialAssign<T volatile> : public std::false_type
{};

template<typename T>
struct HasTrivialAssign<T &> : public std::false_type
{};

template<typename T>
struct HasTrivialAssign<T &&> : public std::false_type
{};

// Arrays are not explictly assignable:
template<typename T, std::size_t N>
struct HasTrivialAssign<T[N]> : public std::false_type
{};

template<typename T>
struct HasTrivialAssign<T[]> : public std::false_type
{};

} // stdext
} // pdk

#endif // PDK_STDEXT_HAS_TRIVIAL_ASSIGN_H
