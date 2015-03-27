//
// Created by sancar koyunlu on 23/12/13.
//

#include "hazelcast/client/serialization/pimpl/DataAdapter.h"
#include  "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                const int DataAdapter::ST_TYPE = 1;
                const int DataAdapter::ST_SIZE = 2;
                const int DataAdapter::ST_VALUE = 3;
                const int DataAdapter::ST_HASH = 4;
                const int DataAdapter::ST_ALL = 5;

                DataAdapter::DataAdapter(PortableContext& context, const Data& data)
                : status(ST_TYPE)
                , data(data)
                , context(context){

                }

                DataAdapter::DataAdapter(PortableContext& context)
                : status(ST_TYPE)
                , context(context) {

                }


                DataAdapter::~DataAdapter() {
                }

                const Data& DataAdapter::getData() const {
                    return data;
                }

                void DataAdapter::setData(Data& data) {
                    this->data = data;
                }


                PortableContext& DataAdapter::getPortableContext() const {
                    return context;
                }

                void DataAdapter::setStatus(int bit) {
                    status = util::setBit(status, bit);
                }

                bool DataAdapter::isStatusSet(int bit) const {
                    return util::isBitSet(status, bit);
                }

            }
        }
    }
}

