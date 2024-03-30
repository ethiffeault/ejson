//  MIT License
//  
//  Copyright (c) 2024 Eric Thiffeault
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//  
//  https://github.com/ethiffeault/ejson

#pragma once

//
// Configurations

#define EJSON_CONFIG_FILE 0
#define EJSON_IMPL_FILE 0

#if EJSON_CONFIG_FILE
#include "ejson_config.h"
#else

// std default implementation

#include <fstream>
#include <map>
#include <string_view>
#include <vector>
#include <sstream>
#include <unordered_map>

#define EJSON_MOVE std::move
#define EJSON_FORWARD std::forward

// char or wchar_t
#define EJSON_WCHAR 0

#if EJSON_WCHAR
    #define EJSON_TEXT(str) L##str
#else
    #define EJSON_TEXT(str) str
#endif

// error handling
#ifndef EJSON_ASSERT
    // Assert and Error
    #include <cassert>
    #include <iostream>
    #define EJSON_ASSERT(cond, msg) \
                        do { \
                            if (!(cond)) { \
                                ::std::cerr << msg << ::std::endl; \
                                assert(cond); \
                            } \
                        } while (0)
    #define EJSON_ERROR(msg) EJSON_ASSERT(true, msg)
#endif

// keep insertion/parsing order ? For serialization loading only when order don't matter, set this to 0 for speed
#define EJSON_MAP_ORDERED 1

namespace ejson
{
    // double or float
    using number = double;

    // pod type
    using s8 = std::int8_t;
    using s16 = std::int16_t;
    using s32 = std::int32_t;
    using s64 = std::int64_t;

    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using f32 = std::float_t;
    using f64 = std::double_t;

#if EJSON_WCHAR
    using string_char = wchar_t;
    using string_view = std::wstring_view;
    using string = std::wstring;
    using input_stream = std::basic_istream<wchar_t>;
    using output_stream = std::basic_ostream<wchar_t>;
#else
    using string_char = char;
    using string_view = std::string_view;
    using string = std::string;
    using input_stream = std::basic_istream<char>;
    using output_stream = std::basic_ostream<char>;
#endif

    template <typename T>
    size_t StringSize(const T& str)
    {
        return str.size();
    }

    inline void StringClear(string& str)
    {
        str.clear();
    }

    inline void StringAdd(string& str, string_char car)
    {
        str.push_back(car);
    }

    inline void StringAdd(string& str, const string& other)
    {
        str.append(other);
    }

    inline void WriteNumber(number value, string& output)
    {
        // todo: make our own function with no allocation
#if EJSON_WCHAR
        std::wstringstream wss;
        wss << value;
        output = wss.str();
#else
        std::stringstream wss;
        wss << value;
        output = wss.str();
#endif
    }

    // stream

    inline bool StreamWrite(output_stream& stream, string_char car)
    {
        stream << car;
        return true;
    }

    inline size_t StreamWrite(output_stream& stream, const string_view& str)
    {
        stream << str;
        return StringSize(str);
    }

    // vector

    template<typename VALUE>
    using vector = std::vector<VALUE>;

    template<typename VALUE>
    size_t VectorSize(const vector<VALUE>& vector)
    {
        return vector.size();
    }

    template<typename VALUE>
    void VectorRemoveLast(vector<VALUE>& vector)
    {
        vector.pop_back();
    }
    template<typename VALUE>
    void VectorEmplace(vector<VALUE>& vector, VALUE&& value)
    {
        vector.emplace_back(EJSON_FORWARD<VALUE>(value));
    }

    // map

#if EJSON_MAP_ORDERED

    template<typename KEY, typename VALUE>
    class OrderedMap
    {

    public:

        using KeyValuePair = std::pair<const KEY&, const VALUE&>;

        class Iterator
        {
            using KeyIterator = typename std::vector<KEY>::const_iterator;
            const OrderedMap* dict;
            KeyIterator it;

        public:

            Iterator(const OrderedMap* dict, KeyIterator it) : dict(dict), it(EJSON_MOVE(it)) {}

            Iterator& operator++()
            {
                ++it;
                return *this;
            }

            bool operator!=(const Iterator& other) const
            {
                return it != other.it;
            }

            KeyValuePair operator*() const
            {
                const KEY& key = *it;
                return KeyValuePair(key, dict->map.at(key));
            }
        };

        Iterator begin() const { return Iterator(this, keys.begin()); }
        Iterator end() const { return Iterator(this, keys.end()); }

        size_t size() const { return map.size(); }

        VALUE& operator[](const KEY& key)
        {
            VALUE* existing = Find(key);
            if (existing)
                return *existing;
            return *TryEmplace(key, {});
        }

        const VALUE& operator[](const KEY& key) const
        {
            return map[key];
        }

        auto emplace(const KEY& key, VALUE&& value)
        {
            auto result = map.emplace(key, EJSON_FORWARD<VALUE>(value));
            if (result.second)
                keys.push_back(key);
            return result;
        }

        // try add a new value and return it pointer, else return pointer of existing value
        VALUE* TryEmplace(const KEY& key, const VALUE&& value)
        {
            auto result = map.try_emplace(key, EJSON_MOVE(value));
            if (result.second)
                keys.push_back(result.first->first);
            return &result.first->second;
        }

