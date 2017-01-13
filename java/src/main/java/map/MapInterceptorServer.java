package map;

import com.hazelcast.config.Config;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;

import java.io.IOException;

class MapInterceptor implements IdentifiedDataSerializable, com.hazelcast.map.MapInterceptor {

    public MapInterceptor () {

    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 7;
    }

    public void writeData(ObjectDataOutput out) throws IOException {

    }

    public void readData(ObjectDataInput in) throws IOException {

    }

    @Override
    public Object interceptGet(Object value) {
        return value;
    }

    @Override
    public void afterGet(Object value) {
    }

    @Override
    public Object interceptPut(Object oldValue, Object newValue) {
        return null;
    }

    @Override
    public void afterPut(Object value) {
    }

    @Override
    public Object interceptRemove(Object removedValue) {
        return null;
    }

    @Override
    public void afterRemove(Object value) {
    }
}

public class MapInterceptorServer {
    public static void main(String args[]) {
        final Config config = new XmlConfigBuilder().build();
        config.getSerializationConfig().addDataSerializableFactory(1, new DataSerializableFactory() {
            @Override
            public IdentifiedDataSerializable create(int typeId) {
                return new MapInterceptor();
            }
        });
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
    }
}
