#include <cassert>
#include <errno.h>
#include <sys/time.h>
#include <limits>
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"

namespace hazelcast {
    namespace util {
#define MILLIS_IN_A_SECOND 1000
#define NANOS_IN_A_SECOND (NANOS_IN_A_MILLISECOND * MILLIS_IN_A_SECOND)
#define NANOS_IN_A_MILLISECOND (NANOS_IN_A_USECOND * 1000)
#define NANOS_IN_A_USECOND 1000

        ConditionVariable::ConditionVariable() {
            int error = pthread_cond_init(&condition, NULL);
            (void) error;
            assert(EAGAIN != error);
            assert(ENOMEM != error);
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        ConditionVariable::~ConditionVariable() {
            int error = pthread_cond_destroy(&condition);
            (void) error;
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        bool ConditionVariable::waitNanos(Mutex &mutex, int64_t nanos) {
            struct timespec ts = calculateTimeFromNanos(nanos);

            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            (void) error;
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        bool ConditionVariable::waitFor(Mutex &mutex, int64_t timeInMilliseconds) {
            struct timespec ts = calculateTimeFromMilliseconds(timeInMilliseconds);

            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            (void) error;
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        struct timespec ConditionVariable::calculateTimeFromMilliseconds(int64_t timeInMilliseconds) const {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * NANOS_IN_A_USECOND;
            int64_t seconds = timeInMilliseconds / MILLIS_IN_A_SECOND;
            if (seconds > std::numeric_limits<time_t>::max()) {
                ts.tv_sec = std::numeric_limits<time_t>::max();
            } else {
                ts.tv_sec += (time_t) seconds;
                int64_t nsec = ts.tv_nsec + (timeInMilliseconds % MILLIS_IN_A_SECOND) * NANOS_IN_A_MILLISECOND;
                if (nsec >= NANOS_IN_A_SECOND) {
                    nsec -= NANOS_IN_A_SECOND;
                    ++ts.tv_sec;
                }
                ts.tv_nsec = nsec;
            }
            return ts;
        }

        struct timespec ConditionVariable::calculateTimeFromNanos(int64_t nanos) const {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * NANOS_IN_A_USECOND;
            int64_t seconds = nanos / NANOS_IN_A_SECOND;
            if (seconds > std::numeric_limits<time_t>::max()) {
                ts.tv_sec = std::numeric_limits<time_t>::max();
            } else {
                ts.tv_sec += (time_t) seconds;
                int64_t nsec = ts.tv_nsec + (nanos % NANOS_IN_A_SECOND);
                if (nsec >= NANOS_IN_A_SECOND) {
                    nsec -= NANOS_IN_A_SECOND;
                    ++ts.tv_sec;
                }
                ts.tv_nsec = nsec;
            }
            return ts;
        }

        void ConditionVariable::wait(Mutex &mutex) {
            int error = pthread_cond_wait(&condition, &(mutex.mutex));
            (void) error;
            assert (EPERM != error);
            assert (EINVAL != error);
        }

        void ConditionVariable::notify() {
            int error = pthread_cond_signal(&condition);
            (void) error;
            assert(EINVAL != error);
        }

        void ConditionVariable::notify_all() {
            int error = pthread_cond_broadcast(&condition);
            (void) error;
            assert(EINVAL != error);
        }
    }
}
