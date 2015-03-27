//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/exception/IllegalStateException.h"

using namespace hazelcast::client::exception;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableReader::DefaultPortableReader(PortableContext& portableContext,
                        DataInput& input, boost::shared_ptr<ClassDefinition> cd)
                : PortableReaderBase(portableContext, input, cd) {
                }
            }
        }
    }
}

