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
            case Kind::Template:
                WriteTemplate(declaration, value);
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

        void WriteTemplate(const eti::Declaration& declaration, const void* value) noexcept
        {
            // vector
            if (declaration.Type->Name.starts_with("std::vector"))
            {
                jsonWriter.WriteArrayBegin();
                size_t size;
                declaration.Type->GetMethod("GetSize")->UnSafeCall((void*)value, &size, {});
                const Type& itemType = *declaration.Type->Templates[0].Type;
                const Method* getAt = declaration.Type->GetMethod("GetAt");
                for (size_t i = 0; i < size; ++i)
                {
                    if (declaration.Type->Templates[0].IsPtr)
                    {
                        void** ptr = nullptr;
                        void* args[1] = { &i };
                        getAt->UnSafeCall((void*)value, &ptr, args);
                        Write(declaration.Type->Templates[0], *ptr);
                    }
                    else
                    {
                        void* ptr = nullptr;
                        void* args[1] = { &i };
                        getAt->UnSafeCall((void*)value, &ptr, args);
                        Write(declaration.Type->Templates[0], ptr);

                    }
                }
                jsonWriter.WriteArrayEnd();
            }

            // map
            if (declaration.Type->Name.starts_with("std::map"))
            {
                // todo
                jsonWriter.WriteNull();
            }
        }

        JSON_WRITER& jsonWriter;

    };

    struct TypeReader
    {
        TypeReader(void* root, const Declaration& declaration) noexcept
        {
            PushContext(root, declaration);
        }

        void* TryGetRootValue() const
        {
            if(current != nullptr && contexts.size() == 1)
                return current->Value;
            return nullptr;
        }

        ~TypeReader()
        {
            PopContext();
        }

        void ObjectBegin() noexcept
        {
        }

        void ObjectEnd() noexcept
        {
        }

        void PropertyBegin(const string_view& key) noexcept
        {
            EJSON_ASSERT(pendingType == false, "invalid pending status");

            if (current->Value == nullptr)
            {
                pendingKey = key;
                pendingType = true;
                return;
            }
            else
            {
                if (current->Value != nullptr)
                {
                    const Property* property = current->Declaration.Type->GetProperty(ToString(key));
                    if (property)
                    {
                        void* propertyPtr = ((u8*)current->Value) + property->Offset;
                        PushContext(propertyPtr, property->Variable.Declaration);
                        return;
                    }
                }
            }
            PushInvalid();
        }

        void PropertyEnd() noexcept
        {
            PopContext();
        }

        void ArrayBegin() noexcept
        {
            if (ValueBegin())
            {
                EJSON_ASSERT(current->Type == ContextType::Vector, "should be vector");

                if (current->Value == nullptr)
                {
                    // inside vector or map
                    EJSON_ASSERT(contexts.size() > 1, "internal error");
                    Context* parentContext = &contexts[contexts.size() - 2];
                    if (parentContext->Declaration.Type->Name.starts_with("std::vector"))
                    {
                        Context* vectorContext = parentContext;
                        const Type* vectorType = vectorContext->Declaration.Type;
                        const Method* addDefault = vectorType->GetMethod("AddDefault");

                        if (current->Declaration.IsPtr)
                        {
                            if (current->Value == nullptr)
                            {
                                current->Value = current->Declaration.Type->New();
                            }
                            void** addValue = nullptr;
                            addDefault->UnSafeCall(vectorContext->Value, &addValue, {});
                            *addValue = current->Value;
                        }
                        else
                        {
                            void* addValue = nullptr;
                            addDefault->UnSafeCall(vectorContext->Value, &addValue, {});
                            current->Value = addValue;
                        }
                    }
                    else if ( parentContext->Declaration.Type->Name.starts_with("std::map"))
                    {
                        EJSON_ERROR("not implemented");
                    }
                    else
                    {
                        EJSON_ERROR("internal errror");
                    }
                }
                else
                {
                    // clear vector if already exist
                    if (current->Value != nullptr)
                        current->Declaration.Type->GetMethod("Clear")->UnSafeCall(current->Value, NoReturn, {});                    
                }



                PushContext(nullptr, current->Declaration.Type->Templates[0]);
            }
            else
            {
                PushInvalid();
            }
        }

        void ArrayEnd() noexcept
        {
            PopContext();
        }

        void ValueBool(bool b) noexcept
        {
            if (!ValueBegin())
                return;

            if ( *current->Declaration.Type == TypeOf<bool>())
            {
                *(bool*)current->Value = b;
            }
        }

        void ValueNull() noexcept
        {
            if (!ValueBegin())
                return;

            if (current->Declaration.IsPtr)
            {
                *(void**)current->Value = nullptr;
            }
            ValueEnd();
        }

        void ValueString(const string_view& str) noexcept
        {
            if (!ValueBegin())
                return;

            if (current->Declaration.Type->Kind == Kind::Enum)
            {
                size_t enumValue = current->Declaration.Type->GetEnumValue(ToString(str));
                if (enumValue != InvalidIndex)
                {
                    switch (current->Declaration.Type->Parent->Id )
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
            else if (*current->Declaration.Type == TypeOf<std::string>())
            {
                *((std::string*)current->Value) = ToString(str);
            }
            else if (*current->Declaration.Type == TypeOf<std::wstring>())
            {
                *((std::wstring*)current->Value) = ToWString(str);
            }
            ValueEnd();
        }

        void ValueNumber(number value) noexcept
        {
            if (!ValueBegin())
                return;

            switch (current->Declaration.Type->Id)
            {
                case GetTypeId<s8>():
                    SetTypedValue((s8)value);
                    break;
                case GetTypeId<s16>():
                    SetTypedValue((s16)value);
                    break;
                case GetTypeId<s32>():
                    SetTypedValue((s32)value);
                    break;
                case GetTypeId<s64>():
                    SetTypedValue((s64)value);
                    break;
                case GetTypeId<u8>():
                    SetTypedValue((u8)value);
                    break;
                case GetTypeId<u16>():
                    SetTypedValue((u16)value);
                    break;
                case GetTypeId<u32>():
                    SetTypedValue((u32)value);
                    break;
                case GetTypeId<u64>():
                    SetTypedValue((u64)value);
                    break;
                case GetTypeId<f32>():
                    SetTypedValue((f32)value);
                    break;
                case GetTypeId<f64>():
                    SetTypedValue(value);
                    break;
                default:
                    break;
            }

            ValueEnd();
        }

    private:

        enum class ContextType : u8
        {
            Invalid,
            Value,
            Vector,
            Map
        };

        struct Context
        {
            ContextType Type = ContextType::Invalid;
            void* Value = nullptr;
            Declaration Declaration;
        };

        bool pendingType = false;
        string pendingKey = {};


        bool ValueBegin(string_view typeName = {})
        {
            if (current->Type == ContextType::Invalid)
                return false;

            HandlePendingType(typeName);

            return true;
        }

        void ValueEnd()
        {
        }

        void PushInvalid()
        {
            Context context;
            context.Type = ContextType::Invalid;
            contexts.push_back(context);
        }

        void PushContext(void* value, const Declaration& declaration)
        {
            if (declaration.Type->Name.starts_with("std::vector"))
            {
                Context context;
                context.Type = ContextType::Vector;
                context.Value = value;
                context.Declaration = declaration;
                contexts.push_back(context);
            }
            else
            {
                Context context;
                context.Type = ContextType::Value;
                context.Value = value;
                context.Declaration = declaration;
                contexts.push_back(context);
            }

            current = &contexts[contexts.size() - 1];
        }

        void PopContext()
        {
            contexts.pop_back();
            current = contexts.size() ? &contexts[contexts.size() - 1] : nullptr;
        }

        void HandlePendingType(string_view typeName = {})
        {
            if (pendingType)
            {
                if (current->Value == nullptr)
                {
                    if (pendingKey == EJSON_TEXT("@type"))
                    {
                        const Type* type = eti::Repository::Instance().GetType(ToString(typeName));
                        if (type != nullptr)
                        {
                            if (IsA(*type, *current->Declaration.Type))
                                current->Value = type->New();
                        }
                    }

                    if (current->Value == nullptr)
                        current->Value = current->Declaration.Type->New();
                }

                if (current->Value)
                {
                    const Property* property = current->Declaration.Type->GetProperty(ToString(pendingKey));
                    if (property)
                    {
                        void* propertyPtr = ((u8*)current->Value) + property->Offset;
                        pendingKey = {};
                        PushContext(propertyPtr, property->Variable.Declaration);
                        return;
                    }
                }

                pendingKey = {};
                PushInvalid();
            }
            pendingType = false;
        }

        template<typename T>
        void SetTypedValue(T value) noexcept
        {
            if (current->Value == nullptr)
            {
                Context* parentContext = &contexts[contexts.size() - 2];

                if (parentContext->Declaration.Type->Name.starts_with("std::vector"))
                {
                    // vector or map
                    Context* vectorContext = parentContext;
                    const Type* vectorType = vectorContext->Declaration.Type;
                    const Method* addDefault = vectorType->GetMethod("AddDefault");

                    if (current->Declaration.IsPtr)
                    {
                        if (current->Value == nullptr)
                        {
                            current->Value = current->Declaration.Type->New();
                            *(T*)current->Value = value;
                        }
                        void** addValue = nullptr;
                        addDefault->UnSafeCall(vectorContext->Value, &addValue, {});
                        *addValue = current->Value;
                    }
                    else
                    {
                        void* addValue = nullptr;
                        addDefault->UnSafeCall(vectorContext->Value, &addValue, {});
                        *(T*)addValue = value;
                    }
                }
                else if (parentContext->Declaration.Type->Name.starts_with("std::map"))
                {
                    EJSON_ERROR("not implemented");
                }
                else
                {
                    EJSON_ERROR("internal errror");
                }
            }
            else if (current->Type == ContextType::Value)
            {
                if (current->Declaration.IsPtr)
                {
                    if (current->Value == nullptr)
                    {
                        current->Value = current->Declaration.Type->New();
                    }
                }
                *(T*)current->Value = value;
            }
        }

        Context* current = nullptr;

        std::vector<Context> contexts;
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

        void* ptr = nullptr;

        if constexpr ( std::is_pointer_v<T> )
            ptr = value;
        else
            ptr = &value;

        TypeReader typeReader(ptr, eti::internal::MakeDeclaration<T>());
        JsonReader jsonReader(typeReader, stringReader);
        if (jsonReader.Parse())
        {
            if constexpr ( std::is_pointer_v<T> )
                value = (T)typeReader.TryGetRootValue();
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
