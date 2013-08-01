//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_COLLECTION
#define HAZELCAST_PORTABLE_COLLECTION


#include "ProtocolConstants.h"
#include "Data.h"
#include "Portable.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableCollection : public Portable {
            public:
                PortableCollection();

                const std::vector<serialization::Data>& getCollection() const;

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeBool("l", true);
                    writer.writeInt("s", collection.size());
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    for (int i = 0; i < collection.size(); ++i) {
                        collection[i].writeData(*out);
                    }

                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    bool isList = reader.readBool("l");
                    int size = reader.readInt("s");
                    if (size < 0)
                        return;
                    collection.resize(size);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        collection[i].readData(*in);
                    }
                };
            private:
                std::vector<serialization::Data> collection;
            };
        }
    }
}

#endif //__PortableCollection_H_
