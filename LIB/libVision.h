#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif
#ifndef DEBUG_WITH_IMAGES
#define DEBUG_WITH_IMAGES 1
#endif

typedef struct {
	int x;
	int y;
} ScreenPoint;

typedef struct {
	int numberOfPoints;
	ScreenPoint* polyPoints;
} Polygon;

/// **LibVisionParams**
/// Here you can insert your
/// custom functions paramaters
typedef struct
{
	// Real time camera acqusition
	// frame size
	int cameraFrameSize[2];
	
	// Thresholds for preprocessing
	int otsuThresh; 	// [0 - 255] 
	int adptThreshSize;	// [3 - INT_MAX_SIZE]
	int adptThreshMean;	// [INT_MIN_SIZE - INT_MAX_SIZE];
	
	// Path of the static image
	// to analyze
	char* imagePath;
	
	// Path of the image used for check ROI texture
	char* patternImagePath;
	
	// Always _six_ elements.
	// This is the range used to 
	// check the color of a candidate.
	// [MIN_RED, MIN_GREEN, MIN_BLU, MAX_RED, MAX_GREEN, MAX_BLUE] 
	// [0-255]
	int* colorRange;
	
	// The polygons founds by the LibVision.
	// Every polygon is an array of ScreenPoint
	int polygonsFounds;
	Polygon* polygons;
	
} LibVisionParams;


/// Init and alloc a new 
/// LibVisionParams. Alloc here
/// your custom parameters if needed
LibVisionParams* newLbParams() {
    // Alloc LibVisionParams
    LibVisionParams *params =(LibVisionParams*)malloc(sizeof(LibVisionParams));
    if (params == NULL) {return NULL;}

    // Alloc LibVisionParams colorRange
    params->colorRange = (int*)malloc(0 * sizeof (int));
    if (params->colorRange == NULL) {
        free (params->colorRange);
        return NULL;
    }
	static const int defaultColorRange[6] = {0, 0, 0, 10, 10, 10}; // Black
	memcpy(params->colorRange, defaultColorRange, sizeof(defaultColorRange));
	
	// Alloc LibVisionParams polygons
	params->polygonsFounds = 0;
	params->polygons = (Polygon*)malloc(0 * sizeof (Polygon));
	if (params->polygons == NULL) {
		free(params->polygons);
		return NULL;
	}
	params->polygons[0].numberOfPoints = 0;
	params->polygons[0].polyPoints = (ScreenPoint*)malloc(0 * sizeof (ScreenPoint));
		
	// Default params
	params->otsuThresh 		= 130;
	params->adptThreshSize 	= 7;
	params->adptThreshMean 	= 7;
	
    return params;
}
	
#ifdef __cplusplus
#include <stdlib.h>
#define DEFAULT_IMAGE_SLEEP_TIME_MS 3000
#define DEFAULT_OTSU_THRESH 130
#define DEFAULT_ADPT_SIZE	7
#define DEFAULT_ADPT_MEAN	7
#define LIB_VISION_DPRINT(text) printf("LibVision::"text"\n")

/// **LibVision**
/// Class that execute computer vision operation on
/// camera acquired frame or static images from memory.
/// The class is available under a C interface,
/// in order to be usable from C programs.
/// Core functionality is the *lbFunctions* lookup table,
/// where you insert methods that should be available.
/// These methods are inserted in the table at the class init time,
/// in the class initializer method.
/// Also the class store a struct of parameters that are used by
/// functions presents in the lookup table, because these functions
/// cannot take input arguments and returns always void.
/// Typical use of the class is reported in the test.c example,
/// by the way you use it like this:
///
///	Initialize the class:
///	-> CLibVision_ptr clv = CLibVision_init() 
///
///	Get the reference to the class parameters and modify it:
/// -> LibVisionParams* clvParams_ptr = CLibVision_params(clv)
/// -> clvParams_ptr->imagePath = "testImage.png"
/// 
/// Construct an array of operations to do:
/// -> char* operations[8] = {"loadImageFromMem", "preprocessingADPT", "detectSquares"}
///
/// Execute these operations:
/// -> CLibVision_requireOperations(clv, operations, (sizeof(operations) / sizeof(operations[0])) )
///
class LibVision 
{
public:
	LibVision();
	~LibVision();
	typedef void(LibVision::*LibVisionFunction)(void);

