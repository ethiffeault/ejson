#include "doctest.h"

#include <iostream>

#include <ejson/ejson.h>

namespace playground
{
    
    TEST_CASE("doc_intro")
    {
        using namespace ejson;        

    #if EJSON_WCHAR

        string jsonInput = L"{\"name\":\"John\"}";
        Value value;
        // read input to Value
        Json::Read(jsonInput, value);
        std::wcout << "value name is : " << value[L"name"].AsString() << std::endl << std::endl;

        // write it back to a string
        string jsonOutput;
        Json::Write(value, jsonOutput);

        if (jsonInput == jsonOutput)
            std::wcout << "input and output are the same: " << std::endl;
        std::wcout << jsonOutput << std::endl << std::endl;

        // for prettify output
        string prettifyOutput;
        Json::Write(value, prettifyOutput, true);
        std::wcout << L"prettify:" << std::endl;
        std::wcout << prettifyOutput << std::endl << std::endl;

    #endif
    }
}