        // find an entry, return it's value ptr if exist, else nullptr
        VALUE* Find(const KEY& key)
        {
            auto it = map.find(key);
            if (it != map.end())
                return &it->second;
            else
                return nullptr;
        }

        const VALUE* Find(const KEY& key) const
        {
            auto it = map.find(key);
            if (it != map.end())
                return &it->second;
            else
                return nullptr;
        }

    private:

        std::vector<KEY> keys;
        std::unordered_map<KEY, VALUE> map;
    };

    template<typename KEY, typename VALUE>
    using map = OrderedMap<KEY, VALUE>;

    template<typename KEY, typename VALUE>
    VALUE* MapFind(map<KEY, VALUE>& m, const KEY& key)
    {
        return m.Find(key);
    }

    template<typename KEY, typename VALUE>
    const VALUE* MapFind(const map<KEY, VALUE>& m, const KEY& key)
    {
        return m.Find(key);
    }

    template<typename KEY, typename VALUE>
    VALUE* MapTryEmplace(map<KEY, VALUE>& m, const KEY& key, VALUE&& value)
    {
        return m.TryEmplace(key, EJSON_FORWARD<VALUE>(value));
    }

#else // #if EJSON_MAP_ORDERED

    template<typename KEY, typename VALUE>
    using map = std::map<KEY, VALUE>;

    template<typename KEY, typename VALUE>
    VALUE* MapFind(map<KEY, VALUE>& m, const KEY& key)
    {
        auto result = m.find(key);
        if (result != m.end())
            return &result->second;
        else
            return nullptr;
    }

    template<typename KEY, typename VALUE>
    VALUE* MapTryEmplace(map<KEY, VALUE>& m, const KEY& key, VALUE&& value)
    {
        auto [it, success] = m.try_emplace(key, EJSON_FORWARD<VALUE>(value));
        return &(it->second);
    }
#endif // #if EJSON_MAP_ORDERED

}
#endif // #if EJSON_CONFIG_FILE

//
// End configurations 

namespace ejson
{
    struct ParserError
    {
        int Line = 0;
        int Column = 0;
        string File;
        string Error;
    };

    inline bool IsDigit(string_char c)
    {
        return c >= EJSON_TEXT('0') && c <= EJSON_TEXT('9');
    }

    inline bool ParseNumber(string_view str, number& result)
    {
        size_t strSize = StringSize(str);
        number sign = 1.0;
        bool decimalFound = false;
        bool digitFound = false;
        number decimalFactor = 0.1f;
        result = 0.0;

        if (strSize == 0)
            return false;

        size_t i = 0;
        if (str[i] == EJSON_TEXT('-'))
        {
            sign = -1.0;
            ++i;
        }

        for (; i < strSize && str[i] != EJSON_TEXT('e') && str[i] != EJSON_TEXT('E'); ++i)
        {
            if (IsDigit(str[i]))
            {
                if (!decimalFound)
                {
                    digitFound = true;
                    result = result * 10 + (str[i] - EJSON_TEXT('0'));
                }
                else
                {
                    result += (str[i] - EJSON_TEXT('0')) * decimalFactor;
                    decimalFactor /= 10;
                }
            }
            else if (str[i] == EJSON_TEXT('.') && !decimalFound)
            {
                if (!digitFound)
                    return false;
                decimalFound = true;
            }
            else
            {
                return false;
            }
        }

        if (i < strSize && (str[i] == EJSON_TEXT('e') || str[i] == EJSON_TEXT('E')))
        {
            ++i;

            if (i >= strSize)
                return false;

            number exponentSign = 1.0;
            if (str[i] == EJSON_TEXT('+'))
            {
                ++i;
            }
            else if (str[i] == EJSON_TEXT('-'))
            {
                exponentSign = -1.0;
                ++i;
            }

            if (i >= strSize)
                return false;

            number exponent = 0.0;
            for (; i < strSize; ++i)
            {
                if (!IsDigit(str[i]))
                {
                    return false;
                }
                exponent = exponent * 10 + (str[i] - EJSON_TEXT('0'));
            }
            while (exponent > 0)
            {
                result *= exponentSign > 0.0f ? 10.0f : 0.1f;
                --exponent;
            }
        }

        result *= sign;
        return true;
    }

    class Value
    {
    public:

        enum class Type : std::uint8_t
        {
            Invalid,
            Null,
            Bool,
            Number,
            String,
            Array,
            Object
        };

        Value() = default;

        Value(const Value& value)
        {
            Set(value);
        }

        Value(Value&& value)
        {
            Set(EJSON_FORWARD<Value>(value));
        }

        ~Value()
        {
            SetInvalid();
        }

        Value& operator=(const Value& other)
        {
            if (this != &other)
            {
                Set(other);
            }
            return *this;
        }

        Type GetType() const
        {
            return type;
        }

