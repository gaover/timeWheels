#include <iostream>

using namespace std;

#include "TimeWheels.hpp"
using namespace tws;


int main() {

	Wheelement *pkData = new Wheelement(0, 0, 2030);
	TimeWheels skWheels;
	skWheels.AddNewTimer(pkData, false);
	skWheels.AddNewTimer(pkData, false, eMiniteLvl);


	return 0;

}