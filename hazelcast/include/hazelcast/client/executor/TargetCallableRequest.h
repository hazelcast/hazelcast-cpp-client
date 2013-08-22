//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TargetCallableRequest
#define HAZELCAST_TargetCallableRequest

#include "Address.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "hazelcast/client/executor/DataSerializableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace executor {
            template <typename Callable>
            class TargetCallableRequest : public IdentifiedDataSerializable {
            public:
                TargetCallableRequest(std::string& name, Callable& callable, Address& address)
                :name(name)
                , callable(callable)
                , address(address) {

                }

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::TARGET_CALLABLE_REQUEST;
                }

                void writeData(serialization::ObjectDataOutput & writer) const {
                    writer.writeUTF(name);
                    writer.writeObject(&callable);
                    address.writeData(writer);
                }

                void readData(serialization::ObjectDataInput & reader) {
                    name = reader.readUTF();
                    callable = reader.readObject<Callable>();
                    address.readData(reader);
                }


            private:
                std::string &name;
                Callable& callable;
                Address& address;
            };
        }
    }
}

#endif //HAZELCAST_TargetCallableRequest

