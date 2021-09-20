require("hazelcast")

print("Hello World")
err, client = hazelcast.hazelcast_new_client("127.0.0.1", 5701)

err, counter = hazelcast.hazelcast_get_atomic_long_value(client, "my_counter")
print(counter)

err, counter = hazelcast.hazelcast_increment_atomic_long(client, "my_counter")

err, counter = hazelcast.hazelcast_get_atomic_long_value(client, "my_counter")
print(counter)

hazelcast.hazelcast_shutdown_client(client)

