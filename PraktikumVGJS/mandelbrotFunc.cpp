#include "tests.h"
#include <complex>

namespace mandelbrotFunc {
	
	// Dimensions
	constexpr uint32_t WIDTH = 1000;					
	constexpr uint32_t HEIGHT = 1000;

	constexpr uint32_t MAX_ITERATIONS = 50;			// Maximum number of iterations per pixel

	uint16_t pixels[WIDTH * HEIGHT];				// Store pixel values while working

	// Create ppm file and write values into it
	void draw() {
		std::cout << "Drawing..." << std::endl;
		std::ofstream mandelbrotImage("mandelbrotFunc.ppm");
		if (mandelbrotImage.is_open()) {
			mandelbrotImage << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";		// PPM Header data
			for (uint32_t i = 0; i < HEIGHT; i++) {
				for (uint32_t j = 0; j < WIDTH; j++) {
					uint16_t value = pixels[i * WIDTH + j];
					mandelbrotImage << value << " " << value << " " << value << "\n";		// Write values from pixel storage into file
				}
			}
			mandelbrotImage.close();
		}
		else
			std::cout << "File error" << std::endl;
	}

	// Calculate the value of each pixel up to a certain number of iterations using coordinates as complex number
	void calculatePixel(uint32_t x, uint32_t y) {
		std::complex<double> point( x / (WIDTH*0.5) - 1.5, y / (HEIGHT*0.5) - 1.0);			// Project onto image - the complex number to add in the iteration
		
		std::complex<double> z(0, 0);								// Mandelbrot starting value
		uint32_t iterations = 0;											// Current number of iterations on this point
		while (std::abs(z) < 2 && iterations <= MAX_ITERATIONS) {	// Abort after certain iterations or when point is unstable
			z = z * z + point;
			iterations++;
		}
		if (iterations < MAX_ITERATIONS) {
			pixels[y * WIDTH + x] = (255*iterations)/(MAX_ITERATIONS-1);							// Unstable points - add color gradient
		}
		else {
			pixels[y * WIDTH + x] = 0;																// Stable points
		}
	}
	
	n_pmr::vector<std::function<void(void)>> g_vector;						// Use same vector for every recursive call (save ram when thread count in VGJS is low)

	void mandelbrotRecursive(uint32_t y) {
		g_vector.clear();													// Clear vector to reuse for this round

		for (uint32_t i = 0; i < WIDTH; i++) {
			g_vector.emplace_back([=]() {calculatePixel(i, y); });			// Parallelise per pixel
		}
		schedule( g_vector);
		continuation([=]() {
			if (y < (HEIGHT - 1))
				schedule([=]() {mandelbrotRecursive(y + 1); });
		});

	}

	void test() {
		//std::cout << "Starting MandelbrotFunc" << std::endl;

		auto start = std::chrono::high_resolution_clock::now();
		schedule([]() {mandelbrotRecursive(0); });
		continuation([=]() {
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
			std::cout << std::endl << 	"   Test: mandelbrotFunc" << std::endl;
			std::cout << 				"   Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
			//draw();		// Start drawing
			//std::cout << "End MandelbrotFunc" << std::endl;
		});				
	}

	// Store fixed-time benchmark data
	std::atomic<uint32_t>						g_call_count = 0;               // Number of times measureAll has been called
	std::chrono::duration<double, std::micro>	g_total_timed_runtime = {};     // Sum of measureAll execution times (check overhead of surrounding code)
	std::vector<double>                         g_runtime_vec;                  // Vector of batch execution times for processing

	n_pmr::vector<std::function<void(void)>> g_vec;    // Reuse vector for work jobs

	// Recursively benchmark batches of work until time runs out
	void measureAll(const std::chrono::time_point<std::chrono::high_resolution_clock> end_of_benchmark) {
		auto measureAll_start = std::chrono::high_resolution_clock::now();

		g_call_count++;

		auto start = std::chrono::high_resolution_clock::now();
		schedule([]() {mandelbrotRecursive(0); });

		continuation([=]() {

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::micro> elapsed_work_microseconds = end - start;
			g_runtime_vec.push_back(elapsed_work_microseconds.count());


			auto measureAll_end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::micro> elapsed_measureAll_microseconds = measureAll_end - measureAll_start;
			g_total_timed_runtime += elapsed_measureAll_microseconds;

			if (std::chrono::high_resolution_clock::now() < end_of_benchmark) {
				measureAll(end_of_benchmark);
			}
		});
	}

	// Benchmark work until time runs out
	void benchmarkWithFixedTime(const uint32_t num_sec, const uint32_t num_threads) {
		std::chrono::time_point<std::chrono::high_resolution_clock> end_of_benchmark;
		end_of_benchmark = std::chrono::high_resolution_clock::now() + std::chrono::seconds(num_sec);

		schedule([=]() { measureAll(end_of_benchmark); });

		continuation([=]() {

			draw();		// Start drawing

			double batch_median = 0;
			if (g_runtime_vec.size() > 0) {
				std::sort(g_runtime_vec.begin(), g_runtime_vec.end()); // To find median
				size_t size = g_runtime_vec.size();
				batch_median = size % 2 == 0 ? (g_runtime_vec.at(size / 2 - 1) + g_runtime_vec.at(size / 2)) / 2 : g_runtime_vec.at(size / 2);
			}

			// Sum of batch execution times
			double total_work_execution_time = 0;
			for (const auto& time : g_runtime_vec) {
				total_work_execution_time += time;
			}

			// Calculate values for a single job
			double work_mean = total_work_execution_time / g_call_count;
			double work_median = batch_median;
			double measureAll_median = g_total_timed_runtime.count() / g_call_count;

			std::cout << std::endl
				<< "    Test: mandelbrotFunc (Fixed Time)" << std::endl;
			std::cout << "        Number of calls:                  " << g_call_count << std::endl;
			std::cout << "        Mean execution time:              " << work_mean / 1000.0 << " ms" << std::endl;
			std::cout << "        Median execution time:            " << work_median / 1000.0 << " ms" << std::endl;
			//std::cout << "        Mean execution time (measureAll):   " << measureAll_median << " us" << std::endl;

			// Output files
			std::string raw_file("PraktikumVGJS/results/rawMandelbrotFunc" + std::to_string(num_threads) + ".txt");  // Raw data
			std::string summary_file("PraktikumVGJS/results/mandelbrotFunc" + std::to_string(num_threads) + ".txt"); // Summaries

			std::ofstream outdata;

			// Write execution times of batches to file so they can be processed
			outdata.open(raw_file);
			if (outdata)
			{
				for (auto& batch_execution_time : g_runtime_vec)
				{
					outdata << batch_execution_time << std::endl;
				}
			}
			outdata.close();

			// Append summary of benchmark to file
			outdata.open(summary_file, std::ios_base::app);
			if (outdata)
			{
				outdata << std::endl
					<< "Test: mandelbrotFunc (Fixed Time)" << std::endl;
				outdata << "    Threads used in VJGS:             " << num_threads << std::endl;
				outdata << "    Number of calls:                  " << g_call_count << std::endl;
				outdata << "    Mean execution time:              " << work_mean << " us" << std::endl;
				outdata << "    Median execution time:            " << work_median << " us" << std::endl;
				outdata << "    Mean execution time (measureAll): " << measureAll_median << " us" << std::endl;
				outdata << "    Median batch execution time       " << batch_median / 1000.0 << "ms" << std::endl;
				outdata << std::endl
				   << "    Platform:                         " << g_platform << std::endl;
				outdata << "    CPU hardware threads:             " << g_cpu_hardware_threads << std::endl;
			}
			outdata.close();
		});
	}
}