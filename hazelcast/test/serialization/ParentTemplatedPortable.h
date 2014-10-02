//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_ParentTemplatedPortable
#define HAZELCAST_ParentTemplatedPortable

#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            template<typename T>
            class ParentTemplatedPortable : public serialization::Portable {
            public:
                ParentTemplatedPortable(){

                }

                ParentTemplatedPortable(T* child):child(child){

                }

                virtual ~ParentTemplatedPortable() {

                }

                int getFactoryId() const {
                    return 1;
                }

                int getClassId() const {
                    return 1;
                }

                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writePortable<T>("c", *child);
                }

                void readPortable(serialization::PortableReader& reader) {
                    child = reader.readPortable<T>("c");
                }

                boost::shared_ptr<T> child;
            };
        }
    }
}


#endif //HAZELCAST_ParentTemplatedPortable