        // Value
        void Set(const Value& other)
        {
            switch (other.type)
            {
                case Type::Invalid:
                    SetInvalid();
                    break;
                case Type::Null:
                    SetNull();
                    break;
                case Type::Bool:
                    SetBool(other.AsBool());
                    break;
                case Type::Number:
                    SetNumber(other.AsNumber());
                    break;
                case Type::String:
                    SetString(other.AsString());
                    break;
                case Type::Array:
                    SetArray(other.AsArray());
                    break;
                case Type::Object:
                    SetObject(other.AsObject());
                    break;
            }
        }

        void Set(Value&& other)
        {
            switch (other.type)
            {
                case Type::Invalid:
                    SetInvalid();
                    break;
                case Type::Null:
                    SetNull();
                    break;
                case Type::Bool:
                    SetBool(other.AsBool());
                    break;
                case Type::Number:
                    SetNumber(other.AsNumber());
                    break;
                case Type::String:
                    SetString(EJSON_FORWARD<string>(other.AsString()));
                    break;
                case Type::Array:
                    SetArray(EJSON_FORWARD<vector<Value>>(other.AsArray()));
                    break;
                case Type::Object:
                    SetObject(EJSON_FORWARD<map<string, Value>>(other.AsObject()));
                    break;
            }
            other.type = Type::Invalid;
        }

        // Invalid

        bool IsInvalid() const
        {
            return type == Type::Invalid;
        }

        bool IsValid() const
        {
            return !IsInvalid();
        }

        void SetInvalid()
        {
            switch (type)
            {
                case Type::Invalid:
                    break;
                case Type::Null:
                    break;
                case Type::Bool:
                    break;
                case Type::Number:
                    break;
                case Type::String:
                    ((string*)buffer)->~string();
                    break;
                case Type::Array:
                    using vectorType = vector<Value>;
                    ((vectorType*)buffer)->~vectorType();
                    break;
                case Type::Object:
                    using mapType = map<string, Value>;
                    ((mapType*)buffer)->~mapType();
                    break;
            }
            type = Type::Invalid;
        }

        // Null

        Value(std::nullptr_t v)
        {
            SetNull();
        }

        Value& operator=(std::nullptr_t v)
        {
            SetNull();
            return *this;
        }

        bool IsNull() const
        {
            return type == Type::Null;
        }

        void SetNull()
        {
            SetInvalid();
            type = Type::Null;
        }

        // Bool

        Value(bool v)
        {
            SetBool(v);
        }

        Value& operator=(const bool& v)
        {
            SetBool(v);
            return *this;
        }

        bool IsBool() const
        {
            return type == Type::Bool;
        }

        void SetBool(bool value)
        {
            SetInvalid();
            type = Type::Bool;
            AsBool() = value;
        }

        bool& AsBool()
        {
            EJSON_ASSERT(type == Type::Bool, "expected type: bool");
            return *(bool*)buffer;
        }

        const bool& AsBool() const
        {
            return const_cast<Value*>(this)->AsBool();
        }

        // Number

        Value(s8 v) { SetNumber(v); }
        Value& operator=(s8 v) { SetNumber(v); return *this; }
        Value(s16 v) { SetNumber(v); }
        Value& operator=(s16 v) { SetNumber(v); return *this; }
        Value(s32 v) { SetNumber(v); }
        Value& operator=(s32 v) { SetNumber(v); return *this; }
        Value(s64 v) { SetNumber((number)v); }
        Value& operator=(s64 v) { SetNumber((number)v); return *this; }

        Value(u8 v) { SetNumber(v); }
        Value& operator=(u8 v) { SetNumber(v); return *this; }
        Value(u16 v) { SetNumber(v); }
        Value& operator=(u16 v) { SetNumber(v); return *this; }
        Value(u32 v) { SetNumber(v); }
        Value& operator=(u32 v) { SetNumber(v); return *this; }
        Value(u64 v) { SetNumber((number)v); }
        Value& operator=(u64 v) { SetNumber((number)v); return *this; }

        Value(f32 v) { SetNumber(v); }
        Value& operator=(f32 v) { SetNumber(v); return *this; }
        Value(f64 v) { SetNumber((number)v); }
        Value& operator=(f64 v) { SetNumber((number)v); return *this; }

        bool IsNumber() const
        {
            return type == Type::Number;
        }

        void SetNumber(number value)
        {
            SetInvalid();
            type = Type::Number;
            static_assert(sizeof(number) <= ValueSize);
            static_assert(alignof(number) <= ValueAlign);
            AsNumber() = value;
        }

        number& AsNumber()
        {
            EJSON_ASSERT(type == Type::Number, "expected type: number");
            return *(number*)buffer;
        }

        const number& AsNumber() const
        {
            return const_cast<Value*>(this)->AsNumber();
        }

        // String

        Value(const string& str)
        {
            SetString(str);
        }

        Value(const string_char* str)
        {
            SetString(str);
        }

        Value& operator=(const string& str)
        {
            SetString(str);
            return *this;
        }

        Value& operator=(const string_char* str)
        {
            SetString(string(str));
            return *this;
        }

        bool IsString() const
        {
            return type == Type::String;
        }

        void SetString(const string& value)
        {
            SetInvalid();
            type = Type::String;
            new (buffer) string(value);
        }

