# ejson
json library in c++

single header c++ json library that separate parsing from processing, making it very easy to customize and implement json serialization for any rtti library. parsing processing is stream friendly, excellent to minimize memory footprint for large json file.

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
    std::wcout << L"prettify:" << std::endl;
    std::wcout << prettifyOutput << std::endl << std::endl;

```
```
prettify:
{
    "name": "John"
}
```

## Configutation

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
## Others

eti use awesome great unit tests framework: [doctest](https://github.com/doctest/doctest)