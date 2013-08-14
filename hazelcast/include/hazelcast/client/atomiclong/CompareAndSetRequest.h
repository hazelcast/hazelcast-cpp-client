//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CompareAndSetRequest
#define HAZELCAST_CompareAndSetRequest

#include "AtomicLongRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class CompareAndSetRequest : public AtomicLongRequest {
            public:
                CompareAndSetRequest(const std::string& instanceName, long expect, long value)
                : AtomicLongRequest(instanceName, value)
                , expect(expect) {

                };

                int getClassId() const {
                    return AtomicLongPortableHook::COMPARE_AND_SET;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    AtomicLongRequest::writePortable(writer);
                    writer.writeLong("e", expect);
                };


                void readPortable(serialization::PortableReader& reader) {
                    AtomicLongRequest::readPortable(reader);
                    expect = reader.readLong("e");
                };
            private:
                long expect;

            };
        }
    }
}


#endif //HAZELCAST_CompareAndSetRequest
