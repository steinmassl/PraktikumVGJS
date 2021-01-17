#include "tests.h"

namespace lock_free {
    /*
    // Tagged pointer
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
    */

    /**
    * \brief Lock-free FIFO queue class.
    *
    * The queue allows for multiple producers multiple consumers.
    * When number of threads is not known beforehand, algorithm extensions are necessary.
    * Use maximum number of threads (P) until then.
    */
    template<typename JOB = Job>
    class JobQueue {

        template<typename JOB>
        struct node_t {
            JOB* job{ nullptr };
            std::atomic<node_t<JOB>*> next{ nullptr };

            node_t() {}
        };

        static constexpr unsigned int K{ 2 };       // Hazard Pointer per thread
        static constexpr unsigned int P{ 32 };      // Maximum number of threads
        static constexpr unsigned int N{ K * P };    // Total number of hazard pointer
        static constexpr unsigned int R{ 2 * N };    // Threshold for starting scan (R = H + Omega(H))

        std::atomic<node_t<JOB>*>	    m_head;	        //points to first entry
        std::atomic<node_t<JOB>*>   	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size{ 0 };	//number of entries in the queue
        node_t<JOB>*                    m_HP[N]{};      // Use thread local HP arrays or global array? is global array safe?

        // Per thread private variables
        static inline thread_local node_t<JOB>* m_rlist[R]{};   // list of retired nodes
        static inline thread_local unsigned int m_rcount{ 0 };  // number of nodes in rlist


        /*
        // Not necessary when using global HP array
        struct hp_rec_type {        // Hazard Pointer record
            node_t<JOB>* HP[K];
            hp_rec_type* next;      // Get record of next thread
        };

        hp_rec_type* head_hp_rec;   // Header of the hp_rec list
        */


        // Free nodes no longer in use by any thread
        void scan() {
            unsigned int i, pcount{ 0 }, next_rcount{ 0 };
            node_t<JOB>* hptr{ nullptr }; 
            node_t<JOB>* plist[N]{};
            node_t<JOB>* next_rlist[N]{};

            // Stage 1: Scan HP list and insert non-null values in plist
            for (i = 0; i < N; ++i) {
                hptr = m_HP[i];
                if (hptr != nullptr)
                    plist[pcount++] = hptr;
            }
            // Stage 2: Sort plist to prepare for binary search
            std::sort(plist, plist+pcount);
            // maybe remove duplicates
            // Stage 3: compare plist against rlist and free nodes no longer in use
            for (i = 0; i < R; ++i) {
                if (std::binary_search(plist, plist + pcount, m_rlist[i]))
                    next_rlist[next_rcount++] = m_rlist[i];
                else
                    delete m_rlist[i];    // Alternatively recycle into free_list
            }
            // Stage 4: Form new array of retired nodes
            for (i = 0; i < next_rcount; ++i)
                m_rlist[i] = next_rlist[i];
            m_rcount = next_rcount;
        }

        // Use instead of delete to avoid use-after-free
        void retireNode(node_t<JOB>* node) {
            m_rlist[m_rcount++] = node;
            if (m_rcount >= R)
                scan();
        }
    public:
        JobQueue() {
            // Head and Tail point to dummy node
            node_t<JOB>* start{ new node_t<JOB>() };
            m_head.store(start, std::memory_order_release);
            m_tail.store(start, std::memory_order_release);     
        }
        JobQueue(const JobQueue<JOB>& queue) noexcept : JobQueue() {}
        ~JobQueue() {
            delete m_head.load();   // Delete last dummy node
        }

		/**
        * \brief Deallocate all Jobs in the queue.
        * Since Jobs and coros use different allocation strategies, for each entry a deallocator
        * is used for deallocation.
        */
        uint32_t clear() {
            uint32_t res = m_size;
            JOB* job = pop();                   //deallocate jobs that run a function
            while (job != nullptr) {            //because they were allocated by the JobSystem
                auto da = job->get_deallocator(); //get deallocator
                da.deallocate(job);             //deallocate the memory
                job = pop();                    //get next entry
            }
            return res;
        }

        /**
        * \brief Get the number of jobs currently in the queue.
        * \returns the number of jobs (Coros and Jobs) currently in the queue.
        */
        uint32_t size() {
            return m_size.load(std::memory_order_acquire);
        }

        /**
        * \brief Pushes a job onto the queue tail.
        * \param[in] job_to_push The job to be pushed into the queue.
        */
        void push(JOB* job_to_push) {
            node_t<JOB>* new_node{ new node_t<JOB>() };  // Alternatively use existing node from free_list
            new_node->job = job_to_push;
            node_t<JOB>* tail;
            while (true) {
                // Start setting Hazard Pointer
                tail = m_tail.load(std::memory_order_relaxed);
                m_HP[0] = tail;
                if (tail != m_tail.load(std::memory_order_acquire)) continue;
                // End setting Hazard Pointer
                node_t<JOB>* next{ tail->next.load(std::memory_order_acquire) };
                if (tail != m_tail.load(std::memory_order_acquire)) continue;
                if (next != nullptr) {
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release);
                    continue;
                }
                if (tail->next.compare_exchange_weak(next, new_node, std::memory_order_release)) break;    // Use nullptr instead of next?
            }
            m_tail.compare_exchange_weak(tail, new_node, std::memory_order_acq_rel);
            m_HP[0] = nullptr;
            m_size++;
        }

        /**
        * \brief Pops a job from the head of the queue.
        * \param[in] popped_job The object that will be assigned with the popped job
        * \returns true when pop was successful
        */
        JOB* pop() {
            JOB* res = nullptr;
            node_t<JOB>* head;
            while (true) {
                // Start setting Hazard Pointer
                head = m_head.load(std::memory_order_acquire);
                m_HP[0] = head;
                if (head != m_head.load(std::memory_order_acquire)) continue;
                // End setting Hazard Pointer
                node_t<JOB>* tail{ m_tail.load(std::memory_order_relaxed) };
                node_t<JOB>* next{ head->next.load(std::memory_order_acquire) };   // use-after-free problem when accessing head->next before HP was implemented
                m_HP[1] = next;
                if (head != m_head.load(std::memory_order_relaxed)) continue;
                if (next == nullptr) {
                    m_HP[0] = nullptr;
                    return nullptr;
                }
                if (head == tail) {
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release); 
                    continue;
                }
				res = next->job;
				if(m_head.compare_exchange_weak(head, next, std::memory_order_release)) break;
            }
            m_HP[0] = nullptr;
            m_HP[1] = nullptr;
            retireNode(head);
            m_size--;
            return res;
        }
    };

    JobQueue<Job> queue;
    void test_push() {
        for (int j = 0; j < 1000; j++) {
            Job* job = new Job(n_pmr::new_delete_resource());
            queue.push(job);
        }
    }
    void test_pop() {
        for (int j = 0; j < 2000; j++) {
            Job* job = nullptr;
            job = queue.pop();
            std::cout << "Pop: " << (bool) job << std::endl;
        }
    }

	Coro<> test() {
		for (int i = 0; i < 16; i++) {
            schedule([]() { test_push(); });
		}
		for (int i = 0; i < 16; i++) {
            schedule([]() { test_pop(); });
		}
        co_return;
	}
}
