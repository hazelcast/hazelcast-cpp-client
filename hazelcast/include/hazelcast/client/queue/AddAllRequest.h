//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_ALL_REQUEST
#define HAZELCAST_QUEUE_ADD_ALL_REQUEST

#include "../serialization/Data.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddAllRequest : public Portable {
            public:

                AddAllRequest(const std::string& name, std::vector<serialization::Data>& dataList)
                :name(name)
                , dataList(dataList) {

                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::ADD_ALL;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", 0);
                    writer.writeInt("s", dataList.size());
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    for (int i = 0; i < dataList.size(); ++i) {
                        dataList[i].writeData(out);
                    }
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    int size = reader.readInt("s");
                    dataList.resize(size);
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        dataList[i].readData(in);
                    }
                };
            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_ALL_REQUEST
