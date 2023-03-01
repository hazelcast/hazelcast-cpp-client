1. SqlExecuteCodec is updated isPartitionArgumentIndexExists and partitionArgumentIndex
    hazelcast/src/main/java/com/hazelcast/client/impl/protocol/codec/SqlExecuteCodec.java

2. New property for sql partition cache size
    PARTITION_ARGUMENT_CACHE_SIZE
    hazelcast/src/main/java/com/hazelcast/client/properties/ClientProperty.java

3. Put a variable partitionArgumentIndex to SQLStatement
    hazelcast/src/main/java/com/hazelcast/sql/SqlStatement.java

4. Write LRU cache
    hazelcast/src/main/java/com/hazelcast/sql/impl/client/ReadOptimizedLruCache.java

5. a. Add cache and new properties to the SqlClientService
   b. query the partition id in statement, if does not exist look at the cache via sql statement and get the correct connection
   c. When the execute reponse is received get the partition id from the result and put it to the cache
   hazelcast/src/main/java/com/hazelcast/sql/impl/client/SqlClientService.java



Cache Used methods:

Constructor with size
get by key
size
put(key,value)
remove(key)
values()
