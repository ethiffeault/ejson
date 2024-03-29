# ejson
json library in c++

single header c++ json library that separate parsing from processing, making it very easy to customize and implement json serialization for any rtti library. Parsing and processing are stream friendly to minimize memory footprint for large json file.

## Introduction

ejson provide a "Value" type to handle underlying json type (null, bool, number, string, array and object)

simple usage:
read json from string
```
    string jsonInput = L"{\"name\":\"John\"}";
    Value value;
    // read input to Value
    Json::Read(jsonInput, value);
    std::wcout << "value name is : " << value[L"name"].AsString() << std::endl;
```
```
    value name is : John
```
write it back to a string
```
    string jsonOutput;
    Json::Write(value, jsonOutput);

    if (jsonInput == jsonOutput)
        std::wcout << "input and output are the same: " << std::endl;;
    std::wcout << jsonOutput << std::endl << std::endl;
```
```
    input and output are the same:
    {"name":"John"}
```
write prettify json to string
```
    // for prettify output
    string prettifyOutput;
    Json::Write(value, prettifyOutput, true);
    std::wcout << prettifyOutput << std::endl << std::endl;

```
```
    {
        "name": "John"
    }
```

## File
read from file :
```
    std::wifstream fileInputStream("..\\data\\john_doe.json");
    if (fileInputStream)
    {
        // parse file
        Value value;
        Json::Read(fileInputStream, value);

        // write it back to the console
        string jsonOutput;
        Json::Write(value, jsonOutput);
        std::wcout << jsonOutput;
    }
```
```
    {"FirstName":"John","LastName":"Doe","Age":71,"Music":["punk","country","folk"]}
```

## Error
read file with error:
```
    std::wifstream fileInputStream("..\\data\\john_doe_err.json");
    if (fileInputStream)
    {
        // parse file
        Value value;
        ParserError error;
        if (Json::Read(fileInputStream, value, error))
        {
            // no error ...
        }
        else
        {
            // error
            std::wcout << "error at line/column " << error.Line << "/" << error.Column << ": " << error.Error;
        }
    }
```
```
    error at line/column 3/6: invalid token
```

## Configuration

modify configuration at the beginning of ejson.h

### char/wchar_t
```
    #define EJSON_WCHAR 1 // wchar_t (default)
    #define EJSON_WCHAR 0 // char
```
### Assert
```
    #define EJSON_ASSERT (default impl use cassert)
    #define EJSON_ERROR 
```
### float/double
```
    using number = double; (default)
    using number = float;
```
### order ot not
```
#define EJSON_MAP_ORDERED 1 (default, keep load/write order)
#define EJSON_MAP_ORDERED 0 (faster, don't keep order, suitable for final build that only load)
```
## Others

eti use awesome great unit tests framework: [doctest](https://github.com/doctest/doctest)