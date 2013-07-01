//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERIALIZABLE_CONNECTION
#define HAZELCAST_SERIALIZABLE_CONNECTION

#include "../serialization/Data.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class SerializableCollection {
            public:
                SerializableCollection();

                ~SerializableCollection();

                std::vector<serialization::Data *> getCollection() const;

                int getSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;


                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) {
                    writer << datas.size();
                    for (std::vector < serialization::Data * > ::const_iterator it = datas.begin(); it != datas.end();
                         ++it) {
                        (*it)->writeData(writer);
                    }
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    int size;
                    reader >> size;
                    if (size == -1)
                        return;
                    for (int i = 0; i < size; i++) {
                        serialization::Data *data = new serialization::Data();
                        data->readData(reader);
                        datas.push_back(data);
                    }
                };

            private:
                std::vector<serialization::Data *> datas;
            };
        }
    }
}

#endif //HAZELCAST_SERIALIZABLE_CONNECTION
