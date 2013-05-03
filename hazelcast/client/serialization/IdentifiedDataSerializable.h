//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_IDENTIFIED_DATA_SERIALIZABLE
#define HAZELCAST_IDENTIFIED_DATA_SERIALIZABLE


namespace hazelcast {
    namespace client {
        namespace serialization {
            class IdentifiedDataSerializable {
            public:
                virtual int getFactoryId() const = 0;

                virtual int getId() const = 0;

            };

        }
    }
}


#endif //HAZELCAST_IDENTIFIED_DATA_SERIALIZABLE
