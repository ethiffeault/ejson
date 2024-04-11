#pragma once

#include <eti/eti.h>

namespace ejson
{
    using namespace eti;

    template <typename JSON_WRITER>
    class TypeWriter
    {

    public:

        TypeWriter(JSON_WRITER& jsonWriter) noexcept
            :jsonWriter(jsonWriter)
        {}

        void Write(const eti::Declaration& declaration, const void* value) noexcept
        {
            if (declaration.IsPtr)
            {
                if (value == nullptr)
                {
                    jsonWriter.WriteNull();
                    return;
                }
            }

            EJSON_ASSERT(value != nullptr, "cannot be null when not a ptr");

            switch (declaration.Type->Kind)
            {
            case Kind::Void:
                jsonWriter.WriteNull();
                break;
            case Kind::Class:
                WriteClass(declaration, value);
                break;
            case Kind::Struct:
                WriteStruct(declaration, value);
                break;
            case Kind::Pod:
                WritePod(declaration, value);
                break;
            case Kind::Enum:
                WriteEnum(declaration, value);
                break;
            case Kind::Unknown:
                jsonWriter.WriteNull();
                break;
            case Kind::Forward:
                jsonWriter.WriteNull();
                break;
            }
        }

    private:

        void WriteStruct(const eti::Declaration& declaration, const void* value) noexcept
        {
            EJSON_ASSERT(value != nullptr, "cannot be null if not ptr");

            jsonWriter.WriteObjectBegin();

            for (const Property& property : declaration.Type->Properties)
            {
                jsonWriter.WriteProperty(property.Variable.Name);
                void* propertyValue = property.UnSafeGetPtr(const_cast<void*>(value));
                Write(property.Variable.Declaration, propertyValue);
            }
            jsonWriter.WriteObjectEnd();
        }

        void WriteClass(const eti::Declaration& declaration, const void* value) noexcept
        {
            if (*declaration.Type  == TypeOf<std::string>())
            {
                jsonWriter.WriteString(*(std::string*)value);
            }
            else if (*declaration.Type  == TypeOf<std::wstring>())
            {
                jsonWriter.WriteString(*(std::wstring*)value);
            }
            else if (declaration.Type->Name.starts_with("std::vector"))
            {
                jsonWriter.WriteArrayBegin();
                size_t size;
                declaration.Type->GetMethod("GetSize")->UnSafeCall((void*)value, &size, {});
                const Type& itemType = *declaration.Type->Templates[0].Type;
                const Method* getAt = declaration.Type->GetMethod("GetAt");
                for (size_t i = 0; i < size; ++i)
                {
                    void* ptr = nullptr;
                    void* args[1] = { &i };
                    getAt->UnSafeCall((void*)value, &ptr, args);

                    if (declaration.Type->Templates[0].IsPtr)
                        Write(declaration.Type->Templates[0], *(void**)ptr);
                    else
                        Write(declaration.Type->Templates[0], ptr);
                }
                jsonWriter.WriteArrayEnd();
            }
            else if (declaration.Type->Name.starts_with("std::map"))
            {
                jsonWriter.WriteObjectBegin();

                const Type& keyType = *declaration.Type->Templates[0].Type;

                if ( (keyType == TypeOf<c_string>() || keyType == TypeOf<w_string>()) && declaration.Type->Templates[0].IsValue)
                {
                    const Type& valueType = *declaration.Type->Templates[1].Type;

                    // get all keys
                    const Method* mapGetKeys = declaration.Type->GetMethod("GetKeys");
                    const Method* mapGetValue = declaration.Type->GetMethod("GetValue");
                    const Type& keysType = *mapGetKeys->Arguments[1].Declaration.Type;
                    void* keys = keysType.New();
                    void* args[1] = { &keys };
                    mapGetKeys->UnSafeCall((void*)value, NoReturn, args);

                    size_t size;
                    keysType.GetMethod("GetSize")->UnSafeCall(keys, &size, {});
                    const Method* keysGetAt = keysType.GetMethod("GetAt");

                    for (size_t i = 0; i < size; ++i)
                    {
                        void* ptrKey = nullptr;
                        void* getAtArgs[1] = { &i };
                        keysGetAt->UnSafeCall(keys, &ptrKey, getAtArgs);

                        void* keyValue;
                        void* ptrValue = nullptr;
                        void* getValueArgs[1] = {&ptrKey};
                        mapGetValue->UnSafeCall((void*)value, &keyValue, getValueArgs);

                        if (keyType == TypeOf<c_string>() )
                        {
                            c_string* str = (c_string*)ptrKey;
                            std::vector<std::string>* stringKey = (std::vector<std::string>*) keys;
                            jsonWriter.WriteProperty(*str);
                        }
                        else // keyType == TypeOf<w_string>()
                        {
                            w_string* str = (w_string*)ptrKey;
                            std::vector<std::string>* stringKey = (std::vector<std::string>*) keys;
                            jsonWriter.WriteProperty(*str);
                        }

                        if (declaration.Type->Templates[1].IsPtr)
                            Write(declaration.Type->Templates[1], *(void**)keyValue);
                        else
                            Write(declaration.Type->Templates[1], keyValue);
                    }
                }
                jsonWriter.WriteObjectEnd();
            }
            else
            {
                // Object

                EJSON_ASSERT(value != nullptr, "cannot be null if not ptr");

                const Type* type = declaration.Type;
                if (declaration.IsPtr)
                {
                    if (IsA(*type, TypeOf<eti::Object>()))
                    {
                        eti::Object* object = (eti::Object*)value;
                        type = &object->GetType();
                    }
                }

                jsonWriter.WriteObjectBegin();

                if (*type != *declaration.Type)
                {
                    jsonWriter.WriteProperty(EJSON_TEXT("@type"));
                    jsonWriter.WriteString(ToWString(type->Name));
                }

                for (const Property& property : type->Properties)
                {
                    jsonWriter.WriteProperty(property.Variable.Name);
                    if (property.Variable.Declaration.IsPtr)
                    {
                        void** propertyValue = (void**)property.UnSafeGetPtr(const_cast<void*>(value));
                        Write(property.Variable.Declaration, *propertyValue);
                    }
                    else
                    {
                        void* propertyValue = property.UnSafeGetPtr(const_cast<void*>(value));
                        Write(property.Variable.Declaration, propertyValue);
                    }
                }
                jsonWriter.WriteObjectEnd();
            }
        }

