/*
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __TYPES_H_
#define __TYPES_H_

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

/*
  define some specific length types
*/
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;


// Helper functions for templating

template <typename, typename>
constexpr bool is_same_v = false;
template <typename T>
constexpr bool is_same_v<T, T> = true;

template<typename T, typename... Ts>
constexpr bool is_any_of_v = (is_same_v<T, Ts> || ...);

template <typename T>
struct remove_const_volatile {
    using type = T;
};

template <typename T>
struct remove_const_volatile<volatile T> {
    using type = T;
};

template <typename T>
struct remove_const_volatile<const T> {
    using type = T;
};

template <typename T>
struct remove_const_volatile<const volatile T>{
using type = T;
};

template<typename T>
using remove_cv_t = typename remove_const_volatile<T>::type;

template<typename Ty>
constexpr bool is_int_like_v = is_any_of_v<
    remove_cv_t<Ty>,
    char,
    signed char,
    unsigned char,
    wchar_t,
    char16_t,
    char32_t,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long,
    long long,
    unsigned long long
>;

template<typename Ty>
constexpr bool is_sint_like_v = is_any_of_v<
    remove_cv_t<Ty>,
    signed char,
    signed short,
    signed int,
    signed long,
    signed long long
>;

template<typename Ty>
constexpr bool is_uint_like_v = is_any_of_v<
    remove_cv_t<Ty>,
    unsigned char,
    unsigned short,
    unsigned int,
    unsigned long,
    unsigned long long
>;

template <class T>
concept int_like = is_int_like_v<T>; // either signed or unsigned. no bool?

template <class T>
concept uint_like = is_uint_like_v<T>;

template <class T>
concept sint_like = is_sint_like_v<T>;




#endif
