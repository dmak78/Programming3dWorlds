#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxSyphon.h"
#include "DepthExporter.h"

class testApp : public ofBaseApp {
public:
	void loadSettings();

	void clearBundle();
	template <class T>
	void addMessage(string address, T data);
	void sendBundle();

	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void updatePhysicalMesh();
	void setVideoSource(bool useCamera);

	bool bUseCamera, bPaused, bIncludeAllVertices;
    
	int camWidth, camHeight;
	int movieWidth, movieHeight;
	int sourceWidth, sourceHeight;

	string host;
	int port;
	ofxOscSender osc;
	ofxOscBundle bundle;

	ofVideoGrabber cam;
	ofVideoPlayer movie;
	ofBaseVideoDraws *videoSource;
    
    ofxSyphonServer syphonServer;
    
    ofxCv::Calibration calibration;
	ofLight light;
	
	ofMesh physicalMesh;
   
    ofMesh storedMesh;
    ofTexture storedTexture;

    
	ofxFaceTracker tracker;
	float scale;
	ofMatrix4x4 rotationMatrix;
    
    bool exportPly;
    
    vector<ofVec3f> memoryVecs;
    

};
