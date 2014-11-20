#include "cinder/app/AppNative.h"
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"

#include "Leap.h"
#include "LeapMath.h"

#include <vector>
#include <string>
#include <cmath>

using namespace ci;
using namespace ci::app;
using namespace Leap;
using namespace std;

const string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

class ARUI : public AppNative {
public:
	void setup();
	void update();
	void draw();
	void prepareSettings( Settings *);
	void locateMarker(cv::Mat);
	double distanceCalculate(double, double, double, double);
private:
	int              windowWidth;
    int             windowHeight;
	Controller              leap;
	CaptureRef			mCapture;
	gl::TextureRef		mTexture;
	gl::Texture           screen;
	ci::Surface8u  webcamSurface;
	cv::Mat            webcamMat;
	vector<cv::Point>     allPts;
	int               xAvg, yAvg;
	bool              menuSwitch;
	float               c_x, c_y;
};

double ARUI::distanceCalculate(double x1, double y1, double x2, double y2){
    double x = x1 - x2;
    double y = y1 - y2;
    double dist;
    dist = pow(x,2)+pow(y,2);
    dist = sqrt(dist);
    return dist;
}

void ARUI::locateMarker(cv::Mat cvInput){
	int T = 25;
	int nRows = webcamMat.rows;
	int nCols = webcamMat.cols;
	allPts.clear();
	for(int i = 50; i < nRows-50; i++){
		for(int j = 50; j < nCols-50; j++){
			cv::Point3_<uchar>* pt1 = webcamMat.ptr<cv::Point3_<uchar>>(i-10,j);
			cv::Point3_<uchar>* pt2 = webcamMat.ptr<cv::Point3_<uchar>>(i+5,j-5);
			cv::Point3_<uchar>* pt3 = webcamMat.ptr<cv::Point3_<uchar>>(i+5,j+5);
			if((int)pt1->z > (int)pt2->z + T){
				if((int)pt1->z > (int)pt3->z + T){
					if((int)pt2->x > (int)pt1->x + T){
						if((int)pt2->x > (int)pt3->x + T){
							if((int)pt3->y > (int)pt1->y + T){
								if((int)pt3->y > (int)pt2->y + T){
									allPts.push_back(cv::Point(j,i));
								}
							}
						}
					}
				}
			}
		}
	}

	c_x = -1000; c_y = -1000;
	xAvg = -1; yAvg = -1;
	if(allPts.size()!=0){
		int xSum = 0;
		int ySum = 0;
		for(int i=0; i<allPts.size(); i++){
			xSum += allPts[i].x;
			ySum += allPts[i].y;
		}
		xAvg = xSum/allPts.size();
		yAvg = ySum/allPts.size();

		cout << xAvg << "," << yAvg << endl;
	}
}

void ARUI::prepareSettings( Settings *settings ){
	settings->enableConsoleWindow();
}

void ARUI::setup(){
	screen = gl::Texture( loadImage( loadAsset( "iphone.jpg" ) ) );

	menuSwitch = false;
	gl::enableAlphaBlending();
	this->setFrameRate(60);
	windowHeight = 360;
	windowWidth = 640;

	for( auto device = Capture::getDevices().begin(); device != Capture::getDevices().end(); ++device ) {
		console() << "Device: " << (*device)->getName() << endl;
	}

	try {
		mCapture = Capture::create( windowWidth, windowHeight );
		mCapture->start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << endl;
	}

	leap.enableGesture(Gesture::TYPE_CIRCLE);
}

