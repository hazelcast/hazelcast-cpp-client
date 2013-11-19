//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TargetCallableRequest
#define HAZELCAST_TargetCallableRequest

#include "hazelcast/client/Address.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/executor/DataSerializableHook.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace executor {
            template <typename Callable>
            class TargetCallableRequest : public impl::IdentifiedDataSerializableRequest {
            public:
                TargetCallableRequest(const std::string &name, Callable &callable, Address &address)
                :name(name)
                , callable(&callable)
                , address(&address) {

                }

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::TARGET_CALLABLE_REQUEST;
                }

                void writeData(serialization::ObjectDataOutput &writer) const {
                    writer.writeUTF(name);
                    writer.writeObject<Callable>(callable);
                    address->writeData(writer);
                }

            private:
                const std::string &name;
                Callable *callable;
                Address *address;
            };
        }
    }
}

#endif //HAZELCAST_TargetCallableRequest

