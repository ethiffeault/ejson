
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <ejson/ejson.h>
#include <eti/eti.h>

using namespace eti;
using namespace ejson;

ETI_REPOSITORY_IMPL()

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

    class Animal : public eti::Object
    {
        ETI_CLASS_EXT(Animal, eti::Object, ETI_PROPERTIES( ETI_PROPERTY(MaxSpeed) ), ETI_METHODS())
    public:
        float MaxSpeed = 0.0f;
    };

    class Bird : public Animal
    {
        ETI_CLASS_EXT(Bird, Animal, ETI_PROPERTIES(ETI_PROPERTY(Wingspan)), ETI_METHODS())
    public:
        float Wingspan = 1.2f;
    };

    class Cat : public Animal
    {
        ETI_CLASS_EXT(Cat, Animal, ETI_PROPERTIES(ETI_PROPERTY(TailsLength)), ETI_METHODS())
    public:
        float TailsLength = 0.3f;
    };

    struct Zoo
    {
        ETI_STRUCT_EXT(Zoo, ETI_PROPERTIES( ETI_PROPERTY(Animals) ), ETI_METHODS())

        std::vector<Animal*> Animals;

        Zoo()
        {
            Animals.push_back(new Cat());
            Animals.push_back(new Bird());
            Animals.push_back(new Animal());
        }

        ~Zoo()
        {
            for (Animal* a : Animals)
                delete(a);
            Animals.clear();
        }
    };

    struct FooArray
    {
        ETI_STRUCT_EXT
        (
            FooArray,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(Data),
            ),
            ETI_METHODS()
        )
        std::vector<u32> Data;
    };

    struct FooArrayPtr
    {
        ETI_STRUCT_EXT
        (
            FooArrayPtr,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(Data),
            ),
            ETI_METHODS()
        )
        std::vector<u32*> Data;
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
            Repository::Instance().Register(TypeOf<Zoo>());
            Repository::Instance().Register(TypeOf<FooArray>());
            Repository::Instance().Register(TypeOf<FooArrayPtr>());
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
        string refJson = EJSON_TEXT("1.23456");
        float refValue = 1.23456f;
        // write
        {
            f32 value = refValue;
            string json;
            WriteType(value, json);
            REQUIRE(value == refValue);
            REQUIRE(json == refJson);
        }

        {
            f32 value = refValue;
            f32* ptr = &value;
            string json;
            WriteType(ptr, json);
            REQUIRE(*ptr == refValue);
            REQUIRE(json == refJson);
        }

        {
            f32* ptr = nullptr;
            string json;
            WriteType(ptr, json);
            REQUIRE(ptr == nullptr);
            REQUIRE(json == EJSON_TEXT("null"));
        }

        // read
        {
            f32 value = 1.0f;
            ReadType(refJson, value);
            REQUIRE(value == refValue);
        }

        {
            f32 value = 1.0f;
            f32* ptr = &value;
            ReadType(refJson, ptr);
            REQUIRE(value == refValue);
            REQUIRE(*ptr == refValue);
        }

        {
            f32* ptr = nullptr;
            ReadType(refJson, ptr);
            REQUIRE(ptr != nullptr);
            REQUIRE(*ptr == refValue);
            delete(ptr);
        }

    }
}

namespace test_02
{
    TEST_CASE("test_02")
    {
        string refJson = EJSON_TEXT("[1.23456]");
        {
            std::vector<float> value = { 1.23456f };
            string json;
            WriteType(value, json);
            REQUIRE(value.size() == 1);
            REQUIRE(value[0] == 1.23456f);
            REQUIRE(json == refJson);
        }

        {
            std::vector<float> value;
            ReadType(refJson, value);
            REQUIRE(value.size() == 1);
            REQUIRE(value[0] == 1.23456f);
        }

        {
            std::vector<float*> value;
            value.push_back( (f32*)malloc(sizeof(float)) );
            *value[0] = 1.23456f;
            string json;
            WriteType(value, json);
            REQUIRE(value.size() == 1);
            REQUIRE(*value[0] == 1.23456f);
            REQUIRE(json == refJson);
            free(value[0]);
        }

        {
            std::vector<float*> value;
            ReadType(refJson, value);
            REQUIRE(value.size() == 1);
            REQUIRE(*value[0] == 1.23456f);
            delete(value[0]);
        }
    }
}

