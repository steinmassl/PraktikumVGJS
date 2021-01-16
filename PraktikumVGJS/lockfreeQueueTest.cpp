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
    * \brief Lockless FIFO queue class.
    *
    * The queue allows for multiple producers multiple consumers.
    * When number of threads is not known beforehand (most of the time), algorithm extensions are necessary.
    * Lockless but suffering from use-after-delete problem.  
    */
    template<typename JOB = Job>
    class JobQueue {

        template<typename JOB>
        struct node_t {
            JOB* job = nullptr;
            std::atomic<node_t<JOB>*> next = nullptr;

            node_t() {}
        };

        std::atomic<node_t<JOB>*>	    m_head;	        //points to first entry
        std::atomic<node_t<JOB>*>   	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size = 0;		//number of entries in the queue

        // Use for testing
        static constexpr int K = 2;     // Hazard Pointer per thread
        static constexpr int N = 16;    // Number of threads being used in VGJS
        static constexpr int H = K * N; // Total number of hazard pointer
        static constexpr int R = 2 * N; // Threshold for starting scan (R = H + Omega(H))

        // Not necessary when using global HP array
        struct hp_rec_type {        // Hazard Pointer record
            node_t<JOB>* HP[K];
            hp_rec_type* next;      // Get record of next thread
        };

        hp_rec_type* head_hp_rec;   // Header of the hp_rec list

        // Per thread private variables
        static inline thread_local node_t<JOB>* HP[K];          // Use thread local HP arrays or global array? is global array safe?
        static inline thread_local node_t<JOB>* rlist[R];       // list of retired nodes
        static inline thread_local unsigned int rcount{ 0 };    // number of nodes in rlist

        // Delete nodes no longer in use by any thread
        void scan(hp_rec_type* head) {
            unsigned int i;
            unsigned int pcount = 0;
            unsigned int new_rcount = 0;
            node_t<JOB>* plist[N];
            node_t<JOB>* next_rlist[N];

            // Stage 1: Scan HP list and insert non-null values in plist
            hp_rec_type* hp_rec = head;
            while (hp_rec != nullptr) {
                for (i = 0; i < K - 1; ++i) {
                    if (hp_rec->HP[i] != nullptr)
                        plist[pcount++] = hp_rec->HP[i];
                }
                hp_rec = hp_rec->next;
            }
            // sort plist to prepare for binary search
            std::sort(plist, plist+pcount);

            // Stage 2: compare plist against rlist
            for (i = 0; i < R; ++i) {
                if (std::binary_search(plist, plist+pcount, rlist[i]))
                    next_rlist[new_rcount++] = rlist[i];
                else
                    delete rlist[i];
            }
            for (i = 0; i < new_rcount; ++i) 
                rlist[i] = next_rlist[i];
            rcount = new_rcount;            
        }

        // Use instead of delete to avoid use-after-free
        void retireNode(node_t<JOB>* node) {
            rlist[rcount++] = node;
            if (rcount >= R)
                scan(head_hp_rec);
        }
    public:
        JobQueue() {
            // Head and Tail point to dummy node
            node_t<JOB>* start = new node_t<JOB>();
            m_head.store(start, std::memory_order_release);
            m_tail.store(start, std::memory_order_release);     
        }
        JobQueue(const JobQueue<JOB>& queue) noexcept : JobQueue() {}
        ~JobQueue() {
            delete m_head.load();   // Delete last dummy node
        }

        /**
        * \brief Pushes a job onto the queue tail.
        * \param[in] job_to_push The job to be pushed into the queue.
        */
        void push(JOB* job_to_push) {
            node_t<JOB>* new_node = new node_t<JOB>();
            new_node->job = job_to_push;
            node_t<JOB>* tail = nullptr;
            while (true) {
                // Start setting Hazard Pointer
                tail = m_tail.load(std::memory_order_relaxed);
                HP[0] = tail;
                if (tail != m_tail.load(std::memory_order_acquire)) continue;
                // End setting Hazard Pointer
                node_t<JOB>* next = tail->next.load(std::memory_order_acquire);
                if (tail != m_tail.load(std::memory_order_acquire)) continue;
                if (next != nullptr) {
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release);
                    continue;
                }
                if (tail->next.compare_exchange_weak(next, new_node, std::memory_order_release)) break;    // Use nullptr instead of next?
            }
            m_tail.compare_exchange_weak(tail, new_node, std::memory_order_acq_rel);
            HP[0] = nullptr;
            m_size++;
        }

        /**
        * \brief Pops a job from the head of the queue.
        * \param[in] popped_job The object that will be assigned with the popped job
        * \returns true when pop was successful
        */
        bool pop(JOB*& popped_job) {
            node_t<JOB>* head;
            while (true) {
                // Start setting Hazard Pointer
                head = m_head.load(std::memory_order_acquire);
                HP[0] = head;
                if (head != m_head.load(std::memory_order_acquire)) continue;
                // End setting Hazard Pointer
                node_t<JOB>* tail = m_tail.load(std::memory_order_relaxed);
                node_t<JOB>* next = head->next.load(std::memory_order_acquire);   // use-after-free problem when accessing head->next before HP was implemented
                HP[1] = next;
                if (head != m_head.load(std::memory_order_relaxed)) continue;
                if (next == nullptr) {
                    HP[0] = nullptr;
                    return false;
                }
                if (head == tail) {
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release); 
                    continue;
                }
				popped_job = next->job;
				if(m_head.compare_exchange_weak(head, next, std::memory_order_release)) break;
            }
            HP[0] = nullptr;
            HP[1] = nullptr;
            retireNode(head);
            m_size--;
            return true;
        }
    };

    JobQueue<Job> queue;
    void test_push() {
        for (int j = 0; j < 1; j++) {
            Job* job = new Job(n_pmr::new_delete_resource());
            queue.push(job);
        }
    }
    void test_pop() {
        for (int j = 0; j < 2; j++) {
            Job* job = nullptr;
            bool pop = queue.pop(job);
            std::cout << "Pop: " << pop << std::endl;
        }
    }

	Coro<> test() {
		for (int i = 0; i < 16; i++) {
            co_await[]() { test_push(); };
		}

		for (int i = 0; i < 16; i++) {
            co_await[]() { test_pop(); };
		}
        co_return;
	}
}