        void SetString(string&& value)
        {
            SetInvalid();
            type = Type::String;
            static_assert(sizeof(string) <= ValueSize);
            static_assert(alignof(string) <= ValueAlign);
            new (buffer) string(EJSON_FORWARD<string>(value));
        }

        string& AsString()
        {
            EJSON_ASSERT(type == Type::String, "expected type: string");
            return *(string*)buffer;
        }

        const string& AsString() const
        {
            return const_cast<Value*>(this)->AsString();
        }

        // Array

        Value(const vector<Value>& value)
        {
            SetArray(value);
        }

        Value& operator=(const vector<Value>& value)
        {
            SetArray(value);
            return *this;
        }

        bool IsArray() const
        {
            return type == Type::Array;
        }

        void SetArray(const vector<Value>& value)
        {
            SetInvalid();
            type = Type::Array;
            new (buffer) vector<Value>(value);
        }

        void SetArray(vector<Value>&& value)
        {
            SetInvalid();
            type = Type::Array;
            static_assert(sizeof(vector<Value>) <= ValueSize);
            static_assert(alignof(vector<Value>) <= ValueAlign);
            new (buffer) vector<Value>(EJSON_FORWARD<vector<Value>>(value));
        }

        vector<Value>& AsArray()
        {
            EJSON_ASSERT(type == Type::Array, "expected type: array");
            return *(vector<Value>*)buffer;
        }

        const vector<Value>& AsArray() const
        {
            return const_cast<Value*>(this)->AsArray();
        }

        Value& operator[](size_t index)
        {
            static Value invalid;
            if (!IsArray())
                SetArray({});
            vector<Value>& array = AsArray();
            while(VectorSize(array) <= index )
                VectorEmplace(array, {});

            return AsArray()[index];
        }

        // Object

        Value(const map<string, Value>& value)
        {
            SetObject(value);
        }

        Value& operator=(const map<string, Value>& value)
        {
            SetObject(value);
            return *this;
        }

        bool IsObject() const
        {
            return type == Type::Object;
        }

        void SetObject(const map<string, Value>& value)
        {
            SetInvalid();
            type = Type::Object;
            static_assert(sizeof(map<string, Value>) <= ValueSize);
            static_assert(alignof(map<string, Value>) <= ValueAlign);
            new (buffer) map<string, Value>(value);
        }

        void SetObject(map<string, Value>&& value)
        {
            SetInvalid();
            type = Type::Object;
            static_assert(sizeof(map<string, Value>) <= ValueSize);
            static_assert(alignof(map<string, Value>) <= ValueAlign);
            new (buffer) map<string, Value>(EJSON_FORWARD<map<string, Value>>(value));
        }

        map<string, Value>& AsObject()
        {
            EJSON_ASSERT(type == Type::Object, "expected type: object");
            return *(map<string, Value>*)buffer;
        }

        const map<string, Value>& AsObject() const
        {
            return const_cast<Value*>(this)->AsObject();
        }

        const Value& operator[](size_t index) const
        {
            static Value invalid;
            if (IsArray() && index < VectorSize(AsArray()))
            {
                return AsArray()[index];
            }
            else
            {
                return invalid;
            }
        }

        Value& operator[](const string& str)
        {
            static Value invalid;
            if (!IsObject())
            {
                SetObject({});
            }
            return *MapTryEmplace(AsObject(), str, {});
        }

        const Value& operator[](string_view str) const
        {
            static Value invalid;
            if (IsObject())
            {
                const Value* value = MapFind(AsObject(), string(str));
                if (value)
                    return *value;
                else
                    return invalid;
            }
            else
            {
                return invalid;
            }
        }


    private:

        static constexpr size_t ValueSize = std::max(std::max(std::max(sizeof(vector<Value>), sizeof(map<string, void*>)), sizeof(string)), sizeof(number));
        static constexpr size_t ValueAlign = std::max(std::max(std::max(alignof(vector<Value>), alignof(map<string, void*>)), alignof(string)), alignof(number));

        alignas(ValueAlign) char buffer[ValueSize];
        Type type = Type::Invalid;

    };

    template<typename LISTENER, typename STRING_READER>
    class JsonReader
    {
    public:

        JsonReader(LISTENER& listener, STRING_READER& reader)
            :listener(&listener), reader(&reader)
        {}

        bool Parse()
        {
            if (!reader->Read(next))
                next = 0;

            if (!ParseNextToken())
                return false;

            if (!ParseValue())
                return false;

            Read();
            SkipSpaces();

            if (HaveError())
                return false;

            if (cur != 0)
                return ReportError(EJSON_TEXT("invalid input after value"));

            return true;
        }

        ParserError GetError() const { return error; }
        bool HaveError() const { return StringSize(error.Error) != 0; }

    private:

        enum class Token : std::uint8_t
        {
            Invalid,
            CurlyOpen,
            CurlyClose,
            SquaredOpen,
            SquaredClose,
            Colon,
            Comma,
            String,
            Number,
            True,
            False,
            Null
        };

        LISTENER* listener = nullptr;
        STRING_READER* reader = nullptr;
        string_char cur = 0;
        string_char next = 0;
        Token token = Token::Invalid;
        string value;
        ParserError error;
        std::uint32_t line = 1;
        std::uint32_t column = 0;