namespace test_03
{
    TEST_CASE("test_03")
    {
        string refJson = EJSON_TEXT("[[1.23456]]");
        {
            std::vector<std::vector<float>> value;
            value.emplace_back();
            value[0].push_back(1.23456f);
            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
        }

        {
            std::vector<std::vector<float>> value;
            ReadType(refJson, value);
            REQUIRE(value.size() == 1);
            REQUIRE(value[0].size() == 1);
            REQUIRE(value[0][0] == 1.23456f);
        }

        {
            std::vector<std::vector<float>*> value;
            value.emplace_back( new std::vector<float>());
            value[0]->push_back(1.23456f);
            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
            delete(value[0]);
        }

        {
            std::vector<std::vector<float>*> value;
            ReadType(refJson, value);
            REQUIRE(value.size() == 1);
            REQUIRE(value[0]->size() == 1);
            REQUIRE((*value[0])[0] == 1.23456f);
            TypeOf<std::vector<float>*>().Delete(value[0]);
        }
    }
}

namespace test_04
{
    TEST_CASE("test_04")
    {
        {
            string refJson = EJSON_TEXT("\"1212\"");
            {
                c_string value = "1212";
                string json;
                WriteType(value, json);
                REQUIRE(json == refJson);
            }

            {
                c_string value;
                ReadType(refJson, value);
                REQUIRE(value == "1212");
            }
        }

        {
            string refJson = EJSON_TEXT("\"1212\"");
            {
                w_string value = L"1212";
                string json;
                WriteType(value, json);
                REQUIRE(json == refJson);
            }
            {
                w_string value;
                ReadType(refJson, value);
                REQUIRE(value == L"1212");
            }

            {
                w_string* value = nullptr;
                ReadType(refJson, value);
                REQUIRE(*value == L"1212");
                delete value;
            }
        }

        {
            string refJson = EJSON_TEXT("{\"1212\":\"1212\"}");
            std::map<w_string, c_string > value;
            value[L"1212"] = "1212";
            
            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
        }
        
        {
            string refJson = EJSON_TEXT("[\"1212\",\"3434\"]");
            std::vector<w_string> value;
            value.emplace_back(L"1212");
            value.emplace_back(L"3434");

            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
        }

        {
            string refJson = EJSON_TEXT("{\"3434\":[\"3434\"]}");
            std::map< c_string, std::vector<w_string> > value;
            value["3434"] = {};
            value["3434"].emplace_back(L"3434");

            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
        }
    }
}

namespace test_05
{
    TEST_CASE("test_05")
    {

        string refJson = EJSON_TEXT("{\"1212\":1212}");


        // c string
        {
            {
                std::map<std::string, int> map;
                map["1212"] = 1212;
                string json;
                WriteType(map, json);
                REQUIRE(json == refJson);
            }

            {
                std::map<std::string, int*> map;
                map["1212"] = new int;;
                *map["1212"] = 1212;
                string json;
                WriteType(map, json);
                REQUIRE(json == refJson);
                delete map["1212"];
            }

            {
                std::map<std::string, int> map;
                ReadType(refJson, map);
                REQUIRE(map.size() == 1);
                REQUIRE(map["1212"] == 1212);
            }

            {
                std::map<std::string, int*> map;
                ReadType(refJson, map);
                REQUIRE(map.size() == 1);
                REQUIRE(*map["1212"] == 1212);
                delete map["1212"];
            }
        }

        // w string
        {
            {
                std::map<std::wstring, int> map;
                map[L"1212"] = 1212;
                string json;
                WriteType(map, json);
                REQUIRE(json == refJson);
            }

            {
                std::map<std::wstring, int*> map;
                map[L"1212"] = new int;;
                *map[L"1212"] = 1212;
                string json;
                WriteType(map, json);
                REQUIRE(json == refJson);
                delete map[L"1212"];
            }

            {
                std::map<std::wstring, int> map;
                ReadType(refJson, map);
                REQUIRE(map.size() == 1);
                REQUIRE(map[L"1212"] == 1212);
            }

            {
                std::map<std::wstring, int*> map;
                ReadType(refJson, map);
                REQUIRE(map.size() == 1);
                REQUIRE(*map[L"1212"] == 1212);
                delete map[L"1212"];
            }
        }
    }
}

namespace test_06
{
    TEST_CASE("test_06")
    {
        test::Register();
        {
            Point p;
            string json;
            WriteType(p, json);
            REQUIRE(json == EJSON_TEXT("{\"X\":0,\"Y\":0}"));
        }

        {
            string json = EJSON_TEXT("{\"X\":1,\"Y\":1}");
            Point p;
            ReadType(json, p);
            REQUIRE(p.X == 1.0f);
        }
    }
}

namespace test_07
{
    TEST_CASE("test_07")
    {
        string refJson = EJSON_TEXT("null");
        {
            void* foo = nullptr;
            string json;
            WriteType(foo, json);
            REQUIRE(json == refJson);
        }

        {
            Foo* foo = nullptr;
            string json;
            WriteType(foo, json);
            REQUIRE(json == refJson);
        }

        {
            Foo* foo = new Foo();
            ReadType(refJson, foo);
            REQUIRE(foo == nullptr);
        }
    }
}

