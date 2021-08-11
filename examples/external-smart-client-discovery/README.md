#External Smart Client Discovery

The client sends requests directly to cluster members in the smart client mode (default) in order to reduce hops to accomplish operations. Because of that, the client should know the addresses of members in the cluster.

In cloud-like environments, or Kubernetes, there are usually two network interfaces, the private and the public network. When the client is in the same network as the members, it uses their private network addresses. Otherwise, if the client and the Hazelcast cluster are on different networks, the client cannot connect to members using their private network addresses. Hazelcast 4.2 introduced External Smart Client Discovery to solve that issue.

In order to use this feature, make sure your cluster members are accessible from the network the client resides in, then set config `client_network_config()::use_public_address(true)` to true. You should specify the public address of at least one member in the configuration:

```c++
    hazelcast::client::client_config config;
    constexpr const int32_t server_port = 5701;
    constexpr const char *server_public_address = "myserver.publicaddress.com";
    config.get_network_config().use_public_address(true).add_address(
            hazelcast::client::address{server_public_address, server_port});
```

This solution works everywhere without further configuration: Kubernetes, AWS, GCP, Azure, etc. as long as the corresponding plugin is enabled in Hazelcast server configuration.
