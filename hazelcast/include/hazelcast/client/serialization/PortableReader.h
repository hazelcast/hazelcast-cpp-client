//
// Created by sancar koyunlu on 8/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_PortableReader
#define HAZELCAST_PortableReader

#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/serialization/pimpl/MorphingPortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            /**
            * Provides a mean of reading portable fields from a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableReader {
            public:
                /**
                * Internal Api constructor
                */
                PortableReader(pimpl::DefaultPortableReader *defaultPortableReader);

                /**
                * Internal Api constructor
                */
                PortableReader(pimpl::MorphingPortableReader *morphingPortableReader);

                /**
                * @param fieldName name of the field
                * @return the int value read
                * @throws IOException
                */
                int readInt(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the long value read
                * @throws IOException
                */
                long readLong(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the boolean value read
                * @throws IOException
                */
                bool readBoolean(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte value read
                * @throws IOException
                */
                byte readByte(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char value read
                * @throws IOException
                */
                char readChar(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double value read
                * @throws IOException
                */
                double readDouble(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float value read
                * @throws IOException
                */
                float readFloat(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the short value read
                * @throws IOException
                */
                short readShort(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the utf string value read
                * @throws IOException
                */
                std::string readUTF(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte array value read
                * @throws IOException
                */
                std::vector<byte> readByteArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char array value read
                * @throws IOException
                */
                std::vector<char> readCharArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int array value read
                * @throws IOException
                */
                std::vector<int> readIntArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the long array value read
                * @throws IOException
                */
                std::vector<long> readLongArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double array value read
                * @throws IOException
                */
                std::vector<double> readDoubleArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float array value read
                * @throws IOException
                */
                std::vector<float> readFloatArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the short array value read
                * @throws IOException
                */
                std::vector<short> readShortArray(const char *fieldName);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @return the portable value read
                * @throws IOException
                */
                template<typename T>
                boost::shared_ptr<T> readPortable(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortable<T>(fieldName);
                    return morphingPortableReader->readPortable<T>(fieldName);
                };

                /**
                * @tparam type of the portable class in array
                * @param fieldName name of the field
                * @return the portable array value read
                * @throws IOException
                */
                template<typename T>
                std::vector<T> readPortableArray(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortableArray<T>(fieldName);
                    return morphingPortableReader->readPortableArray<T>(fieldName);
                };

                /**
                * @see PortableWriter#getRawDataOutput
                *
                * Note that portable fields can not read after getRawDataInput() is called. In case this happens,
                * IOException will be thrown.
                *
                * @return rawDataInput
                * @throws IOException
                */
                ObjectDataInput &getRawDataInput();

                /**
                * Internal Api. Should not be called by end user.
                */
                void end();

            private:
                pimpl::DefaultPortableReader *defaultPortableReader;
                pimpl::MorphingPortableReader *morphingPortableReader;
                bool isDefaultReader;

            };
        }
    }
}


#endif //HAZELCAST_PortableReader

