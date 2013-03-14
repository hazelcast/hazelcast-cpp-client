//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ClassDefinition.h"
#include "SerializationContext.h"
#include "SerializationService.h"
#include "zlib.h"
#include "HazelcastException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationContext::SerializationContext(PortableFactory const *portableFactory, int version, SerializationService *service)
            : portableFactory(portableFactory)
            , version(version)
            , service(service) {
            };

            SerializationContext::~SerializationContext() {
            };

            SerializationContext::SerializationContext(const SerializationContext& rhs) {
            };

            void SerializationContext::operator = (const SerializationContext& rhs) {
            };

            bool SerializationContext::isClassDefinitionExists(int classId) {
                return isClassDefinitionExists(classId, version);
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int classId) {
                long key = SerializationService::combineToLong(classId, version);
                return versionedDefinitions[key];
            };

            bool SerializationContext::isClassDefinitionExists(int classId, int version) {
                long key = SerializationService::combineToLong(classId, version);
                return (versionedDefinitions.count(key) > 0);
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int classId, int version) {
                long key = SerializationService::combineToLong(classId, version);
                return versionedDefinitions[key];

            };

            std::auto_ptr<Portable> SerializationContext::createPortable(int classId) {
                return std::auto_ptr<Portable >(portableFactory->create(classId));
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::createClassDefinition(std::vector<byte>& binary) {

                decompress(binary);

                DataInput dataInput = DataInput(binary, service);
                boost::shared_ptr<ClassDefinition> cd(new ClassDefinition);
                cd->readData(dataInput);
                cd->setBinary(binary);

                long key = service->combineToLong(cd->getClassId(), version);

                versionedDefinitions[key] = cd;

                registerNestedDefinitions(cd);
                return cd;
            };

            void SerializationContext::registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd) {
                vector<boost::shared_ptr<ClassDefinition> > nestedDefinitions = cd->getNestedClassDefinitions();
                for (vector<boost::shared_ptr<ClassDefinition> >::iterator it = nestedDefinitions.begin(); it < nestedDefinitions.end(); it++) {
                    registerClassDefinition(*it);
                    registerNestedDefinitions(*it);
                }
            };

            void SerializationContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {

                if (!isClassDefinitionExists(cd->getClassId(), cd->getVersion())) {
                    if (cd->getBinary().size() == 0) {
                        DataOutput *output = service->pop();
                        cd->writeData(*output);
                        std::vector<byte> binary = output->toByteArray();
                        compress(binary);
                        cd->setBinary(binary);
                        service->push(output);
                    }
                    long versionedClassId = service->combineToLong(cd->getClassId(), cd->getVersion());
                    versionedDefinitions[versionedClassId] = cd;
                }
            };

            int SerializationContext::getVersion() {
                return version;
            };

            void SerializationContext::compress(std::vector<byte>& binary) {
                uLong ucompSize = binary.size();
                uLong compSize = compressBound(ucompSize);
                byte uncompressedTemp[binary.size()];
                for (int i = 0; i < binary.size(); i++)
                    uncompressedTemp[i] = binary[i];

                byte compressedTemp[compSize];
                int err = compress2((Bytef *) compressedTemp, &compSize, (Bytef *) uncompressedTemp, ucompSize, Z_BEST_COMPRESSION);
                switch (err) {
                    case Z_BUF_ERROR:
                        throw hazelcast::client::HazelcastException("not enough room in the output buffer at compression");
                    case Z_DATA_ERROR:
                        throw hazelcast::client::HazelcastException( "data is corrupted at compression");
                    case Z_MEM_ERROR:
                        throw hazelcast::client::HazelcastException("if there was not  enough memory at compression");
                }
                std::vector<byte> compressed(compressedTemp, compressedTemp + compSize);
                binary = compressed;
            };

            void SerializationContext::decompress(std::vector<byte>& binary) {
                uLong compSize = binary.size();

                uLong ucompSize = 512;
                byte *temp = NULL;
                byte compressedTemp[binary.size()];
                for (int i = 0; i < binary.size(); i++)
                    compressedTemp[i] = binary[i];
                int err = Z_OK;
                do {
                    ucompSize *= 2;
                    delete [] temp;
                    temp = new byte[ucompSize];
                    err = uncompress((Bytef *) temp, &ucompSize, (Bytef *) compressedTemp, compSize);
                    switch (err) {
                        case Z_BUF_ERROR:
                            std::cerr << "buffer size is not enough" << std::endl;
                        case Z_DATA_ERROR:
                            std::cerr << "data is corrupted or incomplete at decompression" << std::endl;
                        case Z_MEM_ERROR:
                            std::cerr << "there was not  enough memory at decompression" << std::endl;
                    }
                } while (err != Z_OK);
                std::vector<byte> decompressed(temp, temp + ucompSize);
                binary = decompressed;
                delete [] temp;
            };

        }
    }
}