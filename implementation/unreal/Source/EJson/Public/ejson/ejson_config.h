#pragma once

#include "CoreMinimal.h"

// todo: remove all std stuff
#include <sstream>
#include <string_view>

#define EJSON_TEXT(str) TEXT(str)
#define EJSON_ASSERT(cond, msg) checkf(cond, TEXT(msg))
#define EJSON_ERROR(msg)        checkf(false, TEXT(msg))
#define EJSON_MOVE MoveTemp
#define EJSON_FORWARD Forward

namespace ejson
{
    using number = double;

    using string_char = TCHAR;
    using string_view = TStringView<TCHAR>;
    using string = FString;

    // string
    template <typename T>
    size_t StringSize(const T& str)
    {
        return (size_t) str.Len();
    }
    
    inline void StringClear(string& str)
    {
        str.Reset();
    }

    inline void StringAdd(string& str, string_char car)
    {
        str = str + car; // todo: better call ?
    }

    inline void StringAdd(string& str, const string& other)
    {
        str = str + other; // todo: better call ?
    }

    inline void WriteNumber(number value, string& output)
    {
        output = FString::Printf(TEXT("%f"), value);
    }

    // stream

    // todo: use ue stream
    using input_stream = std::basic_istream<wchar_t>;
    using output_stream = std::basic_ostream<wchar_t>;

    inline bool StreamWrite(output_stream& stream, string_char car)
    {
        stream << car;
        return true;
    }

    inline size_t StreamWrite(output_stream& stream, const string_view& str)
    {
        stream << std::wstring_view(str.begin(), str.Len()); // todo!
        return StringSize(str);
    }

    // vector
    template<typename VALUE>
    using vector = TArray<VALUE>;

    template<typename VALUE>
    size_t VectorSize(const vector<VALUE>& vector)
    {
        return vector.Num();
    }

    template<typename VALUE>
    void VectorRemoveLast(vector<VALUE>& vector)
    {
        vector.RemoveAtSwap(vector.Num()-1);
    }
    template<typename VALUE>
    void VectorEmplace(vector<VALUE>& vector, VALUE&& value)
    {
        vector.Emplace(Forward<VALUE>(value));
    }

    // map
    template<typename KEY, typename VALUE>
    using map = TMap<KEY, VALUE>;

    template<typename KEY, typename VALUE>
    VALUE* MapFind(map<KEY, VALUE>& m, const KEY& key)
    {
        return m.Find(key);
    }

    template<typename KEY, typename VALUE>
    VALUE* MapTryEmplace(map<KEY, VALUE>& m, const KEY& key, VALUE&& value)
    {
        return &m.Emplace(key, Forward<VALUE>(value));
    }

    using input_stream = std::basic_istream<wchar_t>; // todo
    using output_stream = std::basic_ostream<wchar_t>;  // todo
}