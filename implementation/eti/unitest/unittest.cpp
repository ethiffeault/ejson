
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <ejson/ejson.h>
#include <eti/eti.h>

using namespace eti;
using namespace ejson;

namespace eti
{
    Repository& Repository::Instance()
    {
        static Repository repository;
        return repository;
    }
}

namespace test
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

    class Doo : public Object
    {
        ETI_CLASS_EXT
        (
            Doo, Object,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(PointPtr)
            ),
            ETI_METHODS()
        )
    public:
        Point* PointPtr = nullptr;
    };

    class Animal
    {
        ETI_BASE(Animal)
    public:
        virtual ~Animal(){}
    };

    class Bird : public Animal
    {
        ETI_CLASS_EXT(Bird, Animal, ETI_PROPERTIES(), ETI_METHODS())
    public:
    };

    class Cat : public Animal
    {
        ETI_CLASS_EXT(Cat, Animal, ETI_PROPERTIES(), ETI_METHODS())
    public:
    };

    void Register()
    {
        static bool registered = false;
        if (!registered)
        {
            Repository::Instance().Register(TypeOf<Day>());
            Repository::Instance().Register(TypeOf<Point>());
            Repository::Instance().Register(TypeOf<Foo>());
            Repository::Instance().Register(TypeOf<Doo>());
            Repository::Instance().Register(TypeOf<Animal>());
            Repository::Instance().Register(TypeOf<Bird>());
            Repository::Instance().Register(TypeOf<Cat>());
            registered = true;
        }
    }
}
ETI_ENUM_IMPL(test::Day);

using namespace test;

namespace test_01
{
    TEST_CASE("test_01")
    {
        test::Register();

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

namespace test_02
{
    TEST_CASE("test_02")
    {
        test::Register();
        {
            string json;
            Doo foo;
            WriteType(foo, json);
            REQUIRE(json == EJSON_TEXT("{\"PointPtr\":null}"));
        }

        {
            string json = EJSON_TEXT("{\"PointPtr\":null}");
            ParserError error;
            Doo* foo = nullptr;
            ReadType(json, foo, error);
            REQUIRE(foo != nullptr);
            REQUIRE(foo->PointPtr == nullptr);
        }

        {
            string json;
            Doo foo;
            foo.PointPtr = new Point();
            WriteType(foo, json);
            REQUIRE(json == EJSON_TEXT("{\"PointPtr\":{\"X\":0,\"Y\":0}}"));
            delete (foo.PointPtr);
            foo.PointPtr = nullptr;
        }

    }
}

