#include "tests.h"

namespace lock_free {

    // Add this to thread_task function for initialization
    void thread_task() {
        /*
        JobQueue_base::m_thread_index = threadIndex;        // Set threadindex for HP

        // Correctly initialize HP in all JobQueues
        for (auto& queue : m_global_queues) queue.initHP();
        for (auto& queue : m_local_queues) queue.initHP();
        for (auto& queue : m_tag_queues) queue.second->initHP();
        m_recycle.initHP();
        m_delete.initHP();
        */
    }

    class JobQueue_base {
        friend JobSystem;
    protected:
        static constexpr uint32_t K{ 2 };       // Hazard Pointer per thread
        static constexpr uint32_t P{ 16 };      // Maximum number of threads - fixed value until better solution
        static constexpr uint32_t N{ K * P };   // Total number of hazard pointer
        static constexpr uint32_t R{ 2 * N };   // Threshold for starting scan (R = N + Omega(H))
        static constexpr uint32_t FREELIST_SIZE{ 1 << 10 };

        static inline thread_local thread_index_t m_thread_index{ 0 };  // Initialize with 0 until threads are ready and call initHP()
    };

    /**
    * \brief Lock-free FIFO queue class.
    *
    * The queue allows for multiple producers multiple consumers.
    * When number of threads is not known beforehand, algorithm extensions are necessary.
    * Use maximum number of threads (P) until then. Performance w/ vs w/o extensions?
    */
    template<typename JOB = Job_base>
    class JobQueue : JobQueue_base {
        friend JobSystem;

        // Node holding pointer to job and next
        template<typename JOBT>
        struct node_t {
            JOBT* job{ nullptr };
            std::atomic<node_t<JOBT>*> next{ nullptr };
        };

        std::atomic<node_t<JOB>*>	    m_head;	        //points to first entry
        std::atomic<node_t<JOB>*>   	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size{ 0 };	//number of entries in the queue
        static inline node_t<JOB>*      m_HP[N]{};      // All Hazard pointers (read all, write only to 2 thread HP)

        // Per thread private variables
        static inline thread_local node_t<JOB>**    m_hp0{ &m_HP[K * m_thread_index.value] };     // Thread hazard pointer
        static inline thread_local node_t<JOB>**    m_hp1{ &m_HP[K * m_thread_index.value + 1] }; // FIFO Queue requires 2 HP 
        static inline thread_local node_t<JOB>*     m_rlist[R]{};       // List of retired nodes
        static inline thread_local uint32_t         m_rcount{ 0 };      // Number of retired nodes
        static inline thread_local node_t<JOB>*     m_freelist[FREELIST_SIZE]{};        // List of nodes that can be re-used
        static inline thread_local uint32_t         m_freecount{ 0 };                   // Number of reusable nodes

        // Free nodes no longer in use by any thread
        void scan() {
            uint32_t i, pcount{ 0 }, next_rcount{ 0 };
            node_t<JOB>* hptr{ nullptr };
            node_t<JOB>* plist[N]{};        // Current non-null hazard pointers
            node_t<JOB>* next_rlist[N]{};   // Temporary list for new rlist

            // Stage 1: Scan HP list and insert non-null values in plist
            for (i = 0; i < N; ++i) {
                hptr = m_HP[i];
                if (hptr != nullptr)
                    plist[pcount++] = hptr;
            }
            // Stage 2: Sort plist to prepare for binary search
            std::sort(plist, plist + pcount);

            // maybe remove duplicates/tallying entries here

            // Stage 3: compare plist against rlist and free nodes no longer in use
            for (i = 0; i < R; ++i) {
                if (std::binary_search(plist, plist + pcount, m_rlist[i]))
                    next_rlist[next_rcount++] = m_rlist[i];
                else {
                    //delete m_rlist[i];        // Simply delete when no longer in use
                    
                    if (m_freecount < FREELIST_SIZE) {      // Recycle into freelist when no longer in use
                        m_freelist[m_freecount++] = m_rlist[i];
                        //std::cout << "Recycling\n";
                    }
                    else {
                        delete m_rlist[i];      // Delete if freelist is full
                        //std::cout << "freelist is full\n";
                    }
                }
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
            delete m_head.load(std::memory_order_relaxed);   // Delete last dummy node
        }

        // Set correct location of thread-owned hazard pointers (currently in thread_task())
        void initHP() {
            m_hp0 = &m_HP[K * m_thread_index.value];
            m_hp1 = &m_HP[K * m_thread_index.value + 1];
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
            //node_t<JOB>* new_node{ new node_t<JOB>() };     // Allocate new node. Alternatively use existing node from freelist
            
            node_t<JOB>* new_node{ nullptr };
            if (m_freecount > 0) {                  // Re-use node from freelist
                new_node = m_freelist[--m_freecount];
                new_node->next = nullptr;           // Clear pointer to successor    
                m_freelist[m_freecount] = nullptr;
                //std::cout << "used recycled node\n";
            }
            else
                new_node = new node_t<JOB>();       // Allocate new node when freelist is empty
            
            new_node->job = job_to_push;                    // Copy job into node - next pointer of node already nullptr
            node_t<JOB>* tail;
            while (true) {                                                          // Try until job pushed into queue
                tail = m_tail.load(std::memory_order_relaxed);                      // Read Queue tail
                *m_hp0 = tail;                                                      // Set hazard pointer
                if (tail != m_tail.load(std::memory_order_acquire)) continue;       // Check if hazard pointer is valid, otherwise try again
                node_t<JOB>* next{ tail->next.load(std::memory_order_acquire) };    // Read next node of tail
                if (tail != m_tail.load(std::memory_order_acquire)) continue;       // Is Queue tail still consistent? if not, try again
                if (next != nullptr) {                                              // Was tail pointing to the last node?
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release);    // Tail was not last node, swing tail to next node - test with compare_exchange_strong as well
                    continue;                                                               // Try to push again
                }
                if (tail->next.compare_exchange_weak(next, new_node, std::memory_order_release)) break;     // Try to link new node to end of list,
            }                                                                                               // if successful, push is done, exit loop
            m_tail.compare_exchange_weak(tail, new_node, std::memory_order_acq_rel);            // Swing tail to the inserted node
            *m_hp0 = nullptr;           // No longer hazardous
            m_size++;
        }

        /**
        * \brief Pops a job from the head of the queue.
        * \returns a job or nullptr.
        */
        JOB* pop() {
            JOB* res = nullptr;     // Pointer to return
            node_t<JOB>* head;
            while (true) {          // Try until job popped from queue
                head = m_head.load(std::memory_order_acquire);                      // Read Queue head
                *m_hp0 = head;                                                      // Set first hazard pointer
                if (head != m_head.load(std::memory_order_acquire)) continue;       // Check if hazard pointer is valid, otherwise try again
                node_t<JOB>* tail{ m_tail.load(std::memory_order_relaxed) };        // Read Queue tail
                node_t<JOB>* next{ head->next.load(std::memory_order_acquire) };    // Read next node of head
                *m_hp1 = next;                                                      // Set second hazard pointer
                if (head != m_head.load(std::memory_order_relaxed)) continue;       // Is Queue head still consistent? if not, try again
                if (next == nullptr) {                                              // Is Queue empty?
                    *m_hp0 = nullptr;                                               // No longer hazardous, second HP already nullptr
                    return nullptr;                                                 // Queue was empty, no pop
                }
                if (head == tail) {                                                         // Is tail falling behind?
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release);    // Tail falling behind, advance it
                    continue;                                                               // Try to pop again
                }
                res = next->job;                                                                    // Read value before CAS, otherwise another pop might free the next node
                if (m_head.compare_exchange_weak(head, next, std::memory_order_release)) break;     // Try to swing Queue head to the next node,
            }                                                                                       // if successful, pop is done, exit loop
            *m_hp0 = nullptr;       // No longer hazardous
            *m_hp1 = nullptr;
            retireNode(head);       // Safe to retire previous Queue head node
            m_size--;
            return res;             // Return job from previous Queue head
        }
    };

    lock_free::JobQueue<Job> queue;
    void test_push() {
        for (int j = 0; j < 100000; j++) {
            Job* job = new Job(n_pmr::new_delete_resource());
            queue.push(job);
        }
    }
    void test_pop() {
        for (int j = 0; j < 200000; j++) {
            Job* job = nullptr;
            job = queue.pop();
            std::cout << "Pop: " << (bool) job << std::endl;
        }
    }

	void test() {
		for (int i = 0; i < 16; i++) 
            schedule( []() { test_push(); });
		for (int i = 0; i < 16; i++) 
            schedule( []() { test_pop(); });
        for (int i = 0; i < 16; i++)
            schedule( []() { test_push(); });

        continuation([]() {vgjs::terminate(); });        
	}
}