namespace test_08
{
    TEST_CASE("test_08")
    {
        string refJson = EJSON_TEXT("true");
        {
            bool value = true;
            string json;
            WriteType(value, json);
            REQUIRE(json == refJson);
        }

        {
            bool value = false;
            ReadType(refJson, value);
            REQUIRE(value == true);
        }
    }
}

namespace test_09
{
    TEST_CASE("test_09")
    {
        string refJson = EJSON_TEXT("\"Friday\"");
        {
            Day day = Day::Friday;
            string json;
            WriteType(day, json);
            REQUIRE(json == refJson);
        }

        {
            Day day = Day::Monday;
            ReadType(refJson, day);
            REQUIRE(day == Day::Friday);
        }
    }
}



namespace test_11
{
    TEST_CASE("test_11")
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

// todo: fix it!
//namespace test_12
//{
//    TEST_CASE("test_12")
//    {
//        test::Register();
//        {
//            string json;
//            Doo foo;
//            WriteType(foo, json);
//            REQUIRE(json == EJSON_TEXT("{\"PointPtr\":null}"));
//        }
//
//        {
//            string json = EJSON_TEXT("{\"PointPtr\":null}");
//            ParserError error;
//            Doo* foo = nullptr;
//            ReadType(json, foo, error);
//            REQUIRE(foo != nullptr);
//            REQUIRE(foo->PointPtr == nullptr);
//        }
//
//        {
//            string json;
//            Doo foo;
//            foo.PointPtr = new Point();
//            WriteType(foo, json);
//            REQUIRE(json == EJSON_TEXT("{\"PointPtr\":{\"X\":0,\"Y\":0}}"));
//            delete (foo.PointPtr);
//            foo.PointPtr = nullptr;
//        }
//
//    }
//}

namespace test_13
{
    TEST_CASE("test_13")
    {
        test::Register();
        {
            Cat cat;
            const Type& type = cat.GetType();
            Animal* animal = &cat;
            string json;
            WriteType(animal, json);
            REQUIRE(json == EJSON_TEXT("{\"@type\":\"test::Cat\",\"TailsLength\":0.3}"));
        }

        {
            string json = EJSON_TEXT("{\"@type\":\"test::Cat\",\"TailsLength\":0.3}");
            Animal* animal = nullptr;
            ParserError error;
            ReadType(json, animal, error);
            REQUIRE(IsA<Cat>(*animal));
        }
    }
}

namespace test_14
{
    TEST_CASE("test_14")
    {
        {
            Zoo zoo;
            const Property* property = TypeOf<Zoo>().GetProperty("Animals");
            const Method* getAt = property->Variable.Declaration.Type->GetMethod("GetAt");
            int index = 0;
            void** ptr = nullptr;
            void* args[1] = { &index };
            getAt->CallMethod(zoo.Animals, &ptr, (size_t)0);
            REQUIRE(*ptr == zoo.Animals[0]);

            void** ptrU = nullptr;
            size_t indexU = 0;
            void* argsU[1] = { &indexU };
            getAt->UnSafeCall(&zoo.Animals, &ptrU, argsU);
            REQUIRE(*ptrU == zoo.Animals[0]);
        }

        string ref = EJSON_TEXT("{\"Animals\":[{\"@type\":\"test::Cat\",\"TailsLength\":0.3},{\"@type\":\"test::Bird\",\"Wingspan\":1.2},{\"MaxSpeed\":0}]}");

        test::Register();
        {
            Zoo zoo;
            string json;
            WriteType(zoo, json);
            REQUIRE(json == ref);
        }

        {
            Zoo zoo;
            for (auto a : zoo.Animals)
                delete (a);
            zoo.Animals.clear();
            ParserError error;
            ReadType(ref, zoo, error);
            // todo!
            REQUIRE(zoo.Animals.size() == 3);
            REQUIRE(IsA<Cat>(*zoo.Animals[0]));
            REQUIRE(IsA<Bird>(*zoo.Animals[1]));
            REQUIRE(IsA<Animal>(*zoo.Animals[2]));
        }
    }
}

//namespace test_15
//{
//    TEST_CASE("test_15")
//    {
//        {
//            FooArray foo1;
//            foo1.Data.push_back(2);
//            string json;
//            WriteType(foo1, json);
//            REQUIRE(json == EJSON_TEXT("{\"Data\":[2]}"));
//
//            FooArray foo2;
//            ParserError error;
//            ReadType(json, foo2, error);
//            REQUIRE(foo2.Data.size() == 1);
//            REQUIRE(foo2.Data[0] == 2);
//        }
//    }
//}