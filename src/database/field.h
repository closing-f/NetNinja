#ifndef _FIELD_H
#define _FIELD_H

#include <mysql.h> 
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <string>

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

class Field
{
private:
    uint32 length;          // Length (prepared strings only)
    void* value;            // Actual data in memory
    enum_field_types type;  // Field type
    bool raw;               // Raw bytes? (Prepared statement or ad hoc)

    void Init()
    {
        value = NULL;
        type = MYSQL_TYPE_NULL;
        length = 0;
        raw = false;
    }

    public:

        bool GetBool() const // Wrapper, actually gets integer
        {
            return GetUInt8() == 1 ? true : false;
        }

        uint8 GetUInt8() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_TINY))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetUInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<uint8*>(value);
            return static_cast<uint8>(atol((char*)value));
        }

        int8 GetInt8() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_TINY))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<int8*>(value);
            return static_cast<int8>(atol((char*)value));
        }

        uint16 GetUInt16() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_SHORT) && !IsType(MYSQL_TYPE_YEAR))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetUInt16() on non-smallint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<uint16*>(value);
            return static_cast<uint16>(atol((char*)value));
        }

        int16 GetInt16() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_SHORT) && !IsType(MYSQL_TYPE_YEAR))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetInt16() on non-smallint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<int16*>(value);
            return static_cast<int16>(atol((char*)value));
        }

        uint32 GetUInt32() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_INT24) && !IsType(MYSQL_TYPE_LONG))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetUInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<uint32*>(value);
            return static_cast<uint32>(atol((char*)value));
        }

        int32 GetInt32() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_INT24) && !IsType(MYSQL_TYPE_LONG))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<int32*>(value);
            return static_cast<int32>(atol((char*)value));
        }

        uint64 GetUInt64() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_LONGLONG) && !IsType(MYSQL_TYPE_BIT))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetUInt64() on non-bigint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<uint64*>(value);
            return static_cast<uint64>(atol((char*)value));
        }

        int64 GetInt64() const
        {
            if (!value)
                return 0;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_LONGLONG) && !IsType(MYSQL_TYPE_BIT))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetInt64() on non-bigint field. Using type: %s.", FieldTypeToString(type));
                return 0;
            }
            #endif

            if (raw)
                return *reinterpret_cast<int64*>(value);
            return static_cast<int64>(strtol((char*)value, NULL, 10));
        }

        float GetFloat() const
        {
            if (!value)
                return 0.0f;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_FLOAT))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetFloat() on non-float field. Using type: %s.", FieldTypeToString(type));
                return 0.0f;
            }
            #endif

            if (raw)
                return *reinterpret_cast<float*>(value);
            return static_cast<float>(atof((char*)value));
        }

        double GetDouble() const
        {
            if (!value)
                return 0.0f;

            #ifdef TRINITY_DEBUG
            if (!IsType(MYSQL_TYPE_DOUBLE))
            {
                TC_LOG_WARN("sql.sql", "Warning: GetDouble() on non-double field. Using type: %s.", FieldTypeToString(type));
                return 0.0f;
            }
            #endif

            if (raw)
                return *reinterpret_cast<double*>(value);
            return static_cast<double>(atof((char*)value));
        }

        char const* GetCString() const
        {
            if (!value)
                return NULL;

            #ifdef TRINITY_DEBUG
            if (IsNumeric())
            {
                TC_LOG_WARN("sql.sql", "Error: GetCString() on numeric field. Using type: %s.", FieldTypeToString(type));
                return NULL;
            }
            #endif
            return static_cast<char const*>(value);

        }

        std::string GetString() const
        {
            if (!value)
                return "";

            if (raw)
            {
                char const* string = GetCString();
                if (!string)
                    string = "";
                return std::string(string, length);
            }
            return std::string((char*)value);
        }

        bool IsNull() const
        {
            return value == NULL;
        }

    public:
        Field()
		{
			Init();
		}
        ~Field()
		{
			CleanUp();
		}
        Field(const Field &other)
		{
			Init();
			if (other.raw)
				SetByteValue(other.value, other.length, other.type, other.length);
			else
				SetStructuredValue((char*)other.value, other.type);
		}
        Field & operator = (const Field &other)
		{
			if (this == &other)
				return *this; 

			Init();
			if (other.raw)
				SetByteValue(other.value, other.length, other.type, other.length);
			else
				SetStructuredValue((char*)other.value, other.type);
			return *this;
		}

        void SetByteValue(void const* newValue, size_t const newSize, enum_field_types newType, uint32 length)
		{
			if (value)
				CleanUp();

			// This value stores raw bytes that have to be explicitly cast later
			if (newValue)
			{
				value = new char[newSize];
				memcpy(value, newValue, newSize);
				length = length;
			}
			type = newType;
			raw = true;
		}

        void SetStructuredValue(char* newValue, enum_field_types newType)
		{
			if (value)
				CleanUp();

			// This value stores somewhat structured data that needs function style casting
			if (newValue)
			{
				size_t size = strlen(newValue);
				value = new char [size+1];
				strcpy((char*)value, newValue);
				length = size;
			}

			type = newType;
			raw = false;
		}

        void CleanUp()
        {
            delete[] ((char*)value);
            value = NULL;
        }

        static size_t SizeForType(MYSQL_FIELD* field)
        {
            switch (field->type)
            {
                case MYSQL_TYPE_NULL:
                    return 0;
                case MYSQL_TYPE_TINY:
                    return 1;
                case MYSQL_TYPE_YEAR:
                case MYSQL_TYPE_SHORT:
                    return 2;
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_FLOAT:
                    return 4;
                case MYSQL_TYPE_DOUBLE:
                case MYSQL_TYPE_LONGLONG:
                case MYSQL_TYPE_BIT:
                    return 8;

                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_DATETIME:
                    return sizeof(MYSQL_TIME);

                case MYSQL_TYPE_TINY_BLOB:
                case MYSQL_TYPE_MEDIUM_BLOB:
                case MYSQL_TYPE_LONG_BLOB:
                case MYSQL_TYPE_BLOB:
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                    return field->max_length + 1;

                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_NEWDECIMAL:
                    return 64;

                case MYSQL_TYPE_GEOMETRY:
                /*
                Following types are not sent over the wire:
                MYSQL_TYPE_ENUM:
                MYSQL_TYPE_SET:
                */
                default:
                    //TC_LOG_WARN("sql.sql", "SQL::SizeForType(): invalid field type %u", uint32(field->type));
                    return 0;
            }
        }

        bool IsType(enum_field_types type) const
        {
            return type == type;
        }

        bool IsNumeric() const
        {
            return (type == MYSQL_TYPE_TINY ||
                    type == MYSQL_TYPE_SHORT ||
                    type == MYSQL_TYPE_INT24 ||
                    type == MYSQL_TYPE_LONG ||
                    type == MYSQL_TYPE_FLOAT ||
                    type == MYSQL_TYPE_DOUBLE ||
                    type == MYSQL_TYPE_LONGLONG );
        }
};

#endif