        void WritePod(const eti::Declaration& declaration, const void* value) noexcept
        {
            switch (declaration.Type->Id)
            {
                case GetTypeId<bool>():
                    jsonWriter.WriteBool(*(bool*)value);
                    break;

                case GetTypeId<u8>():
                    jsonWriter.WriteNumber(*(u8*)value);
                    break;
                case GetTypeId<u16>():
                    jsonWriter.WriteNumber(*(u16*)value);
                    break;
                case GetTypeId<u32>():
                    jsonWriter.WriteNumber(*(u32*)value);
                    break;
                case GetTypeId<u64>():
                    jsonWriter.WriteNumber((number)*(u64*)value);
                    break;

                case GetTypeId<s8>():
                    jsonWriter.WriteNumber(*(s8*)value);
                    break;
                case GetTypeId<s16>():
                    jsonWriter.WriteNumber(*(s16*)value);
                    break;
                case GetTypeId<s32>():
                    jsonWriter.WriteNumber(*(s32*)value);
                    break;
                case GetTypeId<s64>():
                    jsonWriter.WriteNumber((number)*(s64*)value);
                    break;

                case GetTypeId<f32>():
                    jsonWriter.WriteNumber(*(f32*)value);
                    break;
                case GetTypeId<f64>():
                    jsonWriter.WriteNumber(*(f64*)value);
                    break;

                default:
                    jsonWriter.WriteNull();
                    break;
            }
        }

        void WriteEnum(const eti::Declaration& declaration, const void* value) noexcept
        {
            s64 enumValue = -1;
            switch (declaration.Type->Parent->Id)
            {
                case GetTypeId<u8>():
                    enumValue = *(u8*)value;
                    break;
                case GetTypeId<u16>():
                    enumValue = *(u16*)value;
                    break;
                case GetTypeId<u32>():
                    enumValue = *(u32*)value;
                    break;
                case GetTypeId<u64>():
                    enumValue = (s64)*(u64*)value;
                    break;

                case GetTypeId<s8>():
                    enumValue = *(s8*)value;
                    break;
                case GetTypeId<s16>():
                    enumValue = *(s16*)value;
                    break;
                case GetTypeId<s32>():
                    enumValue = *(s32*)value;
                    break;
                case GetTypeId<s64>():
                    enumValue = *(s64*)value;
                    break;

                default:
                    break;
            }

            if ( enumValue >= 0 && enumValue < (s64)declaration.Type->EnumSize)
            {
                jsonWriter.WriteString(declaration.Type->GetEnumValueName(enumValue));
            }
            else
            {
                jsonWriter.WriteString("invalid");
            }
        }

