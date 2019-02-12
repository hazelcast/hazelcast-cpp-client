/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hazelcast/client/IAtomicLong.h"

namespace hazelcast {
    namespace client {

        int64_t IAtomicLong::addAndGet(int64_t delta) {
            return impl->addAndGet(delta);
        }

        bool IAtomicLong::compareAndSet(int64_t expect, int64_t update) {
            return impl->compareAndSet(expect, update);
        }

        int64_t IAtomicLong::decrementAndGet() {
            return impl->decrementAndGet();
        }

        int64_t IAtomicLong::get() {
            return impl->get();
        }

        int64_t IAtomicLong::getAndAdd(int64_t delta) {
            return impl->getAndAdd(delta);
        }

        int64_t IAtomicLong::getAndSet(int64_t newValue) {
            return impl->getAndSet(newValue);
        }

        int64_t IAtomicLong::incrementAndGet() {
            return impl->incrementAndGet();
        }

        int64_t IAtomicLong::getAndIncrement() {
            return impl->getAndIncrement();
        }

        void IAtomicLong::set(int64_t newValue) {
            impl->set(newValue);
        }

        IAtomicLong::IAtomicLong(const boost::shared_ptr<impl::AtomicLongInterface> &impl) : impl(impl) {}

        const std::string &IAtomicLong::getServiceName() const {
            return impl->getServiceName();
        }

        const std::string &IAtomicLong::getName() const {
            return impl->getName();
        }

        void IAtomicLong::destroy() {
            impl->destroy();
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::addAndGetAsync(int64_t delta) {
            return impl->addAndGetAsync(delta);
        }

        boost::shared_ptr<ICompletableFuture<bool> > IAtomicLong::compareAndSetAsync(int64_t expect, int64_t update) {
            return impl->compareAndSetAsync(expect, update);
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::decrementAndGetAsync() {
            return impl->decrementAndGetAsync();
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAsync() {
            return impl->getAsync();
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndAddAsync(int64_t delta) {
            return impl->getAndAddAsync(delta);
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndSetAsync(int64_t newValue) {
            return impl->getAndSetAsync(newValue);
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::incrementAndGetAsync() {
            return impl->incrementAndGetAsync();
        }

        boost::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndIncrementAsync() {
            return impl->getAndIncrementAsync();
        }

        boost::shared_ptr<ICompletableFuture<void> > IAtomicLong::setAsync(int64_t newValue) {
            return impl->setAsync(newValue);
        }
    }
}
