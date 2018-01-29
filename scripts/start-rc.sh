#!/bin/sh

function cleanup {
    echo "cleanup is being performed."
    if [ "x${rcPid}" != "x" ]
    then
        echo "Killing remote controller server with pid ${rcPid}"
        kill -9 ${rcPid}
    fi
    exit
}

# Disables printing security sensitive data to the logs
set +x

trap cleanup EXIT

HZ_VERSION="3.9.1"
HAZELCAST_TEST_VERSION="3.10-SNAPSHOT"
HAZELCAST_VERSION=${HZ_VERSION}
HAZELCAST_ENTERPRISE_VERSION=${HZ_VERSION}
HAZELCAST_RC_VERSION="0.4-SNAPSHOT"
SNAPSHOT_REPO="https://oss.sonatype.org/content/repositories/snapshots"
RELEASE_REPO="http://repo1.maven.apache.org/maven2"
ENTERPRISE_RELEASE_REPO="https://repository-hazelcast-l337.forge.cloudbees.com/release/"
ENTERPRISE_SNAPSHOT_REPO="https://repository-hazelcast-l337.forge.cloudbees.com/snapshot/"

if [[ ${HZ_VERSION} == *-SNAPSHOT ]]
then
	REPO=${SNAPSHOT_REPO}
	ENTERPRISE_REPO=${ENTERPRISE_SNAPSHOT_REPO}
else
	REPO=${RELEASE_REPO}
	ENTERPRISE_REPO=${ENTERPRISE_RELEASE_REPO}
fi

if [ -f "hazelcast-remote-controller-${HAZELCAST_RC_VERSION}.jar" ]; then
    echo "remote controller already exist, not downloading from maven."
else
    echo "Downloading: remote-controller jar com.hazelcast:hazelcast-remote-controller:${HAZELCAST_RC_VERSION}"
    mvn -q dependency:get -DrepoUrl=${SNAPSHOT_REPO} -Dartifact=com.hazelcast:hazelcast-remote-controller:${HAZELCAST_RC_VERSION} -Ddest=hazelcast-remote-controller-${HAZELCAST_RC_VERSION}.jar
    if [ $? -ne 0 ]; then
        echo "Failed download remote-controller jar com.hazelcast:hazelcast-remote-controller:${HAZELCAST_RC_VERSION}"
        exit 1
    fi
fi

if [ -f "hazelcast-${HAZELCAST_TEST_VERSION}-tests.jar" ]; then
    echo "hazelcast-test.jar already exists, not downloading from maven."
else
    echo "Downloading: hazelcast test jar com.hazelcast:hazelcast:${HAZELCAST_TEST_VERSION}:jar:tests"
    mvn -q dependency:get -DrepoUrl=${TEST_REPO} -Dartifact=com.hazelcast:hazelcast:${HAZELCAST_TEST_VERSION}:jar:tests -Ddest=hazelcast-${HAZELCAST_TEST_VERSION}-tests.jar
    if [ $? -ne 0 ]; then
        echo "Failed download hazelcast test jar com.hazelcast:hazelcast:${HAZELCAST_TEST_VERSION}:jar:tests"
        exit 1
    fi
fi

if [ -f "hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}.jar" ]; then
echo "hazelcast-enterprise.jar already exists, not downloading from maven."
else
    echo "Downloading: hazelcast enterprise jar com.hazelcast:hazelcast-enterprise:${HAZELCAST_ENTERPRISE_VERSION}"
    mvn -q dependency:get -DrepoUrl=${ENTERPRISE_REPO} -Dartifact=com.hazelcast:hazelcast-enterprise:${HAZELCAST_ENTERPRISE_VERSION} -Ddest=hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}.jar
    if [ $? -ne 0 ]; then
        echo "Failed download hazelcast enterprise jar com.hazelcast:hazelcast-enterprise:${HAZELCAST_ENTERPRISE_VERSION}"
        exit 1
    fi
fi
CLASSPATH="hazelcast-remote-controller-${HAZELCAST_RC_VERSION}.jar:hazelcast-enterprise-${HAZELCAST_VERSION}.jar:hazelcast-${HAZELCAST_VERSION}.jar:hazelcast-${HAZELCAST_TEST_VERSION}-tests.jar:test/javaclasses"
echo "Starting Remote Controller ... enterprise ..."

java -Dhazelcast.enterprise.license.key=${HAZELCAST_ENTERPRISE_KEY} -cp ${CLASSPATH} com.hazelcast.remotecontroller.Main &
rcPid=$!
wait ${rcPid}
exit $?
