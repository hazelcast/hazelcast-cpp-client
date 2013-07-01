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
            class PortableEntrySetResponse {
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

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CollectionPortableHook::ENTRY_SET_RESPONSE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["s"] << entrySet.size();
                    for (int i = 0; i < entrySet.size(); ++i) {
                        writer << entrySet[i].first;
                        writer << entrySet[i].second;
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size;
                    reader["s"] >> size;
                    entrySet.resize(size);
                    for (int i = 0; i < size; ++i) {
                        reader >> entrySet[i].first;
                        reader >> entrySet[i].second;
                    }
                };

            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };

        }
    }
}


#endif //HAZELCAST_PortableEntrySetResponse
