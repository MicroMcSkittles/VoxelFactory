#pragma once
#include <mutex>
#include <queue>
#include <vector>
#include <cstdlib>
#include "Core/Utils.h"

// This does what it sounds like it does (nightmare,nightmare,nightmare,nightmare,nightmare,nightmare)

namespace ThreadSafe {

	template <typename T>
	class Atomic {
	public:
		Atomic() { }
		Atomic(const T& value): m_Value(value) { }
		Atomic(const Atomic<T>& other): m_Value(other.m_Value) { }
		~Atomic() { }

		T Get() {
			m_Mutex.lock();
			T value = m_Value;
			m_Mutex.unlock();
			return value;
		}
		void Set(const T& value) {
			m_Mutex.lock();
			m_Value = value;
			m_Mutex.unlock();
		}

	private:
		T m_Value;
		mutable std::mutex m_Mutex;
	};

	template <typename T>
	class Queue {
	public:
		Queue() { }
		Queue(const Queue<T>& other): m_Queue(other.m_Queue) { }
		~Queue() { }

		bool Empty() const {
			m_Mutex.lock();
			bool empty = m_Queue.empty();
			m_Mutex.unlock();
			return empty;
		}
		size_t Size() const {
			m_Mutex.lock();
			size_t size = m_Queue.size();
			m_Mutex.unlock();
			return size;
		}

		T& Front() {
			m_Mutex.lock();
			T& value = m_Queue.front();
			m_Mutex.unlock();
			return value;
		}
		const T& Front() const {
			m_Mutex.lock();
			const T& value = m_Queue.front();
			m_Mutex.unlock();
			return value;
		}

		void Push(T&& value) {
			m_Mutex.lock();

			m_Queue.push(value);
			m_Mutex.unlock();
		}
		void Push(const T& value) {
			m_Mutex.lock();

			m_Queue.push(value);
			m_Mutex.unlock();
		}
		void Pop() {
			m_Mutex.lock();
			m_Queue.pop();
			m_Mutex.unlock();
		}

		bool Next(T& value) {
			m_Mutex.lock();
			if (m_Queue.empty()) {
				m_Mutex.unlock();
				return false;
			}
			
			value = std::move(m_Queue.front());
			m_Queue.pop();
			m_Mutex.unlock();
			return true;
		}

	private:
		std::queue<T> m_Queue;
		mutable std::mutex m_Mutex;
	};

};