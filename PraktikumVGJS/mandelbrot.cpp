#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <complex>
#include <fstream>
#include <functional>

#include "tests.h"

using namespace std::chrono;

namespace mandelbrot {

	const int num_blocks = 50000;
	const int block_size = 1 << 10;

	auto				g_global_mem = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

	auto				g_global_mem_f = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());
	thread_local auto	g_local_mem_f = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

	auto				g_global_mem_c = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());
	thread_local auto	g_local_mem_c = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

	thread_local auto	g_local_mem_m = n_pmr::monotonic_buffer_resource(1 << 20, n_pmr::new_delete_resource());
	
	// Dimensions
	static constexpr uint32_t WIDTH = 1000;					
	static constexpr uint32_t HEIGHT = WIDTH;

	static constexpr uint32_t MAX_ITERATIONS = 50;			// Maximum number of iterations per pixel

	uint16_t pixels[WIDTH * HEIGHT];				// Store pixel values while working

	// Create ppm file and write values into it
	void draw() {
		std::cout << "Drawing..." << std::endl;
		std::ofstream mandelbrotImage("mandelbrot.ppm");
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
	void calculate_pixel_function(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		//auto start = high_resolution_clock::now();
		std::complex<double> point( x / (width*0.5) - 1.5, y / (height*0.5) - 1.0);			// Project onto image - the complex number to add in the iteration
		
		std::complex<double> z(0, 0);								// Mandelbrot starting value
		uint32_t iterations = 0;											// Current number of iterations on this point
		while (std::abs(z) < 2 && iterations <= MAX_ITERATIONS) {	// Abort after certain iterations or when point is unstable
			z = z * z + point;
			iterations++;
		}
		if (iterations < MAX_ITERATIONS) {
			pixels[y * width + x] = (255*iterations)/(MAX_ITERATIONS-1);							// Unstable points - add color gradient
		}
		else {
			pixels[y * width + x] = 0;																// Stable points
		}
		//auto duration = duration_cast<nanoseconds>(high_resolution_clock::now() - start);
		//std::cout << duration.count() << "\n";
	}

	Coro<>calculate_pixel_coro(std::allocator_arg_t, n_pmr::memory_resource* mr, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		calculate_pixel_function(x, y, width, height);
		co_return;
	}

	void mandelbrot(uint32_t width) {
		for (uint32_t i = 0; i < width; i++) {
			for (uint32_t j = 0; j < width; j++) {
				calculate_pixel_function(j, i, width, width);
			}
		}
	}

	template<bool WITHALLOCATE = false, typename FT1 = Function, typename FT2 = std::function<void(void)>>
	Coro<std::tuple<double, double>> performance_function(bool print = true, bool wrtfunc = true, unsigned int num = 1000, unsigned int width = 1000, std::pmr::memory_resource* mr = std::pmr::new_delete_resource()) {
		auto& js = JobSystem::instance();

		double duration0_sum = 0;
		double duration2_sum = 0;

		// calculate mandelbrot multiple times per size
		for (uint32_t i = 0; i < (num / (width * width)); i++) {
			// clear pixels
			for (uint32_t i = 0; i < width * width; i++) {
				pixels[i] = 0;
			}

			// no JS
			auto start0 = high_resolution_clock::now();
			mandelbrot(width);
			auto duration0 = duration_cast<microseconds>(high_resolution_clock::now() - start0);
			//std::cout << "duration0: " << duration0.count() <<"\n";
			duration0_sum += duration0.count();

			// allocation
			std::pmr::vector<FT2> perfv2{ mr };
			if constexpr (!WITHALLOCATE) {
				if constexpr (std::is_same_v<FT1, Function>) {
					for (uint32_t i = 0; i < width; i++) {
						for (uint32_t j = 0; j < width; j++) {
							perfv2.emplace_back([=]() {calculate_pixel_function(j, i, width, width); });
						}
					}
				}
				else {
					perfv2.reserve((size_t)width * width);
					for (uint32_t i = 0; i < width; i++) {
						for (uint32_t j = 0; j < width; j++) {
							perfv2.emplace_back(calculate_pixel_coro(std::allocator_arg, n_pmr::new_delete_resource(), j, i, width, width));
						}
					}
				}
			}

			for (uint32_t i = 0; i < width * width; i++) {
				pixels[i] = 0;
			}

			// multithreaded
			auto start2 = high_resolution_clock::now();
			// time allocation as well
			if constexpr (WITHALLOCATE) {
				if constexpr (std::is_same_v<FT1, Function>) {
					for (uint32_t i = 0; i < width; i++) {
						for (uint32_t j = 0; j < width; j++) {
							perfv2.emplace_back([=]() {calculate_pixel_function(j, i, width, width); });
						}
					}
				}
				else {
					perfv2.reserve((size_t)width * width);
					for (uint32_t i = 0; i < width; i++) {
						for (uint32_t j = 0; j < width; j++) {
							perfv2.emplace_back(calculate_pixel_coro(std::allocator_arg, mr, j, i, width, width));
						}
					}
				}
			}
			co_await perfv2;
			auto duration2 = duration_cast<microseconds>(high_resolution_clock::now() - start2);
			//std::cout << "duration2: " << duration2.count() << "\n";
			duration2_sum += duration2.count();
		}

		

		// calculate + output
		double speedup0 = duration0_sum / duration2_sum;
		double efficiency0 = speedup0 / js.get_thread_count().value;
		if (wrtfunc) {
			if (print) {
				std::cout << "Wrt function calls: Mandelbrot " << std::right << std::setw(4) << width << " Speedup " << std::left << std::setw(8) << speedup0 << " Efficiency " << std::setw(8) << efficiency0 << std::endl;
			}
			co_return std::make_tuple(speedup0, efficiency0);
		}
	}

	template<bool WITHALLOCATE = false, typename FT1, typename FT2>
	Coro<> performance_driver(std::string text, std::pmr::memory_resource* mr = std::pmr::new_delete_resource(), int runtime = 1000000) {
		int num = runtime;
		const int st = 10;
		const int mt = WIDTH;
		const int dt1 = 1;
		const int dt2 = 10;
		const int dt3 = 100;
		const int dt4 = 10;
		int mdt = dt1;
		bool wrt_function = true; //speedup wrt to sequential function calls w/o JS

		auto& js = JobSystem::instance();

		std::cout << "\nPerformance for " << text << " on " << js.get_thread_count().value << " threads\n\n";
		co_await performance_function<WITHALLOCATE, Function, std::function<void(void)>>(false, wrt_function, num, WIDTH); //heat up, allocate enough jobs
		for (int width = st; width <= mt; width += mdt) {
			auto [speedup, eff] = co_await performance_function<WITHALLOCATE, FT1, FT2>(true, wrt_function, num, width);
			if (width >= 10) mdt = dt2;
			if (width >= 100) mdt = dt3;
		}
		co_return;
	}

	Coro<> test() {

		//Job size too small (< 1us) => Speedup < 1


		co_await performance_driver<false, Function, std::function<void(void)>>("std::function calls (w / o allocate)");
		co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate new/delete)", std::pmr::new_delete_resource());
		co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate synchronized)", &g_global_mem_f);
		co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate unsynchronized)", &g_local_mem_f);
		co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate monotonic)", &g_local_mem_m);
		g_local_mem_m.release();

		co_await performance_driver<false, Coro<>, Coro<>>("Coro<> calls (w / o allocate)");
		co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate new/delete)", std::pmr::new_delete_resource());
		co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate synchronized)", &g_global_mem_c);
		co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate unsynchronized)", &g_local_mem_c);
		co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate monotonic)", &g_local_mem_m);
		
		
		//draw();		// Start drawing
		co_return;
	}

/*
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
*/
}