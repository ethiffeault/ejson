#include "doctest.h"

#include <iostream>

#include <ejson/ejson.h>

namespace doc
{
#if EJSON_WCHAR
    using namespace ejson;

    TEST_CASE("doc_intro")
    {
        string jsonInput = L"{\"name\":\"John\"}";
        Value value;
        // read input to Value
        JsonRead(jsonInput, value);
        std::wcout << "value name is : " << value[L"name"].AsString() << std::endl << std::endl;

        // write it back to a string
        string jsonOutput;
        JsonWrite(value, jsonOutput);

        if (jsonInput == jsonOutput)
            std::wcout << "input and output are the same: " << std::endl;
        std::wcout << jsonOutput << std::endl << std::endl;

        // for prettify output
        string prettifyOutput;
        JsonWrite(value, prettifyOutput, true);
        std::wcout << L"prettify:" << std::endl;
        std::wcout << prettifyOutput << std::endl << std::endl;
    }

    TEST_CASE("doc_file")
    {
        std::wifstream fileInputStream("..\\data\\john_doe.json");
        if (fileInputStream)
        {
            // parse file
            Value value;
            JsonRead(fileInputStream, value);

            // write it back to the console
            string jsonOutput;
            JsonWrite(value, jsonOutput);
            std::wcout << jsonOutput << std::endl;;

            // write it back to another file and prettify
            std::wofstream fileOutputStream("..\\data\\john_doe_output.json");
            if (fileOutputStream)
            {
                JsonWrite(value, fileOutputStream, true);
            }
        }
    }

    TEST_CASE("doc_file_err")
    {
        std::wifstream fileInputStream("..\\data\\john_doe_err.json");
        if (fileInputStream)
        {
            // parse file
            Value value;
            ParserError error;
            if (JsonRead(fileInputStream, value, error))
            {
                // no error ...
            }
            else
            {
                // error
                std::wcout << "error at line/column " << error.Line << "/" << error.Column << ": " << error.Error;
                // output : error at line/column 3/6: invalid token
            }
        }
    }
#endif

}