        JSON_WRITER& jsonWriter;

    };

    struct TypeReader
    {
        TypeReader(void* root, const Declaration& declaration) noexcept
        {
            nextValue = root;
            nextDeclaration = declaration;
        }

        ~TypeReader()
        {
            EJSON_ASSERT(contexts.size() == 0, "internal error");
        }

        void ObjectBegin() noexcept
        {
            if (nextDeclaration.Type == nullptr)
            {
                PushInvalid();
                return;
            }

            if (nextDeclaration.Type->Name.starts_with("std::map"))
            {
                if (nextValue == nullptr)
                {
                    TryCreateValue();
                    if (nextValue == nullptr)
                    {
                        PushInvalid();
                        return;
                    }
                }

                PushContext(ContextType::Map, nextDeclaration, nextValue);

                nextDeclaration = nextDeclaration.Type->Templates[1];
                nextValue = nullptr;

            }
            else if (nextDeclaration.Type->Kind == Kind::Struct)
            {
                if (nextValue == nullptr)
                {
                    TryCreateValue();
                    if (nextValue == nullptr)
                    {
                        PushInvalid();
                        return;
                    }
                }

                PushContext(ContextType::Struct, nextDeclaration, nextValue);

                nextDeclaration = {};
                nextValue = nullptr;
            }
            else if (nextDeclaration.Type->Kind == Kind::Class )
            {
                if (nextDeclaration.IsPtr)
                {
                    // wait for @type
                    nextObjectTypeKey = true;
                }
                else
                {
                    if (nextValue == nullptr)
                    {
                        TryCreateValue();
                        if (nextValue == nullptr)
                        {
                            PushInvalid();
                            return;
                        }
                    }

                    PushContext(ContextType::Class, nextDeclaration, nextValue);

                    nextDeclaration = {};
                    nextValue = nullptr;                    
                }
            }
            else
            {
                PushInvalid();
            }
        }

        void ObjectEnd() noexcept
        {
            if (nextObjectTypeKey)
            {
                nextObjectTypeKey = false;
            }
            else
            {
                PopContext();
            }
        }

        void PropertyBegin(const string_view& key) noexcept
        {
            if ( nextObjectTypeKey && key != EJSON_TEXT("@type"))
            {
                nextObjectTypeKey = false;
                if (nextValue == nullptr)
                {
                    TryCreateValue();
                    if (nextValue == nullptr)
                    {
                        PushInvalid();
                        return;
                    }
                }
                else
                {
                    *(void**)nextValue = nextDeclaration.Type->New();
                }

                PushContext(ContextType::Class, nextDeclaration, nextValue);

                nextDeclaration = {};
                nextValue = nullptr;      
            }

            if (current == nullptr)
                return;

            if (current->Type == ContextType::Map)
            {
                nextPropertyName = key;
            }
            else if (current->Type == ContextType::Struct)
            {
                const Property* property = current->Declaration.Type->GetProperty(ToString(key));
                if ( property != nullptr)
                {
                    nextDeclaration = property->Variable.Declaration;
                    nextValue = ((u8*)current->Value) + property->Offset;
                }
                else
                {
                    nextDeclaration = {};
                    nextValue = nullptr;
                }
            }
            else if (current->Type == ContextType::Class )
            {
                if (key == EJSON_TEXT("@type") && nextObjectTypeKey)
                {
                    // skip here, will be created later
                }
                else
                {
                    const Property* property = current->Declaration.Type->GetProperty(ToString(key));
                    if ( property != nullptr)
                    {
                        nextDeclaration = property->Variable.Declaration;
                        if ( current->Declaration.IsPtr)
                            nextValue = ((u8*)*(void**)current->Value) + property->Offset;
                        else
                            nextValue = ((u8*)current->Value) + property->Offset;
                    }
                    else
                    {
                        nextDeclaration = {};
                        nextValue = nullptr;
                    }
                }
            }
        }

        void PropertyEnd() noexcept
        {
            nextPropertyName.clear();
        }

