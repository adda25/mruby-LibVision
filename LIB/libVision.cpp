#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include <raspicam/raspicam_cv.h>
#include <raspicam/raspicam.h>
#include "libVision.h"
#include <stdio.h>

#ifdef __cplusplus
/*
	  		 ____              ____             _     ____  _     _      
 		    / ___| _     _    |  _ \  __ _ _ __| | __/ ___|(_) __| | ___ 
		   | |   _| |_ _| |_  | | | |/ _` | '__| |/ /\___ \| |/ _` |/ _ \
		   | |__|_   _|_   _| | |_| | (_| | |  |   <  ___) | | (_| |  __/
 		    \____||_|   |_|   |____/ \__,_|_|  |_|\_\|____/|_|\__,_|\___|
                                                              
*/

LibVision::LibVision() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("init");
	#endif
	
	// Populate lookup table
	this->lbFunctions.insert(std::make_pair("openCamera", 			&LibVision::openCamera));
	this->lbFunctions.insert(std::make_pair("closeCamera", 			&LibVision::closeCamera));
	this->lbFunctions.insert(std::make_pair("acquireFrame", 		&LibVision::acquireFrame));
	this->lbFunctions.insert(std::make_pair("saveFrame", 			&LibVision::saveFrame));
	this->lbFunctions.insert(std::make_pair("loadImageFromMem", 	&LibVision::loadImageFromMemory));
	this->lbFunctions.insert(std::make_pair("preprocessingOTSU", 	&LibVision::preprocessingFrameOTSU));
	this->lbFunctions.insert(std::make_pair("preprocessingADPT", 	&LibVision::preprocessingFrameADPT));
	this->lbFunctions.insert(std::make_pair("detectSquares", 		&LibVision::detectSquares));
	this->lbFunctions.insert(std::make_pair("detectCircles", 		&LibVision::detectCircles));
	this->lbFunctions.insert(std::make_pair("detectPenta", 			&LibVision::detectPenta));
	this->lbFunctions.insert(std::make_pair("detectExa", 			&LibVision::detectExa));
	this->lbFunctions.insert(std::make_pair("holdSquarePatterns",	&LibVision::checkSquarePatterns));
	this->lbFunctions.insert(std::make_pair("holdOnlyRightColored", &LibVision::checkColor));
	this->lbFunctions.insert(std::make_pair("saveCandidates",	 	&LibVision::saveCandidates));
	this->lbFunctions.insert(std::make_pair("clearCandidates",	 	&LibVision::clearCandidates));
	this->lbFunctions.insert(std::make_pair("drawCandidates",	 	&LibVision::drawAllCandidates));
	// Lookup table debug functions
	this->lbFunctions.insert(std::make_pair("printPolygonsFounds",	&LibVision::debugPrintPolys));
	this->lbFunctions.insert(std::make_pair("printMethods",			&LibVision::debugPrintMethods));
	
	// Print available functions
	#if SHOW_INFO
	this->debugPrintMethods();
	#endif
	
	// Alloc lbParams
	//this->lbParams = newLbParams();
	initParams();
}

LibVision::~LibVision() {
	//this->clearCandidates();
	//free (this->lbParams->colorRange);
	//free (this->lbParams);
	#if DEBUG_MODE
	std::cout << "LibVision::deinit" << std::endl;
	#endif
}

