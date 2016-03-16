#include <stdio.h>
#include <stdlib.h>
#include "libVision.h"

void testCallBack(void (f_ptr) (void));

void test() {
	printf("Test callback is ok \n");
} 

void testCase1();

int main(int argc, char const* argv[]) 
{	
	testCase1();
	return 0;
}

void testCallBack(void (f_ptr) (void)) {
	f_ptr();
}

void testCase1() {
	printf("\n TEST CASE 1 \n");
	// C interface with LibVision
	CLibVision_ptr clv = CLibVision_init();
	
	char* operations[11] = {
		"loadImageFromMem", 
		"preprocessingADPT", 
		"detectSquares",
		"saveCandidates", 
		"holdOnlyRightColored", 
		"detectCircles", 
		"holdOnlyRightColored", 
		"detectPenta", 
		"saveCandidates",
		"detectExa",
		"printPolygonsFounds"};	
		
	// The LibVision parameters
	LibVisionParams* clvParams_ptr = CLibVision_params(clv);
	clvParams_ptr->imagePath = "testImage.png";
	
	// Use the LibVision Run 1 
	CLibVision_requireOperations(clv, operations, (sizeof(operations) / sizeof(operations[0])) );
	
	// Use the LibVision Run 2
	char* operations2[4] = {"clearCandidates", "detectSquares", "saveCandidates", "printPolygonsFounds"};	
	CLibVision_requireOperations(clv, operations2, (sizeof(operations2) / sizeof(operations2[0])) );
}
