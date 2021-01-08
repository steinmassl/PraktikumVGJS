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
    
    template<typename JOB = Job_base>
    struct pointer_t;
    template<typename JOB = Job_base>
    struct node_t;


    struct thread_index {
        int32_t value;
        explicit thread_index(int32_t v = -1 ) : value(v) {};
    };

    struct thread_id {
        int32_t value;
        explicit thread_id(int32_t v = -1) : value(v) {};
    };

    struct thread_type {
        int32_t value;
        explicit thread_type(int32_t v = -1) : value(v) {};
    };

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

        Function(std::function<void(void)>&& f, thread_index index = thread_index{}, thread_type type = thread_type{}, thread_id id = thread_id{})
            : m_function(std::move(f)), m_thread_index(index), m_type(type), m_id(id) {};

        Function(std::function<void(void)>& f, thread_index index = thread_index{}, thread_type type = thread_type{}, thread_id id = thread_id{})
            : m_function(f), m_thread_index(index), m_type(type), m_id(id) {};

        Function(const Function& f) 
            : m_function(f.m_function), m_thread_index(f.m_thread_index), m_type(f.m_type), m_id(f.m_id) {};

        Function(Function& f) 
            : m_function(std::move(f.m_function)), m_thread_index(f.m_thread_index), m_type(f.m_type), m_id(f.m_id) {};

        Function& operator= (const Function& f) {
            m_function = f.m_function; m_thread_index = f.m_thread_index; m_type = f.m_type;  m_id = f.m_id;
            return *this;
        };

        Function& operator= (Function&& f) {
            m_function = std::move(f.m_function); m_thread_index = f.m_thread_index; m_type = f.m_type;  m_id = f.m_id;
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
    * \brief Base class of things you can put into a queue
    */
    /*
    class Queuable {
    public:
        //Queuable* m_next = nullptr;           //next job in the queue
    };
    */

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
        n_pmr::memory_resource*  m_mr = nullptr;  //memory resource that was used to allocate this Job
        Job_base*                   m_continuation = nullptr;   //continuation follows this job (a coro is its own continuation)
        std::function<void(void)>   m_function;      //function to compute

        Job() : Job_base() {
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
    * \brief General FIFO queue class.
    *
    * The queue allows for multiple producers multiple consumers. It uses a lightweight
    * atomic flag as lock. 
    */
    template<typename JOB = Job_base>
    class JobQueue {
        friend JobSystem;
        std::atomic<pointer_t<JOB>>	    m_head;	        //points to first entry
        std::atomic<pointer_t<JOB>> 	m_tail;	        //points to last entry
        std::atomic<int32_t>	        m_size = 0;		//number of entries in the queue

    public:

        JobQueue() {
            node_t<JOB>* start = new node_t<JOB>();
            m_head.store(start);
            m_tail.store(start);
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
                tail = m_tail.load();
                pointer_t<JOB> next = tail.ptr->next.load();
                if (tail == m_tail.load()) {
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
                head = m_head.load();
                pointer_t<JOB> tail = m_tail.load();
                pointer_t<JOB> next = head.ptr->next.load();

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

        /**
        * \brief Deallocate all Jobs in the queue.
        * Since Jobs and coros use different allocation strategies, for each entry a deallocator
        * is used for deallocation.
        */
        uint32_t clear() {
            uint32_t res = m_size;
            JOB* job = nullptr;
            bool successful_pop = pop(job);						//deallocate jobs that run a function
            while (successful_pop && (job != nullptr)) {						//because they were allocated by the JobSystem
                auto da = job->get_deallocator();			//get deallocator
                da.deallocate(job);						//deallocate the memory
                successful_pop = pop(job);								//get next entry
            }
            return res;
        }

        /**
        * \brief Get the number of jobs currently in the queue.
        * \returns the number of jobs (Coros and Jobs) currently in the queue.
        */
        uint32_t size() {
            return m_size;
        }
    };


    /**
    * \brief The main JobSystem class manages the whole VGJS job system.
    *
    * The JobSystem starts N threads and provides them with data structures.
    * It can add new jobs, and wait until they are done.
    */
    class JobSystem {
        const uint32_t                              c_queue_capacity = 100; ///<save at most N Jobs for recycling

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
            Job* job = nullptr;
            bool successful_pop = m_recycle.pop(job);                                //try recycle queue
            if (!successful_pop || job == nullptr) {                                     //none found
                n_pmr::polymorphic_allocator<Job> allocator(m_mr);  //use this allocator
                job = allocator.allocate(1);                           //allocate the object
                if (job == nullptr) {
                    std::cout << "No job available\n";
                    std::terminate();
                }
                new (job) Job();                     //call constructor
                job->m_mr = m_mr;                    //save memory resource for deallocation
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
        JobSystem(uint32_t threadCount = 0, uint32_t start_idx = 0, n_pmr::memory_resource *mr = n_pmr::new_delete_resource() ) noexcept
            : m_mr(mr), m_start_idx(start_idx) {

            m_thread_count = threadCount;
            if (m_thread_count == 0) {
                m_thread_count = std::thread::hardware_concurrency();		///< main thread is also running
            }
            if (m_thread_count == 0) {
                m_thread_count = 1;
            }

            for (uint32_t i = 0; i < m_thread_count; i++) {
                m_global_queues.push_back(JobQueue<Job_base>());     //global job queue
                m_local_queues.push_back(JobQueue<Job_base>());     //local job queue
            }

            for (uint32_t i = start_idx; i < m_thread_count; i++) {
                std::cout << "Starting thread " << i << std::endl;
                m_threads.push_back(std::thread(&JobSystem::thread_task, this, i));	//spawn the pool threads
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
        static JobSystem& instance( uint32_t threadCount = 0
                                    , uint32_t start_idx = 0
                                    , n_pmr::memory_resource* mr = n_pmr::new_delete_resource()) noexcept {
            static JobSystem instance(threadCount, start_idx, mr); //thread safe init guaranteed - Meyer's Singleton
            return instance;
        };

        /**
        * \brief Test whether the job system has been started yet.
        * \returns true if the instance exists, else false.
        */
        static bool is_instance_created() noexcept {
            return m_thread_index.value >= 0;
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
        void thread_task(int32_t threadIndex = 0) noexcept {
            constexpr uint32_t NOOP = 10;                                           //number of empty loops until garbage collection
            m_thread_index.value = threadIndex;	                        //Remember your own thread index number
            static std::atomic<uint32_t> thread_counter = m_thread_count.load();	//Counted down when started

            thread_counter--;			                                    //count down
            while (thread_counter.load() > 0) {}	                        //Continue only if all threads are running

            uint32_t next = rand() % m_thread_count;                        //initialize at random position for stealing
            thread_local uint32_t noop = NOOP;                               //number of empty loops until threads sleeps
            while (!m_terminate) {			                                //Run until the job system is terminated
                bool successful_pop = m_local_queues[m_thread_index.value].pop(m_current_job);       //try get a job from the local queue
                if (m_current_job == nullptr || !successful_pop) {
                    successful_pop = m_global_queues[m_thread_index.value].pop(m_current_job);  //try get a job from the global queue
                }

                if (m_current_job == nullptr || !successful_pop) {                             //try steal job from another thread
                    if (++next >= m_thread_count) next = 0;
                    successful_pop = m_global_queues[next].pop(m_current_job);
                }

                if (m_current_job != nullptr && successful_pop) {
                    std::chrono::high_resolution_clock::time_point t1, t2;	///< execution start and end

                    if (is_logging()) {
                        t1 = std::chrono::high_resolution_clock::now();	//time of starting;
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
        uint32_t thread_count() {
            return m_thread_count;
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
        void schedule(Job_base* job ) noexcept {
            assert(job!=nullptr);

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
        void schedule(Function&& source, Job_base* parent = m_current_job, int32_t children = 1) noexcept {
            Job *job = allocate_job( std::forward<Function>(source) );
            job->m_parent = parent;
            if (parent != nullptr) { parent->m_children.fetch_add((int)children); }
            schedule(job);
        };

        /**
        * \brief Schedule a Job holding a function into the job system.
        * \param[in] source An external function that is copied into the scheduled job.
        * \param[in] parent The parent of this Job.
        * \param[in] children Number used to increase the number of children of the parent.
        */
        void schedule(std::function<void(void)>&& f, Job_base* parent = m_current_job, int32_t children = 1) noexcept {
            schedule(Function{ std::forward<std::function<void(void)>>(f) }, parent, children );
        }

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
        return (Job_base*)JobSystem::instance().current_job();
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule( Function&& f, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<Function>(f), parent, children );
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule(Function& f, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule(std::forward<Function>(f), parent, children);
    }

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule( std::function<void(void)>&& f, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<std::function<void(void)>>(f), parent, children); // forward to the job system
    };

    /**
    * \brief Schedule a function into the system.
    * \param[in] f A function to schedule.
    * \param[in] parent The parent of this Job.
    * \param[in] children Number used to increase the number of children of the parent.
    */
    inline void schedule(std::function<void(void)>& f, Job_base* parent = current_job(), int32_t children = 1) noexcept {
        JobSystem::instance().schedule( std::forward<std::function<void(void)>>(f), parent, children);   // forward to the job system
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
    inline void schedule( n_pmr::vector<T>& functions, Job_base* parent = current_job(), int32_t children = -1) noexcept {
        
        if (children < 0) {                     //default? use vector size.
            children = (int)functions.size(); 
        }

        for (auto& f : functions) { //schedule all elements, use the total number of children for the first call, then 0
            schedule( std::forward<T>(f), parent, children ); //might call the coro version, so do not call job system here!
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