void LibVision::initParams() {
    this->lbParams = (LibVisionParams*)malloc(sizeof(LibVisionParams));
	if (this->lbParams == NULL) {
		#if DEBUG_MODE
		std::cout << "LibVision-> alloc lbParams fails at: " << __LINE__  << std::endl;
		#endif
	}
	this->lbParams->imagePath = (char*)malloc(256 * sizeof(char));
	// Saved frame path
	this->lbParams->savedImagePath = (char*)malloc(256 * sizeof(char));
    // Alloc LibVisionParams colorRange
    /*this->lbParams->colorRange = (int*)malloc(0 * sizeof (int));
    if (this->lbParams->colorRange == NULL) {
        free (this->lbParams->colorRange);
    }
	static const int defaultColorRange[6] = {0, 0, 0, 10, 10, 10}; // Black
	memcpy(this->lbParams->colorRange, defaultColorRange, sizeof(defaultColorRange));
	
	// Alloc LibVisionParams polygons
	this->lbParams->polygonsFounds = 0;
	this->lbParams->polygons = (Polygon*)malloc(0 * sizeof (Polygon));
	if (this->lbParams->polygons == NULL) {
		free(this->lbParams->polygons);
	}
	this->lbParams->polygons[0].numberOfPoints = 0;
	this->lbParams->polygons[0].polyPoints = (ScreenPoint*)malloc(0 * sizeof (ScreenPoint));
		
	// Default params
	this->lbParams->otsuThresh 		= 130;
	this->lbParams->adptThreshSize 	= 7;
	this->lbParams->adptThreshMean 	= 7;

	// Set Raspberry camera
	this->lbParams->cameraIsAvailable = FALSE;*/
}

void LibVision::requireOperations(char* operations[], size_t size) {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("requireOperations");
	#endif
	for (uint i = 0; i < (int)size; i++) {
		if (operations[i] == NULL) {continue;}
		if (this->lbFunctions[std::string(operations[i])]) {
			LibVisionFunction lvfp = this->lbFunctions[std::string(operations[i])];
			(this->*lvfp)();
		}
	}	
}

/*
*	Operations
*/

void LibVision::openCamera() {
	this->camera = new raspicam::RaspiCam_Cv();
	#if DEBUG_MODE
	if (this->camera->open()) {
		this->lbParams->cameraIsAvailable = TRUE;
		#if DEBUG_MODE
		LIB_VISION_DPRINT("openCamera -> camera is available");
		#endif
	} else {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("openCamera -> camera is not available");
		#endif
	}
	#endif
}

void LibVision::closeCamera() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("closeCamera");
	#endif
	if (this->lbParams->cameraIsAvailable == TRUE) {
		this->camera->release();
	}
}

void LibVision::acquireFrame() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("acquireFrame");
	#endif
	if (this->lbParams->cameraIsAvailable == FALSE) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("acquireFrame --> camera is not available");
		#endif
		return;
	}
	this->camera->grab();
	this->camera->retrieve(this->lastFrame);
	this->opFrame = this->lastFrame;
}

void LibVision::saveFrame() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("saveFrame");
	#endif
	if (this->lbParams->savedImagePath == NULL) {
      #if DEBUG_MODE
      LIB_VISION_DPRINT("saveFrame: savedImagePath is NULL --> return");
      #endif
	  return;
	}
	std::cout << "Saved path: " << this->lbParams->savedImagePath << std::endl;
	cv::imwrite(this->lbParams->savedImagePath, this->opFrame);
}

void LibVision::loadImageFromMemory() {
	#if DEBUG_MODE
	std::cout << "LibVision::setFramePathname --> " << lbParams->imagePath << std::endl;
	#endif
	if (lbParams->imagePath == NULL) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("loadImageFromMemory --> imagepath is NULL");
		#endif
		return;
	}
    this->lastFrame = cv::imread(std::string(lbParams->imagePath), CV_LOAD_IMAGE_COLOR);
	if(this->lastFrame.cols == 0 || this->lastFrame.rows == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("loadImageFromMemory --> loaded image is empty");
		#endif
		return;
	}
	this->lastFrame.copyTo(this->opFrame);
}

void LibVision::preprocessingFrameOTSU() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("preprocessingFrameOTSU");
	#endif
	if(this->lastFrame.cols == 0 || this->lastFrame.rows == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("preprocessingFrameOTSU -> lastFrame is empty");
		#endif
		return;
	}
	if(lbParams->otsuThresh < 0 || lbParams->otsuThresh > 255) {
		lbParams->otsuThresh = DEFAULT_OTSU_THRESH;
		#if DEBUG_MODE
		LIB_VISION_DPRINT("preprocessingFrameOTSU -> otsu thresh set to default");
		#endif
	}
	cv::cvtColor(this->lastFrame, this->lastGrayFrame, CV_RGBA2GRAY);
    cv::threshold(lastGrayFrame,
                  lastThrFrame,
      			  lbParams->otsuThresh,
   				  255,
                  CV_THRESH_BINARY_INV && CV_THRESH_OTSU);
}

