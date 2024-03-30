# ejson

json library in c++

Single header c++ [ejson.h](ejson/ejson.h) json library that separate parsing from processing, making it very easy to customize and implement json serialization for any rtti library. Parsing and processing are stream friendly to minimize memory footprint for large json file. Support both char and wide char.

ejson provide a "Value" type to handle underlying json type (null, bool, number, string, array and object)

implementation
* std [ejson.h](ejson/ejson.h)
* [Unreal Engine](implementation/unreal)

read json from string:
```cpp
    string input = L"{\"name\":\"John\"}";
    ejson::Value value;
    ejson::Read(input, value);
    std::wcout << "value name is : " << value[L"name"].AsString() << std::endl;
```
```
    value name is : John
```
write it back to a string:
```cpp
    string output;
    ejson::Write(value, output);

    if (input == output)
        std::wcout << "input and output are the same: " << std::endl;;
    std::wcout << output << std::endl << std::endl;
```
```
    input and output are the same:
    {"name":"John"}
```
write prettify json to string:
```cpp
    string prettifyOutput;
    ejson::Write(value, prettifyOutput, true);
    std::wcout << prettifyOutput << std::endl << std::endl;

```
```json
    {
        "name": "John"
    }
```
create json in typed code
```cpp
        Value json;
        json[L"FirstName"] = L"John";
        json[L"LastName"] = L"Doe";
        json[L"Age"] = 71;
        json[L"Music"][0] = L"punk";
        json[L"Music"][1] = L"country";
        json[L"Music"][2] = L"folk";

        string str;
        Write(json, str);
        std::wcout << str << std::endl << std::endl;
```
```
{"FirstName":"John","LastName":"Doe","Age":71,"Music":["punk","country","folk"]}
```
## File

### Read

```cpp
    std::wifstream input("..\\data\\john_doe.json");
    ejson::Value value;
    ejson::Read(input, value);

    string output;
    ejson::Write(value, output);
    std::wcout << output;
```
```json
    {"FirstName":"John","LastName":"Doe","Age":71,"Music":["punk","country","folk"]}
```

### Write

read from file and write back to another file in pretty format:
```cpp
    // read
    std::wifstream input("..\\data\\john_doe.json");
    ejson::Value value;
    ejson::Read(input, value);

    // write
    std::wofstream output("..\\data\\john_doe_output.json");
    ejson::Write(value, output, true);
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
## Json as cpp
create json in typed code
```cpp
        Value json;
        json[L"FirstName"] = L"John";
        json[L"LastName"] = L"Doe";
        json[L"Age"] = 71;
        json[L"Music"][0] = L"punk";
        json[L"Music"][1] = L"country";
        json[L"Music"][2] = L"folk";

        string str;
        Write(json, str);
        std::wcout << str << std::endl << std::endl;
```
```
{"FirstName":"John","LastName":"Doe","Age":71,"Music":["punk","country","folk"]}
```
could be build composed:
```cpp
    Value json;
    json[L"FirstName"] = L"John";
    json[L"LastName"] = L"Doe";
    json[L"Age"] = 71;
    Value& music = json[L"Music"];
    music[0] = L"punk";
    music[1] = L"country";
    music[2] = L"folk";
```
when loading data from file in read only, to make sure to not change input Value, use const Value& for your queries and make validation like this:
```cpp
    // read
    std::wifstream input("..\\data\\john_doe.json");
    ejson::Value fileValue;
    ejson::Read(input, fileValue);

    // 
    ejson::Value& value = fileValue;
    if (value[L"FirstName"].IsString())
    {
        string firstName = value[L"FirstName"].AsString();
    }
```
## Error

read file with error:
```cpp
    std::wifstream input("..\\data\\john_doe_err.json");
    ejson::Value value;
    ejson::ParserError error;
    if (ejson::Read(input, value, error))
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
## Implementation

* [Unreal Engine](implementation/unreal)

## Others

eti use awesome great unit tests framework: [doctest](https://github.com/doctest/doctest)