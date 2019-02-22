/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_CLIENT_PROXY_CLIENTRINGBUFFERPROXY_H_
#define HAZELCAST_CLIENT_PROXY_CLIENTRINGBUFFERPROXY_H_

#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/protocol/codec/RingbufferReadManyCodec.h"
#include "hazelcast/client/ringbuffer/ReadResultSet.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/impl/DataArrayImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/internal/executor/CompletedFuture.h"
#include "hazelcast/client/protocol/codec/RingbufferReadOneCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferCapacityCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferSizeCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferTailSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferHeadSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferRemainingCapacityCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferAddCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferAddAllCodec.h"
#include "hazelcast/client/proxy/PartitionSpecificClientProxy.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/client/Ringbuffer.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace proxy {
            template<typename E>
            class ClientRingbufferProxy : public Ringbuffer<E>, public PartitionSpecificClientProxy {
            public:
                static const std::string SERVICE_NAME;
                /**
                 * The maximum number of items that can be retrieved in 1 go using the {@link #readManyAsync(long, int, int, IFunction)}
                 * method.
                 */
                static const int32_t MAX_BATCH_SIZE;

                ClientRingbufferProxy(const std::string &objectName, spi::ClientContext *context)
                        : PartitionSpecificClientProxy(SERVICE_NAME, objectName, context), bufferCapacity(-1) {
                }

                virtual ~ClientRingbufferProxy() {
                }

                /****************  RingBuffer<E> interface implementation starts here *********************************/
                int64_t capacity() {
                    if (-1 == bufferCapacity) {
                        std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::encodeRequest(
                                getName());
                        boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                        protocol::codec::RingbufferCapacityCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferCapacityCodec::ResponseParameters::decode(
                                *response);
                        bufferCapacity = resultParamaters.response;
                    }
                    return bufferCapacity;
                }

                int64_t size() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::encodeRequest(
                            getName());
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferSizeCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferSizeCodec::ResponseParameters::decode(
                            *response);
                    return resultParamaters.response;
                }

                int64_t tailSequence() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::encodeRequest(
                            getName());
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferTailSequenceCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferTailSequenceCodec::ResponseParameters::decode(
                            *response);
                    return resultParamaters.response;
                }

                int64_t headSequence() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::encodeRequest(
                            getName());
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters::decode(
                            *response);
                    return resultParamaters.response;
                }

                int64_t remainingCapacity() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::encodeRequest(
                            getName());
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters::decode(
                            *response);
                    return resultParamaters.response;
                }

                int64_t add(const E &item) {
                    serialization::pimpl::Data itemData = toData<E>(item);
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferAddCodec::encodeRequest(
                            getName(), Ringbuffer<E>::OVERWRITE, itemData);
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferAddCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferAddCodec::ResponseParameters::decode(
                            *response);
                    return resultParamaters.response;
                }

                std::auto_ptr<E> readOne(int64_t sequence) {
                    checkSequence(sequence);

                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadOneCodec::encodeRequest(
                            getName(), sequence);
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(msg, partitionId);
                    protocol::codec::RingbufferReadOneCodec::ResponseParameters resultParamaters = protocol::codec::RingbufferReadOneCodec::ResponseParameters::decode(
                            *response);

                    return toObject<E>(resultParamaters.response);
                }

                /**
                 * This method will be removed when the ReliableTopic implementation is fixed later to use client
                 * execution service.
                 */
                boost::shared_ptr<spi::impl::ClientInvocationFuture>
                readManyAsync(int64_t sequence, int32_t maxCount, time_t timeoutSeconds) {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadManyCodec::encodeRequest(
                            getName(), sequence, 1, maxCount, (const serialization::pimpl::Data *) NULL);

                    return invokeAndGetFuture(msg, partitionId);
                }

                /**
                 * This method will be removed when the ReliableTopic implementation is fixed later to use client
                 * execution service.
                 */
                std::auto_ptr<DataArray<E> > getReadManyAsyncResponseObject(
                        boost::shared_ptr<protocol::ClientMessage> responseMsg) {
                    protocol::codec::RingbufferReadManyCodec::ResponseParameters responseParameters =
                            protocol::codec::RingbufferReadManyCodec::ResponseParameters::decode(*responseMsg);
                    return std::auto_ptr<DataArray<E> >(new impl::DataArrayImpl<E>(responseParameters.items,
                                                                                   getContext().getSerializationService()));
                }

                virtual boost::shared_ptr<ICompletableFuture<int64_t> >
                addAsync(const E &item, typename Ringbuffer<E>::OverflowPolicy overflowPolicy) {

                    serialization::pimpl::Data element = toData<E>(item);
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::RingbufferAddCodec::encodeRequest(
                            name, overflowPolicy, element);
                    try {
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = spi::impl::ClientInvocation::create(
                                getContext(), request, getName(), partitionId)->invoke();
                        return boost::shared_ptr<ICompletableFuture<int64_t> >(
                                new internal::ClientDelegatingFuture<int64_t>(invocationFuture,
                                                                              getSerializationService(),
                                                                              impl::PrimitiveMessageDecoder<protocol::codec::RingbufferAddCodec, int64_t>::instance()));
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<ICompletableFuture<int64_t> >();
                }

                virtual boost::shared_ptr<ICompletableFuture<int64_t> >
                addAllAsync(const std::vector<E> &items, typename Ringbuffer<E>::OverflowPolicy overflowPolicy) {
                    util::Preconditions::checkNotEmpty(items, "items can't be empty");
                    util::Preconditions::checkMax((int32_t) items.size(), MAX_BATCH_SIZE, "items");

                    std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(items);
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::RingbufferAddAllCodec::encodeRequest(
                            name, dataCollection, overflowPolicy);

                    try {
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = spi::impl::ClientInvocation::create(
                                getContext(), request, getName(), partitionId)->invoke();
                        return boost::shared_ptr<ICompletableFuture<int64_t> >(
                                new internal::ClientDelegatingFuture<int64_t>(invocationFuture,
                                                                              getSerializationService(),
                                                                              impl::PrimitiveMessageDecoder<protocol::codec::RingbufferAddAllCodec, int64_t>::instance()));
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<ICompletableFuture<int64_t> >();
                }

                /***************** RingBuffer<E> interface implementation ends here ***********************************/
            protected:
                boost::shared_ptr<protocol::ClientMessage>
                invoke(std::auto_ptr<protocol::ClientMessage> &clientMessage, int32_t partitionId) {
                    try {
                        return invokeOnPartition(clientMessage, partitionId);
                    } catch (exception::ExecutionException &e) {
                        boost::shared_ptr<exception::IException> cause = e.getCause();
                        if (cause->getErrorCode() == exception::StaleSequenceException::ERROR_CODE) {
                            // can not use static_pointer_cast
                            boost::shared_ptr<exception::StaleSequenceException> se = boost::dynamic_pointer_cast<exception::StaleSequenceException>(
                                    cause);
                            int64_t l = headSequence();
                            throw (exception::ExceptionBuilder<exception::StaleSequenceException>(se->getSource())
                                    << se->getMessage() << ", head sequence:" << l).build();
                        }
                        util::ExceptionUtil::rethrow(e);
                    } catch (const exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<protocol::ClientMessage>();
                }

                virtual boost::shared_ptr<ICompletableFuture<ringbuffer::ReadResultSet<E> > >
                readManyAsyncInternal(int64_t startSequence, int32_t minCount, int32_t maxCount,
                                      const serialization::pimpl::Data &filterData) {
                    checkSequence(startSequence);
                    util::Preconditions::checkNotNegative(minCount, "minCount can't be smaller than 0");
                    util::Preconditions::checkTrue(maxCount >= minCount,
                                                   "maxCount should be equal or larger than minCount");

                    try {
                        capacity();
                    } catch (exception::IException &e) {
                        //in case of exception return the exception via future to behave consistently to member
                        boost::shared_ptr<exception::IException> executionException(
                                new exception::ExecutionException("ClientRingbufferProxy::readManyAsync",
                                                                  "capacity() method failed", e));
                        boost::shared_ptr<ExecutorService> userExecutor = getContext().getClientExecutionService().getUserExecutor();
                        return boost::shared_ptr<ICompletableFuture<ringbuffer::ReadResultSet<E> > >(
                                new internal::executor::CompletedFuture<ringbuffer::ReadResultSet<E> >(
                                        executionException, userExecutor));
                    }

                    util::Preconditions::checkTrue(maxCount <= bufferCapacity.get(),
                                                   "the maxCount should be smaller than or equal to the capacity");
                    util::Preconditions::checkMax(maxCount, MAX_BATCH_SIZE, "maxCount");

                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::RingbufferReadManyCodec::encodeRequest(
                            name,
                            startSequence,
                            minCount,
                            maxCount,
                            &filterData);

                    try {
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = spi::impl::ClientInvocation::create(
                                getContext(), request, getName(), partitionId)->invoke();
                        return boost::shared_ptr<ICompletableFuture<ringbuffer::ReadResultSet<E> > >(
                                new internal::ClientDelegatingFuture<ringbuffer::ReadResultSet<E> >(invocationFuture,
                                                                                                    getSerializationService(),
                                                                                                    READ_MANY_ASYNC_RESPONSE_DECODER));

                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }

                    return boost::shared_ptr<ICompletableFuture<ringbuffer::ReadResultSet<E> > >();
                }

                virtual SerializationService &getSerializationService() {
                    return getContext().getSerializationService();
                }

            private:
                class ReadManyAsyncResponseDecoder : public impl::ClientMessageDecoder<ringbuffer::ReadResultSet<E> > {
                public:
                    virtual boost::shared_ptr<ringbuffer::ReadResultSet<E> >
                    decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                        serialization::pimpl::SerializationService &serializationService) {
                        protocol::codec::RingbufferReadManyCodec::ResponseParameters params = protocol::codec::RingbufferReadManyCodec::ResponseParameters::decode(
                                *clientMessage);
                        return boost::shared_ptr<ringbuffer::ReadResultSet<E> >(
                                new ringbuffer::ReadResultSet<E>(params.readCount, params.items,
                                                                 serializationService, params.itemSeqs,
                                                                 params.itemSeqsExist,
                                                                 (params.nextSeqExist ? params.nextSeq
                                                                                      : ringbuffer::ReadResultSet<E>::SEQUENCE_UNAVAILABLE)));
                    }
                };

                static void checkSequence(int64_t sequence) {
                    if (sequence < 0) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                "ClientRingbufferProxy::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                                        << sequence).build();
                    }
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> &request, int partitionId) {
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request, partitionId);

                    return (T) CODEC::decode(*response).response;
                }

                util::Atomic<int64_t> bufferCapacity;
                static const boost::shared_ptr<ReadManyAsyncResponseDecoder> READ_MANY_ASYNC_RESPONSE_DECODER;
            };

            template<typename E>
            const int32_t ClientRingbufferProxy<E>::MAX_BATCH_SIZE = 1000;

            template<typename E>
            const std::string ClientRingbufferProxy<E>::SERVICE_NAME = "hz:impl:ringbufferService";

            template<typename E>
            const boost::shared_ptr<typename ClientRingbufferProxy<E>::ReadManyAsyncResponseDecoder> ClientRingbufferProxy<E>::READ_MANY_ASYNC_RESPONSE_DECODER(
                    new ClientRingbufferProxy<E>::ReadManyAsyncResponseDecoder());
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_CLIENTRINGBUFFERPROXY_H_

