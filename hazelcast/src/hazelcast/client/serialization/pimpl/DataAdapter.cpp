//
// Created by sancar koyunlu on 23/12/13.
//

#include "hazelcast/client/serialization/pimpl/DataAdapter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataAdapter::DataAdapter(PortableContext& context, const Data& data)
                : status(ST_TYPE)
                , factoryId(0)
                , classId(0)
                , version(0)
                , classDefSize(0)
                , skipClassDef(false)
                , bytesRead(0)
                , bytesWritten(0)
                , data(data)
                , context(context) {

                }

                DataAdapter::DataAdapter(PortableContext& context)
                : status(ST_TYPE)
                , factoryId(0)
                , classId(0)
                , version(0)
                , classDefSize(0)
                , skipClassDef(false)
                , bytesRead(0)
                , bytesWritten(0)
                , context(context) {

                }


                DataAdapter::~DataAdapter() {

                }

                const Data& DataAdapter::getData() const{
                    return data;
                }

                void DataAdapter::setData(Data& data) {
                    this->data = data;
                }

                bool DataAdapter::writeTo(util::ByteBuffer& destination) {
                    if (!isStatusSet(ST_TYPE)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.getType());
                        setStatus(ST_TYPE);
                    }
                    if (!isStatusSet(ST_CLASS_ID)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        classId = data.cd == NULL ? Data::NO_CLASS_ID : data.cd->getClassId();
                        destination.writeInt(classId);
                        if (classId == Data::NO_CLASS_ID) {
                            setStatus(ST_FACTORY_ID);
                            setStatus(ST_VERSION);
                            setStatus(ST_CLASS_DEF_SIZE);
                            setStatus(ST_CLASS_DEF);
                        }
                        setStatus(ST_CLASS_ID);
                    }
                    if (!isStatusSet(ST_FACTORY_ID)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.cd->getFactoryId());
                        setStatus(ST_FACTORY_ID);
                    }
                    if (!isStatusSet(ST_VERSION)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        int version = data.cd->getVersion();
                        destination.writeInt(version);
                        setStatus(ST_VERSION);
                    }
                    if (!isStatusSet(ST_CLASS_DEF_SIZE)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        classDefSize = data.cd->getBinary().size();
                        destination.writeInt(classDefSize);
                        setStatus(ST_CLASS_DEF_SIZE);
                        if (classDefSize == 0) {
                            setStatus(ST_CLASS_DEF);
                        }
                    }
                    if (!isStatusSet(ST_CLASS_DEF)) {
                        if (destination.remaining() < data.cd->getBinary().size()) {
                            return false;
                        }
                        destination.readFrom(data.cd->getBinary());
                        setStatus(ST_CLASS_DEF);
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
                        bytesWritten += destination.readFrom(*(data.buffer), bytesWritten);
                        if (bytesWritten != data.buffer->size()) {
                            return false;
                        }
                        setStatus(ST_VALUE);
                    }
                    if (!isStatusSet(ST_HASH)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.getPartitionHash());
                        setStatus(ST_HASH);
                    }
                    setStatus(ST_ALL);
                    return true;
                }

                bool DataAdapter::readFrom(util::ByteBuffer& source) {
                    if (!isStatusSet(ST_TYPE)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        data.setType(source.readInt());
                        setStatus(ST_TYPE);
                    }
                    if (!isStatusSet(ST_CLASS_ID)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        classId = source.readInt();
                        setStatus(ST_CLASS_ID);
                        if (classId == Data::NO_CLASS_ID) {
                            setStatus(ST_FACTORY_ID);
                            setStatus(ST_VERSION);
                            setStatus(ST_CLASS_DEF_SIZE);
                            setStatus(ST_CLASS_DEF);
                        }
                    }
                    if (!isStatusSet(ST_FACTORY_ID)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        factoryId = source.readInt();
                        setStatus(ST_FACTORY_ID);
                    }
                    if (!isStatusSet(ST_VERSION)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        version = source.readInt();
                        setStatus(ST_VERSION);
                    }
                    if (!isStatusSet(ST_CLASS_DEF)) {
                        if (!skipClassDef) {
                            if (context.isClassDefinitionExists(factoryId, classId, version)) {
                                data.cd = context.lookup(factoryId, classId, version);
                                skipClassDef = true;
                            }
                        }
                        if (!isStatusSet(ST_CLASS_DEF_SIZE)) {
                            if (source.remaining() < 4) {
                                return false;
                            }
                            classDefSize = source.readInt();
                            setStatus(ST_CLASS_DEF_SIZE);
                        }
                        if (!isStatusSet(ST_CLASS_DEF)) {
                            if (source.remaining() < classDefSize) {
                                return false;
                            }
                            if (skipClassDef) {
                                source.skip(classDefSize);
                            } else {
                                std::auto_ptr<std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                                source.writeTo(*(classDefBytes.get()));
                                data.cd = context.createClassDefinition(factoryId, classDefBytes);
                            }
                            setStatus(ST_CLASS_DEF);
                        }
                    }
                    if (!isStatusSet(ST_SIZE)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        int valueSize = source.readInt();
                        data.buffer.reset(new std::vector<byte>(valueSize));
                        setStatus(ST_SIZE);
                    }
                    if (!isStatusSet(ST_VALUE)) {
                        bytesRead += source.writeTo((*(data.buffer)), bytesRead);
                        if (bytesRead != data.buffer->size()) {
                            return false;
                        }
                        setStatus(ST_VALUE);
                    }

                    if (!isStatusSet(ST_HASH)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        data.setPartitionHash(source.readInt());
                        setStatus(ST_HASH);
                    }
                    setStatus(ST_ALL);
                    return true;
                }

                void DataAdapter::setStatus(int bit) {
                    status |= 1 << bit;
                }

                bool DataAdapter::isStatusSet(int bit) const {
                    return (status & 1 << bit) != 0;
                }
            }
        }
    }
}

