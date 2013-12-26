//
// Created by sancar koyunlu on 23/12/13.
//

#include "hazelcast/client/serialization/DataAdapter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            DataAdapter::DataAdapter(const Data& data)
            :status(stType)
            , factoryId(0)
            , classId(0)
            , version(0)
            , classDefSize(0)
            , skipClassDef(false)
            , bytesRead(0)
            , data(data){

            }

            DataAdapter::DataAdapter()
            :status(stType)
            , factoryId(0)
            , classId(0)
            , version(0)
            , classDefSize(0)
            , skipClassDef(false)
            , bytesRead(0) {

            }

            Data &DataAdapter::getData() {
                assert(!isStatusSet(stAll));
                return data;
            }


            bool DataAdapter::writeTo(util::CircularBuffer &destination) {
                if (!isStatusSet(stType)) {
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
                    destination.writeInt(data.type);
                    setStatus(stType);
                }
                if (!isStatusSet(stClassId)) {
                    if (destination.remainingSpace() < 4) {
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
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
                    destination.writeInt(data.cd->getFactoryId());
                    setStatus(stFactoryId);
                }
                if (!isStatusSet(stVersion)) {
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
                    int version = data.cd->getVersion();
                    destination.writeInt(version);
                    setStatus(stVersion);
                }
                if (!isStatusSet(stClassDefSize)) {
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
//                        final BinaryClassDefinition cd = (BinaryClassDefinition) data.classDefinition;
//                        final byte[] binary = cd.getBinary();
//                        classDefSize = binary == null ? 0 : binary.length;
                    destination.writeInt(classDefSize);
                    setStatus(stClassDefSize);
                    if (classDefSize == 0) {
                        setStatus(stClassDef);
                    } else {
//                            buffer = ByteBuffer.wrap(binary);
                    }
                }
                if (!isStatusSet(stClassDef)) {
//                        IOUtil.copyToHeapBuffer(buffer, destination);
//                        if (buffer.hasRemaining()) {
//                            return false;
//                        }
                    setStatus(stClassDef);
                }
                if (!isStatusSet(stSize)) {
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
                    int size = data.bufferSize();
                    destination.writeInt(size);
                    setStatus(stSize);
                    if (size <= 0) {
                        setStatus(stValue);
                    } else {
//                            buffer = ByteBuffer.wrap(data.buffer);
                    }
                }
                if (!isStatusSet(stValue)) {
//                        IOUtil.copyToHeapBuffer(buffer, destination);
//                        if (buffer.hasRemaining()) {
//                            return false;
//                        }
                    setStatus(stValue);
                }
                if (!isStatusSet(stHash)) {
                    if (destination.remainingSpace() < 4) {
                        return false;
                    }
                    destination.writeInt(data.getPartitionHash());
                    setStatus(stHash);
                }
                setStatus(stAll);
                return true;
            }

            bool DataAdapter::readFrom(util::CircularBuffer &source) {
                if (!isStatusSet(stType)) {
                    if (source.remainingData() < 4) {
                        return false;
                    }
                    data.type = source.readInt();
                    setStatus(stType);
                }
                if (!isStatusSet(stClassId)) {
                    if (source.remainingData() < 4) {
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
                    if (source.remainingData() < 4) {
                        return false;
                    }
                    factoryId = source.readInt();
                    data.isError = (factoryId == protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                            && (classId == protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                    setStatus(stFactoryId);
                }
                if (!isStatusSet(stVersion)) {
                    if (source.remainingData() < 4) {
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
                        if (source.remainingData() < 4) {
                            return false;
                        }
                        classDefSize = source.readInt();
                        setStatus(stClassDefSize);
                    }
                    if (!isStatusSet(stClassDef)) {
                        if (source.remainingData() < classDefSize) {
                            return false;
                        }
                        if (skipClassDef) {
                            source.advance(classDefSize);
                        } else {
                            std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                            source.readFully(*(classDefBytes.get()));
                            data.cd = context->createClassDefinition(factoryId, classDefBytes);
                        }
                        setStatus(stClassDef);
                    }
                }
                if (!isStatusSet(stSize)) {
                    if (source.remainingData() < 4) {
                        return false;
                    }
                    int valueSize = source.readInt();
                    data.buffer.reset(new std::vector<byte>(valueSize));
                    setStatus(stSize);
                }
                if (!isStatusSet(stValue)) {
                    bytesRead += source.read(&((*(data.buffer))[0]), data.buffer->size() - bytesRead);
                    if (bytesRead != data.buffer->size()) {
                        return false;
                    }
                    setStatus(stValue);
                }

                if (!isStatusSet(stHash)) {
                    if (source.remainingData() < 4) {
                        return false;
                    }
                    data.partitionHash = source.readInt();
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
