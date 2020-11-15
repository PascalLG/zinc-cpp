//========================================================================
// Zinc - Web Server
// Copyright (c) 2019, Pascal Levy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//========================================================================

#include "../misc/logger.h"
#include "thread_pool.h"

//========================================================================
// ThreadPool
//
// Implement a thread pool. Initially the pool is empty. New threads are
// created on the fly as tasks are added to the queue. When a task is done,
// its worker thread remains active and can be affected to another task
// in the queue.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ThreadPool::ThreadPool()
  : idle_(0),
    stop_(false) {
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

ThreadPool::~ThreadPool() {
    stopAll();
}

//--------------------------------------------------------------
// Add a task to the queue, increasing the number of worker
// threads if none is available to process this new task.
//--------------------------------------------------------------

bool ThreadPool::addTask(std::unique_ptr<Task> task, size_t limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (idle_ == 0) {
        size_t n = workers_.size();
        if (n >= limit) {
            return false;
        }
        workers_.emplace_back([this, n] {
            this->worker(static_cast<int>(n + 1));
        });
    }
    tasks_.push(std::move(task));
    condition_.notify_one();
    return true;
}

//--------------------------------------------------------------
// Stop all the worker threads and wait for their termination.
//--------------------------------------------------------------

void ThreadPool::stopAll() {
    if (workers_.size()) {
        stop_ = true;
        condition_.notify_all();
        for (std::thread & th: workers_) {
            th.join();
        }
        workers_.clear();
        idle_ = 0;
        stop_ = false;
    }
}

//--------------------------------------------------------------
// Worker thread.
//--------------------------------------------------------------

void ThreadPool::worker(int no) {
    logger::registerWorkerThread(no);
    LOG_TRACE("Start thread #" << no);
    for( ; ; ) {
        std::unique_ptr<Task> task;
        if (true) {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->idle_++;
            this->condition_.wait(lock, [this] {
                return this->stop_ || !this->tasks_.empty();
            });
            if(this->stop_) {
                LOG_TRACE("Stop thread #" << no);
                return;
            }
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
            this->idle_--;
        }
        task->run(no);
    }
}

//========================================================================
