#include <iostream>
#include <fstream>
#include <complex>
#include "VEGameJobSystem.h"

using namespace vgjs;

namespace mandelbrotFunc {
	
	constexpr int WIDTH = 5000;					
	constexpr int HEIGHT = 5000;

	const int MAX_ITERATIONS = 50;			//maximum number of iterations per pixel

	short pixels[WIDTH * HEIGHT];				//array for pixel values while working


	//create ppm file and draw values into it
	void draw() {
		std::cout << "Drawing..." << std::endl;
		std::ofstream mandelbrotImage("mandelbrotFunc.ppm");
		if (mandelbrotImage.is_open()) {
			mandelbrotImage << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";		//PPM Header data
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					short value = pixels[i * WIDTH + j];
					mandelbrotImage << value << " " << value << " " << value << "\n";		//write values from pixel storage into file
				}
			}
			mandelbrotImage.close();
			std::cout << "End MandelbrotFunc" << std::endl;
		}
		else
			std::cout << "File error" << std::endl;
	}

	//calculate the value of each pixel up to a certain number of iterations using coordinates as complex number
	void calculatePixel(int x, int y) {
		std::complex<double> point( x / (WIDTH*0.5) - 1.5, y / (HEIGHT*0.5) - 1.0);			//Project onto image - the complex number to add in the iteration
		
		std::complex<double> z(0, 0);								//Mandelbrot starting value
		int iterations = 0;											//current number of iterations on this point
		while (std::abs(z) < 2 && iterations <= MAX_ITERATIONS) {	//abort after certain iterations or when point is unstable
			z = z * z + point;
			iterations++;
		}
		if (iterations < MAX_ITERATIONS) {
			pixels[y * WIDTH + x] = (255*iterations)/(MAX_ITERATIONS-1);							//unstable points - add color gradient
		}
		else {
			pixels[y * WIDTH + x] = 0;																//stable points
		}
	}
	
	void mandelbrot() {
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				schedule([=]() {calculatePixel(j, i); });			//parallelise per pixel - what about iterations per pixel?
			}
		}
		//continuation([]() {draw(); });				//start drawing
	}

	void test() {
		std::cout << "Starting MandelbrotFunc" << std::endl;
		schedule([]() {mandelbrot(); });
	}
}