/*
 * dump_vs.cpp
 *
 *  Created on: Aug 11, 2013
 *      Author: mrcaps
 */

#include <iostream>
#include <fstream>
#include <inttypes.h>

int main(int argc, const char* argv[]) {
	std::cout << argv[1] << std::endl;
	std::ifstream ifs(argv[1], std::ios_base::in | std::ios_base::binary);

	int32_t r;

	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(&r), 4);
		if (!ifs.eof()) {
			std::cout << r << " " << std::endl;
		}
	}

	ifs.close();
	std::cout << argv[1] << std::endl;
}
