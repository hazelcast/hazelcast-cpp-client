#pragma once

#include "hazelcast/util/export.h"
#include "hazelcast/client/hazelcast_client.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#ifdef __cplusplus
extern "C" {
#endif
    /**
     *
     * @param client_id The id of the client
     * @param server The server hostname or ip address
     * @param port The port the server is started at
     * @return 0 on success. On error, non-zero error code is returned.
     */
    int hazelcast_new_client(int *client_id, const char *server, int port);

    /**
     *
     * @param client_id The id of the client
     * @param atomic_long_name the name of the \atomic_long to be incremented
     * @return 0 on success, error code on failure
     */
    int hazelcast_increment_atomic_long(int client_id, const char *atomic_long_name);

    /**
     *
     * @param client_id The id of the client
     * @param atomic_long_name the name of the \atomic_long
     * @param value The value for the \atomic_long
     * @return 0 on success, non-zero on error.
     */
    int hazelcast_get_atomic_long_value(int client_id, const char *atomic_long_name, long *value);

    /**
     *
     * @param client_id The client to be shutdown and release its resources.
     * @return 0 on success, non-zero on failure.
     */
    int hazelcast_shutdown_client(int client_id);

#ifdef __cplusplus
}
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
