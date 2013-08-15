import com.hazelcast.config.Config;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;

/**
 * Created with IntelliJ IDEA.
 * User: turkmen
 * Date: 14.08.2013
 * Time: 10:45
 * To change this template use File | Settings | File Templates.
 */
public class creator {
    public static void main(String [] args){
        try{
        final Config cfg = new XmlConfigBuilder("/Users/turkmen/IdeaProjects/hzSample/src/main/resources/hazelcast.xml").build();
        HazelcastInstance hazelcastInstance = Hazelcast.newHazelcastInstance(cfg);
        }
        catch (Exception e){
                  e.printStackTrace();
        }
    }
}
