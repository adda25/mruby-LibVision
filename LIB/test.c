#include <stdio.h>
#include "libVision.h"

void testCallBack(void (f_ptr) (void));

void test() {
	printf("Test callback is ok \n");
} 

int main(int argc, char const* argv[]) {
	char* operations[5] = {"preprocessingADPT", "detectSquares", "detectCircles", "detectPenta", "detectExa"};

	CLibVision_ptr clv = CLibVision_init();
	CLibVision_testInterface(clv);
	
	CLibVision_setFramePath(clv, "testImage.png");
	
	CLibVision_requireOperations(clv, operations, (sizeof(operations) / sizeof(operations[0])));
	
	//testCallBack(test);
	return 0;
}

void testCallBack(void (f_ptr) (void)) {
	f_ptr();
}