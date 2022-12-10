#include <stdio.h>
#include "safety_feature.hpp"



int main(int argc, char* argv[])
{
	State_t state;
	state = Timer_State();
	printf("%d\n",state);

	return 0;
}