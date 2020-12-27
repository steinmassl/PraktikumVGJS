#include "mandelbrotCoro.h"
#include <complex>

namespace mandelbrotCoro {

	// Dimensions
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 1000;

	const int MAX_ITERATIONS = 50;				// Maximum number of iterations per pixel

	short pixels[WIDTH * HEIGHT];				// Array for pixel values while working

	// Create ppm file and write pixel values into it
	Coro<> draw() {
		std::cout << "Drawing..." << std::endl;
		std::ofstream mandelbrotImage("mandelbrotCoro.ppm");
		if (mandelbrotImage.is_open()) {
			mandelbrotImage << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";		// PPM Header data
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					short value = pixels[i * WIDTH + j];
					mandelbrotImage << value << " " << value << " " << value << "\n";		// Write values from pixel storage into file
				}
			}
			mandelbrotImage.close();
		}
		else
			std::cout << "File error" << std::endl;
		co_return;
	}

	// Calculate the value of each pixel up to a certain number of iterations using coordinates as complex number
	Coro<> calculatePixel( int x, int y) {
		std::complex<double> point(x / (WIDTH * 0.5) - 1.5, y / (HEIGHT * 0.5) - 1.0);			// Project onto image - The complex number to add in the iteration

		std::complex<double> z(0, 0);								// Mandelbrot starting value
		int iterations = 0;											// Current number of iterations on this point
		while (std::abs(z) < 2 && iterations <= MAX_ITERATIONS) {	// Abort after certain iterations or when point is unstable
			z = z * z + point;
			iterations++;
		}
		if (iterations < MAX_ITERATIONS) {
			pixels[y * WIDTH + x] = (255 * iterations) / (MAX_ITERATIONS - 1);							// Unstable points - add color gradient
		}
		else {
			pixels[y * WIDTH + x] = 0;																	// Stable points
		}
		co_return;
	}


	n_pmr::vector<Coro<>> g_vector;							// Use same vector for every recursive call (save ram because coros don't finish until children finished)

	Coro<>mandelbrotRecursive( int y) {
		g_vector.clear();									// Clear vector to reuse for this round

		for (int i = 0; i < WIDTH; i++) {
			g_vector.emplace_back(calculatePixel( i, y));			// Parallelise per pixel - what about iterations per pixel?
		}
		co_await g_vector;
		if ( y < (HEIGHT - 1))
			co_await mandelbrotRecursive(y + 1);
	}

	Coro<>mandelbrotLoop() {
		n_pmr::vector<Coro<>> vec;											// Create std::pmr::vector<Coro<>> for multiple children
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				vec.emplace_back(calculatePixel(j, i));						// Parallelise per pixel - what about iterations per pixel?
			}
			co_await vec;													// Split workload into chunks to avoid huge amounts of ram being allocated(20mb vs 3gb at 2000x2000)
			vec.clear();
		}
	}

	//JobQueue<Coro<>> jobQueue;

	Coro<> test() {
		
		//std::cout << "Starting MandelbrotCoro" << std::endl;

		/*
		int x = 0, y = 0;
		auto calculatePixelFiber = calculatePixel(std::allocator_arg, mr, x,y);					// Calculate Pixels in fibers - how to use JobQueue for this?
		jobQueue.push(calculatePixelFiber);
		//todo
		*/

		auto start = std::chrono::high_resolution_clock::now();
		co_await mandelbrotRecursive(0);		//start calculations recursively
		//co_await mandelbrotLoop();			//start calculations 
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
		std::cout << "   Test: mandelbrotCoro" << std::endl << "   Execution time: " << elapsed_milliseconds.count() << " ms" << std::endl;
		//co_await draw();						//start drawing
		//std::cout << "End MandelbrotCoro" << std::endl;
		co_return;
	}
}