void LibVision::preprocessingFrameADPT() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("preprocessingFrameADPT");
	#endif
	if(this->lastFrame.cols == 0 || this->lastFrame.rows == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("preprocessingFrameADPT -> lastFrame is empty");
		#endif
		return;
	}
	if(lbParams->adptThreshSize < 3) {
		lbParams->adptThreshSize = DEFAULT_ADPT_SIZE;
		#if DEBUG_MODE	
		LIB_VISION_DPRINT("preprocessingFrameADPT -> adpt size set to default");
		#endif
	}
	cv::cvtColor(this->lastFrame, this->lastGrayFrame, CV_RGBA2GRAY);
    cv::adaptiveThreshold(lastGrayFrame,
                          lastThrFrame,
                          255,
                          CV_ADAPTIVE_THRESH_MEAN_C,
                          CV_THRESH_BINARY_INV,
                          lbParams->adptThreshSize, 
						  lbParams->adptThreshMean);
}

std::vector <std::vector<cv::Point> > LibVision::findContours(int minContourPointsAllowed) {
    static std::vector<std::vector<cv::Point> > allContours;
	std::vector <std::vector<cv::Point> > contours;
    allContours.reserve(2000);
	if (lastThrFrame.rows == 0 || lastThrFrame.cols == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("no threshold frame, call preprocessingFrame_* --> return empty");
		#endif
		return contours;
	}
    cv::Mat thrFrameCopy;
    lastThrFrame.copyTo(thrFrameCopy);
    cv::findContours(thrFrameCopy, allContours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    contours.clear();
    for (int i = 0; i < allContours.size(); i++) {
        if (allContours[i].size() > minContourPointsAllowed)
            contours.push_back(allContours[i]);
    }
    allContours.clear();
	return contours;
}

std::vector <std::vector<cv::Point> > LibVision::findCandidates(std::vector <std::vector<cv::Point> > contours, uint vertexCount) {
    static std::vector<cv::Point> approxCurve;
	std::vector <std::vector<cv::Point> > candidates;
    for (int i = 0; i < contours.size(); i++) {
        if(vertexCount == 1000) {
        	// Circles
	        cv::approxPolyDP(contours[i], approxCurve, 3, true);
			if (approxCurve.size() > 8) {
			candidates.push_back(approxCurve);
			}
        } else {
        	// Others
	        cv::approxPolyDP(contours[i], approxCurve, cv::arcLength(cv::Mat(contours[i]), true) * 0.02, true);
			if (approxCurve.size() == vertexCount && cv::isContourConvex(approxCurve)) {
				if (verifyDistance(approxCurve[0], approxCurve[2])) {
					candidates.push_back(approxCurve);
				}
			}
        }
        approxCurve.clear();
    }
	return candidates;
}

inline bool LibVision::verifyDistance(cv::Point p_o, cv::Point p_t) {
    const int min_dist = 30; // DEF
    float dx = p_o.x - p_t.x;
    float dy = p_o.y - p_t.y;
    float dist = sqrtf(powf(dx, 2) + powf(dy, 2));
    if (dist > min_dist) {return true;}
    return false;
}

inline void LibVision::sortVertices(std::vector<cv::Point>& approxCurve) {
    std::vector<cv::Point> approxCurveBuffer;
    cv::Point v1 = approxCurve[1] - approxCurve[0];
    cv::Point v2 = approxCurve[2] - approxCurve[0];
    double o = (v1.x * v2.y) - (v1.y * v2.x);
    if (o < 0.0) {
        std::swap(approxCurve[1], approxCurve[3]);
    }
}

void LibVision::detectSquares() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("detectSquares");
	#endif
	this->candidates = this->findCandidates(this->findContours(100), 4);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectCircles() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("detectCircles");
	#endif
	this->candidates = this->findCandidates(this->findContours(1), 1000);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectPenta() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("detectPentagons");
	#endif
	this->candidates = this->findCandidates(this->findContours(100), 5);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectExa() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("detectExagons");
	#endif
	this->candidates = this->findCandidates(this->findContours(100), 6);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::checkSquarePatterns() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("checkSquaresPatterns");
	#endif
	if (candidates.size() == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("no candidates, call detectSquares --> exit");
		#endif
		return;
	}
	if (lbParams->patternImagePath == NULL) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("patternImagePath is NULL --> exit");
		#endif
		return;
	}
    cv::Mat patternImage = cv::imread(std::string(lbParams->patternImagePath), CV_LOAD_IMAGE_COLOR);
	if (patternImage.rows == 0 || patternImage.cols == 0) {
		#if DEBUG_MODE
		LIB_VISION_DPRINT("impossible to load pattern image --> exit");	
		#endif
		return;
	}
	std::vector<std::vector<cv::Point> > newCandidates;
	for (int i = 0; i < this->candidates.size(); i++) {
		if (this->candidates[i].size() != 4) {continue;}
		cv::Mat warpThRoi = this->thresholdAfterWarp(this->getWarpPerspective(lastFrame, candidates[i]));
		int r = 0;
		int idM = calcCorrelationCoefficient(warpThRoi, patternImage, &r);
		if (idM == 1) {
			// Pattern found
			newCandidates.push_back(this->candidates[i]);
			#if DEBUG_WITH_IMAGES
			showImageForDebug(warpThRoi, 3000);
			#endif
			#if DEBUG_MODE
			std::cout << "LibVision::patternFound N --> " << i << std::endl;
			#endif
		}
	}
	this->candidates = newCandidates;
}

