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
	std::atomic<hazard_ptr>	m_head;	        //points to first entry
	std::atomic<hazard_ptr> m_tail;	        //points to last entry
	std::atomic<int32_t>	m_size = 0;		//number of entries in the queue

public:
	new_JobQueue() {
		hazard_ptr start = hazard_ptr(std::forward<new_Queuable*>(new new_Queuable()),0);
		m_head.store(start);
		m_tail.store(start);
	}
	~new_JobQueue() {}



	void push(new_Queuable* queuable) {
		queuable->m_next.store({});
		hazard_ptr job(std::forward<new_Queuable*>(queuable),0);
		hazard_ptr tail;
		while (true) {
			tail = std::atomic_load(&m_tail);
			hazard_ptr next = { std::atomic_load(&tail.ptr->m_next) };
			if (tail == std::atomic_load(&m_tail)) {
				if (next.ptr == nullptr) {
					bool CAS_successful = std::atomic_compare_exchange_weak(&tail.ptr->m_next, &next, job.updateCount(next.count + 1));
					//std::cout << "Count: " << tail.ptr->m_next.load().count << std::endl;
					if (CAS_successful) {
						break;
					}
				}
				else {
					std::atomic_compare_exchange_weak(&m_tail, &tail, next.updateCount(tail.count + 1));
				}
			}
		}
		std::atomic_compare_exchange_weak(&m_tail, &tail, job.updateCount(tail.count + 1));
		m_size++;
	}
	
	
	bool pop(new_Queuable* dest) {
		hazard_ptr head;
		while (true) {
			head = std::atomic_load(&m_head);
			hazard_ptr tail = std::atomic_load(&m_tail);
			hazard_ptr next = { std::atomic_load(&head.ptr->m_next) };

			if (head == std::atomic_load(&m_head)) {
				if (head.ptr == tail.ptr) {
					if (next.ptr == nullptr) {
						return false;
					}
					std::atomic_compare_exchange_weak(&m_tail, &tail, next.updateCount(tail.count + 1));
				}
				else {
					dest = next.ptr;
					bool CAS_successful = std::atomic_compare_exchange_weak(&m_head, &head, next.updateCount(head.count + 1));
					if (CAS_successful)
						break;
				}
			}
		}
		// free dummy head node
		return true;
	}
	
	/*
	uint32_t clear() {
		uint32_t res = m_size;
		new_Queuable* job = pop();						//deallocate jobs that run a function
		while (job != nullptr) {						//because they were allocated by the JobSystem
			//auto da = job->get_deallocator();			//get deallocator
			//da.deallocate(job);						//deallocate the memory
			job = pop();								//get next entry
		}
		return res;
	}
	*/

	uint32_t size() {
		return m_size;
	}
};