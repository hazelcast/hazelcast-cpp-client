//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/ContainsRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            ContainsRequest::ContainsRequest(const std::string& name, std::vector<serialization::Data>& dataList)
            :name(name)
            , dataList(dataList) {

            };

            int ContainsRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int ContainsRequest::getClassId() const {
                return queue::QueuePortableHook::CONTAINS;
            };


            void ContainsRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
                writer.writeInt("s", dataList.size());
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                for (int i = 0; i < dataList.size(); ++i) {
                    dataList[i].writeData(out);
                }
            };
        }
    }
}

