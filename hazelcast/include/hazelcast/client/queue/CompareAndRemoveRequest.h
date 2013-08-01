//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST
#define HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class CompareAndRemoveRequest {
            public:

                CompareAndRemoveRequest(const std::string& name, std::vector<serialization::Data>& dataList, bool retain)
                :name(name)
                , dataList(dataList)
                , retain(retain) {

                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::COMPARE_AND_REMOVE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeBoolean("r", retain);
                    writer.writeInt("s", dataList.size());
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    for (int i = 0; i < dataList.size(); ++i) {
                        dataList[i].writeData(*out);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    retain = reader.readBoolean("r");
                    int size = reader.readInt("s");
                    dataList.resize(size);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        dataList[i].readData(*in);
                    }
                };
            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
                bool retain;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
