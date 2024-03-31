# eti implementation 

[eti - Extended Type Information for c++](https://github.com/ethiffeault/eti)

**WIP**

```cpp
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

    TEST_CASE("sample")
    {
        Point p;
        string json;
        WriteType(p, json, true);
        wcout << json;
    }
```
output
```json
{
    "X": 0,
    "Y": 0
}
```