void ARUI::update(){
	if( mCapture && mCapture->checkNewFrame() ) {
		mTexture = gl::Texture::create( mCapture->getSurface() );
		webcamSurface = mCapture->getSurface();
		webcamMat = toOcv( webcamSurface );
		locateMarker(webcamMat);
	}

	//	int T = 25;
	//	int nRows = webcamMat.rows;
	//	int nCols = webcamMat.cols;
	//	allPts.clear();
	//	for(int i = 50; i < nRows-50; i++){
	//		for(int j = 50; j < nCols-50; j++){
	//			cv::Point3_<uchar>* pt1 = webcamMat.ptr<cv::Point3_<uchar>>(i-10,j);
	//			cv::Point3_<uchar>* pt2 = webcamMat.ptr<cv::Point3_<uchar>>(i+5,j-5);
	//			cv::Point3_<uchar>* pt3 = webcamMat.ptr<cv::Point3_<uchar>>(i+5,j+5);
	//			if((int)pt1->z > (int)pt2->z + T){
	//				if((int)pt1->z > (int)pt3->z + T){
	//					if((int)pt2->x > (int)pt1->x + T){
	//						if((int)pt2->x > (int)pt3->x + T){
	//							if((int)pt3->y > (int)pt1->y + T){
	//								if((int)pt3->y > (int)pt2->y + T){
	//									allPts.push_back(cv::Point(j,i));
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//c_x = -1000; c_y = -1000;
	//xAvg = -1; yAvg = -1;
	//if(allPts.size()!=0){
	//	int xSum = 0;
	//	int ySum = 0;
	//	for(int i=0; i<allPts.size(); i++){
	//		xSum += allPts[i].x;
	//		ySum += allPts[i].y;
	//	}
	//	xAvg = xSum/allPts.size();
	//	yAvg = ySum/allPts.size();

	//	cout << xAvg << "," << yAvg << endl;
	//}

	

	InteractionBox iBox = leap.frame().interactionBox();
	const GestureList gestures = leap.frame().gestures();
	for (int g = 0; g < gestures.count(); g++){
		Gesture gesture = gestures[g];

		switch (gesture.type()){
			case Gesture::TYPE_CIRCLE:{
				CircleGesture circle = gesture;
				Vector circleCenter = circle.center();
				std::string clockwiseness;

				Vector normalizedCircleCenter = iBox.normalizePoint(circleCenter);
				c_x = normalizedCircleCenter.x * windowWidth;
				c_y = windowHeight - normalizedCircleCenter.y * windowHeight;

				if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
					clockwiseness = "clockwise";
				}
				else{
					clockwiseness = "counterclockwise";
				}

				if(distanceCalculate(c_x, xAvg, c_y, yAvg)<100){
					if(clockwiseness.compare("clockwise")){
						menuSwitch = true;
					}
					else{
						menuSwitch = false;
					}
				}		
				break;
			}
		}
	}
}

void ARUI::draw(){
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	gl::color(1, 1, 1, 1);
	if( mTexture ) {
		gl::draw( mTexture );
	}

	if(menuSwitch==true && xAvg>0 && yAvg>0){
		gl::color(1, 1, 1, 0.5);
		gl::draw( screen, Area(xAvg, yAvg, xAvg+180, yAvg+320) );
	}

	PointableList pointables = leap.frame().pointables();
    InteractionBox iBox = leap.frame().interactionBox();

	for( int p = 0; p < pointables.count(); p++ ){
        Pointable pointable = pointables[p];
        Vector normalizedPosition = iBox.normalizePoint(pointable.stabilizedTipPosition());
        float x = normalizedPosition.x * windowWidth;
        float y = windowHeight - normalizedPosition.y * windowHeight;
        
        if(pointable.touchDistance() > 0 && pointable.touchZone() != Pointable::Zone::ZONE_NONE){
            gl::color(0, 1, 0, 1 - pointable.touchDistance());
        }
        else if(pointable.touchDistance() <= 0){
            gl::color(1, 0, 0, -pointable.touchDistance());
        }
        else{
            gl::color(0, 0, 1, .05);
        }
       
        gl::drawSolidCircle(Vec2f(x,y), 10);
		//gl::color( Color::black() );
		//mTextureFont->drawString( toString((int)x)+","+toString((int)y), Vec2f( x, y) );
    }
}

CINDER_APP_NATIVE( ARUI, RendererGl )
