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

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

//--------------------------------------------------------------

//--------------------------------------------------------------

class ThreadPool {
public:
    ThreadPool();
    virtual ~ThreadPool();

    class Task {
    public:
        virtual ~Task() = default;

        virtual void run(int no) = 0;
    };

    bool    addTask(std::unique_ptr<Task> obj, size_t limit);
    void    stopAll();

    size_t  getThreadCount() const          { return workers_.size();               }
    size_t  getIdleThreadCount() const      { return static_cast<size_t>(idle_);    }

private:
    std::queue<std::unique_ptr<Task>>   tasks_;         // queue of tasks waiting to be processed
    std::vector<std::thread>            workers_;       // array of worker threads
    std::condition_variable             condition_;     // thread synchronization
    std::mutex                          mutex_;         // thread synchronization
    int                                 idle_;          // number of currently idle threads
    bool                                stop_;          // shutdown request

    void    worker(int no);
};

//--------------------------------------------------------------

#endif

//========================================================================
