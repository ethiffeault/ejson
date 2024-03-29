
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <functional>

#include "doctest.h"

#include <ejson/ejson.h>

namespace playground
{
    using namespace ejson;
    TEST_CASE("playground")
    {
    }
}

namespace test_value
{
    using namespace ejson;

    TEST_CASE("test_value")
    {
        Value value;

        // Invalid
        REQUIRE(value.GetType() == Value::Type::Invalid);
        REQUIRE(value.IsInvalid());

        // Null
        value.SetNull();
        REQUIRE(value.GetType() == Value::Type::Null);
        REQUIRE(value.IsNull());

        // Bool
        value.SetBool(true);
        REQUIRE(value.GetType() == Value::Type::Bool);
        REQUIRE(value.IsBool());
        REQUIRE(value.AsBool() == true);
        value.AsBool() = false;
        REQUIRE(value.AsBool() == false);

        // Number
        value.SetNumber(2);
        REQUIRE(value.GetType() == Value::Type::Number);
        REQUIRE(value.IsNumber());
        REQUIRE(value.AsNumber() == 2);
        value.AsNumber() = 3;
        REQUIRE(value.AsNumber() == 3);

        // String
        value.SetString(EJSON_TEXT("hello"));
        REQUIRE(value.GetType() == Value::Type::String);
        REQUIRE(value.IsString());
        REQUIRE(value.AsString() == EJSON_TEXT("hello"));
        value.AsString() = EJSON_TEXT("world");
        REQUIRE(value.AsString() == EJSON_TEXT("world"));

        // Array
        value.SetArray({});
        REQUIRE(value.GetType() == Value::Type::Array);
        REQUIRE(value.IsArray());
        REQUIRE(value.AsArray().size() == 0);

        Value value0;
        value0.SetBool(true);
        Value value1;
        value1.SetString(EJSON_TEXT("hello"));

        Value::ArrayType& array = value.AsArray();
        array.push_back(value0);
        array.emplace_back(std::move(value1));
        array.emplace_back();
        REQUIRE(array.size() == 3);

        // Object
        value.SetObject({});
        REQUIRE(value.GetType() == Value::Type::Object);
        REQUIRE(value.IsObject());
        REQUIRE(value.AsObject().size() == 0);

        value.AsObject().emplace(EJSON_TEXT("1212"), Value());
        REQUIRE(value.AsObject().size() == 1);
        value[EJSON_TEXT("1212")].SetBool(true);
        REQUIRE(value[EJSON_TEXT("1212")].AsBool() == true);
    }
}

namespace test_parse_number
{
    using namespace ejson;

    TEST_CASE("test_parse_number")
    {
        number result;

        // Integer
        CHECK(ParseNumber(EJSON_TEXT("123") , result));
        CHECK(result == doctest::Approx(123));

        // Floating-point
        CHECK(ParseNumber(EJSON_TEXT("123.456") , result));
        CHECK(result == doctest::Approx(123.456));

        // Negative number
        CHECK(ParseNumber(EJSON_TEXT("-123") , result));
        CHECK(result == doctest::Approx(-123));

        // Floating-point with negative sign
        CHECK(ParseNumber(EJSON_TEXT("-123.456") , result));
        CHECK(result == doctest::Approx(-123.456));

        // Number with exponent
        CHECK(ParseNumber(EJSON_TEXT("1e3") , result));
        CHECK(result == doctest::Approx(1000));

        // Number with negative exponent
        CHECK(ParseNumber(EJSON_TEXT("1e-3") , result));
        CHECK(result == doctest::Approx(0.001));

        // Number with exponent and floating-point
        CHECK(ParseNumber(EJSON_TEXT("1.23e2") , result));
        CHECK(result == doctest::Approx(123));

        // Invalid number (letter in the middle)
        CHECK_FALSE(ParseNumber(EJSON_TEXT("123a456") , result));

        // Invalid number (multiple decimal points)
        CHECK_FALSE(ParseNumber(EJSON_TEXT("123.45.6") , result));

        // Invalid format (empty string)
        CHECK_FALSE(ParseNumber(EJSON_TEXT("") , result));

        // Zero
        CHECK(ParseNumber(EJSON_TEXT("0") , result));
        CHECK(result == doctest::Approx(0));

        // Negative zero
        CHECK(ParseNumber(EJSON_TEXT("-0") , result));
        CHECK(result == doctest::Approx(0));

        // Only decimal point
        CHECK_FALSE(ParseNumber(EJSON_TEXT(".") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("..") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("w") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("0.w") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("-w") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("-0.w") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("-.0") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT(".0") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("1e") , result));

        CHECK_FALSE(ParseNumber(EJSON_TEXT("1e-") , result));
    }
}

