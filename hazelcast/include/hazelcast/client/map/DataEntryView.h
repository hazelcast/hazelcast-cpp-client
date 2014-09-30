//
// Created by sancar koyunlu on 20/02/14.
//


#ifndef HAZELCAST_DataEntryView
#define HAZELCAST_DataEntryView

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API DataEntryView : public impl::IdentifiedDataSerializableResponse {
            public:

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput &in);

                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                long cost;
                long creationTime;
                long expirationTime;
                long hits;
                long lastAccessTime;
                long lastStoredTime;
                long lastUpdateTime;
                long version;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataEntryView

