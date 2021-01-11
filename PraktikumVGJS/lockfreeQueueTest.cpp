#include "lockfreeQueueTest.h"

namespace lock_free {

	template<typename JOB>
    struct pointer_t {
        node_t<JOB>* ptr = nullptr;
        uint32_t count = 0;

        pointer_t() {}
        pointer_t(node_t<JOB>* node_ptr, uint32_t count = 0) : ptr(node_ptr), count(count) {}

        bool operator== (const pointer_t<JOB>& other) const {
            return (ptr == other.ptr && count == other.count);
        }
    };

    template<typename JOB>
    struct node_t {
        JOB* job = nullptr;
        std::atomic<pointer_t<JOB>> next = {};

        node_t() {}
    };


    /**
    * \brief Lockless FIFO queue class.
    *
    * The queue allows for multiple producers multiple consumers. 
    * Lockless but suffering from use-after-delete problem.  
    */
    template<typename JOB = Job>
    class JobQueue {
        std::atomic<pointer_t<JOB>>	    m_head;	        //points to first entry
        std::atomic<pointer_t<JOB>> 	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size = 0;		//number of entries in the queue

    public:

        JobQueue() {
            node_t<JOB>* start = new node_t<JOB>();
            m_head.store(start, std::memory_order_release);
            m_tail.store(start, std::memory_order_release);
        }

        JobQueue(const JobQueue<JOB>& queue) noexcept : JobQueue() {}

        ~JobQueue() {
            delete m_head.load().ptr;
        }



        /**
        * \brief Pushes a job onto the queue tail.
        * \param[in] job The job to be pushed into the queue.
        */
        void push(JOB* job_to_push) {
            node_t<JOB>* new_node = new node_t<JOB>();
            new_node->job = job_to_push;
            pointer_t<JOB> tail;
            while (true) {
                tail = m_tail.load(std::memory_order_acquire);
                pointer_t<JOB> next = tail.ptr->next.load(std::memory_order_acquire);
                if (tail == m_tail.load(std::memory_order_acquire)) {
                    if (next.ptr == nullptr) {
                        bool successful_cas = tail.ptr->next.compare_exchange_weak(next, pointer_t(new_node, next.count + 1));
                        if (successful_cas) {
                            break;
                        }
                    }
                    else {
                        m_tail.compare_exchange_weak(tail, pointer_t(next.ptr, tail.count + 1));
                    }
                }
            }
            m_tail.compare_exchange_weak(tail, pointer_t(new_node, tail.count + 1));
            m_size++;
        }

        /**
        * \brief Pops a job from the head of the queue.
        * \returns a job or nullptr.
        */
        bool pop(JOB*& popped_job) {
            pointer_t<JOB> head;
            while (true) {
                head = m_head.load(std::memory_order_acquire);
                pointer_t<JOB> tail = m_tail.load(std::memory_order_acquire);
                pointer_t<JOB> next = head.ptr->next.load(std::memory_order_acquire);

                if (head == m_head.load()) {
                    if (head.ptr == tail.ptr) {
                        if (next.ptr == nullptr) {
                            return false;
                        }
                        m_tail.compare_exchange_weak(tail, pointer_t(next.ptr, tail.count + 1));
                    }
                    else {
                        popped_job = next.ptr->job;
                        bool successful_cas = m_head.compare_exchange_weak(head, pointer_t(next.ptr, head.count + 1));
                        if (successful_cas)
                            break;
                    }
                }
            }
            // free dummy head node
            delete head.ptr;
            //std::cout << m_size << std::endl;
            m_size--;
            //std::cout << m_size << std::endl;
            return true;
        }
    };
    
    JobQueue<Job> queue;
	void test() {
		for (int i = 0; i < 16; i++) {
			schedule([]() {
				for (int j = 0; j < 1; j++) {
                    Job* job = new Job(n_pmr::new_delete_resource());
					queue.push(job);
				}
			});
		}
		//continuation([]() {
			for (int i = 0; i < 16; i++) {
				schedule([]() {
					for (int j = 0; j < 2; j++) {
                        Job* job = nullptr;
						bool pop = queue.pop(job);
						std::cout << "Pop: " << pop << std::endl;
					}
				});
			}
			continuation([]() {vgjs::terminate(); });
		//});
	}
}