        bool Read()
        {
            cur = next;

            if (cur == L'\n')
            {
                ++line;
                column = 0;
            }
            else
            {
                ++column;
            }

            if (cur == 0)
                return false;
            if (!reader->Read(next))
                next = 0;

            if (cur == L'\r')
            {
                if (next != L'\n')
                    return ReportError(EJSON_TEXT("invalid line ending"));
                ++line;
                column = 0;
                cur = next;
                if (!reader->Read(next))
                    next = 0;
            }

            return true;
        }

        bool SkipSpaces()
        {
            while (cur == L' ' || cur == L'\t' || cur == L'\n')
            {
                if (!Read())
                    return false;
            }
            return true;
        }

        bool ReportError(const string_view& msg)
        {
            error.Line = line;
            error.Column = column;
            error.Error = msg;
            return false;
        }

        bool ParseLiteral(const string_char* literal)
        {
            EJSON_ASSERT(literal[0] == cur, "internal error");
            size_t i = 1;
            while (literal[i] == next && next != 0)
            {
                ++i;
                if (!Read())
                    return ReportError(EJSON_TEXT("expected: literal"));
            }
            if (literal[i] == 0)
                return true;
            else
                return ReportError(EJSON_TEXT("expected: literal"));
        }

        bool ParseNumber()
        {
            StringClear(value);

            if (cur == L'-')
            {
                if (!Read())
                    return ReportError(EJSON_TEXT("invalid number"));
                StringAdd(value, EJSON_TEXT('-'));
            }

            bool valid = false;
            while ((cur >= L'0' && cur <= L'9') || cur == L'.')
            {
                // cannot start with '.'
                if (!valid && cur == L'.')
                    return ReportError(EJSON_TEXT("invalid number"));

                StringAdd(value, cur);
                valid = true;
                if ((next < EJSON_TEXT('0') || next > EJSON_TEXT('9')) && next != L'.')
                    break;
                Read();
            }
            // cannot end with '.'
            if (cur == L'.')
                return ReportError(EJSON_TEXT("invalid number"));

            return valid;
        }

        bool ParseString()
        {
            EJSON_ASSERT(cur == EJSON_TEXT('"'), "internal error");

            StringClear(value);

            while (true)
            {
                if (!Read())
                    return ReportError(EJSON_TEXT("invalid string"));

                if (cur == L'"')
                    return true;

                StringAdd(value, cur);

                // skip '\'
                if (cur == L'\\')
                {
                    if (next != L'\"' &&
                        next != L'\\' &&
                        next != L'/' &&
                        next != L'b' &&
                        next != L'f' &&
                        next != L'n' &&
                        next != L'r' &&
                        next != L't' &&
                        next != L'u')
                        return ReportError(EJSON_TEXT("invalid escape car"));

                    if (next == L'u')
                    {
                        // must be followed by 4 numbers
                        for (int i = 0; i < 4; ++i)
                        {
                            Read();
                            StringAdd(value, cur);
                            if (next < L'0' || next > '9')
                                return ReportError(EJSON_TEXT("escape \\u in string must be followed by 4 numbers"));
                        }
                    }

                    Read();
                    StringAdd(value, cur);
                }
            }
        }

