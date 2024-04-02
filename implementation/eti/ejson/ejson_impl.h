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

        void Write(const eti::Type& type, const void* value) noexcept
        {
            switch (type.Kind)
            {
            case Kind::Void:
                jsonWriter.WriteNull();
                break;
            case Kind::Class:
                WriteStruct(type, value);
                break;
            case Kind::Struct:
                WriteStruct(type, value);
                break;
            case Kind::Pod:
                WritePod(type, value);
                break;
            case Kind::Enum:
                WriteEnum(type, value);
                break;
            case Kind::Template:
                WriteTemplate(type, value);
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

        void WriteStruct(const eti::Type& type, const void* value) noexcept
        {
            jsonWriter.WriteObjectBegin();
            for( const Property& property : type.Properties )
            {
                jsonWriter.WriteProperty(property.Variable.Name);
                void* propertyValue = property.UnSafeGetPtr(const_cast<void*>(value));
                Write(*property.Variable.Declaration.Type, propertyValue);
            }
            jsonWriter.WriteObjectEnd();
        }

        void WritePod(const eti::Type& type, const void* value) noexcept
        {
            switch (type.Id)
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

        void WriteEnum(const eti::Type& type, const void* value) noexcept
        {
            s64 enumValue = -1;
            switch (type.Parent->Id)
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

            if ( enumValue >= 0 && enumValue < (s64)type.EnumSize)
            {
                jsonWriter.WriteString(type.GetEnumValueName(enumValue));
            }
            else
            {
                jsonWriter.WriteString("invalid");
            }
        }

        void WriteTemplate(const eti::Type& type, const void* value) noexcept
        {
            // vector
            if (type.Name.starts_with("std::vector"))
            {
                jsonWriter.WriteArrayBegin();
                size_t size;
                type.GetMethod("GetSize")->UnSafeCall((void*)value, &size, {});
                const Type& itemType = *type.Templates[0];
                const Method* getAt = type.GetMethod("GetAt");
                for (size_t i = 0; i < size; ++i)
                {
                    void* ptr = nullptr;
                    void* args[1] = { &i };
                    getAt->UnSafeCall((void*)value, &ptr, args );
                    Write(*type.Templates[0], ptr);
                }
                jsonWriter.WriteArrayEnd();
            }

            // map
            if (type.Name.starts_with("std::map"))
            {
                // todo
                jsonWriter.WriteNull();
            }
        }

        JSON_WRITER& jsonWriter;

    };

    struct TypeReader
    {
        TypeReader(void* root, const Type* type) noexcept
        {
            rootDeclaration = { type, true };
            PushContext(root, &rootDeclaration);
        }

        void ObjectBegin() noexcept
        {
            if (!current->Value)
                return;

        }

        void ObjectEnd() noexcept
        {
            if (!current->Value)
                return;
        }

        void PropertyBegin(const string_view& key) noexcept
        {
            if (current->Value)
            {
                const Property* property = current->Declaration->Type->GetProperty(ToString(key));
                if (property)
                {
                    void* propertyPtr = ((u8*)current->Value) + property->Offset;
                    PushContext(propertyPtr, &property->Variable.Declaration);
                    return;
                }
            }

            PushContext(nullptr, nullptr);
        }

        void PropertyEnd() noexcept
        {
            PopContext();
        }

        void ArrayBegin() noexcept
        {
            if (!current->Value)
                return;
            if (current->IsVector)
                current->Declaration->Type->GetMethod("Clear")->UnSafeCall(current->Value, NoReturn, {});
        }

        void ArrayEnd() noexcept
        {
            if (!current->Value)
                return;
        }

        void ValueBool(bool b) noexcept
        {
            if (!current->Value)
                return;

            if ( *current->Declaration->Type == TypeOf<bool>())
            {
                *(bool*)current->Value = b;
            }
        }

        void ValueNull() noexcept
        {
            if (!current->Value)
                return;

            if (current->Declaration->IsPtr)
            {
                *(void**)current->Value = nullptr;
            }
        }

        void ValueString(const string_view& str) noexcept
        {
            if (!current->Value)
                return;

            if (current->Declaration->Type->Kind == Kind::Enum)
            {
                size_t enumValue = current->Declaration->Type->GetEnumValue(ToString(str));
                if (enumValue != InvalidIndex)
                {
                    switch (current->Declaration->Type->Parent->Id )
                    {
                        case GetTypeId<s8>():
                            *(s8*)current->Value = (s8)enumValue;
                            break;
                        case GetTypeId<s16>():
                            *(s16*)current->Value = (s16)enumValue;
                            break;
                        case GetTypeId<s32>():
                            *(s32*)current->Value = (s32)enumValue;
                            break;
                        case GetTypeId<s64>():
                            *(s64*)current->Value = (s64)enumValue;
                            break;
                        case GetTypeId<u8>():
                            *(u8*)current->Value = (u8)enumValue;
                            break;
                        case GetTypeId<u16>():
                            *(u16*)current->Value = (u16)enumValue;
                            break;
                        case GetTypeId<u32>():
                            *(u32*)current->Value = (u32)enumValue;
                            break;
                        case GetTypeId<u64>():
                            *(u64*)current->Value = enumValue;
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (*current->Declaration->Type == TypeOf<std::string>())
            {
                *((std::string*)current->Value) = ToString(str);
            }
            else if (*current->Declaration->Type == TypeOf<std::wstring>())
            {
                *((std::wstring*)current->Value) = ToWString(str);
            }
        }

        void ValueNumber(number value) noexcept
        {
            if (!current->Value)
                return;

            if (current->IsVector)
            {
                switch (current->Declaration->Type->Templates[0]->Id)
                {
                    case GetTypeId<s8>():
                        VectorAdd((s8)value);
                        break;
                    case GetTypeId<s16>():
                        VectorAdd((s16)value);
                        break;
                    case GetTypeId<s32>():
                        VectorAdd((s32)value);
                        break;
                    case GetTypeId<s64>():
                        VectorAdd((s64)value);
                        break;
                    case GetTypeId<u8>():
                        VectorAdd((u8)value);
                        break;
                    case GetTypeId<u16>():
                        VectorAdd((u16)value);
                        break;
                    case GetTypeId<u32>():
                        VectorAdd((u32)value);
                        break;
                    case GetTypeId<u64>():
                        VectorAdd((u64)value);
                        break;
                    case GetTypeId<f32>():
                        VectorAdd((f32)value);
                        break;
                    case GetTypeId<f64>():
                        VectorAdd(value);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch (current->Declaration->Type->Id)
                {
                    case GetTypeId<s8>():
                        *(s8*)current->Value = (s8)value;
                        break;
                    case GetTypeId<s16>():
                        *(s16*)current->Value = (s16)value;
                        break;
                    case GetTypeId<s32>():
                        *(s32*)current->Value = (s32)value;
                        break;
                    case GetTypeId<s64>():
                        *(s64*)current->Value = (s64)value;
                        break;
                    case GetTypeId<u8>():
                        *(u8*)current->Value = (u8)value;
                        break;
                    case GetTypeId<u16>():
                        *(u16*)current->Value = (u16)value;
                        break;
                    case GetTypeId<u32>():
                        *(u32*)current->Value = (u32)value;
                        break;
                    case GetTypeId<u64>():
                        *(u64*)current->Value = (u64)value;
                        break;
                    case GetTypeId<f32>():
                        *(f32*)current->Value = (f32)value;
                        break;
                    case GetTypeId<f64>():
                        *(f64*)current->Value = (f64)value;
                        break;
                    default:
                        break;
                }
            }
        }

    private:

        struct Context
        {
            void* Value = nullptr;
            const Declaration* Declaration = nullptr;
            bool IsVector = false;
            const Type* T1 = nullptr;
            const Type* T2 = nullptr;
            const Method* Add = nullptr;
            
        };

        void PushContext(void* value, const Declaration* declaration)
        {
            if (declaration->Type->Name.starts_with("std::vector"))
            {
                contexts.push_back({value, declaration, true, declaration->Type->Templates[0], nullptr, declaration->Type->GetMethod("Add")});
                current = &contexts[contexts.size() - 1];
            }
            else
            {
                contexts.push_back({ value, declaration, false });
                current = &contexts[contexts.size() - 1];
            }
        }

        void PopContext()
        {
            contexts.pop_back();
            current = contexts.size() ? &contexts[contexts.size() - 1] : nullptr;
        }

        template <typename T>
        void VectorAdd(const T& value)
        {
            EJSON_ASSERT(current->Add != nullptr, "not a vector");
            EJSON_ASSERT(*current->Declaration->Type->Templates[0] == TypeOf<T>(), "not vaid vector type" );
            const T* ptr = &value;
            void* args[1] = { (void*) &ptr};
            current->Add->UnSafeCall(current->Value, NoReturn, args);
        }

        Context* current = nullptr;
        Declaration rootDeclaration = {};

        std::vector<Context> contexts;
    };

    template <typename T>
    bool ReadType(string_view json, T& value, ParserError& error) noexcept
    {
        StringReader stringReader(json);
        TypeReader valueReader(&value, &TypeOf<T>());
        JsonReader jsonReader(valueReader, stringReader);
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
            valueWriter.Write(TypeOf<T>(), &value);
        }
        else
        {
            JsonWriter<StringWriter, true> jsonWriter(stringWriter);
            TypeWriter valueWriter(jsonWriter);
            valueWriter.Write(TypeOf<T>(), &value);
        }
    }

    template <typename T>
    void Write(const T& value, output_stream& stream, bool prettify = false) noexcept
    {
        StreamWriter streamWriter(stream);
        if (!prettify)
        {
            JsonWriter jsonWriter(streamWriter);
            TypeWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
        else
        {
            JsonWriter<StreamWriter, true> jsonWriter(streamWriter);
            TypeWriter valueWriter(jsonWriter);
            valueWriter.Write(value);
        }
    }
}
