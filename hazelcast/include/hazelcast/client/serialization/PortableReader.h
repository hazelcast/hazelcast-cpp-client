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
            class HAZELCAST_API PortableReader {
            public:
                PortableReader(pimpl::DefaultPortableReader *defaultPortableReader);

                PortableReader(pimpl::MorphingPortableReader *morphingPortableReader);

                int readInt(const char *fieldName);

                long readLong(const char *fieldName);

                bool readBoolean(const char *fieldName);

                byte readByte(const char *fieldName);

                char readChar(const char *fieldName);

                double readDouble(const char *fieldName);

                float readFloat(const char *fieldName);

                short readShort(const char *fieldName);

                std::string readUTF(const char *fieldName);

                std::vector<byte> readByteArray(const char *fieldName);

                std::vector<char> readCharArray(const char *fieldName);

                std::vector<int> readIntArray(const char *fieldName);

                std::vector<long> readLongArray(const char *fieldName);

                std::vector<double> readDoubleArray(const char *fieldName);

                std::vector<float> readFloatArray(const char *fieldName);

                std::vector<short> readShortArray(const char *fieldName);

                template<typename T>
                boost::shared_ptr<T> readPortable(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortable<T>(fieldName);
                    return morphingPortableReader->readPortable<T>(fieldName);
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortableArray<T>(fieldName);
                    return morphingPortableReader->readPortableArray<T>(fieldName);
                };

                ObjectDataInput& getRawDataInput();

                void end();

            private:
                bool isDefaultReader;
                pimpl::MorphingPortableReader *morphingPortableReader;
                pimpl::DefaultPortableReader *defaultPortableReader;

            };
        }
    }
}


#endif //HAZELCAST_PortableReader
