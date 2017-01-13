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

class Person implements IdentifiedDataSerializable {
    private String name;
    private boolean male;
    private int age;
    public Person () {

    }
    public Person (Person p) {
        name = p.name;
        male = p.male;
        age = p.age;
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 3;
    }

    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeUTF(name);
        out.writeBoolean(male);
        out.writeInt(age);
    }

    public void readData(ObjectDataInput in) throws IOException {
        name = in.readUTF();
        male = in.readBoolean();
        age = in.readInt();
    }
}

public class IndexQueryServer {

    public static void main(String args[]) {
        final Config config = new XmlConfigBuilder().build();
        config.getSerializationConfig().addDataSerializableFactory(1, new DataSerializableFactory() {
            @Override
            public IdentifiedDataSerializable create(int typeId) {
                return new Person();
            }
        });
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
    }
}