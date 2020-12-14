#include "include.h"
#include "workFunc.h"
#include "workCoro.h"

// Tests
namespace mandelbrotFunc {
	void test();
}
namespace mandelbrotCoro {
	void test();
}
namespace mctsFunc {
	void test();
}
namespace mctsCoro {
	void test();
}

// Google Benchmarks
static void BM_MandelbrotFunc(benchmark::State&);
static void BM_MandelbrotCoro(benchmark::State&);


// General Settings
const int g_num_threads = 1;
const int g_num_seconds = 20;

//const int g_num_loops	= 365; // 10us
//const int g_num_loops	= 253; // 7us
const int g_num_loops	= 190; // 5us
const int g_num_jobs	= std::thread::hardware_concurrency();



// Start a selected timed Benchmark
void startTimedBenchmark(const int num_loops, const int num_jobs, const int num_sec) {

	std::chrono::time_point<std::chrono::system_clock> end;
	end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

	schedule([=]() {workFunc::benchmarkTimedWork(num_loops, num_jobs, end); });
	//schedule(workCoro::benchmarkTimedWork(num_loops, num_jobs, end));

	continuation([]() {vgjs::terminate(); });

	//recursive call with switch case to work through each schedule one after another? maybe just vector?
}


// Start a selected single Benchmark
void startSingleBenchmark(const int num_loops, const int num_jobs) {
	schedule([=]() {workFunc::benchmarkWork(num_loops, num_jobs); });
	//schedule(workCoro::benchmarkWork(num_loops, num_jobs));

	continuation([]() {vgjs::terminate(); });
}


void startGoogleBenchmarks(const int num_loops) {

	int argc = 0;
	char** argv = {};

	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };

	// C++ Functions
	benchmark::RegisterBenchmark("work()", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMicrosecond)->Arg(num_loops);

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
}



int main() {

	startGoogleBenchmarks(g_num_loops);

	JobSystem::instance(g_num_threads);
	std::cout << std::endl <<	"Number of Threads used in VGJS: " << g_num_threads << std::endl;
	std::cout <<				"Number of work() Jobs:          " << g_num_jobs << std::endl << std::endl;

	schedule([]() {startTimedBenchmark(g_num_loops, g_num_jobs, g_num_seconds); });

	//schedule([]() {startSingleBenchmark(g_num_loops, g_num_jobs); });

	wait_for_termination();
}













// (Google) Benchmark wrapper for Mandelbrot Func Test
static void BM_MandelbrotFunc(benchmark::State& state) {

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotFunc::test(); });
		wait_for_termination();
	}
}

// (Google) Benchmark wrapper for Mandelbrot Coro Test
static void BM_MandelbrotCoro(benchmark::State& state) {

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotCoro::test(); });
		wait_for_termination();
	}
}