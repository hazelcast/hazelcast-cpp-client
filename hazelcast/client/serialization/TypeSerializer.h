//
// Created by sancar koyunlu on 5/8/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef __TypeSerializer_H_
#define __TypeSerializer_H_

#include <iostream>
#include "DataOutput.h"
#include "DataInput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class TypeSerializer {
            public:
                virtual int getTypeId() = 0;
                //--------- I!!!!! IMPORTANT !!!!!!! ------ //
                // implement these functions in derived class
                /*
                template<typename T>
                void write(DataOutput& out, T& object) = 0;

                template<typename T>
                T* read(DataOutput& in) = 0;
                 */

                virtual void write(DataOutput *out, void *object) = 0;

                virtual void *read(DataInput& in) = 0;

            };
        }
    }
}


#endif //__TypeSerializer_H_