namespace test_parser
{
    using namespace ejson;

    TEST_CASE("test_parser")
    {
        // Null
        {
            StringReader stringReader(EJSON_TEXT("null"));
            Value jsonValue;
            ValueReader valueReader(jsonValue);
            JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
            bool result = jsonReader.Parse();
            CHECK(result == true);
            REQUIRE(jsonValue.IsNull());
        }

        // Bool
        {
            {
                StringReader stringReader(EJSON_TEXT("true"));
                Value jsonValue;
                ValueReader valueReader(jsonValue);
                JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
                bool result = jsonReader.Parse();
                CHECK(result == true);
                REQUIRE(jsonValue.IsBool());
                REQUIRE(jsonValue.AsBool() == true);
            }

            {
                StringReader stringReader(EJSON_TEXT("false"));
                Value jsonValue;
                ValueReader valueReader(jsonValue);
                JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
                bool result = jsonReader.Parse();
                CHECK(result == true);
                REQUIRE(jsonValue.IsBool());
                REQUIRE(jsonValue.AsBool() == false);
            }
        }
        
        // Number
        {
            StringReader stringReader(EJSON_TEXT("1"));
            Value jsonValue;
            ValueReader valueReader(jsonValue);
            JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
            bool result = jsonReader.Parse();
            CHECK(result == true);
            REQUIRE(jsonValue.IsNumber());
            REQUIRE(jsonValue.AsNumber() == doctest::Approx(1));
        }

        // String
        {
            StringReader stringReader(EJSON_TEXT("\"hello\""));
            Value jsonValue;
            ValueReader valueReader(jsonValue);
            JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
            bool result = jsonReader.Parse();
            CHECK(result == true);
            REQUIRE(jsonValue.IsString());
            REQUIRE(jsonValue.AsString() == EJSON_TEXT("hello"));
        }

        // Array
        {
            StringReader stringReader(EJSON_TEXT("[true, null, 123, \"hello\"]"));
            Value jsonValue;
            ValueReader valueReader(jsonValue);
            JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
            bool result = jsonReader.Parse();
            CHECK(result == true);
            REQUIRE(jsonValue.IsArray());
            REQUIRE(jsonValue.AsArray().size() == 4);
            REQUIRE(jsonValue.AsArray()[0].IsBool() == true);
            REQUIRE(jsonValue.AsArray()[0].AsBool() == true);
            REQUIRE(jsonValue.AsArray()[1].IsNull() == true);
            REQUIRE(jsonValue.AsArray()[2].IsNumber() == true);
            REQUIRE(jsonValue.AsArray()[2].AsNumber() == doctest::Approx(123));
            REQUIRE(jsonValue.AsArray()[3].IsString() == true);
            REQUIRE(jsonValue.AsArray()[3].AsString() == EJSON_TEXT("hello"));
        }

        // Object
        {
            StringReader stringReader(EJSON_TEXT("{ \"p0\" : true, \"p1\" : \"hello\"}"));
            Value jsonValue;
            ValueReader valueReader(jsonValue);
            JsonReader<ValueReader, StringReader> jsonReader(valueReader, stringReader);
            bool result = jsonReader.Parse();
            CHECK(result == true);
            
            REQUIRE(jsonValue.IsObject());
            Value::ObjectType& object = jsonValue.AsObject();
            REQUIRE(object.size() == 2);

            REQUIRE(object[EJSON_TEXT("p0")].IsBool());
            REQUIRE(object[EJSON_TEXT("p0")].AsBool() == true);

            REQUIRE(object[EJSON_TEXT("p1")].IsString());
            REQUIRE(object[EJSON_TEXT("p1")].AsString() == EJSON_TEXT("hello"));

        }

    }
}