void LibVision::saveCandidates() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("saveCandidates");
	#endif
	// Calc new current lbParams Polygons size
	int newSize = lbParams->polygonsFounds + (int)this->candidates.size();
	// Alloc new size
	if (lbParams->polygonsFounds == 0) {
		lbParams->polygons = (Polygon*)malloc(newSize * sizeof (Polygon));
	} else {
		lbParams->polygons = (Polygon*)realloc(lbParams->polygons, newSize * sizeof (Polygon));
	}
	if (lbParams->polygons == NULL) {
		free(lbParams->polygons);
	}
	// Save new data
	for (int i = 0; i < (int)this->candidates.size(); i++) {
		int numberOfPointsOfNewPoly = (int)this->candidates[i].size();
		lbParams->polygons[i + lbParams->polygonsFounds].polyPoints = (ScreenPoint*)malloc(numberOfPointsOfNewPoly * sizeof (ScreenPoint));
		if (lbParams->polygons[i + lbParams->polygonsFounds].polyPoints == NULL) {
			free(lbParams->polygons[i + lbParams->polygonsFounds].polyPoints);
		}
		lbParams->polygons[i + lbParams->polygonsFounds].numberOfPoints = numberOfPointsOfNewPoly;
		for (int j = 0; j < numberOfPointsOfNewPoly; j++) {
			lbParams->polygons[i + lbParams->polygonsFounds].polyPoints[j].x = this->candidates[i][j].x;
			lbParams->polygons[i + lbParams->polygonsFounds].polyPoints[j].y = this->candidates[i][j].y;
		}
	}
	// Save new state
	lbParams->polygonsFounds = newSize;
}

void LibVision::clearCandidates() {
	#if DEBUG_MODE
	LIB_VISION_DPRINT("clearCandidates");
	#endif
	for (int i = 0; i < lbParams->polygonsFounds; i++) {
		free(lbParams->polygons[i].polyPoints);
	}
	if (lbParams->polygonsFounds == 0) {
		return;
	}
	free(lbParams->polygons);
	lbParams->polygonsFounds = 0;
}

