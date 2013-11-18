//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LISTENER_SUPPORT
#define HAZELCAST_LISTENER_SUPPORT

#include "Data.h"
#include "ResponseStream.h"
#include "InvocationService.h"
#include "ClientContext.h"
#include "CountDownLatch.h"
#include <boost/thread.hpp>

namespace hazelcast {
    namespace client {
        namespace spi {

            class ClientContext;

            class ListenerSupportBase {
            public:
                virtual ~ListenerSupportBase() = 0;

                virtual void listen() = 0;

                virtual void stop() = 0;
            };

            inline  ListenerSupportBase::~ListenerSupportBase() {

            };

            template <typename Request, typename EventHandler, typename Event>
            class ListenerSupport : public ListenerSupportBase {
            public:
                ListenerSupport(InvocationService& invocationService, const Request& request, const EventHandler& eventHandler, const serialization::Data& key)
                : invocationService(invocationService)
                , request(request)
                , eventHandler(eventHandler)
                , key(key)
                , hasKey(true)
                , active(true)
                , latch(1) {

                };

                ListenerSupport(InvocationService& invocationService, const Request& request, const EventHandler& eventHandler)
                : invocationService(invocationService)
                , request(request)
                , eventHandler(eventHandler)
                , hasKey(false)
                , active(true)
                , latch(1) {

                };

                void listen() {
                    boost::thread listenerThread(boost::bind(&ListenerSupport<Request, EventHandler, Event>::listenImpl, this));
                    if (!latch.await(1000)) {
                        throw exception::IException("ListenerSupport::listen", "Could not register listener!!!");
                    }
                };


                void stop() {
                    active = false;
                    lastStream->end();
                };


            private:
                void listenImpl() {
                    EventResponseHandler eventResponseHandler(this);
                    while (active) {
                        try{
                            if (hasKey) {
                                invocationService.invokeOnKeyOwner(request, key, eventResponseHandler);
                            } else {
                                invocationService.invokeOnRandomTarget(request, eventResponseHandler);
                            }
                        }catch(...){
                        }
                    }
                };


                class EventResponseHandler {

                public:
                    EventResponseHandler(ListenerSupport *listenerSupport)
                    : listenerSupport(listenerSupport) {

                    };

                    void handle(ResponseStream & stream) {
                        stream.read<std::string>(); // initial ok response  // registrationId
                        listenerSupport->lastStream = &stream;
                        listenerSupport->latch.countDown();
                        while (listenerSupport->active) {
                            try {
                                Event event = stream.read<Event>();
                                if (!listenerSupport->active)
                                    break;
                                listenerSupport->eventHandler.handle(event);
                            } catch(exception::IOException& e){
                                throw e;
                            } catch (exception::IException&) {
                                try {
                                    stream.end();
                                } catch (exception::IOException&) {
                                }
                                listenerSupport->active = false;
                            }
                        }
                    };
                private :
                    std::auto_ptr<ListenerSupport> listenerSupport;
                };

                InvocationService& invocationService;
                ResponseStream *lastStream;
                bool hasKey;
                serialization::Data key;
                Request request;
                EventHandler eventHandler;
                boost::atomic<bool> active;
                util::CountDownLatch latch;

            };
        }
    }
}

#endif //HAZELCAST_LISTENER_SUPPORT
