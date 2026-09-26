#include "Core/ThreadPool.h"
#include "Core/Utils.h"

ThreadPool::ThreadPool(int target_count): m_Finished(false) { 
	const int hardware_thread_count = std::thread::hardware_concurrency();
	int thread_count = target_count;
	if (thread_count > hardware_thread_count) thread_count = hardware_thread_count - 1;

	try { 
		for (int i = 0; i < thread_count; i++) {
			m_Threads.push_back(std::thread(&ThreadPool::WorkerThreadProc, this));
		}
	}
	catch (...) {
		m_Finished = true;
		ASSERT(false, "Failed to create thread pool worker threads");
	}
}
ThreadPool::~ThreadPool() { 
	m_Finished = true;
	for (std::thread& thread : m_Threads) { thread.join(); }
}

void ThreadPool::WorkerThreadProc() {
	while (!m_Finished) {
		if (!m_TaskQueue.Empty()) {
			m_ActiveThreadCount++;
			std::function<void()> task;
			if (m_TaskQueue.Next(task)) task();
			m_ActiveThreadCount--;
		}
		else std::this_thread::yield();
	}
}