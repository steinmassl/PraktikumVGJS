#ifndef VEGAMEJOBSYSTEM_H
#define VEGAMEJOBSYSTEM_H


/**
*
* \file
* \brief The Vienna Game Job System (VGJS)
*
* Designed and implemented by Prof. Helmut Hlavacs, Faculty of Computer Science, University of Vienna
* See documentation on how to use it at https://github.com/hlavacs/GameJobSystem
* The library is a single include file, and can be used under MIT license.
*
*/


#include <iostream>
#include <fstream>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <functional>
#include <condition_variable>
#include <queue>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>
#include <assert.h>
#include <type_traits>
#include <chrono>
#include <string>
#include <sstream>

#if(defined(_MSC_VER))
    #include <memory_resource>
    namespace n_exp = std::experimental;
    namespace n_pmr = std::pmr;
#elif(defined(__clang__))
    #include <experimental/coroutine>
    #include <experimental/memory_resource>
    #include <experimental/vector>
    namespace n_exp = std::experimental;
    namespace n_pmr = std::experimental::pmr;
#elif(defined(__GNUC__))
    #include <coroutine>
    #include <memory_resource>
    namespace n_exp = std;
    namespace n_pmr = std::pmr;
#else
#endif


namespace vgjs {

    class Job;
    class Job_base;
    class JobSystem;

    template<typename T, typename P, int D = -1>
    struct int_type {
        using type_name = T;

        T value{};
        int_type() {
            static_assert(!(std::is_unsigned_v<T> && static_cast<int>(D) < 0));
            value = static_cast<T>(D);
        };
        explicit int_type(const T& t) : value(t) {};
        int_type(const int_type<T, P>& t) : value(t.value) {};
        int_type(const int_type<T, P>&& t) : value(t.value) {};
        void operator=(const int_type& rhs) { value = rhs.value; };
        void operator=(const int_type&& rhs) { value = rhs.value; };
        auto operator<=>(const int_type& v) const = default;
        auto operator<=>(const T& v) { return value <=> v; };

        struct hash {
            std::size_t operator()(const int_type<T,P,D>& tg) const { return std::hash<T>()(tg.value); };
        };

