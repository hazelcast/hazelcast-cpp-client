//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"
#include "hazelcast/client/multimap/TxnMultiMapPutRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapGetRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/multimap/TxnMultiMapRemoveRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapValueCountRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

            }

            bool TransactionalMultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                multimap::TxnMultiMapPutRequest *request = new multimap::TxnMultiMapPutRequest(getName(), key, value);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::get(const serialization::pimpl::Data& key) {

                multimap::TxnMultiMapGetRequest *request = new multimap::TxnMultiMapGetRequest(getName(), key);
                boost::shared_ptr<impl::PortableCollection> portableCollection = toObject<impl::PortableCollection>(invoke(request));
                return portableCollection->getCollection();
            }

            bool TransactionalMultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), key, value);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::remove(const serialization::pimpl::Data& key) {
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), key);
                boost::shared_ptr<impl::PortableCollection> portableCollection = toObject<impl::PortableCollection>(invoke(request));
                return portableCollection->getCollection();
            }

            int TransactionalMultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                multimap::TxnMultiMapValueCountRequest *request = new multimap::TxnMultiMapValueCountRequest(getName(), key);
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }

            int TransactionalMultiMapImpl::size() {
                multimap::TxnMultiMapSizeRequest *request = new multimap::TxnMultiMapSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }

        }
    }
}
