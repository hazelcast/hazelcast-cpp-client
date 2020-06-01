# Code Samples

This folder contains an extensive collection of Hazelcast C++ Client code samples, which helps you to learn how to use Hazelcast features. The following lists the samples with their brief descriptions.

**aws** — Configure and use AWS discovery.

**backpressure** — Enable backpressure on client invocations.

**command-line-tool** — Simple tool which can be used to insert simple entries to hazelcast structures and display contents of Hazelcast structures in the cluster.

**distributed-collections** — Usage of basic collections.
  * **blocking-queue**: Shows simple usage of IQueue, blocking queue.
  * **item-listeners**: Shows simple usage of IQueue item listeners.
  * **list**: Shows simple usage of IList add and read.
  * **ringbuffer**: Shows simple usages of Ringbuffer item add and read for simple type and polymorphic objects.
  * **set**: Shows simple usage of ISet add and read.

**distributed-map** — Different examples of using IMap interface.
  * ****basic****: Illustrates simple IMap put, get and netrySet methods.
  * **custom-attributes**: Illustrates IMap query based on object attributes.
  * **entry-listener**: Illustrates IMap entry listener usage.
  * **entry-processor**: Illustrates different IMap entry processor usages.
  * **eviction**: Illustrates IMap eviction process and how it works.
  * **index**: Illustrates IMap Sql query on an indexed field.
  * **locking**: Illustrates how races on IMap updates can occur and how to avoid races using optimistic or pessimistic updates which involves IMap lock usages.
  * **map-interceptor**: Illustrates how to add interceptor for the IMap at the server side and how it affects the IMap get.
  * **multimap**: Illustrates how to use MultiMap which allows multiple values for a key.
  * **near-cache**: Illustrates how you can configure near-cache feature for IMap with different eviction policies.
  * **partitionaware**: Illustrates how you can use partition aware keys for IMap for putting the data into certain partitions.
  * **query**: Illustrates how you can use different Predicate types for querying data from IMap.
  * **removeAll**: Illustrates how you can use IMap::removeAll with query predicate to remove certain entries based on the query matching.

**distributed primitives** — Examples for basic distributed primitive structures.
  * **crdt-pncounter**: Illustrates simple conflict free pozitive negtative counter usage.

**distributed-topic** — Usage of distributed topic.
  * **basic-pub-sub**: Illustrates basic publish/subcribe for distributed topic usage.
  * **reliable-topic**: Illustrates reliable topic publish and message listener.

**event-properties** — Example for configuring event delivery properties, the event thread count and max event queue size.

**invocation-timeouts** — Simple example to demonstrate how to set the client invocation timeout.

**learning-basic** — Basic examples related to the logger, instance management and name id generation.
  * **configure-logging**: Shows how you can set the log level for the client.
  * **destroying-instances**: Shows simple usage for detroying Hazelcast structures such as IQueue.
  * **unique-names**: Shows an example on how you can get cluster-wide unique names.

**monitoring** — Illustrates how you can register for cluster member changes and listen to member changes.

**network-configuration** — Examples related to network configuration.
  * **connection-strategy**: Illustrates how you can set different network strategies such as async start, async reconnect or non-reconnecting client.
  * **suffle-memberlist**: Shows how to turn the memberslist shuffling on when connecting to the cluster.
  * **socket-interceptor**: Shows an example interceptor which you can use to intercept cluster connection.
  * **tcpip**: Illustrates the configuration for member discovery using a provided tcp/ip address for the cluster member.

**Org.Web.Samples** — The examples presented at the https://hazelcast.org/ website.

**serialization** — Examples for different Hazelcast serialization techniques.
  * **custom**: Illustrates how you can use configure and custom serializers.
  * **global-serializer**: Illustrates how you can use configure global serializer to be used when no matching serializer exist for a provided object.
  * **identified-data-serializable**: Shows hoe to implement and configure IdentifiedDataSerializable serialization for objects.
  * **portable**: Illustrates the configuration and usage of Portable objects.
  * **portable**: Illustrates the configuration and usage of Portable objects.

**spi**: - Example for sing the HazelcastClient::getDistributedObject API.

**tls** — Example for showing how to configure SSL/TLS encryption for cluster communication.

**transactions** — Illustrates the usage of transactions in the cluster.