        struct equal_to {
            constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; };
        };
    };

    using thread_index = int_type<int, struct P0, -1>;
    using thread_id = int_type<int, struct P1, -1>;
    using thread_type = int_type<int, struct P2, -1>;
    using thread_count = int_type<int, struct P3, -1>;
    using tag = int_type<int, struct P4, -1>;

    bool is_logging();
    void log_data(  std::chrono::high_resolution_clock::time_point& t1
                    , std::chrono::high_resolution_clock::time_point& t2
                    , thread_index exec_thread, bool finished, thread_type type, thread_id id);
    void save_log_file();

    /**
    * \brief Function struct wraps a c++ function of type std::function<void(void)>.
    * 
    * It can hold a function, and additionally a thread index where the function should
    * be executed, a type and an id for dumping a trace file to be shown by
    * Google Chrome about::tracing.
    */
    struct Function {
        std::function<void(void)>   m_function = []() {};  //empty function
        thread_index                m_thread_index;        //thread that the f should run on
        thread_type                 m_type;                //type of the call
        thread_id                   m_id;                  //unique identifier of the call

        Function(std::function<void(void)>& f, thread_index index = thread_index{}, thread_type type = thread_type{}, thread_id id = thread_id{})
            : m_function(f), m_thread_index(index), m_type(type), m_id(id) {};

        Function(std::function<void(void)>&& f, thread_index index = thread_index{}, thread_type type = thread_type{}, thread_id id = thread_id{})
            : m_function(std::move(f)), m_thread_index(index), m_type(type), m_id(id) {};

        Function(const Function& f) 
            : m_function(f.m_function), m_thread_index(f.m_thread_index), m_type(f.m_type), m_id(f.m_id) {};

        Function(Function&& f)
            : m_function(std::move(f.m_function)), m_thread_index(f.m_thread_index), m_type(f.m_type), m_id(f.m_id) {};

        Function& operator= (const Function& f) {
            m_function = f.m_function; 
            m_thread_index = f.m_thread_index; 
            m_type = f.m_type;  
            m_id = f.m_id;
            return *this;
        };

        Function& operator= (Function&& f) {
            m_function = std::move(f.m_function); 
            m_thread_index = f.m_thread_index; 
            m_type = f.m_type;
            m_id = f.m_id;
            return *this;
        };
    };


    //-----------------------------------------------------------------------------------------

    /**
    * \brief Base class for deallocating jobs and coros
    */
    struct job_deallocator {
        virtual void deallocate(Job_base* job) noexcept;
    };

    /**
    * \brief Base class of coro task promises and jobs.
    */
    class Job_base {
    public:
        std::atomic<int>    m_children = 0;             //number of children this job is waiting for
        Job_base*           m_parent = nullptr;         //parent job that created this job
        thread_index        m_thread_index;             //thread that the job should run on and ran on
        thread_type         m_type;                     //for logging performance
        thread_id           m_id;                       //for logging performance
        bool                m_is_function = false;      //default - this is not a function

        Job_base() : m_children{ 0 }, m_parent{ nullptr }, m_thread_index{}, m_type{}, m_id{}, m_is_function{ false } {}

        virtual bool resume() = 0;                      //this is the actual work to be done
        void operator() () noexcept {           //wrapper as function operator
            resume();
        }
        bool is_function() noexcept { return m_is_function; }         //test whether this is a function or e.g. a coro
        virtual job_deallocator get_deallocator() noexcept { return job_deallocator{}; };    //called for deallocation
    };


    /**
    * \brief Job class calls normal C++ functions, is allocated and deallocated, and can be reused.
    */
    class Job : public Job_base {
    public:
        n_pmr::memory_resource*     m_mr = nullptr;  //memory resource that was used to allocate this Job
        Job_base*                   m_continuation = nullptr;   //continuation follows this job (a coro is its own continuation)
        std::function<void(void)>   m_function;      //function to compute

        Job( n_pmr::memory_resource* pmr) : Job_base(), m_mr(pmr), m_continuation(nullptr) {
            m_children = 1;
            m_is_function = true;
        }

        void reset() noexcept {         //call only if you want to wipe out the Job data
            m_children = 1;
            m_parent = nullptr;
            m_continuation = nullptr;
            m_thread_index = thread_index{};
            m_type = thread_type{};
            m_id = thread_id{};
        }

        bool resume() noexcept {    //work is to call the function
            m_children = 1;         //job is its own child, so set to 1
            m_function();           //run the function, can schedule more children here
            return true;
        }

        bool deallocate() noexcept { return true; };  //assert this is a job so it has been created by the job system
    };


    /**
    * \brief Deallocate a Job instance.
    * \param[in] job Pointer to the job.
    */
    inline void job_deallocator::deallocate(Job_base* job) noexcept {
        n_pmr::polymorphic_allocator<Job> allocator(((Job*)job)->m_mr); //construct a polymorphic allocator
        ((Job*)job)->~Job();                                          //call destructor
        allocator.deallocate(((Job*)job), 1);                         //use pma to deallocate the memory
    }


    /**
    * \brief Data structure storing times when jobs where called and ended.
    * Can be saved to a log file and loaded into Google Chrom about:://tracing.
    */
    struct JobLog {
        std::chrono::high_resolution_clock::time_point m_t1, m_t2;	///< execution start and end
        thread_index    m_exec_thread;
        bool			m_finished;
        thread_type     m_type;
        thread_id       m_id;

        JobLog(std::chrono::high_resolution_clock::time_point& t1, std::chrono::high_resolution_clock::time_point& t2,
            thread_index exec_thread, bool finished, thread_type type, thread_id id)
                : m_t1(t1), m_t2(t2), m_exec_thread(exec_thread), m_finished(finished), m_type(type), m_id(id) {
        };
    };

    class JobQueue_base {
        friend JobSystem;
    protected:
        static constexpr unsigned int K{ 2 };       // Hazard Pointer per thread
        static constexpr unsigned int P{ 32 };      // Maximum number of threads - fixed value until better solution
        static constexpr unsigned int N{ K * P };   // Total number of hazard pointer
        static constexpr unsigned int R{ 2 * N };   // Threshold for starting scan (R = N + Omega(H))

        static inline thread_local thread_index m_thread_index{0};  // Initialize with 0 until threads are ready and call initHP()
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
        template<typename JOB>
        struct node_t {
            JOB* job{ nullptr };
            std::atomic<node_t<JOB>*> next{ nullptr };
        };

        std::atomic<node_t<JOB>*>	    m_head;	        //points to first entry
        std::atomic<node_t<JOB>*>   	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size{ 0 };	//number of entries in the queue
        static inline node_t<JOB>*      m_HP[N]{};      // All Hazard pointers (read all, write only to 2 thread HP)

        // Per thread private variables
        static inline thread_local node_t<JOB>** m_hp0{ &m_HP[K * m_thread_index.value] };     // Thread hazard pointer
        static inline thread_local node_t<JOB>** m_hp1{ &m_HP[K * m_thread_index.value + 1] }; // FIFO Queue requires 2 HP 
        static inline thread_local node_t<JOB>* m_rlist[R]{};       // List of retired nodes
        static inline thread_local unsigned int m_rcount{ 0 };      // Number of retired nodes

        // Free nodes no longer in use by any thread
        void scan() {
            unsigned int i, pcount{ 0 }, next_rcount{ 0 };
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
            node_t<JOB>* new_node{ new node_t<JOB>() };     // Allocate new node. Alternatively use existing node from free_list
            new_node->job = job_to_push;                    // Copy job into node - next pointer of node already nullptr
            node_t<JOB>* tail;
            while (true) {                                                          // Try until job pushed into queue
                tail = m_tail.load(std::memory_order_relaxed);                      // Read Queue tail
                *m_hp0 = tail;                                                      // Set hazard pointer
                if (tail != m_tail.load(std::memory_order_acquire)) continue;       // Check if hazard pointer is valid, otherwise try again
                node_t<JOB>* next{ tail->next.load(std::memory_order_acquire) };    // Read next node of tail
                if (tail != m_tail.load(std::memory_order_acquire)) continue;       // Is Queue tail still consistent? if not, try again
                if (next != nullptr) {                                              // Was tail pointing to the last node?
                    m_tail.compare_exchange_weak(tail, next, std::memory_order_release);    // Tail was not last node, swing tail to next node
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


    /**
    * \brief The main JobSystem class manages the whole VGJS job system.
    *
    * The JobSystem starts N threads and provides them with data structures.
    * It can add new jobs, and wait until they are done.
    */
    class JobSystem {
        const uint32_t                          c_queue_capacity = 1<<20; ///<save at most N Jobs for recycling

    private:
        n_pmr::memory_resource*                 m_mr;                   ///<use to allocate/deallocate Jobs
        std::vector<std::thread>	            m_threads;	            ///<array of thread structures
        std::atomic<uint32_t>   		        m_thread_count = 0;     ///<number of threads in the pool
        std::atomic<bool>                       m_terminated = false;   ///<flag set true when the last thread has exited
        thread_index							m_start_idx;            ///<idx of first thread that is created
        static inline thread_local thread_index	m_thread_index = thread_index{};  ///<each thread has its own number
        std::atomic<bool>						m_terminate = false;	///<Flag for terminating the pool
        static inline thread_local Job_base*    m_current_job = nullptr;///<Pointer to the current job of this thread0
        std::vector<JobQueue<Job_base>>         m_global_queues;	    ///<each thread has its own Job queue, multiple produce, single consume
        std::vector<JobQueue<Job_base>>         m_local_queues;	        ///<each thread has its own Job queue, multiple produce, single consume

        std::unordered_map<tag,std::unique_ptr<JobQueue<Job_base>>,tag::hash>   m_tag_queues;

        JobQueue<Job>                           m_recycle;              ///<save old jobs for recycling
        JobQueue<Job>                           m_delete;               ///<save old jobs for recycling
        n_pmr::vector<n_pmr::vector<JobLog>>	m_logs;				    ///< log the start and stop times of jobs
        bool                                    m_logging = false;      ///< if true then jobs will be logged
        std::map<int32_t, std::string>          m_types;                ///<map types to a string for logging
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time = std::chrono::high_resolution_clock::now();	//time when program started

        /**
        * \brief Allocate a job so that it can be scheduled.
        * 
        * If there is a job in the recycle queue we use this. Else a new
        * new Job struct is allocated from the memory resource m_mr.
        * 
        * \returns a pointer to the job.
        */
        Job* allocate_job() {
            Job* job = m_recycle.pop();                                 //try recycle queue
            if (job == nullptr ) {                                      //none found
                n_pmr::polymorphic_allocator<Job> allocator(m_mr);      //use this allocator
                job = allocator.allocate(1);                            //allocate the object
                if (job == nullptr) {
                    std::cout << "No job available\n";
                    std::terminate();
                }
                new (job) Job(m_mr);                 //call constructor
            }
            else {                                  //job found
                job->reset();                       //reset it
            }
            return job;
        }

        /**
        * \brief Allocate a job so that it can be scheduled.
        * \param[in] f Function that should be executed by the job.
        * \returns a pointer to the Job.
        */
        Job* allocate_job( Function&& f) noexcept {
            Job* job            = allocate_job();
            job->m_function     = std::move(f.m_function);    //move the job
            if (!job->m_function) {
                std::cout << "Empty function\n";
                std::terminate();
            }
            job->m_thread_index = f.m_thread_index;
            job->m_type         = f.m_type;
            job->m_id           = f.m_id;
            return job;
        }

    public:

        /**
        * \brief JobSystem class constructor.
        * \param[in] threadCount Number of threads in the system.
        * \param[in] start_idx Number of first thread, if 1 then the main thread should enter as thread 0.
        * \param[in] mr The memory resource to use for allocating Jobs.
        */
        JobSystem(thread_count threadCount = thread_count(0), thread_index start_idx = thread_index(0), n_pmr::memory_resource* mr = n_pmr::new_delete_resource()) noexcept
            : m_mr(mr), m_start_idx(start_idx) {

            m_thread_count = threadCount.value;
            if (m_thread_count <= 0) {
                m_thread_count = std::thread::hardware_concurrency();		///< main thread is also running
            }
            if (m_thread_count == 0) {
                m_thread_count = 1;
            }

            for (uint32_t i = 0; i < m_thread_count; i++) {
                m_global_queues.push_back(JobQueue<Job_base>());     //global job queue
                m_local_queues.push_back(JobQueue<Job_base>());     //local job queue
            }

            for (uint32_t i = start_idx.value; i < m_thread_count; i++) {
                //std::cout << "Starting thread " << i << std::endl;
                m_threads.push_back(std::thread(&JobSystem::thread_task, this, thread_index(i) ));	//spawn the pool threads
                m_threads[i].detach();
            }

            m_logs.resize(m_thread_count, n_pmr::vector<JobLog>{mr});    //make room for the log files
        };

        /**
        * \brief Singleton access through class.
        * \param[in] threadCount Number of threads in the system.
        * \param[in] start_idx Number of first thread, if 1 then the main thread should enter as thread 0.
        * \param[in] mr The memory resource to use for allocating Jobs.
        * \returns a pointer to the JobSystem instance.
        */
        static JobSystem& instance( thread_count threadCount = thread_count(0)
                                    , thread_index start_idx = thread_index(0)
                                    , n_pmr::memory_resource* mr = n_pmr::new_delete_resource()) noexcept {
            static JobSystem instance(threadCount, start_idx, mr); //thread safe init guaranteed - Meyer's Singleton
            return instance;
        };

        /**
        * \brief Test whether the job system has been started yet.
        * \returns true if the instance exists, else false.
        */
        static bool is_instance_created() noexcept {
            return m_current_job != nullptr;
        };

        JobSystem(const JobSystem&) = delete;				// non-copyable,
        JobSystem& operator=(const JobSystem&) = delete;
        JobSystem(JobSystem&&) = default;					// but movable
        JobSystem& operator=(JobSystem&&) = default;

        /**
        * \brief JobSystem class destructor.
        *
        * By default shuts down the system and waits for the threads to terminate.
        */
        ~JobSystem() noexcept {
            m_terminate = true;
            wait_for_termination();
        };

        void on_finished(Job* job) noexcept;            //called when the job finishes, i.e. all children have finished

        /**
        * \brief Child tells its parent that it has finished.
        *
        * A child that finished calls this function for its parent, thus decreasing
        * the number of left children by one. If the last one finishes (including the
        * parent itself) then the parent also finishes (and may call its own parent).
        * Note that a Job is also its own child, so it must have returned from
        * its function before on_finished() is called. Note that a Job is also its own
        * child so that the Job can only finish after its function has returned.
        */
        inline bool child_finished(Job_base* job) noexcept {
            uint32_t num = job->m_children.fetch_sub(1);        //one less child
            if (num == 1) {                                     //was it the last child?

                if (job->is_function()) {            //Jobs call always on_finished()
                    on_finished((Job*)job);     //if yes then finish this job
                }
                else {
                    schedule(job);   //a coro just gets scheduled again so it can go on
                }
                return true;
            }
            return false;
        }

        /**
        * \brief Every thread runs in this function
        * \param[in] threadIndex Number of this thread
        */
        void thread_task(thread_index threadIndex = thread_index(0) ) noexcept {
            constexpr uint32_t NOOP = 10;                                   //number of empty loops until garbage collection
            m_thread_index = threadIndex;	                                //Remember your own thread index number

            JobQueue_base::m_thread_index = threadIndex;        // Set threadindex for HP
            
            // Correctly initialize HP in all JobQueues
            for (auto& queue : m_global_queues) queue.initHP();
            for (auto& queue : m_local_queues) queue.initHP();
            m_recycle.initHP();
            m_delete.initHP();

            static std::atomic<uint32_t> thread_counter = m_thread_count.load();	//Counted down when started

            thread_counter--;			                                    //count down
            while (thread_counter.load() > 0) {}	                        //Continue only if all threads are running

            uint32_t next = rand() % m_thread_count;                        //initialize at random position for stealing
            thread_local uint32_t noop = NOOP;                              //number of empty loops until threads sleeps
            while (!m_terminate) {			                                //Run until the job system is terminated
                m_current_job = m_local_queues[m_thread_index.value].pop();       //try get a job from the local queue
                if (m_current_job == nullptr) {
                    m_current_job = m_global_queues[m_thread_index.value].pop();  //try get a job from the global queue
                }
                if (m_current_job == nullptr) {                             //try steal job from another thread
                    if (++next >= m_thread_count) next = 0;
                    m_current_job = m_global_queues[next].pop();
                }

                if (m_current_job != nullptr) {
                    std::chrono::high_resolution_clock::time_point t1, t2;	///< execution start and end

                    if (is_logging()) {
                        t1 = std::chrono::high_resolution_clock::now();	//time of finishing;
                    }

                    auto is_function = m_current_job->is_function();      //save certain info since a coro might be destroyed
                    auto type = m_current_job->m_type;
                    auto id = m_current_job->m_id;

                    (*m_current_job)();   //if any job found execute it - a coro might be destroyed here!

                    if (is_logging()) {
                        t2 = std::chrono::high_resolution_clock::now();	//time of finishing
                        log_data(t1, t2, m_thread_index, false, type, id );
                    }

                    if (is_function) {
                        child_finished((Job*)m_current_job);  //a job always finishes itself, a coro will deal with this itself
                    }
                }
                --noop;
                if (noop == 0) {                //if none found too longs let thread sleep
                    noop = NOOP;
                    if (m_thread_index.value == 0) {  //thread 0 is the garbage collector
                        m_delete.clear();       //delete jobs to reclaim memory
                    }
                }
            };

           //std::cout << "Thread " << m_thread_index << " left " << m_thread_count << "\n";

           m_global_queues[m_thread_index.value].clear(); //clear your global queue
           m_local_queues[m_thread_index.value].clear();  //clear your local queue

           uint32_t num = m_thread_count.fetch_sub(1);  //last thread clears recycle and garbage queues
           if (num == 1) {
               m_recycle.clear();
               m_delete.clear();

               if (m_logging) {         //dump trace file
                   save_log_file();
               }
               //std::cout << "Last thread " << m_thread_index << " terminated\n";
               m_terminated = true;
           }
        };

        /**
        * \brief An old Job can be recycled. 
        * 
        * There is one recycle queue that can store old Jobs. 
        * If it is full then put the Job to the delete queue.
        * 
        * \param[in] job Pointer to the finished Job.
        */
        void recycle(Job* job) noexcept {
            if (m_recycle.size() <= c_queue_capacity) {
                m_recycle.push(job);        //save it so it can be reused later
            }
            else {
                m_delete.push(job);   //push to delete queue
            }
        }

        /**
        * \brief Terminate the job system.
        */
        void terminate() noexcept {
            m_terminate = true;
        }

        /**
        * \brief Wait for termination of all jobs.
        *
        * Can be called by the main thread to wait for all threads to terminate.
        * Returns as soon as all threads have exited.
        */
        void wait_for_termination() noexcept {
            while (m_terminated.load() == false) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        };

        /**
        * \brief Get a pointer to the current job.
        * \returns a pointer to the current job.
        */
        Job_base* current_job() noexcept {
            return m_current_job;
        }

        /**
        * \brief Get the thread index the current job is running on.
        * \returns the index of the thread the current job is running on, or -1.
        */
        thread_index get_thread_index() {
            return m_thread_index;
        }

        /**
        * \brief Get the number of threads in the system.
        * \returns the number of threads in the system.
        */
        thread_count get_thread_count() {
            return thread_count( m_thread_count );
        }

        /**
        * \brief Get the memory resource used for allocating job structures.
        * \returns the memory resource used for allocating job structures.
        */
        n_pmr::memory_resource* memory_resource() {
            return m_mr;
        }

        /**
        * \brief Schedule a job into the job system.
        * The Job will be put into a thread's queue for consumption.
        * 
        * \param[in] job A pointer to the job to schedule.
        */
        void schedule(Job_base* job, tag tg = tag{}) noexcept {
            assert(job!=nullptr);

            if ( tg.value >= 0 ) {
                if(!m_tag_queues.contains(tg)) {
                    m_tag_queues[tg] = std::make_unique<JobQueue<Job_base>>();
                }
                m_tag_queues.at(tg)->push(job);
                return;
            }

            if (job->m_thread_index.value < 0 || job->m_thread_index.value >= (int)m_thread_count ) {
                 m_global_queues[rand() % m_thread_count].push(job);
                 return;
            }

            m_local_queues[job->m_thread_index.value].push(job);
        };

        /**
        * \brief Schedule a Job holding a function into the job system.
        * \param[in] source An external function that is copied into the scheduled job.
        * \param[in] parent The parent of this Job.
        * \param[in] children Number used to increase the number of children of the parent.
        */
        void schedule(Function&& source, tag tg = tag{}, Job_base* parent = m_current_job, int32_t children = 1) noexcept {
            Job *job = allocate_job( std::forward<Function>(source) );

            job->m_parent = nullptr;
            if (tg.value < 0 ) { 
                job->m_parent = parent;
                if (parent != nullptr) { parent->m_children.fetch_add((int)children); }
            }
            schedule(job, tg);
        };

        /**
        * \brief Schedule a Job holding a function into the job system.
        * \param[in] f An external function that is copied into the scheduled job.
        * \param[in] tg The tag that is scheduled
        * \param[in] parent The parent of this Job.
        * \param[in] children Number used to increase the number of children of the parent.
        */
        void schedule(std::function<void(void)>&& f, tag tg = tag{}, Job_base* parent = m_current_job, int32_t children = 1) noexcept {
            schedule(Function{ std::forward<std::function<void(void)>>(f) }, tg, parent, children );
        }

        /**
        * \brief Schedule all Jobs from a tag
        * \param[in] tg The tag that is scheduled
        * \param[in] parent The parent of this Job.
        * \param[in] children Number used to increase the number of children of the parent.
        * \returns the number of scheduled jobs.
        */
        uint32_t schedule(tag tg, tag tg2 = tag{}, Job_base* parent = m_current_job, int32_t children = -1) noexcept {
            if (!m_tag_queues.contains(tg)) return 0;

            JobQueue<Job_base>* queue = m_tag_queues[tg].get();   //get the queue for this tag
            uint32_t num_jobs = queue->size();

            if (parent != nullptr) { 
                if (children < 0) children = num_jobs;     //if the number of children is not given, then use queue size
                parent->m_children.fetch_add((int)children);    //add this number to the number of children of parent
            }

            uint32_t num = num_jobs;        //schedule at most num_jobs, since someone could add more jobs now
            Job_base* job = queue->pop();
            while ( num>0 && job) {     //schedule all jobs from the tag queue
                job->m_parent = parent;
                schedule(job, tag{});
                --num;
                job = queue->pop();
            }
            return num_jobs;
        };

        /**
        * \brief Store a continuation for the current Job. Will be scheduled once the current Job finishes.
        * \param[in] f The function to schedule as continuation.
        */
        void continuation( Function&& f ) noexcept {
            Job_base* current = current_job();
            if (current == nullptr || !current->is_function()) {
                return;
            }
            ((Job*)current)->m_continuation = allocate_job(std::forward<Function>(f));
        }

        //-----------------------------------------------------------------------------------------

        /**
        * \brief Get the logging data so it can be saved to file.
        * \returns a reference to the logging data.
        */
        auto& get_logs() {
            return m_logs;
        }

        /**
        * \brief Clear all logs.
        */
        void clear_logs() {
            for (auto& log : m_logs) {
                log.clear();
            }
        }

        /**
        * \brief Enable logging.
        * If logging is enabled, start/stop times and other data of each thread is saved
        * in a memory data structure.
        */
        void enable_logging() {
            m_logging = true;
        }

        /**
        * \brief Disable logging.
        * If logging is enabled, start/stop times and other data of each thread is saved
        * in a memory data structure.
        */
        void disable_logging() {
            if (m_logging) {
                save_log_file();
            }
            m_logging = false;
        }

        /**
        * \brief Ask whether logging is currently enabled or not
        * \returns true or false
        */
        bool is_logging() {
            return m_logging;
        }

        /**
        * \brief Get the the time when the job system was started (for logging)
        * \returns the time the job system was started
        */
        auto start_time() {
            return m_start_time;
        }

        /**
        * \brief Get the mapping between type number and a string explaining the type.
        * 
        * Jobs can have a type integer that ids the type, like a function or a coro.
        * If logging is to be used, this map should be filled with strings explaining the types.
        * This should be done before using the job system.
        * \returns a std::map that maps type integers to strings
        */
        auto& types() {
            return m_types;
        }

    };

    //----------------------------------------------------------------------------------------------

    /**
    * \brief A Job holding a function and all its children have finished.
    *
    * This is called when a Job and its children has finished.
    * If there is a continuation stored in the job, then the continuation
    * gets scheduled. Also the job's parent is notified of this new child.
    * Then, if there is a parent, the parent's child_finished() function is called.
    */
    inline void JobSystem::on_finished(Job *job) noexcept {

        if (job->m_continuation != nullptr) {		//is there a successor Job?
            
            if (job->m_parent != nullptr) {         //is there is a parent?                
                job->m_parent->m_children++;
                job->m_continuation->m_parent = job->m_parent;   //add successor as child to the parent
            }
            schedule(job->m_continuation);    //schedule the successor
        }

        if (job->m_parent != nullptr) {		//if there is parent then inform it	
            child_finished((Job*)job->m_parent);	//if this is the last child job then the parent will also finish
        }

        recycle(job);       //recycle the Job
    }


    //----------------------------------------------------------------------------------

    /**
    * \brief Get the current job that is executed by the system.
    * \returns the job that is currently executed.
    */
    inline Job_base* current_job() {
        return JobSystem::is_instance_created() ? (Job_base*)JobSystem::instance().current_job() : nullptr;
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule( Function&& f, tag tg = tag{}, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<Function>(f), tg, parent, children );
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule(Function& f, tag tg = tag{}, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule(std::forward<Function>(f), tg, parent, children);
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule( std::function<void(void)>&& f, tag tg = tag{}, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<std::function<void(void)>>(f), tg, parent, children); // forward to the job system
    };

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule(std::function<void(void)>& f, tag tg = tag{}, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<std::function<void(void)>>(f), tg, parent, children);   // forward to the job system
    };


    /**
    * \brief Schedule all functions from a tag
    * \param[in] tg The tag to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule(tag tg, tag tg2 = tag{}, Job_base * parent = current_job(), int32_t children = -1) noexcept {
        JobSystem::instance().schedule(tg, tg2, parent, children);   // forward to the job system
    };


    /**
    * \brief Schedule functions into the system. T can be a Function, std::function or a task<U>.
    * 
    * The parameter children here is used to pre-increase the number of children to avoid races
    * between more schedules and previous children finishing and destroying e.g. a coro.
    * When a tuple of vectors is scheduled, in the first call children is the total number of all children
    * in all vectors combined. After this children is set to 0 (by the caller).
    * When a vector is scheduled, children should be the default -1, and setting the number of 
    * children is handled by the function itself.
    * 
    * \param[in] functions A vector of functions to schedule
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    template<typename T>
    inline void schedule( n_pmr::vector<T>& functions, tag tg = tag{}, Job_base* parent = current_job(), int32_t children = -1) noexcept {
        
        if (children < 0) {                     //default? use vector size.
            children = (int)functions.size(); 
        }

        for (auto& f : functions) { //schedule all elements, use the total number of children for the first call, then 0
            schedule( std::forward<T>(f), tg, parent, children ); //might call the coro version, so do not call job system here!
            children = 0;
        }
    };

    /**
    * \brief Store a continuation for the current Job. The continuation will be scheduled once the job finishes.
    * \param[in] f A function to schedule
    */
    inline void continuation(Function&& f) noexcept {
        JobSystem::instance().continuation(std::forward<Function>(f)); // forward to the job system
    }

    /**
    * \brief Store a continuation for the current Job. The continuation will be scheduled once the job finishes.
    * \param[in] f A function to schedule
    */
    inline void continuation(Function& f) noexcept {
        JobSystem::instance().continuation(Function{ std::forward<Function>(f) }); // forward to the job system
    }

    /**
    * \brief Store a continuation for the current Job. The continuation will be scheduled once the job finishes.
    * \param[in] f A function to schedule
    */
    inline void continuation(std::function<void(void)>&& f) noexcept {
        JobSystem::instance().continuation(Function{ f }); // forward to the job system
    }

    /**
    * \brief Store a continuation for the current Job. The continuation will be scheduled once the job finishes.
    * \param[in] f A function to schedule
    */
    inline void continuation(std::function<void(void)>& f) noexcept {
        JobSystem::instance().continuation(Function{ f }); // forward to the job system
    }

    //----------------------------------------------------------------------------------

    /**
    * \brief Terminate the job system
    */
    inline void terminate() {
        JobSystem::instance().terminate();
    }

    /**
    * \brief Wait for the job system to terminate
    */
    inline void wait_for_termination() {
        JobSystem::instance().wait_for_termination();
    }

    /**
    * \brief Enable logging.
    * If logging is enabled, start/stop times and other data of each thread is saved
    * in a memory data structure.
    */
    inline void enable_logging() {
        JobSystem::instance().enable_logging();
    }

    /**
    * \brief Enable logging.
    * If logging is enabled, start/stop times and other data of each thread is saved
    * in a memory data structure.
    */
    inline void disable_logging() {
        JobSystem::instance().disable_logging();
    }

    /**
    * \returns whether logging is turned on
    */
    inline bool is_logging() {
        return JobSystem::instance().is_logging();
    }

    /**
    * \brief Get the logging data so it can be saved to file.
    * \returns a reference to the logging data.
    */
    inline auto& get_logs() {
        return JobSystem::instance().get_logs();
    }

    /**
    * \brief Clear all logs.
    */
    inline void clear_logs() {
        JobSystem::instance().clear_logs();
    }

    /**
    * \brief Store a job run in the log data
    * 
    * \param[in] t1 Start time of the job.
    * \param[in] t2 End time of the job.
    * \param[in] exec_thread Index of the thread that ran the job.
    * \param[in] finished If true, then the job finished. 
    * \param[in] type The job type.
    * \param[in] id A unique ID.
    */
    inline void log_data(
        std::chrono::high_resolution_clock::time_point& t1, std::chrono::high_resolution_clock::time_point& t2,
        thread_index exec_thread, bool finished, thread_type type, thread_id id) {

        auto& logs = JobSystem::instance().get_logs();
        logs[JobSystem::instance().get_thread_index().value].emplace_back( t1, t2, JobSystem::instance().get_thread_index(), finished, type, id);
    }

    /**
    * \brief Store a job run in the log data
    *
    * \param[in] out The output stream for the log file.
    * \param[in] cat Undefined.
    * \param[in] pid Always 0.
    * \param[in] tid The thread index that ran the job.
    * \param[in] ts Start time.
    * \param[in] dur Duration of the job.
    * \param[in] ph Always X
    * \param[in] name Type name of the job.
    * \param[in] args Indicates wehther the job has finshed.
    */
    inline void save_job(   std::ofstream& out
                            , std::string cat
                            , uint64_t pid
                            , uint64_t tid
                            , uint64_t ts
                            , int64_t dur
                            , std::string ph
                            , std::string name
                            , std::string args) {

        std::stringstream time;
        time.precision(15);
        time << ts / 1.0e3;

        std::stringstream duration;
        duration.precision(15);
        duration << dur / 1.0e3;

        out << "{";
        out << "\"cat\": " << cat << ", ";
        out << "\"pid\": " << pid << ", ";
        out << "\"tid\": " << tid << ", ";
        out << "\"ts\": " << time.str() << ", ";
        out << "\"dur\": " << duration.str() << ", ";
        out << "\"ph\": " << ph << ", ";
        out << "\"name\": " << name << ", ";
        out << "\"args\": {" << args << "}";
        out << "}";
    }

    /**
    * \brief Dump all job data into a json log file.
    */
    inline void save_log_file() {
        auto& logs = JobSystem::instance().get_logs();
        std::ofstream outdata;
        outdata.open("log.json");
        auto& types = JobSystem::instance().types();

        if (outdata) {
            outdata << "{" << std::endl;
            outdata << "\"traceEvents\": [" << std::endl;
            bool comma = false;
            for (uint32_t i = 0; i < logs.size(); ++i) {
                for (auto& ev : logs[i]) {
                    if (ev.m_t1 >= JobSystem::instance().start_time() && ev.m_t2 >= ev.m_t1) {

                        if (comma) outdata << "," << std::endl;

                        auto it = types.find(ev.m_type.value);
                        std::string name = "-";
                        if (it != types.end()) name = it->second;

                        save_job(outdata, "\"cat\"", 0, (uint32_t)ev.m_exec_thread.value,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(ev.m_t1 - JobSystem::instance().start_time()).count(),
                            std::chrono::duration_cast<std::chrono::nanoseconds>(ev.m_t2 - ev.m_t1).count(),
                            "\"X\"", "\"" + name + "\"", "\"id\": " + std::to_string(ev.m_id.value));

                        comma = true;
                    }
                }
            }
            outdata << "]," << std::endl;
            outdata << "\"displayTimeUnit\": \"ms\"" << std::endl;
            outdata << "}" << std::endl;
        }
        outdata.close();
        JobSystem::instance().clear_logs();
    }

}



#endif

