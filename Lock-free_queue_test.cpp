#include "Lock-free_queue_test.h"

namespace lock_free_queue {

	class Queuable {
	public:
		//std::atomic<pointer_t> m_next;           //next job in the queue
	};

	struct pointer_t {
		node_t* ptr = nullptr;
		uint32_t count = 0;

		pointer_t() {}
		pointer_t(node_t* node_ptr, uint32_t count = 0) : ptr(node_ptr), count(count) {}

		bool operator== (const pointer_t& other) const {
			return (ptr == other.ptr && count == other.count);
		}
	};

	struct node_t {
		Queuable* job = nullptr;
		std::atomic<pointer_t> next = {};

		node_t() {}
		node_t(Queuable*&& job) : job(std::forward<Queuable*>(job)) {}
	};
	
	class JobQueue {
		std::atomic<pointer_t>	m_head;	        //points to first entry
		std::atomic<pointer_t> 	m_tail;	        //points to last entry
		std::atomic<int32_t>	m_size = 0;		//number of entries in the queue

	public:
		JobQueue() {
			pointer_t start = pointer_t(new node_t());
			m_head.store(start);
			m_tail.store(start);
		}
		~JobQueue() {
			delete m_head.load().ptr;
		}

		void push(Queuable* queuable) {
			node_t* new_node = new node_t(std::forward<Queuable*>(queuable));
			pointer_t tail;
			while (true) {
				tail = std::atomic_load_explicit(&m_tail, std::memory_order_acquire);
				pointer_t next = std::atomic_load_explicit(&tail.ptr->next, std::memory_order_acquire);
				if (tail == std::atomic_load_explicit(&m_tail, std::memory_order_acquire)) {
					if (next.ptr == nullptr) {
						bool successful_cas = std::atomic_compare_exchange_weak(&tail.ptr->next, &next, pointer_t(new_node, next.count + 1) );
						std::cout << "First Count: " << tail.ptr->next.load().count << std::endl;
						if (successful_cas) {
							break;
						}
					}
					else {
						std::atomic_compare_exchange_weak(&m_tail, &tail, pointer_t(next.ptr, tail.count + 1));
						std::cout << "Second Count: " << m_tail.load().count << std::endl;
					}
				}
			}
			std::atomic_compare_exchange_weak(&m_tail, &tail, pointer_t(new_node, tail.count + 1));
			std::cout << "Third Count: " << m_tail.load().count << std::endl;
			m_size++;
			std::cout << "Size: " << m_size << std::endl;
		}

		bool pop(Queuable* out_queuable) {
			pointer_t head;
			while (true) {
				head = std::atomic_load_explicit(&m_head, std::memory_order_acquire);
				pointer_t tail = std::atomic_load_explicit(&m_tail, std::memory_order_acquire);
				pointer_t next = std::atomic_load_explicit(&head.ptr->next, std::memory_order_acquire);

				if (head == std::atomic_load_explicit(&m_head, std::memory_order_acquire)) {
					if (head.ptr == tail.ptr) {
						if (next.ptr == nullptr) {
							return false;
						}
						std::atomic_compare_exchange_weak(&m_tail, &tail, pointer_t(next.ptr, tail.count + 1));
						std::cout << "First Count: " << m_tail.load().count << std::endl;
					}
					else {
						out_queuable = next.ptr->job;
						bool successful_cas = std::atomic_compare_exchange_weak(&m_head, &head, pointer_t(next.ptr, head.count + 1));
						std::cout << "Second Count: " << m_head.load().count << std::endl;
						if (successful_cas)
							break;
					}
				}
			}
			// free dummy head node
			delete head.ptr;
			m_size--;
			std::cout << "Size: " << m_size << std::endl;
			return true;
		}


		uint32_t clear() {
			uint32_t res = m_size;
			Queuable* job = nullptr;
			bool successful_pop = pop(job);						//deallocate jobs that run a function
			while (successful_pop) {						//because they were allocated by the JobSystem
				//auto da = job->get_deallocator();			//get deallocator
				//da.deallocate(job);						//deallocate the memory
				successful_pop = pop(job);								//get next entry
			}
			return res;
		}


		uint32_t size() {
			return m_size;
		}
	};

	JobQueue queue;
	
	void test() {
		for (int i = 0; i < 16; i++) {
			schedule([]() {
				for (int j = 0; j < 100; j++) {
					Queuable queuable;
					queue.push(&queuable);
				}
			});
		}
		//continuation([]() {
			for (int i = 0; i < 16; i++) {
				schedule([]() {
					for (int j = 0; j < 200; j++) {
						Queuable queuable;
						bool pop = queue.pop(&queuable);
						std::cout << "Pop: " << pop << std::endl;
					}
				});
			}
			continuation([]() {vgjs::terminate(); });
		//});
	}
}
