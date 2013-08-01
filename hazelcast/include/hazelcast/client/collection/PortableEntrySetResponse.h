//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_PortableEntrySetResponse
#define HAZELCAST_PortableEntrySetResponse

#include "CollectionPortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class PortableEntrySetResponse : public Portable {
            public:
                PortableEntrySetResponse() {

                };

                PortableEntrySetResponse(const std::vector<std::pair<serialization::Data, serialization::Data > >& entrySet)
                :entrySet(entrySet) {

                };

                const std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet() const {
                    return entrySet;
                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getClassId() const {
                    return CollectionPortableHook::ENTRY_SET_RESPONSE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("s", entrySet.size());
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    for (int i = 0; i < entrySet.size(); ++i) {
                        entrySet[i].first.writeData(*out);
                        entrySet[i].second.writeData(*out);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size = reader.readInt("s");
                    entrySet.resize(size);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        entrySet[i].first.readData(*in);
                        entrySet[i].second.readData(*in);
                    }
                };

            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };

        }
    }
}


#endif //HAZELCAST_PortableEntrySetResponse
