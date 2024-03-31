
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <ejson/ejson.h>
#include <eti/eti.h>

using namespace eti;
using namespace ejson;

namespace unittest
{
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

    TEST_CASE("test_01")
    {
        Point p;
        string json;
        WriteType(p, json);
        REQUIRE(json == EJSON_TEXT("{\"X\":0,\"Y\":0}"));
    }
}