        void ArrayBegin() noexcept
        {
            if (nextDeclaration.Type == nullptr || !nextDeclaration.Type->Name.starts_with("std::vector"))
            {
                PushInvalid();
                return;
            }

            if (nextValue == nullptr)
            {
                TryCreateValue();
                if (nextValue == nullptr)
                {
                    PushInvalid();
                    return;
                }
            }

            PushContext(ContextType::Vector, nextDeclaration, nextValue);

            nextDeclaration = nextDeclaration.Type->Templates[0];
            nextValue = nullptr;

        }

        void ArrayEnd() noexcept
        {
            PopContext();
        }

        void ValueBool(bool b) noexcept
        {
            if (nextDeclaration.Type == nullptr)
                return;

            if (nextValue == nullptr)
            {
                TryCreateValue();
                if (nextValue == nullptr)
                    return;
            }

            if ( *nextDeclaration.Type == TypeOf<bool>())
            {
                SetPodValue(&b);
            }
        }

        void ValueNull() noexcept
        {
            if (nextDeclaration.Type == nullptr)
                return;

            if (nextDeclaration.IsPtr)
            {
                if (nextValue != nullptr)
                {
                    if (*(void**)nextValue != nullptr)
                        nextDeclaration.Type->Delete(*(void**)nextValue);
                    *(void**)nextValue = nullptr;
                }
            }
        }

