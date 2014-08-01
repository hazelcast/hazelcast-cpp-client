//
// Created by sancar koyunlu on 01/08/14.
//


#ifndef HAZELCAST_MapIsEmptyRequest
#define HAZELCAST_MapIsEmptyRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API MapIsEmptyRequest : public impl::ClientRequest {
            public:
                MapIsEmptyRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MapIsEmptyRequest
