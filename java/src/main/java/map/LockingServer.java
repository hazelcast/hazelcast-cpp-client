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

class Value1 implements IdentifiedDataSerializable {
    int amount;
    int version;

    public Value1 () {

    }
    public Value1 (Value1 v) {
        amount = v.amount;
        version = v.version;
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 5;
    }

    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeInt(amount);
        out.writeInt(version);
    }

    public void readData(ObjectDataInput in) throws IOException {
        amount = in.readInt();
        version = in.readInt();
    }
}

class Value2 implements IdentifiedDataSerializable {
    int amount;

    public Value2 () {

    }
    public Value2 (Value1 v) {
        amount = v.amount;
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 6;
    }

    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeInt(amount);
    }

    public void readData(ObjectDataInput in) throws IOException {
        amount = in.readInt();
    }
}

public class LockingServer {

    public static void main(String args[]) {
        final Config config = new XmlConfigBuilder().build();
        config.getSerializationConfig().addDataSerializableFactory(1, new DataSerializableFactory() {
            @Override
            public IdentifiedDataSerializable create(int typeId) {
                if(typeId == 5)
                    return new Value1();
                else
                    return new Value2();
            }
        });
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
    }
}
