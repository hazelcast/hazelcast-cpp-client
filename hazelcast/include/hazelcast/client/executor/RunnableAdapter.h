//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_RunnableAdapter
#define HAZELCAST_RunnableAdapter

#include "IdentifiedDataSerializable.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "hazelcast/client/executor/DataSerializableHook.h"

namespace hazelcast {
    namespace client {
        namespace executor {
            template<typename Runnable>
            class RunnableAdapter : public IdentifiedDataSerializable {
            public:
                RunnableAdapter(Runnable& runnable)
                :runnable(runnable) {

                }

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::RUNNABLE_ADAPTER;
                }

                void writeData(serialization::ObjectDataOutput & writer) const {
                    writer.writeObject(&runnable);
                }

                void readData(serialization::ObjectDataInput & reader) {
                    runnable = reader.readObject<Runnable>();
                }

            private:
                Runnable& runnable;


            };
        }
    }
}

#endif //HAZELCAST_RunnableAdapter

