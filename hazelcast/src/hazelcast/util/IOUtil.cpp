//
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace util {
        void IOUtil::closeResource(Closeable *closable) {
            if (closable != NULL) {
                try {
                    closable->close();
                } catch (client::exception::IException& e) {
                    std::stringstream message;
                    message << "closeResource failed" << e.what();
                    ILogger::getLogger().finest(message.str());
                }

            }
        }
    }
}

