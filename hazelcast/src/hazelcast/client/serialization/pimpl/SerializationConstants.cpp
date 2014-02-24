//
// Created by sancar koyunlu on 21/02/14.
//

#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationConstants::SerializationConstants()
                :size(25)
                , typeIdNameVector(size) {
                    typeIdNameVector[idToIndex(-1)] = "portable";
                    typeIdNameVector[idToIndex(-2)] = "data";
                    typeIdNameVector[idToIndex(-3)] = "byte";
                    typeIdNameVector[idToIndex(-4)] = "boolean";
                    typeIdNameVector[idToIndex(-5)] = "char";
                    typeIdNameVector[idToIndex(-6)] = "short";
                    typeIdNameVector[idToIndex(-7)] = "integer";
                    typeIdNameVector[idToIndex(-8)] = "long";
                    typeIdNameVector[idToIndex(-9)] = "float";
                    typeIdNameVector[idToIndex(-10)] = "double";
                    typeIdNameVector[idToIndex(-11)] = "string";
                    typeIdNameVector[idToIndex(-12)] = "byteArray";
                    typeIdNameVector[idToIndex(-13)] = "charArray";
                    typeIdNameVector[idToIndex(-14)] = "shortArray";
                    typeIdNameVector[idToIndex(-15)] = "integerArray";
                    typeIdNameVector[idToIndex(-16)] = "longArray";
                    typeIdNameVector[idToIndex(-17)] = "floatArray";
                    typeIdNameVector[idToIndex(-18)] = "doubleArray";
                    typeIdNameVector[idToIndex(-19)] = "class";
                    typeIdNameVector[idToIndex(-20)] = "data";
                    typeIdNameVector[idToIndex(-21)] = "bigInteger";
                    typeIdNameVector[idToIndex(-22)] = "bigDecimal";
                    typeIdNameVector[idToIndex(-23)] = "object";
                    typeIdNameVector[idToIndex(-24)] = "externalizable";
                    typeIdNameVector[idToIndex(-25)] = "enum";
                }

                std::string SerializationConstants::typeIdToName(int typeId) {
                    int i = idToIndex(typeId);
                    if (i < 0 || i >= size)
                        return "custom";
                    return typeIdNameVector[i];
                };

                int SerializationConstants::idToIndex(int id) {
                    return id + size;
                }
            }
        }
    }
}