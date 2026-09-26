#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <type_traits>
#include <memory>

#include "Core/ThreadSafe.h"

// Multithreading is a nightmare...
// its been days

class ThreadPool {
public:
	ThreadPool(int target_count);
	~ThreadPool();

	int ThreadCount() const { return m_Threads.size(); }
	int ActiveThreadCount() const { return m_ActiveThreadCount; }
	int TaskCount() const { return m_TaskQueue.Size(); }

	template<typename T>
	void Submit(T func) {
		m_TaskQueue.Push(func);
	}

private:
	void WorkerThreadProc();

private:
	std::atomic_bool m_Finished;

	std::vector<std::thread> m_Threads;
	
	ThreadSafe::Queue<std::function<void()>> m_TaskQueue;
	std::atomic_int m_ActiveThreadCount;
};