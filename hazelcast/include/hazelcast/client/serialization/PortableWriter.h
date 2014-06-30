//
// Created by sancar koyunlu on 8/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_PortableWriter
#define HAZELCAST_PortableWriter

#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            /**
            * Provides a mean of writing portable fields to a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableWriter {
            public:
                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter);

                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeInt(const char *fieldName, int value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeLong(const char *fieldName, long value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeBoolean(const char *fieldName, bool value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeByte(const char *fieldName, byte value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeChar(const char *fieldName, int value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeDouble(const char *fieldName, double value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeFloat(const char *fieldName, float value);

                /**
                * @param fieldName name of the field
                * @param value     int value to be written
                * @throws IOException
                */
                void writeShort(const char *fieldName, short value);

                /**
                * @param fieldName name of the field
                * @param value     utf string value to be written
                * @throws IOException
                */
                void writeUTF(const char *fieldName, const std::string &value);

                /**
                * @param fieldName name of the field
                * @param values     byte array to be written
                * @throws IOException
                */
                void writeByteArray(const char *fieldName, const std::vector<byte> &values);

                /**
                * @param fieldName name of the field
                * @param values     char array to be written
                * @throws IOException
                */
                void writeCharArray(const char *fieldName, const std::vector<char> &values);

                /**
                * @param fieldName name of the field
                * @param values     short array to be written
                * @throws IOException
                */
                void writeShortArray(const char *fieldName, const std::vector<short> &values);

                /**
                * @param fieldName name of the field
                * @param values     int array to be written
                * @throws IOException
                */
                void writeIntArray(const char *fieldName, const std::vector<int> &values);

                /**
                * @param fieldName name of the field
                * @param values     long array to be written
                * @throws IOException
                */
                void writeLongArray(const char *fieldName, const std::vector<long> &values);

                /**
                * @param fieldName name of the field
                * @param values    float array to be written
                * @throws IOException
                */
                void writeFloatArray(const char *fieldName, const std::vector<float> &values);

                /**
                * @param fieldName name of the field
                * @param values    double array to be written
                * @throws IOException
                */
                void writeDoubleArray(const char *fieldName, const std::vector<double> &values);

                /**
                * Internal api , should not be called by end user.
                */
                void end();

                /**
                * To write a null portable value, user needs to provide class and factoryIds of related class.
                *
                * @tparam type of the portable field
                * @param fieldName name of the field
                * @throws IOException
                */
                template<typename T>
                void writeNullPortable(const char *fieldName) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writeNullPortable<T>(fieldName);
                    return classDefinitionWriter->writeNullPortable<T>(fieldName);
                }

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param portable  Portable to be written
                * @throws IOException
                */
                template<typename T>
                void writePortable(const char *fieldName, const T &portable) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortable(fieldName, portable);
                    return classDefinitionWriter->writePortable(fieldName, portable);

                };

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param values portable array to be written
                * @throws IOException
                */
                template<typename T>
                void writePortableArray(const char *fieldName, const std::vector<T> &values) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortableArray(fieldName, values);
                    return classDefinitionWriter->writePortableArray(fieldName, values);
                };

                /**
                * After writing portable fields, one can write remaining fields in old fashioned way consecutively at the end
                * of stream. User should not that after getting rawDataOutput trying to write portable fields will result
                * in IOException
                *
                * @return ObjectDataOutput
                * @throws IOException
                */
                ObjectDataOutput &getRawDataOutput();

            private:
                pimpl::DefaultPortableWriter *defaultPortableWriter;
                pimpl::ClassDefinitionWriter *classDefinitionWriter;
                bool isDefaultWriter;
            };
        }
    }
}

#endif //HAZELCAST_PortableWriter

