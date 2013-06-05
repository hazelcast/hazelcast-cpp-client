//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_NULL_PORTABLE
#define HAZELCAST_NULL_PORTABLE

namespace hazelcast {
    namespace client {
        namespace serialization {
            class NullPortable {
            public:
                NullPortable(int factoryId, int classId);

                int factoryId;
                int classId;
            };

        }


    }
}


#endif //__NullPortable_H_
