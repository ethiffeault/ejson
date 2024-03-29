# ejson

json library in c++

single header c++ json library that separate parsing from processing, making it very easy to customize and implement json serialization for any rtti library. Parsing and processing are stream friendly to minimize memory footprint for large json file.

ejson provide a "Value" type to handle underlying json type (null, bool, number, string, array and object)

read json from string:
```cpp
    string jsonInput = L"{\"name\":\"John\"}";
    Value value;
    // read input to Value
    Json::Read(jsonInput, value);
    std::wcout << "value name is : " << value[L"name"].AsString() << std::endl;
```
```
    value name is : John
```
write it back to a string:
```cpp
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
write prettify json to string:
```cpp
    // for prettify output
    string prettifyOutput;
    Json::Write(value, prettifyOutput, true);
    std::wcout << prettifyOutput << std::endl << std::endl;

```
```json
    {
        "name": "John"
    }
```

## File

### Read

```cpp
    std::wifstream fileInputStream("..\\data\\john_doe.json");
    Value value;
    Json::Read(fileInputStream, value);

    string jsonOutput;
    Json::Write(value, jsonOutput);
    std::wcout << jsonOutput;
```
```json
    {"FirstName":"John","LastName":"Doe","Age":71,"Music":["punk","country","folk"]}
```

### Write

read from file and write back to another file in pretty format:
```cpp
    // read
    std::wifstream fileInputStream("..\\data\\john_doe.json");
    Value value;
    Json::Read(fileInputStream, value);

    // write
    std::wofstream fileOutputStream("..\\data\\john_doe_output.json");
    Json::Write(value, fileOutputStream, true);
```
john_doe_output.json:
```json
    {
        "FirstName": "John",
        "LastName": "Doe",
        "Age": 71,
        "Music": [
            "punk",
            "country",
            "folk"
        ]
    }
```

## Error

read file with error:
```cpp
    std::wifstream fileInputStream("..\\data\\john_doe_err.json");
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
```
```
    error at line/column 3/6: invalid token
```

## Configuration

modify configuration at the beginning of ejson.h

### char/wchar_t

```cpp
    #define EJSON_WCHAR 1 // wchar_t (default)
    #define EJSON_WCHAR 0 // char
```

### Assert

```cpp
    #define EJSON_ASSERT // (default impl use cassert)
    #define EJSON_ERROR 
```

### float/double

```cpp
    using number = double; // (default)
    using number = float;
```

### order ot not

```cpp
    #define EJSON_MAP_ORDERED 1 // (default, keep load/write ordered)
    #define EJSON_MAP_ORDERED 0 // (faster, don't keep ordered, ex: suitable for final build that only read)
```
## Others

eti use awesome great unit tests framework: [doctest](https://github.com/doctest/doctest)