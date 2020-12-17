#pragma once

#include <iostream>

struct hazard_ptr;
class new_Queuable;

struct hazard_ptr {
	new_Queuable* ptr = nullptr;
	uint32_t count = 0;

	hazard_ptr() {}
	hazard_ptr(new_Queuable*&& job, uint32_t count) : ptr(std::forward<new_Queuable*>(job)), count(count) {}

	bool operator== (const hazard_ptr& other) const {
		return (ptr == other.ptr && count == other.count);
	}
	hazard_ptr& updateCount(const uint32_t& val) {
		count = val;
		return *this;
	}
};

class new_Queuable {
public:
	std::atomic<hazard_ptr> m_next;           //next job in the queue
};




class new_JobQueue {
	std::atomic<hazard_ptr> m_head;	        //points to first entry
	std::atomic<hazard_ptr> m_tail;	        //points to last entry
	int32_t m_size = 0;				//number of entries in the queue

public:
	new_JobQueue() {
		hazard_ptr start = hazard_ptr(std::forward<new_Queuable*>(new new_Queuable()),0);
		m_head.store(start);
		m_tail.store(start);
	}
	~new_JobQueue() {}
	uint32_t clear() { return 0; }
	uint32_t size() { return 0; }

	void push(new_Queuable* queuable) {
		queuable->m_next.store({});
		hazard_ptr job(std::forward<new_Queuable*>(queuable),0);
		hazard_ptr tail;
		while (true) {
			tail = std::atomic_load(&m_tail);
			hazard_ptr next = { std::atomic_load(&tail.ptr->m_next) };
			if (tail == std::atomic_load(&m_tail)) {
				if (next.ptr == nullptr) {
					bool exchanged = std::atomic_compare_exchange_weak(&tail.ptr->m_next, &next, job.updateCount(next.count + 1));
					std::cout << "Count: " << tail.ptr->m_next.load().count << std::endl;
					if (exchanged) {
						break;
					}
				}
				else {
					std::atomic_compare_exchange_weak(&m_tail, &tail, next.updateCount(tail.count + 1));
				}
			}
		}
		std::atomic_compare_exchange_weak(&m_tail, &tail, job.updateCount(tail.count + 1));
	}
};