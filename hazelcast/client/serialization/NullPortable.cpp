//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "NullPortable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            NullPortable::NullPortable(int factoryId, int classId)
            :factoryId(factoryId)
            , classId(classId) {

            }


        }
    }
}