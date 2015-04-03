//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_READER
#define HAZELCAST_PORTABLE_READER

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "PortableReaderBase.h"

#include <string>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            namespace pimpl {
                class PortableContext;

                class PortableReaderBase;

                class HAZELCAST_API DefaultPortableReader : public PortableReaderBase {
                public:

                    DefaultPortableReader(PortableContext &portableContext, DataInput &input, boost::shared_ptr<ClassDefinition> cd);

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

                        Portable *baseArray[len];
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

#endif /* HAZELCAST_PORTABLE_READER */

