#include <stdio.h>
#include <iostream>
#include "MockDistance.hpp"
#include "MockWeight.hpp"

extern "C" void app_main(void)
{
	Fullness::MockDistance md{std::vector<int32_t>{1, 2}};
	md.getDistance();
	Weight::MockWeight mw{10};
	mw.getWeight();

}
