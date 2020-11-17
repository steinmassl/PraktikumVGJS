#include <iostream>
#include <fstream>
#include <complex>
#include "VEGameJobSystem.h"
#include "VECoro.h"

using namespace vgjs;

namespace mandelbrotCoro {

	const int WIDTH = 2000;
	const int HEIGHT = 2000;

	const int MAX_ITERATIONS = 50;				//maximum number of iterations per pixel

	short pixels[WIDTH * HEIGHT];				//array for pixel values while working

	auto g_global_mem = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = 1000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());

	//create ppm file and draw values into it
	Coro<> draw() {
		std::ofstream mandelbrotImage("mandelbrotCoro.ppm");
		if (mandelbrotImage.is_open()) {
			mandelbrotImage << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";		//PPM Header data
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					short value = pixels[i * WIDTH + j];
					mandelbrotImage << value << " " << value << " " << value << "\n";		//write values from pixel storage into file
				}
			}
			mandelbrotImage.close();
			std::cout << "End Mandelbrot" << std::endl;
		}
		else
			std::cout << "File error" << std::endl;
		co_return;
	}

	//calculate the value of each pixel up to a certain number of iterations using coordinates as complex number
	Coro<> calculatePixel(std::allocator_arg_t, n_pmr::memory_resource* mr, int x, int y) {
		std::complex<double> point(x / (WIDTH * 0.5) - 1.5, y / (HEIGHT * 0.5) - 1.0);			//Project onto image - the complex number to add in the iteration

		std::complex<double> z(0, 0);								//Mandelbrot starting value
		int iterations = 0;											//current number of iterations on this point
		while (std::abs(z) < 2 && iterations <= MAX_ITERATIONS) {	//abort after certain iterations or when point is unstable
			z = z * z + point;
			iterations++;
		}
		if (iterations < MAX_ITERATIONS) {
			pixels[y * WIDTH + x] = (255 * iterations) / (MAX_ITERATIONS - 1);							//unstable points - add color gradient
			//std::cout << "255" << std::endl;
		}
		else {
			pixels[y * WIDTH + x] = 0;																//stable points
			//std::cout << "0" << std::endl;
		}
		co_return;
	}

	JobQueue<Coro<>> jobQueue;
	

	Coro<> mandelbrot(std::allocator_arg_t, n_pmr::memory_resource* mr) {
		n_pmr::vector<Coro<>> vec{ mr };									//create std::pmr::vector<Coro<>> for children

		int x = 0, y = 0;
		auto calculatePixelFiber = calculatePixel(std::allocator_arg, mr, x,y);
		jobQueue.push(calculatePixelFiber);
		
		/*
		Coro<>* vecArray = vec.data();												//access underlying array
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				vecArray[i * WIDTH + j] = std::move(calculatePixel(j, i));						//parallelise per pixel - what about iterations per pixel?	
			}
		}
		
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				vec.emplace_back(calculatePixel(std::allocator_arg, mr, j, i));						//parallelise per pixel - what about iterations per pixel?	
			}
		}
		*/
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {

			}
		}



		co_await vec;					//start calculations
		co_await draw();				//start drawing
		co_return;
	}

	void test() {
		std::cout << "Start Mandelbrot" << std::endl;
		schedule(mandelbrot(std::allocator_arg, &g_global_mem));
	}
}