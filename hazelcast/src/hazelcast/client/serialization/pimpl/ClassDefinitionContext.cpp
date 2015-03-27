//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include <zlib.h>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                ClassDefinitionContext::ClassDefinitionContext(PortableContext *portableContext)
                : portableContext(portableContext) {

                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    boost::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    boost::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, boost::shared_ptr<int>(new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion", error.str());
                    }
                }

                bool ClassDefinitionContext::isClassDefinitionExists(int classId, int version) const {
                    long long key = combineToLong(classId, version);
                    return (versionedDefinitions.containsKey(key));
                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionContext::createClassDefinition(hazelcast::util::ByteVector_ptr compressedBinary) {
                    if (compressedBinary.get() == NULL || compressedBinary.get()->size() == 0) {
                        throw exception::IOException("ClassDefinitionContext::createClassDefinition", "Illegal class-definition binary! ");
                    }
                    std::vector<byte> decompressed = decompress(*(compressedBinary.get()));

                    DataInput dataInput(decompressed);
                    boost::shared_ptr<ClassDefinition> cd(new ClassDefinition());
                    cd->readData(dataInput);
                    cd->setBinary(compressedBinary);

                    return registerClassDefinition(cd);
                }

                boost::shared_ptr<ClassDefinition>  ClassDefinitionContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    cd->setVersionIfNotSet(portableContext->getVersion());
                    setClassDefBinary(cd);

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    boost::shared_ptr<ClassDefinition> currentCD = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCD == NULL) {
                        return cd;
                    }

                    versionedDefinitions.put(versionedClassId, cd);
                    return cd;
                }

                void ClassDefinitionContext::compress(std::vector<byte>& binary) {
                    uLong ucompSize = binary.size();
                    uLong compSize = compressBound(ucompSize);
                    std::vector<byte> uncompressedTemp(binary.begin(), binary.end());

                    byte *compressedTemp = new byte[compSize];

                    int err = compress2((Bytef *)&(compressedTemp[0]), &compSize, (Bytef *)&(uncompressedTemp[0]), ucompSize, Z_BEST_COMPRESSION);
                    switch (err) {
                        case Z_BUF_ERROR:
                            throw exception::IOException("ClassDefinitionContext::compress", "not enough room in the output buffer at compression");
                        case Z_DATA_ERROR:
                            throw exception::IOException("ClassDefinitionContext::compress", "data is corrupted at compression");
                        case Z_MEM_ERROR:
                            throw exception::IOException("ClassDefinitionContext::compress", "there was not  enough memory at compression");
                        default:
                            break;
                    }
                    binary.resize(compSize);
                    std::copy(compressedTemp, compressedTemp + compSize, binary.begin());
                    delete[] compressedTemp;

                }

                std::vector<byte> ClassDefinitionContext::decompress(std::vector<byte> const& binary) const {
                    uLong compSize = binary.size();

                    uLong ucompSize = 512;
                    byte *temp = NULL;
                    std::vector<byte> compressedTemp(binary.begin(), binary.end());

                    int err = Z_OK;
                    do {
                        ucompSize *= 2;
                        delete[] temp;
                        temp = new byte[ucompSize];
                        err = uncompress((Bytef *)temp, &ucompSize, (Bytef *)&(compressedTemp[0]), compSize);
                        switch (err) {
                            case Z_BUF_ERROR:
                                throw exception::IOException("ClassDefinitionContext::compress", "not enough room in the output buffer at decompression");
                            case Z_DATA_ERROR:
                                throw exception::IOException("ClassDefinitionContext::compress", "data is corrupted at decompression");
                            case Z_MEM_ERROR:
                                throw exception::IOException("ClassDefinitionContext::compress", "there was not  enough memory at decompression");
                        }
                    } while (err != Z_OK);
                    std::vector<byte> decompressed(temp, temp + ucompSize);
                    delete[] temp;
                    return decompressed;
                }

                long long ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((long long)x) << 32 | (((long long)y) & 0xFFFFFFFL);
                }

                void ClassDefinitionContext::setClassDefBinary(boost::shared_ptr<ClassDefinition> cd) {
                    if (cd->getBinary().size() == 0) {
                        DataOutput output;
                        cd->writeData(output);
                        hazelcast::util::ByteVector_ptr binary = output.toByteArray();
                        compress(*(binary.get()));
                        cd->setBinary(binary);
                    }
                }

            }
        }
    }
}

