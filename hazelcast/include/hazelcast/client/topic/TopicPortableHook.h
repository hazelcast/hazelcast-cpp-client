//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TopicPortableHook
#define HAZELCAST_TopicPortableHook

namespace hazelcast {
    namespace client {
        namespace topic {

            namespace TopicPortableHook {
                enum {
                    F_ID = -18,

                    PUBLISH = 1,
                    ADD_LISTENER = 2,
                    REMOVE_LISTENER = 3,
                    PORTABLE_MESSAGE = 4
                };
            };
        }
    }
}


#endif //HAZELCAST_TopicPortableHook