        bool ParseNextToken()
        {
            if (!Read())
                return ReportError(EJSON_TEXT("invalid token"));
            if (!SkipSpaces())
                return ReportError(EJSON_TEXT("invalid token"));

            token = Token::Invalid;
            switch (cur)
            {
                case EJSON_TEXT('{'):
                {
                    token = Token::CurlyOpen;
                    return true;
                }
                case EJSON_TEXT('}'):
                {
                    token = Token::CurlyClose;
                    return true;
                }
                case EJSON_TEXT('['):
                {
                    token = Token::SquaredOpen;
                    return true;
                }
                case EJSON_TEXT(']'):
                {
                    token = Token::SquaredClose;
                    return true;
                }
                case EJSON_TEXT(','):
                {
                    token = Token::Comma;
                    return true;
                }
                case EJSON_TEXT(':'):
                {
                    token = Token::Colon;
                    return true;
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '-':
                {
                    if (!ParseNumber())
                        return false;
                    token = Token::Number;
                    return true;
                }
                case EJSON_TEXT('"'):
                {
                    if (!ParseString())
                        return false;
                    token = Token::String;
                    return true;
                }
                case EJSON_TEXT('t'):
                {
                    if (!ParseLiteral(EJSON_TEXT("true")))
                        return false;
                    token = Token::True;
                    return true;
                }
                case EJSON_TEXT('f'):
                {
                    if (!ParseLiteral(EJSON_TEXT("false")))
                        return false;
                    token = Token::False;
                    return true;
                }
                case EJSON_TEXT('n'):
                {
                    if (!ParseLiteral(EJSON_TEXT("null")))
                        return false;
                    token = Token::Null;
                    return true;
                }
                default:
                    return ReportError(EJSON_TEXT("invalid token"));
            }
        }

        bool ParseValue()
        {
            switch (token)
            {
                case Token::CurlyOpen:
                    return ParseObject();
                case Token::SquaredOpen:
                    return ParseArray();
                case Token::Number:
                    listener->ValueNumber(value);
                    return true;
                case Token::String:
                    listener->ValueString(value);
                    return true;
                case Token::Null:
                    listener->ValueNull();
                    return true;
                case Token::True:
                    listener->ValueBool(true);
                    return true;
                case Token::False:
                    listener->ValueBool(false);
                    return true;
                default:
                    return ReportError(EJSON_TEXT("unexpected value"));
            }
        }

        bool ParseObject()
        {
            EJSON_ASSERT(token == Token::CurlyOpen, "internal error");

            listener->ObjectBegin();

            while (true)
            {
                if (!ParseNextToken())
                    return false;

                switch (token)
                {
                    case Token::String:
                        if (!ParseProperty())
                            return false;
                        break;
                    case Token::Comma:
                        break;
                    case Token::CurlyClose:
                        listener->ObjectEnd();
                        return true;
                    default:
                        return ReportError(EJSON_TEXT("unexpected object property"));
                }
            }
        }

        bool ParseProperty()
        {
            EJSON_ASSERT(token == Token::String, "internal error");

            listener->PropertyBegin(value);

            if (!ParseNextToken())
                return false;

            if (token != Token::Colon)
                return ReportError(EJSON_TEXT("unexpected object property, missing ':'"));

            if (!ParseNextToken())
                return false;
            if (!ParseValue())
                return false;

            listener->PropertyEnd();
            return true;
        }

        bool ParseArray()
        {
            EJSON_ASSERT(token == Token::SquaredOpen, "internal error");

            listener->ArrayBegin();

            if (!ParseNextToken())
                return false;

            while (true)
            {
                if (token == Token::SquaredClose)
                {
                    listener->ArrayEnd();
                    return true;
                }

                if (!ParseValue())
                    return false;

                if (!ParseNextToken())
                    return false;

                if (token == Token::Comma)
                {
                    if (!ParseNextToken())
                        return false;
                }
            }

        }
    };

    template<typename STRING_WRITER, bool PRETTIFY = false>
    class JsonWriter
    {
    public:

        JsonWriter(STRING_WRITER& writer)
            : writer(&writer)
        {
            PushState(StateType::Root);
        }

        void WriteNull()
        {
            WriteValueBegin();
            writer->Write(EJSON_TEXT("null"));
            WriteValueEnd();
        }

        void WriteBool(bool value)
        {
            WriteValueBegin();
            writer->Write(value ? EJSON_TEXT("true") : EJSON_TEXT("false"));
            WriteValueEnd();
        }

        void WriteNumber(number value)
        {
            WriteValueBegin();
            StringClear(tmpString); // not needed ?
            ::ejson::WriteNumber(value, tmpString);
            writer->Write(tmpString);
            WriteValueEnd();
        }

        void WriteString(const string_view& value)
        {
            WriteValueBegin();
            writer->Write(EJSON_TEXT("\""));
            writer->Write(value);
            writer->Write(EJSON_TEXT("\""));
            WriteValueEnd();
        }

        void WriteObjectBegin()
        {
            WriteValueBegin();
            WriteContainerBegin();
            PushState(StateType::Object);
            writer->Write(EJSON_TEXT("{"));
        }

        void WriteObjectEnd()
        {
            EJSON_ASSERT(GetState().Type == StateType::Object, "internal error");
            WriteContainerEnd();
            writer->Write(EJSON_TEXT("}"));
            WriteValueEnd();
        }

        void WriteProperty(const string_view& name)
        {
            State& root = GetState();
            EJSON_ASSERT(root.Type == StateType::Object, "internal error");
            WriteValuePrefix();
            PushState(StateType::Property);
            writer->Write(EJSON_TEXT("\""));
            writer->Write(name);
            writer->Write(EJSON_TEXT("\""));
            writer->Write(EJSON_TEXT(":"));
            if constexpr (PRETTIFY)
                writer->Write(EJSON_TEXT(" "));
        }

        void WriteArrayBegin()
        {
            WriteValueBegin();
            WriteContainerBegin();
            PushState(StateType::Array);
            writer->Write(EJSON_TEXT("["));
        }

        void WriteArrayEnd()
        {
            EJSON_ASSERT(GetState().Type == StateType::Array, "internal error");
            WriteContainerEnd();
            writer->Write(EJSON_TEXT("]"));
            WriteValueEnd();
        }

    private:

        enum class StateType : std::uint8_t
        {
            Root,
            Value,
            Object,
            Property,
            Array,
        };

        struct State
        {
            std::int16_t Indentation = 0;
            std::int16_t Count = 0;
            StateType Type = StateType::Root;
        };

        void WriteIndentation()
        {
            static_assert(PRETTIFY);
            for (int i = 0; i < indentation; ++i)
                writer->Write(tab);
        }

        void WriteValuePrefix()
        {
            if (GetState().Count != 0)
                writer->Write(EJSON_TEXT(","));
            if constexpr (PRETTIFY)
            {
                if (GetState().Type != StateType::Root)
                {
                    writer->Write(EJSON_TEXT("\n"));
                    WriteIndentation();
                }
            }
        }

