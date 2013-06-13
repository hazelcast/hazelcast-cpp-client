//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_COLLECTION
#define HAZELCAST_PORTABLE_COLLECTION


#include "ProtocolConstants.h"
#include "Data.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableCollection {
            public:
                PortableCollection() {

                };

                PortableCollection(std::vector<serialization::Data>& collection)
                //TODO not efficent  vector<Data> is copied
                :collection(collection) {

                };

                //TODO not efficent  vector<Data> is copied
                std::vector<serialization::Data> getCollection() {
                    return collection;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
                };

                int getClassId() const {
                    return protocol::SpiConstants::COLLECTION;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["l"] << true;
                    writer["s"] << collection.size();
                    std::vector<serialization::Data>::iterator it;
                    for (it = collection.begin(); it != collection.end(); ++it) {
                        writer << (*it);
                    }

                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    bool isList = true;
                    reader["l"] >> isList;
                    int size = 0;
                    reader["s"] >> size;
                    if (size < 0)
                        return;
                    collection.resize(size);
                    for (int i = 0; i < size; ++i) {
                        reader >> collection[i];
                    }
                };
            private:
                std::vector<serialization::Data> collection;
            };
        }
    }
}

#endif //__PortableCollection_H_
