#include "hazelcast/util/WindowsThread.inl"

namespace hazelcast {
    namespace util {
#define MILLIS_IN_A_SECOND 1000
#define NANOS_IN_A_SECOND (NANOS_IN_A_MILLISECOND * MILLIS_IN_A_SECOND)
#define NANOS_IN_A_MILLISECOND (NANOS_IN_A_USECOND * 1000)
#define NANOS_IN_A_USECOND 1000

        ConditionVariable::ConditionVariable() {
            InitializeConditionVariable(&condition);
        }

        ConditionVariable::~ConditionVariable() {
        }

        void ConditionVariable::wait(Mutex &mutex) {
            BOOL success = SleepConditionVariableCS(&condition, &(mutex.mutex), INFINITE);
            assert(success && "SleepConditionVariable");
        }

        bool ConditionVariable::waitFor(Mutex &mutex, int64_t timeInMilliseconds) {
            BOOL interrupted = SleepConditionVariableCS(&condition, &(mutex.mutex), (DWORD) timeInMilliseconds);
            if (interrupted) {
                return true;
            }
            return false;
        }

        bool ConditionVariable::waitNanos(Mutex &mutex, int64_t nanos) {
            BOOL interrupted = SleepConditionVariableCS(&condition, &(mutex.mutex),
                                                        (DWORD)(nanos / NANOS_IN_A_MILLISECOND));
            if (interrupted) {
                return true;
            }
            return false;
        }

        void ConditionVariable::notify() {
            WakeConditionVariable(&condition);
        }

        void ConditionVariable::notify_all() {
            WakeAllConditionVariable(&condition);
        }
    }
}