        void WriteValueBegin()
        {
            State& state = GetState();
            EJSON_ASSERT(state.Type == StateType::Property || state.Type == StateType::Array || state.Type == StateType::Root, "internal error");
            EJSON_ASSERT(state.Type != StateType::Root || state.Count == 0, "Json root may contain only one value");
            State& root = state.Type == StateType::Property ? GetState(1) : GetState();
            if (state.Type != StateType::Property)
            {
                WriteValuePrefix();
            }
            ++root.Count;
            PushState(StateType::Value);
        }

        void WriteValueEnd()
        {
            EJSON_ASSERT(GetState().Type == StateType::Value, "internal error");
            PopState();
            if (GetState().Type == StateType::Property)
                PopState();
        }

        void WriteContainerBegin()
        {
            if constexpr (PRETTIFY)
                ++indentation;
        }

        void WriteContainerEnd()
        {
            if constexpr (PRETTIFY)
            {
                const std::int16_t previousCount = GetState().Count;
                if (previousCount != 0)
                    writer->Write(EJSON_TEXT("\n"));
                PopState();
                --indentation;
                if (previousCount != 0)
                    WriteIndentation();
            }
            else
            {
                PopState();
            }
        }

        void PushState(StateType type)
        {
            VectorEmplace(states, State{ 0,0,type });
        }

        void PopState()
        {
            EJSON_ASSERT(VectorSize(states) > 0, "internal error");
            VectorRemoveLast(states);
        }

        State& GetState(size_t depth = 0)
        {
            EJSON_ASSERT(VectorSize(states) > 0, "internal error");
            return states[VectorSize(states) - (1 + depth)];
        }

        vector<State> states;
        STRING_WRITER* writer = nullptr;
        std::int32_t indentation = 0;
        const string_char* tab = EJSON_TEXT("    ");
        string tmpString;
    };

    class StringReader
    {
    public:

        StringReader(string_view str, bool owner = false)
        {
            if (owner)
            {
                string = str;
                stringView = string;
            }
            else
            {
                stringView = str;
            }
        }

        StringReader(const StringReader&) = delete;
        StringReader& operator=(const StringReader&) = delete;
        ~StringReader() {}

        bool Read(string_char& car)
        {
            if (position < StringSize(stringView))
            {
                car = stringView[position];
                ++position;
                return true;
            }
            else
            {
                return false;
            }
        }

    private:

        string string;
        string_view stringView;
        size_t position = 0;
    };

    class StringWriter
    {

    public:

        StringWriter()
        {
            string = &stringData;
        }

        StringWriter(string& str)
            : string(&str)
        {}

        StringWriter(const StringWriter&) = delete;
        StringWriter& operator=(const StringWriter&) = delete;

        ~StringWriter() {}


        bool Write(string_char car)
        {
            StringAdd(*string, car);
            return true;
        }

        size_t Write(const string_view& str)
        {
            StringAdd(*string, ::ejson::string(str));
            return StringSize(str);
        }

        string ToString() const { return *string; }

    private:

        string stringData;
        string* string = nullptr;
    };

    class StreamReader
    {
    public:

        StreamReader(input_stream& stream)
            : stream(stream)
        {}

        StreamReader(const StreamReader&) = delete;
        StreamReader& operator=(const StreamReader&) = delete;
        ~StreamReader() {}

        bool Read(string_char& car)
        {
            if (stream.get(car))
                return true;
            else
                return false;
        }

    private:

        input_stream& stream;

    };

    class StreamWriter
    {

    public:

        StreamWriter(output_stream& stream)
            : stream(stream)
        {}

        StreamWriter(const StringWriter&) = delete;
        StreamWriter& operator=(const StreamWriter&) = delete;

        ~StreamWriter() {}


        bool Write(string_char car)
        {
            return StreamWrite(stream, car);
        }

        size_t Write(const string_view& str)
        {
            return StreamWrite(stream, str);
        }

    private:

        output_stream& stream;
    };

    struct ValueReader
    {
        ValueReader(Value& json)
            : root(json)
        {}

        void ObjectBegin()
        {
            Value value;
            value.SetObject(map<string, Value>());
            Value* objectValue = SetValue(EJSON_MOVE(value));
            VectorEmplace(contexts, EJSON_MOVE(objectValue));
        }

        void ObjectEnd()
        {
            EJSON_ASSERT(GetContext().IsObject(), "object type expected");
            VectorRemoveLast(contexts);
        }

        void PropertyBegin(const string_view& key)
        {
            propertyName = key;
        }

        void PropertyEnd()
        {
        }

        void ArrayBegin()
        {
            Value value;
            value.SetArray(vector<Value>());
            Value* arrayValue = SetValue(EJSON_MOVE(value));
            VectorEmplace(contexts, EJSON_MOVE(arrayValue));
        }

        void ArrayEnd()
        {
            EJSON_ASSERT(GetContext().IsArray(), "array type expected");
            VectorRemoveLast(contexts);
        }

        void ValueBool(bool b)
        {
            Value value;
            value.SetBool(b);
            SetValue(EJSON_MOVE(value));
        }

