//========================================================================
// Zinc - Unit Testing
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

#include "gtest/gtest.h"
#include "http/thread_pool.h"

//--------------------------------------------------------------

class TestTask : public ThreadPool::Task {
public:
    TestTask(int & task, int & dest)
      : taskCount_(task), destructorCount_(dest) {
    }

    ~TestTask() {
        std::lock_guard<std::mutex> lock(mutex_);
        destructorCount_++;
    }

    void run(int no) {
        if (true) {
            std::lock_guard<std::mutex> lock(mutex_);
            taskCount_++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

private:
    int &   taskCount_;
    int &   destructorCount_;
    static  std::mutex mutex_;
};

std::mutex TestTask::mutex_;

//--------------------------------------------------------------
// Test the URI class (simple path, no query string).
//--------------------------------------------------------------

TEST(ThreadPool, Pool) {
    ThreadPool pool;
    int tasks = 0, destructors = 0;

    for (int i = 0; i < 8; i++) {
        EXPECT_TRUE(pool.addTask(std::make_unique<TestTask>(tasks, destructors), 16));
    }
    EXPECT_EQ(pool.getThreadCount(), 8);
    EXPECT_LE(pool.getIdleThreadCount(), 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(pool.getThreadCount(), 8);
    EXPECT_EQ(pool.getIdleThreadCount(), 8);

    for (int i = 0; i < 4; i++) {
        EXPECT_TRUE(pool.addTask(std::make_unique<TestTask>(tasks, destructors), 16));
    }
    EXPECT_EQ(pool.getThreadCount(), 8);
    EXPECT_LE(pool.getIdleThreadCount(), 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(pool.getThreadCount(), 8);
    EXPECT_EQ(pool.getIdleThreadCount(), 8);

    EXPECT_EQ(tasks, 12);
    EXPECT_EQ(destructors, 12);
}

//========================================================================
