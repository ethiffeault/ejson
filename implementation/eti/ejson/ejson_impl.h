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

        void WriteStruct(const eti::Type& type, const void* value)
        {
            jsonWriter.WriteObjectBegin();
            for( const Property& property : type.Properties )
            {
                jsonWriter.WriteProperty(property.Variable.Name);
                void* propertyValue = property.UnSafeGetPtr(const_cast<void*>(value));
                Write(property.Variable.Declaration.Type, propertyValue);
            }
            jsonWriter.WriteObjectEnd();
        }

        void WritePod(const eti::Type& type, const void* value)
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

        void WriteEnum(const eti::Type& type, const void* value)
        {
            // todo
        }

        void WriteTemplate(const eti::Type& type, const void* value)
        {
            // todo
        }

        JSON_WRITER& jsonWriter;

    };

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
    inline void Write(const T& value, output_stream& stream, bool prettify /*= false*/) noexcept
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
