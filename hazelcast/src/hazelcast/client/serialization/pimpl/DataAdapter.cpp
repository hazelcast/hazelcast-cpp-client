//
// Created by sancar koyunlu on 23/12/13.
//

#include "hazelcast/client/serialization/pimpl/DataAdapter.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionAdapter.h"
#include "hazelcast/util/ByteBuffer.h"
#include  "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                const int DataAdapter::ST_TYPE = 1;
                const int DataAdapter::ST_SIZE = 2;
                const int DataAdapter::ST_VALUE = 3;
                const int DataAdapter::ST_HASH = 4;
                const int DataAdapter::ST_ALL = 5;

                DataAdapter::DataAdapter(PortableContext& context, const Data& data)
                : status(0)
                , bytesRead(0)
                , bytesWritten(0)
                , data(data)
                , context(context)
                , classDefinitionAdapter(NULL) {

                }

                DataAdapter::DataAdapter(PortableContext& context)
                : status(0)
                , bytesRead(0)
                , bytesWritten(0)
                , context(context)
                , classDefinitionAdapter(NULL) {

                }


                DataAdapter::~DataAdapter() {
                }

                const Data& DataAdapter::getData() const {
                    return data;
                }

                void DataAdapter::setData(Data& data) {
                    this->data = data;
                }


                PortableContext& DataAdapter::getPortableContext() const {
                    return context;
                }

                bool DataAdapter::writeTo(hazelcast::util::ByteBuffer& destination) {

                    if (!isStatusSet(ST_TYPE)) {
                        if (destination.remaining() < 5) {
                            return false;
                        }
                        destination.writeInt(data.getType());

                        bool hasClassDef = data.hasClassDefinition();
                        if (hasClassDef) {
                            classDefinitionAdapter = new ClassDefinitionAdapter(data, context);
                        }
                        destination.writeByte((byte)(hasClassDef ? 1 : 0));

                        setStatus(ST_TYPE);
                    }

                    if (classDefinitionAdapter != NULL) {
                        if (!classDefinitionAdapter->write(destination)) {
                            return false;
                        }
                        delete classDefinitionAdapter;
                    }

                    if (!isStatusSet(ST_HASH)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.hasPartitionHash() ? data.getPartitionHash() : 0);
                        setStatus(ST_HASH);
                    }
                    if (!isStatusSet(ST_SIZE)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        int size = data.bufferSize();
                        destination.writeInt(size);
                        setStatus(ST_SIZE);
                        if (size <= 0) {
                            setStatus(ST_VALUE);
                        } else {
                            bytesWritten = 0;
                        }
                    }
                    if (!isStatusSet(ST_VALUE)) {
                        bytesWritten += destination.readFrom(*(data.data), bytesWritten);
                        if (bytesWritten != data.data->size()) {
                            return false;
                        }
                        setStatus(ST_VALUE);
                    }
                    setStatus(ST_ALL);
                    return true;
                }

                bool DataAdapter::readFrom(hazelcast::util::ByteBuffer& source) {
                    if (!isStatusSet(ST_TYPE)) {
                        if (source.remaining() < 5) {
                            return false;
                        }
                        data.setType(source.readInt());
                        setStatus(ST_TYPE);

                        bool hasClassDef = source.readByte() == 1;
                        if (hasClassDef) {
                            classDefinitionAdapter = new ClassDefinitionAdapter(data, context);
                        }
                    }

                    if (classDefinitionAdapter != NULL) {
                        if (!classDefinitionAdapter->read(source)) {
                            return false;
                        }
                        delete classDefinitionAdapter;
                    }

                    if (!isStatusSet(ST_HASH)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        data.setPartitionHash(source.readInt());
                        setStatus(ST_HASH);
                    }

                    if (!isStatusSet(ST_SIZE)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        int valueSize = source.readInt();
                        data.data.reset(new std::vector<byte>(valueSize));
                        setStatus(ST_SIZE);
                    }
                    if (!isStatusSet(ST_VALUE)) {
                        bytesRead += source.writeTo((*(data.data)), bytesRead);
                        if (bytesRead != data.data->size()) {
                            return false;
                        }
                        setStatus(ST_VALUE);
                    }

                    setStatus(ST_ALL);
                    return true;
                }

                void DataAdapter::setStatus(int bit) {
                    status = util::setBit(status, bit);
                }

                bool DataAdapter::isStatusSet(int bit) const {
                    return util::isBitSet(status, bit);
                }

            }
        }
    }
}

