//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LISTENER_SUPPORT
#define HAZELCAST_LISTENER_SUPPORT

#include "Data.h"
#include "../../util/Thread.h"
#include "ResponseStream.h"
#include "InvocationService.h"
#include "ClientContext.h"

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
                : thread(ListenerSupport::run, this)
                , invocationService(invocationService)
                , request(request)
                , eventHandler(eventHandler)
                , key(key)
                , hasKey(true)
                , active(true) {

                };

                ListenerSupport(InvocationService& invocationService, const Request& request, const EventHandler& eventHandler)
                : invocationService(invocationService)
                , request(request)
                , eventHandler(eventHandler)
                , hasKey(false)
                , active(true) {

                };

                void *run(void *input) {
                    static_cast<ListenerSupport *>(input)->listenImpl();
                    return NULL;

                };


                void listen() {
                    thread.start();
                };


                void stop() {
                    active = false;
                    lastStream.end();
                };


            private:
                void listenImpl() {
                    while (active) {
                        EventResponseHandler eventResponseHandler(*this);
                        try{
                            if (hasKey) {
                                invocationService.invokeOnKeyOwner(request, key, eventResponseHandler);
                            } else {
                                invocationService.invokeOnRandomTarget(request, eventResponseHandler);
                            }
                        }catch(exception::IException & ignored){

                        }
                    }
                };


                class EventResponseHandler {

                public:
                    EventResponseHandler(ListenerSupport& listenerSupport)
                    : listenerSupport(listenerSupport) {

                    };

                    void handle(ResponseStream & stream) {
                        stream.read<std::string>(); // initial ok response  // registrationId
                        listenerSupport.lastStream = stream;
                        while (listenerSupport.active) {
                            try {
                                Event event = stream.read<Event>();
                                listenerSupport.eventHandler.handle(event);
                            } catch(exception::IOException& e){
                                throw e;
                            } catch (exception::IException& e) {
                                try {
                                    stream.end();
                                } catch (exception::IOException& ignored) {
                                }
                                listenerSupport.active = false;
                            }
                        }
                    };
                private :
                    ListenerSupport & listenerSupport;
                };

                util::Thread thread;
                InvocationService& invocationService;
                ResponseStream& lastStream;
                bool hasKey;
                serialization::Data key;
                Request request;
                EventHandler eventHandler;
                volatile bool active;

            };
        }
    }
}

#endif //HAZELCAST_LISTENER_SUPPORT
