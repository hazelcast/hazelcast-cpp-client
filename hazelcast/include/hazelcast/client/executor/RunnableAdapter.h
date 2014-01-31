////
//// Created by sancar koyunlu on 8/15/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//
//#ifndef HAZELCAST_RunnableAdapter
//#define HAZELCAST_RunnableAdapter
//
//#include "hazelcast/client/impl/IdentifiedDataSerializableRequest.h"
//#include "hazelcast/client/serialization/ObjectDataOutput.h"
//#include "hazelcast/client/serialization/ObjectDataInput.h"
//#include "hazelcast/client/executor/DataSerializableHook.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace executor {
//            template<typename Runnable>
//            class RunnableAdapter : public impl::IdentifiedDataSerializableRequest {
//            public:
//                RunnableAdapter(Runnable& runnable)
//                :runnable(runnable) {
//
//                }
//
//                int getFactoryId() const {
//                    return DataSerializableHook::F_ID;
//                }
//
//                int getClassId() const {
//                    return DataSerializableHook::RUNNABLE_ADAPTER;
//                }
//
//                void writeData(serialization::ObjectDataOutput & writer) const {
//                    writer.writeObject<Runnable>(&runnable);
//                }
//
//            private:
//                Runnable& runnable;
//
//
//            };
//        }
//    }
//}
//
//#endif //HAZELCAST_RunnableAdapter
//
