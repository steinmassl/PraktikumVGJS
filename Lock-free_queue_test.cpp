#include "Lock-free_queue_test.h"

namespace lock_free_queue {

	struct node {

	};

	struct hazard_ptr {
		new_Queuable* ptr = nullptr;
		uint32_t count = 0;

		hazard_ptr() {}
		hazard_ptr(new_Queuable*&& job, uint32_t count) : ptr(std::forward<new_Queuable*>(job)), count(count) {}


		hazard_ptr& updateCount(const uint32_t& val) {
			count = val;
			return *this;
		}
	};

	class new_Queuable {
	public:
		std::atomic<hazard_ptr> m_next;           //next job in the queue
	};

	bool operator== (const hazard_ptr& lhs, const hazard_ptr& rhs) {
		return (lhs.ptr == rhs.ptr && lhs.count == rhs.count);
	}

	class new_JobQueue {
		std::atomic<hazard_ptr>	m_head;	        //points to first entry
		std::atomic<hazard_ptr> m_tail;	        //points to last entry
		std::atomic<int32_t>	m_size = 0;		//number of entries in the queue

	public:
		new_JobQueue() {
			hazard_ptr start = hazard_ptr(std::forward<new_Queuable*>(new new_Queuable()), 0);
			m_head.store(start);
			m_tail.store(start);
		}
		~new_JobQueue() {}



		void push(new_Queuable* queuable) {
			queuable->m_next.store({});
			hazard_ptr job(std::forward<new_Queuable*>(queuable), 0);
			hazard_ptr tail;
			while (true) {
				//std::cout << "Looping..." << std::endl;
				tail = std::atomic_load_explicit(&m_tail, std::memory_order_acquire);
				hazard_ptr next = std::atomic_load_explicit(&(tail.ptr->m_next), std::memory_order_acquire);
				if (tail == std::atomic_load_explicit(&m_tail, std::memory_order_acquire)) {
					if (next.ptr == nullptr) {
						bool CAS_successful = std::atomic_compare_exchange_weak(&(tail.ptr->m_next), &next, job.updateCount(next.count + 1));
						std::cout << "First Count: " << tail.ptr->m_next.load().count << std::endl;
						if (CAS_successful) {
							break;
						}
					}
					else {
						std::atomic_compare_exchange_weak(&m_tail, &tail, next.updateCount(tail.count + 1));
						std::cout << "Second Count: " << m_tail.load().count << std::endl;
					}
				}
			}
			std::atomic_compare_exchange_weak(&m_tail, &tail, job.updateCount(tail.count + 1));
			std::cout << "Third Count: " << m_tail.load().count << std::endl;
			m_size++;
			std::cout << "Size: " << m_size << std::endl;
		}


		bool pop(new_Queuable* dest) {
			hazard_ptr head;
			while (true) {
				head = std::atomic_load(&m_head);
				hazard_ptr tail = std::atomic_load(&m_tail);
				hazard_ptr next = std::atomic_load(&head.ptr->m_next);

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
			delete head.ptr;
			return true;
		}

		/*
		uint32_t clear() {
			uint32_t res = m_size;
			new_Queuable* job = nullptr;
			pop(job);						//deallocate jobs that run a function
			while (job != nullptr) {						//because they were allocated by the JobSystem
				//auto da = job->get_deallocator();			//get deallocator
				//da.deallocate(job);						//deallocate the memory
				pop(job);								//get next entry
			}
			return res;
		}
		*/

		uint32_t size() {
			return m_size;
		}
	};

	new_JobQueue queue;
	
	void test() {
		for (int i = 0; i < 100; i++) {
			schedule([]() {
				for (int j = 0; j < 100; j++) {
					new_Queuable queuable;
					queue.push(&queuable);
					//std::cout << "Push Loop: " << j << std::endl;
				}
			});
		}
		/*
		continuation([]() {
			std::cout << "End of push operations" << std::endl;

			for (int i = 0; i < 10; i++) {
				schedule([]() {
					for (int j = 0; j < 10; j++) {
						new_Queuable queuable;
						queue.pop(&queuable);
						std::cout << "Pop Loop: " << j << std::endl;
					}
				});
			}
			continuation([]() {vgjs::terminate(); });
		});
		*/
	continuation([]() { vgjs::terminate(); });
	}
}
