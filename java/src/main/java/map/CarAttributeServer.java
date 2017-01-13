package map;

import com.hazelcast.config.Config;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;
import com.hazelcast.query.extractor.ValueCollector;
import com.hazelcast.query.extractor.ValueExtractor;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

class CarAttributeExtractor extends ValueExtractor<Car, String> {

    @Override
    public void extract(Car car, String argument, ValueCollector valueCollector) {
        valueCollector.addObject(car.getAttribute(argument));
    }
}


class Car implements IdentifiedDataSerializable {
    private Map <String,String> attributes = new HashMap<String, String>();

    public Car () {

    }
    public Car (String name) {
        attributes.put("name",name);
        attributes.put("tripStart","0");
        attributes.put("tripStop","0");
    }

    public Car(String name,int breakHorsePower, int mileage) {
        attributes.put("name",name);
        attributes.put("tripStart","0");
        attributes.put("tripStop","0");
        attributes.put("bhp",breakHorsePower + "");
        attributes.put("mileage",mileage + "");

    }

    public String getAttribute(String key) {
        return attributes.get(key);
    }

    public Map<String,String> getAttributes () {
        return attributes;
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 4;
    }

    public void writeData(ObjectDataOutput out) throws IOException{
        out.writeInt(attributes.size());
        for(Map.Entry<String, String> entry : attributes.entrySet()){
            out.writeUTF( (String) entry.getKey());
            out.writeUTF( (String) entry.getValue());
            System.out.println("Writing " + entry.getKey() + " -->" + entry.getValue());
        }
    }

    public void readData(ObjectDataInput in) throws IOException {
        int size = in.readInt();
        if (size > 0) {
            for (int i = 0; i < size; ++i) {
                String key = in.readUTF();
                String value = in.readUTF();
                System.out.println("Writing " + key + " -->" + value);
                attributes.put(key,value);
            }
        }
        else{
            attributes.clear();
        }
    }
}
public class CarAttributeServer
{
    public static void main(String args[]) {
        final Config config = new XmlConfigBuilder().build();
        config.getSerializationConfig().addDataSerializableFactory(1, new DataSerializableFactory() {
            @Override
            public IdentifiedDataSerializable create(int typeId) {
                return new Car();
            }
        });
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
    }
}
