#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

#ifdef __cplusplus
#include <stdlib.h>

#define OTSU_THRESH 130

class LibVision 
{
public:
	LibVision();
	~LibVision();
	typedef void(LibVision::*LibVisionFunction)(void);
	
	// Real time camera
	// frame acquisition
	void openCamera();
	void closeCamera();
	void acquireFrame(int slp);
	void setFrameSize(int width, int height);
    bool saveFrame(std::string &name);
    bool available();
	
	// Processing offline 
	// images
	void setFramePathname(char* path);
	
	// Operations
	void requireOperations(char* operations[], size_t size);
	
    static int compare (const void * a, const void * b) {
        return ( *(int*)a - *(int*)b );
    }
	
	// Debug
	void testInterface();
	
private:
	std::map<std::string, LibVisionFunction> lbFunctions;
	
	cv::Mat lastFrame;
	cv::Mat lastGrayFrame;
	cv::Mat lastThrFrame;
	
	bool isCameraAvailable;
	
	std::vector<std::vector<cv::Point> > candidates;
	void preprocessingFrameOTSU(void);
	void preprocessingFrameADPT(void);
	std::vector<std::vector<cv::Point> > findContours(int minContourPointsAllowed);
	std::vector<std::vector<cv::Point> > findCandidates(std::vector <std::vector<cv::Point> > contours, uint vertexCount);
	inline bool verifyDistance(cv::Point p_o, cv::Point p_t);
	inline void sortVertices(std::vector<cv::Point>& approxCurve);
	
	void detectSquares(void);
	void detectCircles(void);
	void detectPenta(void);
	void detectExa(void);
	void drawCandidates(std::vector<std::vector<cv::Point> > candidates);
	bool checkColor(std::vector<cv::Point> candidate, cv::Scalar minColor = cv::Scalar(200, 200, 200), cv::Scalar maxColor = cv::Scalar(210, 210, 210));
	bool computeSubRect(std::vector<cv::Point> candidate, cv::Mat& roi);
	
	void showImageForDebug(cv::Mat image, int time = 3000) {
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

void CLibVision_setFramePath(CLibVision_ptr rl, char* path);
void CLibVision_requireOperations(CLibVision_ptr rl, char* operations[], size_t size);


#ifdef __cplusplus
} //extern "C" {
#endif

