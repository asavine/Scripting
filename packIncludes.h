#pragma once

//  Compile time check if a type is part of a pack
//  Use as follows:

//  Pack<T1, T2, T3, ...>::includes<T>() = true if T is part of T1, T2, T3, ... false otherwise
//  includes() is resolved at compile time


template <typename... Vs>
struct Pack;

template <typename V>
struct Pack <typename V>
{
    template <class T>
    static constexpr bool includes()
    {
        return false;
    }

    template <>
    static constexpr bool includes<V>()
    {
        return true;
    }
};

template <typename V, typename... Vs>
struct Pack <V, Vs...>
{
    template <class T>
    static constexpr bool includes()
    {
        return Pack<V>::includes<T>() || Pack<Vs...>::includes<T>();
    }
};