//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/TransactionalListProxy.h"
#include "hazelcast/client/collection/TxnListAddRequest.h"
#include "hazelcast/client/collection/TxnListRemoveRequest.h"
#include "hazelcast/client/collection/TxnListSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalListProxy::TransactionalListProxy(const std::string& objectName, txn::TransactionProxy *context)
            : TransactionalObject("hz:impl:listService", objectName, context) {
            }

            bool TransactionalListProxy::add(const serialization::pimpl::Data& e) {
                collection::TxnListAddRequest *request = new collection::TxnListAddRequest(getName(), e);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            bool TransactionalListProxy::remove(const serialization::pimpl::Data& e) {
                collection::TxnListRemoveRequest *request = new collection::TxnListRemoveRequest(getName(), e);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            int TransactionalListProxy::size() {
                collection::TxnListSizeRequest *request = new collection::TxnListSizeRequest(getName());
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }
        }
    }
}