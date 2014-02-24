//
// Created by sancar koyunlu on 23/12/13.
//

#include "hazelcast/client/serialization/pimpl/DataAdapter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataAdapter::DataAdapter(const Data &data)
                :status(stType)
                , factoryId(0)
                , classId(0)
                , version(0)
                , classDefSize(0)
                , skipClassDef(false)
                , bytesRead(0)
                , bytesWritten(0)
                , data(data) {

                }

                DataAdapter::DataAdapter()
                :status(stType)
                , factoryId(0)
                , classId(0)
                , version(0)
                , classDefSize(0)
                , skipClassDef(false)
                , bytesRead(0)
                , bytesWritten(0) {

                }

                Data &DataAdapter::getData() {
                    assert(isStatusSet(stAll));
                    return data;
                }


                bool DataAdapter::writeTo(util::ByteBuffer &destination) {
                    if (!isStatusSet(stType)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.getType());
                        setStatus(stType);
                    }
                    if (!isStatusSet(stClassId)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        classId = data.cd == NULL ? Data::NO_CLASS_ID : data.cd->getClassId();
                        destination.writeInt(classId);
                        if (classId == Data::NO_CLASS_ID) {
                            setStatus(stFactoryId);
                            setStatus(stVersion);
                            setStatus(stClassDefSize);
                            setStatus(stClassDef);
                        }
                        setStatus(stClassId);
                    }
                    if (!isStatusSet(stFactoryId)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.cd->getFactoryId());
                        setStatus(stFactoryId);
                    }
                    if (!isStatusSet(stVersion)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        int version = data.cd->getVersion();
                        destination.writeInt(version);
                        setStatus(stVersion);
                    }
                    if (!isStatusSet(stClassDefSize)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        classDefSize = data.cd->getBinary().size();
                        destination.writeInt(classDefSize);
                        setStatus(stClassDefSize);
                        if (classDefSize == 0) {
                            setStatus(stClassDef);
                        }
                    }
                    if (!isStatusSet(stClassDef)) {
                        if (destination.remaining() < data.cd->getBinary().size()) {
                            return false;
                        }
                        destination.readFrom(data.cd->getBinary());
                        setStatus(stClassDef);
                    }
                    if (!isStatusSet(stSize)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        int size = data.bufferSize();
                        destination.writeInt(size);
                        setStatus(stSize);
                        if (size <= 0) {
                            setStatus(stValue);
                        } else {
                            bytesWritten = 0;
                        }
                    }
                    if (!isStatusSet(stValue)) {
                        bytesWritten += destination.readFrom(*(data.buffer), bytesWritten);
                        if (bytesWritten != data.buffer->size()) {
                            return false;
                        }
                        setStatus(stValue);
                    }
                    if (!isStatusSet(stHash)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(data.getPartitionHash());
                        setStatus(stHash);
                    }
                    setStatus(stAll);
                    return true;
                }

                bool DataAdapter::readFrom(util::ByteBuffer &source) {
                    if (!isStatusSet(stType)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        data.setType(source.readInt());
                        setStatus(stType);
                    }
                    if (!isStatusSet(stClassId)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        classId = source.readInt();
                        setStatus(stClassId);
                        if (classId == Data::NO_CLASS_ID) {
                            setStatus(stFactoryId);
                            setStatus(stVersion);
                            setStatus(stClassDefSize);
                            setStatus(stClassDef);
                        }
                    }
                    if (!isStatusSet(stFactoryId)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        factoryId = source.readInt();
                        setStatus(stFactoryId);
                    }
                    if (!isStatusSet(stVersion)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        version = source.readInt();
                        setStatus(stVersion);
                    }
                    if (!isStatusSet(stClassDef)) {
                        if (!skipClassDef) {
                            if (context->isClassDefinitionExists(factoryId, classId, version)) {
                                data.cd = context->lookup(factoryId, classId, version);
                                skipClassDef = true;
                            }
                        }
                        if (!isStatusSet(stClassDefSize)) {
                            if (source.remaining() < 4) {
                                return false;
                            }
                            classDefSize = source.readInt();
                            setStatus(stClassDefSize);
                        }
                        if (!isStatusSet(stClassDef)) {
                            if (source.remaining() < classDefSize) {
                                return false;
                            }
                            if (skipClassDef) {
                                source.skip(classDefSize);
                            } else {
                                std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                                source.writeTo(*(classDefBytes.get()));
                                data.cd = context->createClassDefinition(factoryId, classDefBytes);
                            }
                            setStatus(stClassDef);
                        }
                    }
                    if (!isStatusSet(stSize)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        int valueSize = source.readInt();
                        data.buffer.reset(new std::vector<byte>(valueSize));
                        setStatus(stSize);
                    }
                    if (!isStatusSet(stValue)) {
                        bytesRead += source.writeTo((*(data.buffer)), bytesRead);
                        if (bytesRead != data.buffer->size()) {
                            return false;
                        }
                        setStatus(stValue);
                    }

                    if (!isStatusSet(stHash)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        data.setPartitionHash(source.readInt());
                        setStatus(stHash);
                    }
                    setStatus(stAll);
                    return true;
                }

                void DataAdapter::setStatus(StatusBit bit) {
                    status |= 1 << (int) bit;
                }

                bool DataAdapter::isStatusSet(StatusBit bit) const {
                    return (status & 1 << (int) bit) != 0;
                }
            }
        }
    }
}
