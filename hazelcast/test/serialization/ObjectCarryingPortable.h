//
// Created by sancar koyunlu on 05/04/15.
//


#ifndef HAZELCAST_ObjectCarryingPortable
#define HAZELCAST_ObjectCarryingPortable

#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "TestSerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            template <typename T>
            class ObjectCarryingPortable : public serialization::Portable {
            public:
                ObjectCarryingPortable(){

                }

                ObjectCarryingPortable(T* object):object(object){

                }

                int getFactoryId() const{
                    return TestSerializationConstants::TEST_DATA_FACTORY;
                }

                int getClassId() const{
                    return TestSerializationConstants::OBJECT_CARRYING_PORTABLE;
                }

                void writePortable(serialization::PortableWriter& writer) const{
                    serialization::ObjectDataOutput& output = writer.getRawDataOutput();
                    output.writeObject<T>(object.get());
                };

                void readPortable(serialization::PortableReader& reader){
                    serialization::ObjectDataInput& input = reader.getRawDataInput();
                    object = input.readObject<T>();
                };

                virtual bool operator ==(const ObjectCarryingPortable& m) const{
                    if (this == &m)
                        return true;
                    if ((*object) != *(m.object))
                        return false;
                    return true;
                }

                virtual bool operator !=(const ObjectCarryingPortable& m) const{
                    return !(*this == m);
                }

                boost::shared_ptr<T> object;
            };
        }
    }
}

#endif //HAZELCAST_ObjectCarryingPortable