        void ValueString(const string_view& str) noexcept
        {
            // handle object polymorphism creation
            if (nextObjectTypeKey)
            {
                nextObjectTypeKey = false;
                EJSON_ASSERT(nextDeclaration.IsPtr, "internal error");
                const Type* nextObjectType = eti::Repository::Instance().GetType(ToString(str));
                if (nextObjectType == nullptr)
                {
                    nextObjectType = current->Declaration.Type;
                }
                else
                {
                    if (!IsA(*nextObjectType, *nextDeclaration.Type))
                        nextObjectType = nextDeclaration.Type;
                }

                if (nextValue == nullptr)
                {
                    TryCreateValue(nextObjectType);
                    if (nextValue == nullptr)
                    {
                        PushInvalid();
                        return;
                    }
                }
                else
                {
                    *(void**)nextValue = nextObjectType->New();
                }
                PushContext(ContextType::Class, nextDeclaration, nextValue);
                nextDeclaration = {};
                nextValue = nullptr;  
                return;
            }

            if (nextDeclaration.Type == nullptr)
                return;

            if (nextValue == nullptr)
            {
                TryCreateValue();
                if (nextValue == nullptr)
                    return;
            }

            if (nextDeclaration.Type->Kind == Kind::Enum)
            {
                size_t enumValue = nextDeclaration.Type->GetEnumValue(ToString(str));
                if (enumValue != InvalidIndex)
                {
                    switch (nextDeclaration.Type->Parent->Id )
                    {
                        case GetTypeId<s8>():
                        {
                            s8 typedValue = (s8)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<s16>():
                        {
                            s16 typedValue = (s16)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<s32>():
                        {
                            s32 typedValue = (s32)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<s64>():
                        {
                            s64 typedValue = (s64)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<u8>():
                        {
                            u8 typedValue = (u8)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<u16>():
                        {
                            u16 typedValue = (u16)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<u32>():
                        {
                            u32 typedValue = (u32)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        case GetTypeId<u64>():
                        {
                            u64 typedValue = (u64)enumValue;
                            SetPodValue(&typedValue);
                        }
                        break;
                        default:
                            break;
                    }
                }
            }
            else if (*nextDeclaration.Type == TypeOf<std::string>())
            {
                SetValue(ToString(str));
            }
            else if (*nextDeclaration.Type == TypeOf<std::wstring>())
            {
                SetValue(ToWString(str));
            }
        }

        void ValueNumber(number value) noexcept
        {
            if (nextDeclaration.Type == nullptr)
                return;

            if (nextValue == nullptr)
            {
                TryCreateValue();
                if (nextValue == nullptr)
                    return;
            }

            switch (nextDeclaration.Type->Id)
            {
                case GetTypeId<u8>():
                {
                    u8 typedValue = (u8)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<u16>():
                {
                    u16 typedValue = (u16)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<u32>():
                {
                    u32 typedValue = (u32)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<u64>():
                {
                    u64 typedValue = (u64)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<s8>():
                {
                    s8 typedValue = (s8)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<s16>():
                {
                    s16 typedValue = (s16)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<s32>():
                {
                    s32 typedValue = (s32)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<s64>():
                {
                    s64 typedValue = (s64)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<f32>():
                {
                    f32 typedValue = (f32)value;
                    SetPodValue(&typedValue);
                }
                break;
                case GetTypeId<f64>():
                {
                    f64 typedValue = (f64)value;
                    SetPodValue(&typedValue);
                }
                break;
                default:
                    break;
            }
        }

        void SetPodValue(void* value)
        {
            EJSON_ASSERT(nextValue != nullptr, "internal error");
            if (nextDeclaration.IsPtr)
            {
                if (*(void**)nextValue == nullptr)
                    *(void**)nextValue = nextDeclaration.Type->New();
                memcpy(*(void**)nextValue, value, nextDeclaration.Type->Size);
            }
            else
            {
                memcpy(nextValue, value, nextDeclaration.Type->Size);
            }
            nextValue = nullptr;
        }

        template <typename T>
        void SetValue(const T& v)
        {
            EJSON_ASSERT(TypeOf<T>() == *nextDeclaration.Type, "internal error");
            EJSON_ASSERT(nextValue != nullptr, "internal error");
            if (nextDeclaration.IsPtr)
            {
                if (*(void**)nextValue == nullptr)
                    *(void**)nextValue = nextDeclaration.Type->New();
                *(*(T**)nextValue) = v;
            }
            else
            {
                *(T*)nextValue = v;
            }
            nextValue = nullptr;
        }

        void TryCreateValue(const Type* type = nullptr)
        {
            // vector or map
            EJSON_ASSERT(nextValue == nullptr, "internal error");

            if (current == nullptr || nextDeclaration.Type == nullptr)
                return;

            if (current->Declaration.Type->Name.starts_with("std::vector") && current->Type == ContextType::Vector)
            {
                Declaration& vectorDeclaration = current->Declaration;
                Declaration& elementDeclaration = nextDeclaration;
                EJSON_ASSERT(*elementDeclaration.Type == *vectorDeclaration.Type->Templates[0].Type, "internal error");

                const Method* addDefault = vectorDeclaration.Type->GetMethod("AddDefault");
                EJSON_ASSERT(addDefault != nullptr, "internal error");

                if (elementDeclaration.IsPtr)
                {
                    void** newValue = nullptr;
                    if (vectorDeclaration.IsPtr)
                        addDefault->UnSafeCall(*(void**)current->Value, &newValue, {});
                    else
                        addDefault->UnSafeCall(current->Value, &newValue, {});
                    nextValue = newValue;

                    if ( type != nullptr)
                        *(void**)nextValue = type->New();
                    else
                        *(void**)nextValue = elementDeclaration.Type->New();
                }
                else
                {
                    void* newValue = nullptr;
                    if ( vectorDeclaration.IsPtr)
                        addDefault->UnSafeCall(*(void**)current->Value, &newValue, {});
                    else
                        addDefault->UnSafeCall(current->Value, &newValue, {});
                    nextValue = newValue;
                }
            }
            else if (current->Declaration.Type->Name.starts_with("std::map") && nextPropertyName != EJSON_TEXT("")  && current->Type == ContextType::Map)
            {
                Declaration& mapDeclaration = current->Declaration;
                Declaration& elementDeclaration = nextDeclaration;
                EJSON_ASSERT(*elementDeclaration.Type == *mapDeclaration.Type->Templates[1].Type, "internal error");

                const Method* insert = mapDeclaration.Type->GetMethod("InsertDefaultOrGet");
                EJSON_ASSERT(insert != nullptr, "internal error");

                void* args[1] = { nullptr };
                c_string cMapKey;
                w_string wMapKey;
                if (*mapDeclaration.Type->Templates[0].Type == TypeOf<c_string>())
                {
                    cMapKey = ToString(nextPropertyName);
                    c_string* cMapKeyPtr = &cMapKey;
                    args[0] = &cMapKeyPtr;
                }
                else if (*mapDeclaration.Type->Templates[0].Type == TypeOf<w_string>())
                {
                    wMapKey = ToWString(nextPropertyName);
                    w_string* wMapKeyPtr = &wMapKey;
                    args[0] = &wMapKeyPtr;
                }

                if (args[0] != nullptr)
                {
                    if (elementDeclaration.IsPtr)
                    {
                        void** newValue = nullptr;
                        if (mapDeclaration.IsPtr)
                            insert->UnSafeCall(*(void**)current->Value, &newValue, args);
                        else
                            insert->UnSafeCall(current->Value, &newValue, args);
                        nextValue = newValue;

                        if ( type != nullptr)
                            *(void**)nextValue = type->New();
                        else
                            *(void**)nextValue = elementDeclaration.Type->New();
                    }
                    else
                    {
                        void* newValue = nullptr;
                        if (mapDeclaration.IsPtr)
                            insert->UnSafeCall(*(void**)current->Value, &newValue, args);
                        else
                            insert->UnSafeCall(current->Value, &newValue, args);
                        nextValue = newValue;
                    }
                }
            }

            nextPropertyName.clear();
        }

    private:

        enum class ContextType : u8
        {
            Invalid,
            Vector,
            Map,
            Struct,
            Class
        };

        struct Context
        {
            ContextType Type = ContextType::Invalid;
            void* Value = nullptr;
            Declaration Declaration;
        };


        Declaration nextDeclaration{};
        void* nextValue = nullptr;
        string nextPropertyName = {};
        Context* current = nullptr;
        std::vector<Context> contexts;
        bool nextObjectTypeKey = false;

        void PushInvalid()
        {
            Context context;
            context.Type = ContextType::Invalid;
            contexts.push_back(context);
            current = &contexts[contexts.size() - 1];
        }

        void PushContext(ContextType type, const Declaration& declaration, void* value)
        {
            Context context;
            context.Type = type;
            context.Value = value;
            context.Declaration = declaration;
            contexts.push_back(context);

            current = &contexts[contexts.size() - 1];
        }

        void PopContext()
        {
            EJSON_ASSERT(contexts.size() > 0, "internal error");

            nextDeclaration = current->Declaration;
            nextValue = nullptr;

            contexts.pop_back();
            current = contexts.size() ? &contexts[contexts.size() - 1] : nullptr;
        }

    };

    template <typename T>
    bool ReadType(string_view json, T& value) noexcept
    {
        ParserError error;
        return ReadType(json, value, error);
    }

    template <typename T>
    bool ReadType(string_view json, T& value, ParserError& error) noexcept
    {
        StringReader stringReader(json);

        void* ptr = &value;

        TypeReader typeReader(ptr, eti::internal::MakeDeclaration<T>());
        JsonReader jsonReader(typeReader, stringReader);
        if (jsonReader.Parse())
        {
            return true;
        }
        else
        {
            error = jsonReader.GetError();
            return false;
        }
    }

    template <typename T>
    void WriteType(const T& value, string& str, bool prettify = false ) noexcept
    {
        StringClear(str);
        StringWriter stringWriter(str);
        if (!prettify)
        {
            JsonWriter jsonWriter(stringWriter);
            TypeWriter valueWriter(jsonWriter);

            Declaration declaration = {};
            declaration.Type = &TypeOf<T>();
            if constexpr (std::is_pointer_v<T>)
            {
                declaration.IsPtr = true;
                valueWriter.Write(declaration, value);
            }
            else
            {
                valueWriter.Write(declaration, &value);
            }
        }
        else
        {
            JsonWriter<StringWriter, true> jsonWriter(stringWriter);
            TypeWriter valueWriter(jsonWriter);
            
            Declaration declaration = {};
            declaration.Type = &TypeOf<T>();
            if constexpr (std::is_pointer_v<T>)
            {
                declaration.IsPtr = true;
                valueWriter.Write(declaration, value);
            }
            else
            {
                valueWriter.Write(declaration, &value);
            }
        }
    }

    //template <typename T>
    //void Write(const T& value, output_stream& stream, bool prettify = false) noexcept
    //{
    //    StreamWriter streamWriter(stream);
    //    if (!prettify)
    //    {
    //        JsonWriter jsonWriter(streamWriter);
    //        TypeWriter valueWriter(jsonWriter);
    //        valueWriter.Write(value);
    //    }
    //    else
    //    {
    //        JsonWriter<StreamWriter, true> jsonWriter(streamWriter);
    //        TypeWriter valueWriter(jsonWriter);
    //        valueWriter.Write(value);
    //    }
    //}
}
