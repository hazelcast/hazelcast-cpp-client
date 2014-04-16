//
// Created by sancar koyunlu on 23/01/14.
//

#ifndef HAZELCAST_BaseTxnRequest
#define HAZELCAST_BaseTxnRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace txn {
            class HAZELCAST_API BaseTxnRequest : public impl::PortableRequest {
            public:
                void write(serialization::PortableWriter &writer) const;

                void setTxnId(const std::string &txnId);

                void setThreadId(long threadId);

                bool isBindToSingleConnection() const;

            private:
                std::string txnId;
                long threadId;
            };
        }
    }
}
#endif //HAZELCAST_BaseTxnRequest

