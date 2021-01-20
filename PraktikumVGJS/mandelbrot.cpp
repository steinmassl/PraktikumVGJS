#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <functional>
#include <string>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <complex>

#include "tests.h"


namespace mandelbrot {
	
	// Dimensions
	static constexpr uint32_t WIDTH = 1000;					
	static constexpr uint32_t HEIGHT = WIDTH;

	static constexpr uint32_t MAX_ITERATIONS = 50;			// Maximum number of iterations per pixel

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
	void calculate_pixel_function(uint32_t x, uint32_t y) {
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

	Coro<>calculate_pixel_coro(std::allocator_arg_t, n_pmr::memory_resource* mr, uint32_t x, uint32_t y) {
		calculate_pixel_function(x, y);
		co_return;
	}
	
	/*
	void paralellized_mandelbrot_function() {
		n_pmr::vector<std::function<void(void)>> vec;
		for (uint32_t i = 0; i < HEIGHT; i++) {
			for (uint32_t j = 0; j < WIDTH; j++) {
				vec.emplace_back([=]() {calculate_pixel_function(j, i); });
			}
		}
		schedule(vec);
		continuation([]() {});	// sync
	}

	Coro<> paralellized_mandelbrot_coro() {
		n_pmr::vector<Coro<>> vec;
		for (uint32_t i = 0; i < HEIGHT; i++) {
			for (uint32_t j = 0; j < WIDTH; j++) {
				vec.emplace_back(calculate_pixel_coro(std::allocator_arg, n_pmr::new_delete_resource(), j, i));
			}
		}
		co_await vec;
		co_return;
	}
	*/

	void mandelbrot() {
		for (uint32_t i = 0; i < HEIGHT; i++) {
			for (uint32_t j = 0; j < WIDTH; j++) {
				calculate_pixel_function(j, i);
			}
		}
	}

	template<bool WITHALLOCATE = false, typename FT1 = Function, typename FT2 = std::function<void(void)>>
	Coro<std::tuple<double, double>> performance_function(bool print = true, bool wrtfunc = true, unsigned int width = 1000, int micro = 1, std::pmr::memory_resource* mr = std::pmr::new_delete_resource()) {
		auto& js = JobSystem::instance();

		// no JS
		auto start0 = high_resolution_clock::now();
		mandelbrot();
		auto duration0 = duration_cast<microseconds>(high_resolution_clock::now() - start0);

		// allocation
		std::pmr::vector<FT2> perfv2{ mr };
		if constexpr (!WITHALLOCATE) {
			if constexpr (std::is_same_v<FT1, Function>) {
				for (uint32_t i = 0; i < width; i++) {
					for (uint32_t j = 0; j < width; j++) {
						perfv2.emplace_back([=]() {calculate_pixel_function(j, i); });
					}
				}
			}
			else {
				perfv2.reserve(size_t(width*width));
				for (uint32_t i = 0; i < width; i++) {
					for (uint32_t j = 0; j < width; j++) {
						perfv2.emplace_back(calculate_pixel_coro(std::allocator_arg, n_pmr::new_delete_resource(), j, i));
					}
				}
			}
		}

		// multithreaded
		auto start2 = high_resolution_clock::now();
		// time allocation as well
		if constexpr (WITHALLOCATE) {
			if constexpr (std::is_same_v<FT1, Function>) {
				for (uint32_t i = 0; i < width; i++) {
					for (uint32_t j = 0; j < width; j++) {
						perfv2.emplace_back([=]() {calculate_pixel_function(j, i); });
					}
				}
			}
			else {
				perfv2.reserve(size_t(width*width));
				for (uint32_t i = 0; i < width; i++) {
					for (uint32_t j = 0; j < width; j++) {
						perfv2.emplace_back(calculate_pixel_coro(std::allocator_arg, mr, j, i));
					}
				}
			}
		}
		co_await perfv2;
		auto duration2 = duration_cast<microseconds>(high_resolution_clock::now() - start2);

		for (uint32_t i = 0; i < width * width; i++) {
			pixels[i] = 0;
		}

		// calculate + output
		double speedup0 = (double)duration0.count() / (double)duration2.count();
		double efficiency0 = speedup0 / js.get_thread_count().value;
		if (wrtfunc) {
			if (print) {
				std::cout << "Wrt function calls: Mandelbrot " << std::right << std::setw(4) << width << " Speedup " << std::left << std::setw(8) << speedup0 << " Efficiency " << std::setw(8) << efficiency0 << std::endl;
			}
			co_return std::make_tuple(speedup0, efficiency0);
		}
	}

	template<bool WITHALLOCATE = false, typename FT1, typename FT2>
	Coro<> performance_driver(std::string text, std::pmr::memory_resource* mr = std::pmr::new_delete_resource(), int runtime = 400000) {
		int num = runtime;
		const int st = 0;
		const int mt = 100;
		const int dt1 = 1;
		const int dt2 = 1;
		const int dt3 = 1;
		const int dt4 = 10;
		int mdt = dt1;
		bool wrt_function = true; //speedup wrt to sequential function calls w/o JS

		auto& js = JobSystem::instance();

		std::cout << "\nPerformance for " << text << " on " << js.get_thread_count().value << " threads\n\n";
		co_await performance_function<WITHALLOCATE, Function, std::function<void(void)>>(false, wrt_function, WIDTH, 0); //heat up, allocate enough jobs
		auto [speedup, eff] = co_await performance_function<WITHALLOCATE, FT1, FT2>(true, wrt_function, WIDTH);
		/*
		for (int us = st; us <= mt; us += mdt) {
			int loops = (us == 0 ? num : (runtime / us));
			auto [speedup, eff] = co_await performance_function<WITHALLOCATE, FT1, FT2>(true, wrt_function, loops, us, mr);
			if (eff > 0.95) co_return;
			if (us >= 15) mdt = dt2;
			if (us >= 20) mdt = dt3;
			if (us >= 50) mdt = dt4;
		}
		*/
		co_return;
	}

	Coro<> test() {
		co_await performance_driver<false, Function, std::function<void(void)>>("std::function calls (w / o allocate)");
		co_await performance_driver<false, Coro<>, Coro<>>("Coro<> calls (w / o allocate)");
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