void LibVision::drawCandidates(std::vector<std::vector<cv::Point> > candidates) {
	cv::Mat temp;
	if (this->lastFrame.rows == 0 || this->lastFrame.cols == 0) {
		return;
	}
    this->lastFrame.copyTo(temp);
	for(int i = 0; i < candidates.size(); i++) {
		for(int j = 0; j < candidates[i].size() - 1; j++) {
			cv::line(temp, candidates[i][j], candidates[i][j+1], cv::Scalar(0, 0, 255), 2);
		}
		cv::line(temp, candidates[i][candidates[i].size() - 1], candidates[i][0], cv::Scalar(0, 0, 255), 2);
	}
	temp.copyTo(opFrame);
	#if DEBUG_WITH_IMAGES
	showImageForDebug(temp);
	#endif
}

///// REMOVE
void LibVision::checkWithImage() {
	/*std::vector<std::vector<cv::Point> > newCandidates;
		
	for(int i = 0; i < this->candidates.size(); i++) {
		if (checkTextureForRegion(candidates[i], minCol, maxCol)) {
			newCandidates.push_back(candidates[i]);
		}
	}
	this->candidates = newCandidates;*/
}

///// REMOVE
bool LibVision::checkTextureForRegion(std::vector<cv::Point> candidate) {
	return false;
}

void LibVision::checkColor() {
	std::vector<std::vector<cv::Point> > newCandidates;
	cv::Scalar minCol = cv::Scalar(lbParams->colorRange[0], lbParams->colorRange[1], lbParams->colorRange[2]);
	cv::Scalar maxCol = cv::Scalar(lbParams->colorRange[3], lbParams->colorRange[4], lbParams->colorRange[5]);
	std::cout << "DEBUG COLOR RANGE: " << minCol << " " << maxCol << std::endl;
	for(size_t i = 0; i < this->candidates.size(); i++) {
		if (checkColorForRegion(candidates[i], minCol, maxCol)) {
			newCandidates.push_back(candidates[i]);
		}
	}
	this->candidates = newCandidates;
}

bool LibVision::checkColorForRegion(std::vector<cv::Point> candidate, cv::Scalar minColor, cv::Scalar maxColor) {
    unsigned int colors[3];
    unsigned int *c = colors;
    cv::Mat frameWithColor;
 	cv::Point pMaxR;
 	cv::Mat channels[3];
	cv::Mat subframe;	
	if (!computeSubRect(candidate, subframe)) {
		return false;
	}
	if(subframe.cols == 0 || subframe.rows == 0) {return false;}
	cv::inRange(subframe, minColor, maxColor, frameWithColor);
	cv::split(frameWithColor, channels);
	cv::minMaxLoc(channels[0], NULL, NULL, NULL, &pMaxR);	
	cv::Vec3b intensity = frameWithColor.at<cv::Vec3b>(pMaxR.y, pMaxR.x);
    *(c) 		= static_cast<int>(intensity.val[0]);
	*(c + 1) 	= static_cast<int>(intensity.val[1]);
	*(c + 2) 	= static_cast<int>(intensity.val[2]);
	if (*(c) == 255 &&*(c + 1) == 255 && *(c + 2) == 255 ) {
		#if DEBUG_WITH_IMAGES
		showImageForDebug(subframe, 500);
		#endif
		return true;
	} 
	return false;
}

