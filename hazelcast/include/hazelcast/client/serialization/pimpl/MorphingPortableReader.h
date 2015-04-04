//
//  MorphingPortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_MORPHING_PORTABLE_READER
#define HAZELCAST_MORPHING_PORTABLE_READER

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/pimpl/PortableReaderBase.h"

#include <string>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            namespace pimpl {

                class SerializerHolder;

                class HAZELCAST_API MorphingPortableReader : public PortableReaderBase {
                public:

                    MorphingPortableReader(PortableContext &portableContext, DataInput &input, boost::shared_ptr<ClassDefinition> cd);

                    int readInt(const char *fieldName, bool skipTypeCheck = false);

                    long readLong(const char *fieldName, bool skipTypeCheck = false);

                    bool readBoolean(const char *fieldName, bool skipTypeCheck = false);

                    byte readByte(const char *fieldName, bool skipTypeCheck = false);

                    char readChar(const char *fieldName, bool skipTypeCheck = false);

                    double readDouble(const char *fieldName, bool skipTypeCheck = false);

                    float readFloat(const char *fieldName, bool skipTypeCheck = false);

                    short readShort(const char *fieldName, bool skipTypeCheck = false);

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
                        boost::shared_ptr<T> portableInstance(new T);

                        Portable * p = portableInstance.get();
                        getPortableInstance(fieldName, p);
                        return portableInstance;
                    };

                    template<typename T>
                    std::vector<T> readPortableArray(const char *fieldName) {
                        PortableReaderBase::setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                        int len = dataInput.readInt();
                        std::vector<T> portables(len);

                        std::vector<Portable *> baseArray(len);
                        int i = 0;
                        for (typename std::vector<T>::iterator it = portables.begin();
                             portables.end() != it; ++it) {
                            baseArray[i++] = (Portable *)(&(*it));
                        }

                        getPortableInstancesArray(fieldName, baseArray);
                        return portables;
                    };
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_MORPHING_PORTABLE_READER */