        void ValueNull()
        {
            Value value;
            value.SetNull();
            SetValue(EJSON_MOVE(value));
        }

        void ValueString(const string_view& str)
        {
            Value value;
            value.SetString(string(str));
            SetValue(EJSON_MOVE(value));
        }

        void ValueNumber(const string_view& str)
        {
            number number = 0;
            ParseNumber(str, number);
            Value value;
            value.SetNumber(number);
            SetValue(EJSON_MOVE(value));
        }

    private:

        Value& root;
        vector<Value*> contexts;
        string propertyName;

        Value& GetContext()
        {
            EJSON_ASSERT(VectorSize(contexts) > 0, "internal error");
            return *contexts[VectorSize(contexts) - 1];
        }

        Value* SetValue(Value&& value)
        {
            if (VectorSize(contexts) == 0)
            {
                root = value;
                return &root;
            }
            else
            {
                Value& context = GetContext();

                if (context.IsArray())
                {
                    vector<Value>& array = context.AsArray();
                    VectorEmplace(array, EJSON_FORWARD<Value>(value));
                    Value* addedValue = &array[VectorSize(array) - 1];
                    return addedValue;
                }
                else if (context.IsObject())
                {
                    map<string, Value>& object = context.AsObject();
                    Value* addOrExistingValue = MapTryEmplace(object, string(propertyName), EJSON_FORWARD<Value>(value));
                    return addOrExistingValue;
                }
                else
                {
                    EJSON_ERROR("internal error");
                    return nullptr;
                }
            }
        }
    };

    template <typename JSON_WRITER>
    class ValueWriter
    {

    public:

        ValueWriter(JSON_WRITER& jsonWriter)
            :jsonWriter(jsonWriter)
        {}

        void Write(const Value& value)
        {
            switch (value.GetType())
            {
                case Value::Type::Invalid:
                {
                    EJSON_ERROR("cannot write invalid typer");
                    break;
                }
                case Value::Type::Null:
                {
                    jsonWriter.WriteNull();
                    break;
                }
                case Value::Type::Bool:
                {
                    jsonWriter.WriteBool(value.AsBool());
                    break;
                }
                case Value::Type::Number:
                {
                    jsonWriter.WriteNumber(value.AsNumber());
                    break;
                }
                case Value::Type::String:
                {
                    jsonWriter.WriteString(value.AsString());
                    break;
                }
                case Value::Type::Array:
                {
                    jsonWriter.WriteArrayBegin();
                    const vector<Value>& array = value.AsArray();
                    for (const Value& v : array)
                        Write(v);
                    jsonWriter.WriteArrayEnd();
                    break;
                }
                case Value::Type::Object:
                {
                    jsonWriter.WriteObjectBegin();
                    const map<string, Value>& object = value.AsObject();
                    for (const auto& [key, v] : object)
                    {
                        jsonWriter.WriteProperty(key);
                        Write(v);
                    }
                    jsonWriter.WriteObjectEnd();
                    break;
                }
            }
        }

    private:

        JSON_WRITER& jsonWriter;

    };

    // Json

    bool Read(string_view json, Value& value);
    bool Read(string_view json, Value& value, ParserError& error);
    bool Read(input_stream& stream, Value& value);
    bool Read(input_stream& stream, Value& value, ParserError& error);
    void Write(const Value& value, string& str, bool prettify = false);
    void Write(const Value& value, output_stream& stream, bool prettify = false);

    inline bool Read(string_view json, Value& value)
    {
        ParserError error;
        return Read(json, value, error);
    }

    inline bool Read(string_view json, Value& value, ParserError& error)
    {
        StringReader stringReader(json);
        ValueReader valueReader(value);
        JsonReader jsonReader(valueReader, stringReader);
        if (jsonReader.Parse())
        {
            return true;
        }
        else
        {
            error = jsonReader.GetError();
            return false;
        }
    }

    inline bool Read(input_stream& stream, Value& value)
    {
        ParserError error;
        return Read(stream, value, error);
    }

    inline bool Read(input_stream& stream, Value& value, ParserError& error)
    {
        StreamReader streamReader(stream);
        ValueReader valueReader(value);
        JsonReader jsonReader(valueReader, streamReader);
        if (jsonReader.Parse())
        {
            return true;
        }
        else
        {
            error = jsonReader.GetError();
            return false;
        }
    }

    inline void Write(const Value& value, string& str, bool prettify /*= false*/)
    {
        StringClear(str);
        StringWriter stringWriter(str);
        if (!prettify)
        {
            JsonWriter jsonWriter(stringWriter);
            ValueWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
        else
        {
            JsonWriter<StringWriter, true> jsonWriter(stringWriter);
            ValueWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
    }

    inline void Write(const Value& value, output_stream& stream, bool prettify /*= false*/)
    {
        StreamWriter streamWriter(stream);
        if (!prettify)
        {
            JsonWriter jsonWriter(streamWriter);
            ValueWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
        else
        {
            JsonWriter<StreamWriter, true> jsonWriter(streamWriter);
            ValueWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
    }
}

#if EJSON_IMPL_FILE
    #include "ejson_impl.h"
#endif