bool LibVision::computeSubRect(std::vector<cv::Point> candidate, cv::Mat& roi) {
	const float _SIDE_PLUS      = 0;
	const float _SIDE_PLUS_2    = 0;
	const float _MIN_X          = _SIDE_PLUS;
	const float _MAX_X          = lastFrame.cols - _SIDE_PLUS;
	const float _MIN_Y          = _SIDE_PLUS;
	const float _MAX_Y          = lastFrame.rows - _SIDE_PLUS;	
	int xMin = 0, xMax = 0, yMin = 0, yMax = 0;
	float x_corners[candidate.size()];
	float y_corners[candidate.size()];
	for (size_t c = 0; c < candidate.size(); c++) {
		x_corners[c] = candidate[c].x;
		y_corners[c] = candidate[c].y;
	}
	std::qsort(y_corners, 4, sizeof(int), LibVision::compare);
	std::qsort(x_corners, 4, sizeof(int), LibVision::compare);
	xMin = x_corners[0] - _SIDE_PLUS_2;
	xMax = x_corners[3] + _SIDE_PLUS_2;
	yMin = y_corners[0] - _SIDE_PLUS_2;
	yMax = y_corners[3] + _SIDE_PLUS_2;
	if (xMin > _MIN_X  && xMax < _MAX_X  && yMin > _MIN_Y  && yMax < _MAX_Y){
		if (xMax < xMin || yMax < yMin) {
			std::cout << "Error in subrect" << std::endl;
			return false;
		}
		cv::Mat roiD = lastFrame(cv::Rect(xMin, yMin, xMax - xMin, yMax - yMin));
		roiD.copyTo(roi);
		return true;
	}			
	return false;
}

cv::Mat LibVision::getWarpPerspective(cv::Mat roi, std::vector<cv::Point> candidate) {
    cv::Point2f pointsRes[4], pointsIn[4];
    for (int j = 0; j < 4; j++) {
        pointsIn[j] = candidate[j];
    }
	int warpSize = 56;
    pointsRes[0] = cv::Point2f(0, 0) ;
    pointsRes[1] = cv::Point2f(warpSize - 1, 0);
    pointsRes[2] = cv::Point2f(warpSize - 1, warpSize - 1);
    pointsRes[3] = cv::Point2f(0, warpSize - 1);
    cv::Mat prsxTrnsf = cv::getPerspectiveTransform(pointsIn, pointsRes);
    cv::Mat warpRoi;
    cv::warpPerspective(roi, warpRoi, prsxTrnsf, cv::Size(warpSize, warpSize));
    return warpRoi;
}

cv::Mat LibVision::thresholdAfterWarp(cv::Mat roi) {
    cv::Mat patternCandidatesRoiThresh;
    cv::threshold(roi,
                  patternCandidatesRoiThresh,
                  DEFAULT_OTSU_THRESH,
                  255,
                  CV_THRESH_BINARY_INV && CV_THRESH_OTSU);
    cv::bitwise_not(patternCandidatesRoiThresh, patternCandidatesRoiThresh);
    return patternCandidatesRoiThresh;
}

