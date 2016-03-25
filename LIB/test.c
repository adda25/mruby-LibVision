#include <stdio.h>
#include <stdlib.h>
#include "libVision.h"

void testCallBack(void(f_ptr)(void));
void test() { printf("Test callback is ok \n"); }

void testCasePolygonsRecognition();
void testCasePolygonsRecognitionCycle();
void testCasePatternImageRecognition();

int main(int argc, char const *argv[]) {
  // testCasePolygonsRecognition();
  testCasePolygonsRecognitionCycle();
  // testCasePatternImageRecognition();
  return 0;
}

void testCallBack(void(f_ptr)(void)) { f_ptr(); }

void testCasePolygonsRecognition() {
  printf("\n### TEST CASE POLYGONS RECOGNITION ###\n");
  // C interface off LibVision
  CLibVision_ptr clv = CLibVision_init();

  char *operations[11] = {"loadImageFromMem",     "preprocessingADPT",
                          "detectSquares",        "saveCandidates",
                          "holdOnlyRightColored", "detectCircles",
                          "holdOnlyRightColored", "detectPenta",
                          "saveCandidates",       "detectExa",
                          "printPolygonsFounds"};

  // The LibVision parameters
  LibVisionParams *clvParams_ptr = CLibVision_params(clv);
  clvParams_ptr->imagePath = "testImage.png";

  // Use the LibVision Run 1
  CLibVision_requireOperations(clv, operations,
                               (sizeof(operations) / sizeof(operations[0])));

  // Use the LibVision Run 2
  char *operations2[4] = {"clearCandidates", "detectSquares", "saveCandidates",
                          "printPolygonsFounds"};
  CLibVision_requireOperations(clv, operations2,
                               (sizeof(operations2) / sizeof(operations2[0])));
  CLibVision_deinit(clv);
}

void testCasePolygonsRecognitionCycle() {
  printf("\n### TEST CASE POLYGONS RECOGNITION CYCLE###\n");
  // C interface off LibVision
  CLibVision_ptr clv = CLibVision_init();

  // The LibVision parameters
  LibVisionParams *clvParams_ptr = CLibVision_params(clv);
  clvParams_ptr->imagePath = "testImage.png";
  clvParams_ptr->savedImagePath = "out.png";

  // Use the LibVision Run 1
  int i = 0;
  for (i = 0; i < 150; i++) {
    char *operation[1] = {"loadImageFromMem"};
    printf("%d\n", i);
    clvParams_ptr->imagePath = "testImage.png";
    clvParams_ptr->savedImagePath = "out.png";
    CLibVision_requireOperations(clv, operation,
                                 (sizeof(operation) / sizeof(operation[0])));
    char *operation1[1] = {"loadImageFromMem"};

    CLibVision_requireOperations(clv, operation1,
                                 (sizeof(operation1) / sizeof(operation1[0])));
    char *operation2[1] = {"preprocessingADPT"};

    CLibVision_requireOperations(clv, operation2,
                                 (sizeof(operation2) / sizeof(operation2[0])));
    char *operation3[1] = {"detectSquares"};

    CLibVision_requireOperations(clv, operation3,
                                 (sizeof(operation3) / sizeof(operation3[0])));
    char *operation4[1] = {"drawCandidates"};

    CLibVision_requireOperations(clv, operation4,
                                 (sizeof(operation4) / sizeof(operation4[0])));
    char *operation5[1] = {"saveFrame"};

    CLibVision_requireOperations(clv, operation5,
                                 (sizeof(operation5) / sizeof(operation5[0])));
    char *operation6[1] = {"preprocessingADPT"};

    CLibVision_requireOperations(clv, operation6,
                                 (sizeof(operation6) / sizeof(operation6[0])));

    char *operation7[1] = {"detectCircles"};

    CLibVision_requireOperations(clv, operation7,
                                 (sizeof(operation7) / sizeof(operation7[0])));

    char *operation8[1] = {"drawCandidates"};

    CLibVision_requireOperations(clv, operation8,
                                 (sizeof(operation8) / sizeof(operation8[0])));
  }
  // Use the LibVision Run 2
  char *operations2[4] = {"detectSquares", "saveCandidates",
                          "printPolygonsFounds"};
  CLibVision_requireOperations(clv, operations2,
                               (sizeof(operations2) / sizeof(operations2[0])));

  CLibVision_deinit(clv);
}

void testCasePatternImageRecognition() {
  printf("\n### TEST CASE PATTERN IMAGE RECOGNITION ###\n");
  // C interface off LibVision
  CLibVision_ptr clv = CLibVision_init();

  char *operations[4] = {"loadImageFromMem", "preprocessingADPT",
                         "detectSquares", "holdSquarePatterns"};

  // The LibVision parameters
  LibVisionParams *clvParams_ptr = CLibVision_params(clv);
  clvParams_ptr->imagePath = "testPatternImage.png";
  clvParams_ptr->patternImagePath = "patternImage.png";

  // Use the LibVision Run 1
  CLibVision_requireOperations(clv, operations,
                               (sizeof(operations) / sizeof(operations[0])));
  CLibVision_deinit(clv);
}