	// Execute operations
	void requireOperations(char* operations[], size_t size);
	
	// LibVision parameters
	LibVisionParams* lbParams;
	
	// Function needed by the STD QuickSort method
    static int compare (const void * a, const void * b) {
        return ( *(int*)a - *(int*)b );
    }
	
	// Debug
	void testInterface();
	
private:
	std::map<std::string, LibVisionFunction> lbFunctions;
	
   	//	 _     _     _____                 _   _                 
   	//	| |   | |__ |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
   	//	| |   | '_ \| |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
   	//	| |___| |_) |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
   	//	|_____|_.__/|_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/  
	//                                                  
	void openCamera();
	void closeCamera();
	void acquireFrame();
    void saveFrame();
	void loadImageFromMemory();
	void preprocessingFrameOTSU();
	void preprocessingFrameADPT();
	void detectSquares();
	void detectCircles();
	void detectPenta();
	void detectExa();
	void checkSquarePatterns();
	void saveCandidates();
	void clearCandidates();
	void checkColor();
	void checkWithImage();
	// Debug lbFunctions
	void debugPrintPolys();
	//
	raspicam::RaspiCam_Cv* camera;
	//
	cv::Mat lastFrame;
	cv::Mat lastGrayFrame;
	cv::Mat lastThrFrame;
	
	// In candidates are stored the last candidate
	// found by the LibVision. A new function call
	// of type *detect_** overwrite this property.
	// Call the lbFunction *saveCandidates* in order
	// to save this candidate in the lbParams struct.
	std::vector<std::vector<cv::Point> > candidates;
	
	std::vector<std::vector<cv::Point> > findContours(int minContourPointsAllowed);
	std::vector<std::vector<cv::Point> > findCandidates(std::vector <std::vector<cv::Point> > contours, uint vertexCount);
	inline bool verifyDistance(cv::Point p_o, cv::Point p_t);
	inline void sortVertices(std::vector<cv::Point>& approxCurve);
	void drawCandidates(std::vector<std::vector<cv::Point> > candidates);
	bool checkTextureForRegion(std::vector<cv::Point> candidate);
	bool checkColorForRegion(std::vector<cv::Point> candidate, 
								cv::Scalar minColor = cv::Scalar(200, 200, 200),
								cv::Scalar maxColor = cv::Scalar(210, 210, 210));
	bool computeSubRect(std::vector<cv::Point> candidate, cv::Mat& roi);
	void setColorsToCheck(void);
	int	calcCorrelationCoefficient(cv::Mat src, cv::Mat img, int* rot);
	cv::Mat	getWarpPerspective(cv::Mat roi, std::vector<cv::Point> candidate);
	cv::Mat thresholdAfterWarp(cv::Mat roi);
	
	void showImageForDebug(cv::Mat image, int time = DEFAULT_IMAGE_SLEEP_TIME_MS) {
		cv::namedWindow("test", CV_WINDOW_NORMAL);
		cv::imshow("test", image);
		cv::waitKey(time);
	}
};
#endif




/*
              ____   ___       _             __
             / ___| |_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
            | |      | || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
            | |___   | || | | | ||  __/ |  |  _| (_| | (_|  __/
             \____| |___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
*/

typedef void* CLibVision_ptr;
#ifdef __cplusplus
extern "C" {
#endif

// Init and deinit
CLibVision_ptr CLibVision_init();
void CLibVision_deinit(CLibVision_ptr rl);

// Public methods
void CLibVision_testInterface(CLibVision_ptr rl);
void CLibVision_requireOperations(CLibVision_ptr rl, char* operations[], size_t size);
LibVisionParams* CLibVision_params(CLibVision_ptr rl);

#ifdef __cplusplus
} //extern "C" {
#endif

