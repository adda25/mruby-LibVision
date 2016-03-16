#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>
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
	this->lbFunctions.insert(std::make_pair("holdOnlyRightColored", &LibVision::checkColor));
	this->lbFunctions.insert(std::make_pair("saveCandidates",	 	&LibVision::saveCandidates));
	this->lbFunctions.insert(std::make_pair("clearCandidates",	 	&LibVision::clearCandidates));
	// Lookup table debug functions
	this->lbFunctions.insert(std::make_pair("printPolygonsFounds",	&LibVision::debugPrintPolys));
	
	// Alloc lbParams
	this->lbParams = newLbParams();
	if (this->lbParams == NULL) {
		#if DEBUG_MODE
		std::cout << "LibVision-> alloc lbParams fails at: " << __LINE__  << std::endl;
		#endif
	}
}

LibVision::~LibVision() {
	#if DEBUG_MODE
	std::cout << "LibVision::deinit" << std::endl;
	#endif
}

void LibVision::requireOperations(char* operations[], size_t size) {
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
	
}

void LibVision::closeCamera() {
	
}

void LibVision::acquireFrame() {
	
}

void LibVision::saveFrame() {
	
}

void LibVision::loadImageFromMemory() {
	#if DEBUG_MODE
	std::cout << "LibVision::setFramePathname --> " << lbParams->imagePath << std::endl;
	#endif
	if (lbParams->imagePath == NULL) {return;}
    this->lastFrame = cv::imread(std::string(lbParams->imagePath), CV_LOAD_IMAGE_COLOR);
}

void LibVision::preprocessingFrameOTSU() {
	if(this->lastFrame.cols == 0 || this->lastFrame.rows == 0) {return;}
	cv::cvtColor(this->lastFrame, this->lastGrayFrame, CV_RGBA2GRAY);
    cv::threshold(lastGrayFrame,
                  lastThrFrame,
      			  OTSU_THRESH,
   				  255,
                  CV_THRESH_BINARY_INV && CV_THRESH_OTSU);
}

void LibVision::preprocessingFrameADPT() {
	if(this->lastFrame.cols == 0 || this->lastFrame.rows == 0) {return;}
	cv::cvtColor(this->lastFrame, this->lastGrayFrame, CV_RGBA2GRAY);
    cv::adaptiveThreshold(lastGrayFrame,
                          lastThrFrame,
                          255,
                          CV_ADAPTIVE_THRESH_MEAN_C,
                          CV_THRESH_BINARY_INV,
                          7, 7);
}

std::vector <std::vector<cv::Point> > LibVision::findContours(int minContourPointsAllowed) {
    static std::vector<std::vector<cv::Point> > allContours;
	std::vector <std::vector<cv::Point> > contours;
    allContours.reserve(2000);
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
	std::cout << "detecting squares" << std::endl;
	this->candidates = this->findCandidates(this->findContours(100), 4);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectCircles() {
	std::cout << "detecting circles" << std::endl;
	this->candidates = this->findCandidates(this->findContours(1), 1000);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectPenta() {
	std::cout << "detecting penta" << std::endl;
	this->candidates = this->findCandidates(this->findContours(100), 5);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
}

void LibVision::detectExa() {
	std::cout << "detecting exa" << std::endl;
	this->candidates = this->findCandidates(this->findContours(100), 6);
	#if DEBUG_WITH_IMAGES
	this->drawCandidates(this->candidates);
	#endif
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
		std::cout << numberOfPointsOfNewPoly << std::endl;
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
	for (int i = 0; i < lbParams->polygonsFounds; i++) {
		free(lbParams->polygons[i].polyPoints);
	}
	free(lbParams->polygons);
	lbParams->polygonsFounds = 0;
}

void LibVision::drawCandidates(std::vector<std::vector<cv::Point> > candidates) {
    cv::Mat frameCopy;
    this->lastFrame.copyTo(frameCopy);
	for(int i = 0; i < candidates.size(); i++) {
		for(int j = 0; j < candidates[i].size() - 1; j++) {
			cv::line(frameCopy, candidates[i][j], candidates[i][j+1], cv::Scalar(0, 0, 255), 2);
		}
		cv::line(frameCopy, candidates[i][candidates[i].size() - 1], candidates[i][0], cv::Scalar(0, 0, 255), 2);
	}
	#if DEBUG_WITH_IMAGES
	showImageForDebug(frameCopy);
	#endif
}

void LibVision::checkColor() {
	std::vector<std::vector<cv::Point> > newCandidates;
	cv::Scalar minCol = cv::Scalar(lbParams->colorRange[0], lbParams->colorRange[1], lbParams->colorRange[2]);
	cv::Scalar maxCol = cv::Scalar(lbParams->colorRange[3], lbParams->colorRange[4], lbParams->colorRange[5]);
	
	for(int i = 0; i < this->candidates.size(); i++) {
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



