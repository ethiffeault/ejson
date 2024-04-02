
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <ejson/ejson.h>
#include <eti/eti.h>

using namespace eti;
using namespace ejson;

namespace unittest
{
    ETI_ENUM
    (
        std::uint8_t, Day,
            Monday,
            Tuesday,
            Wednesday,
            Thursday,
            Friday,
            Saturday,
            Sunday
    )

    struct Point
    {
        ETI_STRUCT_EXT
        (
            Point,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(X),
                ETI_PROPERTY(Y)
            ),
            ETI_METHODS()
        )

        float X = 0.0f;
        float Y = 0.0f;
    };

    class Foo : public Object
    {
        ETI_CLASS_EXT
        (
            Foo, Object,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(CurrentDay),
                ETI_PROPERTY(Data),
                ETI_PROPERTY(Point)
            ),
            ETI_METHODS()
        )
    public:
        Day CurrentDay = Day::Friday;
        std::vector<u32> Data = { 1,2 };
        Point Point;
    };

    TEST_CASE("test_01")
    {
        Point p;
        string json;
        WriteType(p, json);
        REQUIRE(json == EJSON_TEXT("{\"X\":0,\"Y\":0}"));

        Foo foo;
        WriteType(foo, json);
        REQUIRE(json == EJSON_TEXT("{\"CurrentDay\":\"Friday\",\"Data\":[1,2],\"Point\":{\"X\":0,\"Y\":0}}"));

        Foo foo2;
        foo2.Data.clear();
        foo2.CurrentDay = Day::Monday;
        foo2.Point.X = 1;
        foo2.Point.Y = 1;

        ParserError error;
        ReadType(json, foo2, error);
        REQUIRE(foo2.CurrentDay == Day::Friday);
        REQUIRE(foo2.Data.size() == 2);
        REQUIRE(foo2.Point.X == 0);
        REQUIRE(foo2.Point.Y == 0);
    }
}
ETI_ENUM_IMPL(unittest::Day);