int LibVision::calcCorrelationCoefficient(cv::Mat src, cv::Mat img, int* rot) {
    unsigned int j;
    int i;
    double tempsim;
	int warpSize = 56;
    double N = (double)(warpSize * warpSize / 4);
    double nom, den;
    float confThreshold = 0.9;
    cv::Scalar mean_ext, std_ext, mean_int, std_int;
    std::vector<cv::Mat> loadedPatterns;
    int msize = warpSize;
    
    cv::Mat src2(msize, msize, CV_8UC1);
    cv::Point2f center((msize-1)/2.0f,(msize-1)/2.0f);
    cv::Mat rot_mat(2,3,CV_32F);
    cv::resize(img, src2, cv::Size(msize, msize));
    cv::Mat subImg = src2(cv::Range(msize/4,3*msize/4), cv::Range(msize/4,3*msize/4));
    loadedPatterns.push_back(subImg);
    rot_mat = cv::getRotationMatrix2D(center, 90, 1.0);
    
    for (int i=1; i<4; i++){
        cv::Mat dst = cv::Mat(msize, msize, CV_8UC1);
        rot_mat = cv::getRotationMatrix2D(center, -i*90, 1.0);
        cv::warpAffine(src2, dst , rot_mat, cv::Size(msize,msize));
        cv::Mat subImg = dst(cv::Range(msize/4,3*msize/4), cv::Range(msize/4,3*msize/4));
        loadedPatterns.push_back(subImg);
    }
    
    cv::Mat normROI = cv::Mat(warpSize, warpSize, CV_8UC1); //normalized ROI
    //Masks for exterior(black) and interior area inside the pattern
    cv::Mat patMask = cv::Mat::ones(warpSize, warpSize, CV_8UC1);
    cv::Mat submat = patMask(cv::Range(warpSize / 4, 3 * warpSize / 4), cv::Range(warpSize / 4, 3 * warpSize/4));
    cv::Mat patMaskInt = cv::Mat::zeros(warpSize, warpSize, CV_8UC1);
    submat = patMaskInt(cv::Range(warpSize / 4, 3 * warpSize / 4), cv::Range(warpSize / 4, 3 * warpSize / 4));
    submat = cv::Scalar(1);
    cv::meanStdDev(src, mean_ext, std_ext, patMask);
    cv::meanStdDev(src, mean_int, std_int, patMaskInt);
    if ((mean_ext.val[0]>mean_int.val[0]))
        return -1;
    
    cv::Mat inter = src(cv::Range(warpSize / 4, 3 * warpSize / 4), cv::Range(warpSize / 4, 3 * warpSize / 4));
    double normSrcSq = pow(norm(inter), 2);
    int zero_mean_mode = 1;
    float maxCor = -1.0;
    //int index = 0;
    int ori = 0;
    for (j = 0; j < (loadedPatterns.size() / 4); j++) {
        for(i = 0; i < 4; i++){
            double const nnn = pow(norm(loadedPatterns.at(j * 4 + i)), 2);
            if (zero_mean_mode == 1) {
                double const mmm = mean(loadedPatterns.at(j * 4 + i)).val[0];
                nom = inter.dot(loadedPatterns.at(j * 4 + i)) - (N * mean_int.val[0] * mmm);
                den = sqrt((normSrcSq - (N * mean_int.val[0] * mean_int.val[0])) * (nnn - (N * mmm * mmm)));
                tempsim = nom/den;
            } else {
                tempsim = inter.dot(loadedPatterns.at(j*4+i))/(sqrt(normSrcSq*nnn));
            }
            if(tempsim > maxCor){
                maxCor = tempsim;
                ori = i;
            }
        }
    }
    *rot = ori;
    if (maxCor > confThreshold)
        return 1;
    else
        return 0;
}

void LibVision::debugPrintPolys() {
	printf("LibVision::polygonsFounds: %d \n", lbParams->polygonsFounds);
	for (int i = 0; i < lbParams->polygonsFounds; i++) {
		printf("-> Number of vertex: %d \n", lbParams->polygons[i].numberOfPoints);		
		for (int j = 0; j < lbParams->polygons[i].numberOfPoints; j++) {
			printf("--> Polygon index: %d vertexIndex: %d   \t x: %d \t y: %d \n", i, j, 
				lbParams->polygons[i].polyPoints[j].x, lbParams->polygons[i].polyPoints[j].y);
		}
		printf("\n");
	}
}

void LibVision::debugPrintMethods() {
	LIB_VISION_DPRINT("list of available functions");
	for(std::map<std::string, LibVisionFunction>::iterator it = lbFunctions.begin(); it != lbFunctions.end(); ++it) {
		std::cout << "--> " << it->first << "\n";
	}
}

void LibVision::testInterface() {
	std::cout << "Test succesful" << std::endl;
}

#endif

#pragma mark -
/*
              ____   ___       _             __
             / ___| |_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
            | |      | || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
            | |___   | || | | | ||  __/ |  |  _| (_| | (_|  __/
             \____| |___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
*/

#define LIB_VISION_CLASS(o) reinterpret_cast<LibVision *>(o)

CLibVision_ptr CLibVision_init() {
	return reinterpret_cast<void *>(new LibVision());
}

void CLibVision_deinit(CLibVision_ptr rl) {
	delete LIB_VISION_CLASS(rl);
}

void CLibVision_testInterface(CLibVision_ptr rl) {
	LIB_VISION_CLASS(rl)->testInterface(); 
}

void CLibVision_requireOperations(CLibVision_ptr rl, char* operations[], size_t size) {
	LIB_VISION_CLASS(rl)->requireOperations(operations, size);
}

LibVisionParams* CLibVision_params(CLibVision_ptr rl) {
	return LIB_VISION_CLASS(rl)->lbParams;
}



