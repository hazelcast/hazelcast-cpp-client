//
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace txn {

            void BaseTxnRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("tId", txnId);
                writer.writeLong("cti", threadId);
            }

            void BaseTxnRequest::setTxnId(const std::string &txnId) {
                this->txnId = txnId;
            }

            void BaseTxnRequest::setThreadId(long threadId) {
                this->threadId = threadId;
            }

            bool BaseTxnRequest::isBindToSingleConnection() const{
                return true;
            }
        }
    }
}