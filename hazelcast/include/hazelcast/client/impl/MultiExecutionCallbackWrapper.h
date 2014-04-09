////
//// Created by sancar koyunlu on 8/15/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//
//#ifndef HAZELCAST_MultiExecutionCallbackWrapper
//#define HAZELCAST_MultiExecutionCallbackWrapper
//
//#include "hazelcast/client/Member.h"
//#include "hazelcast/util/AtomicInt.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace impl {
//            template <typename Result, typename MultiExecutionCallback>
//            class HAZELCAST_API MultiExecutionCallbackWrapper {
//            public:
//                MultiExecutionCallbackWrapper(int membersSize, MultiExecutionCallback& callback)
//                :memberCount(membersSize)
//                , multiExecutionCallback(callback) {
//
//                };
//
//                void onResponse(const Member& member, const Result& result) {
//                    multiExecutionCallback.onResponse(member, result);
//                    values[member] = result;
//                    int waitingResponse = memberCount--;
//                    if (waitingResponse == 1) {
//                        multiExecutionCallback.onComplete(values);
//                    }
//                }
//
//            private:
//                std::map <Member, Result> values;
//                util::AtomicInt memberCount;
//                MultiExecutionCallback& multiExecutionCallback;
//
//            };
//        }
//    }
//}
//#endif //HAZELCAST_MultiExecutionCallbackWrapper

