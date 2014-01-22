//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecutorDataSerializableHook
#define HAZELCAST_ExecutorDataSerializableHook


namespace hazelcast {
    namespace client {
        namespace executor {
            namespace DataSerializableHook {
                enum {
                    F_ID = -13,

                    CALLABLE_TASK = 0,
                    MEMBER_CALLABLE_TASK = 1,
                    RUNNABLE_ADAPTER = 2,

                    TARGET_CALLABLE_REQUEST = 6,
                    LOCAL_TARGET_CALLABLE_REQUEST = 7,
                    IS_SHUTDOWN_REQUEST = 9
                };
            }

        }
    }
}


#endif //HAZELCAST_DataSerializableHook
