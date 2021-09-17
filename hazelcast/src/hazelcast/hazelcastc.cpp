#include "hazelcast/hazelcastc.h"
#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

namespace hazelcast {
    namespace client {
        class client_manager {
        public:
            /**
             * Creates a new client and registers to the clients_ map
             *
             * @param config The configuration used to create the client
             * @return the newly created client id.
             */
            static int new_client() {
                return register_client(std::move(hazelcast::new_client().get()));
            }

            /**
             * Creates a new client with the config and registers to the clients_ map
             *
             * @param config The configuration used to create the client
             * @return the newly created client id.
             */
            static int new_client(client_config config) {
                return register_client(hazelcast::new_client(std::move(config)).get());
            }

            /**
             *
             * @param client_id The id of the client
             * @return The future if such a client exists,
             */
            static std::shared_ptr<impl::hazelcast_client_instance_impl> get_client(int client_id) {
                return clients_.get(client_id);
            }

            /**
             *
             * @param client_id The client to be shutdown and release its resources.
             * @return 0 on success, non-zero on failure.
             */
            static int shutdown_client(int client_id) {
                auto client = clients_.remove(client_id);
                if (!client) {
                    return -1;
                }

                try {
                    client->shutdown();
                    return 0;
                } catch (exception::iexception &e) {
                    return e.get_error_code();
                }
            }

        private:
            static util::SynchronizedMap<int, impl::hazelcast_client_instance_impl> clients_;

            /**
             * Creates a new client and registers to the \clients_ map if the client is connected successfully.
             *
             * @return the newly created client id.
             */
            static int register_client(hazelcast_client client) {
                spi::ClientContext ctx(client);
                auto client_impl = ctx.get_hazelcast_client_implementation();
                auto id = client_impl->id();
                clients_.put(id, client_impl);
                return id;
            }
        };

        util::SynchronizedMap<int, impl::hazelcast_client_instance_impl> client_manager::clients_;
    }
}


int hazelcast_new_client(int *id, const char *server, int port) {
    using namespace hazelcast::client;
    using namespace hazelcast;
    client_config config;
    config.get_network_config().add_address(address(server, port));
    try {
        *id = client_manager::new_client(std::move(config));
        return 0;
    } catch (exception::iexception &e) {
        return e.get_error_code();
    }
}

int hazelcast_increment_atomic_long(int client_id, const char *atomic_long_name) {
    using namespace hazelcast::client;
    using namespace hazelcast;

    auto client = client_manager::get_client(client_id);
    if (!client) {
        return -1;
    }

    try {
        auto counter = client->get_cp_subsystem().get_atomic_long(atomic_long_name).get();
        counter->increment_and_get().get();
        return 0;
    } catch (exception::iexception &e) {
        return e.get_error_code();
    }
}

int hazelcast_get_atomic_long_value(int client_id, const char *atomic_long_name, int64_t *value) {
    using namespace hazelcast::client;
    using namespace hazelcast;

    auto client = client_manager::get_client(client_id);
    if (!client) {
        return -1;
    }

    try {
        auto counter = client->get_cp_subsystem().get_atomic_long(atomic_long_name).get();
        *value = counter->get().get();
        return 0;
    } catch (exception::iexception &e) {
        return e.get_error_code();
    }
}

int hazelcast_shutdown_client(int client_id) {
    return hazelcast::client::client_manager::shutdown_client(client_id);
}