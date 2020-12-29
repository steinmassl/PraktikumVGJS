#include "mandelbrotFunc.h"
#include <complex>

namespace mandelbrotFunc {
	
	// Dimensions
	constexpr int WIDTH = 1000;					
	constexpr int HEIGHT = 1000;

	const int MAX_ITERATIONS = 50;			// Maximum number of iterations per pixel

	short pixels[WIDTH * HEIGHT];				// Array for pixel values while working

	// Create ppm file and draw values into it
	void draw() {
		std::cout << "Drawing..." << std::endl;
		std::ofstream mandelbrotImage("mandelbrotFunc.ppm");
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
	}

	// Calculate the value of each pixel up to a certain number of iterations using coordinates as complex number
	void calculatePixel(int x, int y) {
		std::complex<double> point( x / (WIDTH*0.5) - 1.5, y / (HEIGHT*0.5) - 1.0);			// Project onto image - the complex number to add in the iteration
		
		std::complex<double> z(0, 0);								// Mandelbrot starting value
		int iterations = 0;											// Current number of iterations on this point
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

	void mandelbrotRecursive(int y) {
		g_vector.clear();													// Clear vector to reuse for this round

		for (int i = 0; i < WIDTH; i++) {
			g_vector.emplace_back([=]() {calculatePixel(i, y); });			// Parallelise per pixel - what about iterations per pixel?
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
}