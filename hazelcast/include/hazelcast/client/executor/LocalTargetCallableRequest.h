////
//// Created by sancar koyunlu on 9/6/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//#ifndef HAZELCAST_LocalTargetCallableRequest
//#define HAZELCAST_LocalTargetCallableRequest
//
//#include "hazelcast/client/Address.h"
//#include "hazelcast/client/serialization/ObjectDataOutput.h"
//#include "hazelcast/client/executor/DataSerializableHook.h"
//#include "hazelcast/client/impl/IdentifiedDataSerializableRequest.h"
//#include <string>
//
//namespace hazelcast {
//    namespace client {
//        namespace executor {
//            template <typename Callable>
//            class LocalTargetCallableRequest : public impl::IdentifiedDataSerializableRequest {
//            public:
//                LocalTargetCallableRequest(const std::string &name, Callable &callable)
//                :name(name)
//                , callable(callable) {
//
//                }
//
//                int getFactoryId() const {
//                    return DataSerializableHook::F_ID;
//                }
//
//                int getClassId() const {
//                    return DataSerializableHook::LOCAL_TARGET_CALLABLE_REQUEST;
//                }
//
//                void writeData(serialization::ObjectDataOutput &writer) const {
//                    writer.writeUTF(name);
//                    writer.writeObject<Callable>(&callable);
//                }
//
//            private:
//                std::string name;
//                Callable callable;
//            };
//        }
//    }
//}
//
//#endif //HAZELCAST_LocalTargetCallableRequest

