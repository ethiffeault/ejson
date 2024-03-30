# Unreal Engine ejson implementation 

**WIP** Implementation that use unreal types and support unreal rtti

## File

### Read 
todo
### Write
```cpp
    ejson::Value value;
    value.SetString(TEXT("1212"));
    TUniquePtr<FArchive> stream = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(TEXT("file.json")));
    ejson::Write(value, *stream, true);
    stream->Close();
```
