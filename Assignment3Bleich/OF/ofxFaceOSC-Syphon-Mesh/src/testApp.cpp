#include "testApp.h"
#include "ofMeshUtils.h"
using namespace ofxCv;
using namespace cv;

void testApp::loadSettings() {
	ofxXmlSettings xml;
	xml.loadFile("settings.xml");

	bool bUseCamera = true;

	xml.pushTag("source");
	if(xml.getNumTags("useCamera") > 0) {
		bUseCamera = xml.getValue("useCamera", 0);
	}
	xml.popTag();
    
    xml.pushTag("vertices");
	if(xml.getNumTags("includeAll") > 0) {
		bIncludeAllVertices = xml.getValue("includeAll", 0);
	} else {
        bIncludeAllVertices = true;
    }
	xml.popTag();

    cout << "bIncludeAllVertices: " << bIncludeAllVertices << endl;
    
	xml.pushTag("camera");
	if(xml.getNumTags("device") > 0) {
		cam.setDeviceID(xml.getValue("device", 0));
	}
	if(xml.getNumTags("framerate") > 0) {
		cam.setDesiredFrameRate(xml.getValue("framerate", 30));
	}
	camWidth = xml.getValue("width", 640);
	camHeight = xml.getValue("height", 480);
	cam.initGrabber(camWidth, camHeight);
	xml.popTag();
    
	xml.pushTag("movie");
	if(xml.getNumTags("filename") > 0) {
		string filename = ofToDataPath((string) xml.getValue("filename", ""));
		if(!movie.loadMovie(filename)) {
			ofLog(OF_LOG_ERROR, "Could not load movie \"%s\", reverting to camera input", filename.c_str());
			bUseCamera = true; 
		}
		movie.play();
	}
	else {
		ofLog(OF_LOG_ERROR, "Movie filename tag not set in settings, reverting to camera input");
		bUseCamera = true;
	}
	if(xml.getNumTags("volume") > 0) {
		float movieVolume = ofClamp(xml.getValue("volume", 1.0), 0, 1.0); 
		movie.setVolume(movieVolume);
	}
	if(xml.getNumTags("speed") > 0) {
		float movieSpeed = ofClamp(xml.getValue("speed", 1.0), -16, 16); 
		movie.setSpeed(movieSpeed);
	}
	bPaused = false;
	movieWidth = movie.getWidth();
	movieHeight = movie.getHeight();
	xml.popTag();

	if(bUseCamera) {
		ofSetWindowShape(camWidth, camHeight);
		setVideoSource(true);
	}
	else {
		ofSetWindowShape(movieWidth, movieHeight);
		setVideoSource(false);
	}
    
	xml.pushTag("face");
	if(xml.getNumTags("rescale")) {
		tracker.setRescale(xml.getValue("rescale", 1.));
	}
	if(xml.getNumTags("iterations")) {
		tracker.setIterations(xml.getValue("iterations", 5));
	}
	if(xml.getNumTags("clamp")) {
		tracker.setClamp(xml.getValue("clamp", 3.));
	}
	if(xml.getNumTags("tolerance")) {
		tracker.setTolerance(xml.getValue("tolerance", .01));
	}
	if(xml.getNumTags("attempts")) {
		tracker.setAttempts(xml.getValue("attempts", 1));
	}
	tracker.setup();
	xml.popTag();

	xml.pushTag("osc");
	host = xml.getValue("host", "localhost");
	port = xml.getValue("port", 8338);
	osc.setup(host, port);
	xml.popTag();

	osc.setup(host, port);
}

void testApp::updatePhysicalMesh() {
	// 1 load object and image points as Point2f/3f
	vector<Point3f> objectPoints;
	vector<Point2f> imagePoints;
	for(int i = 0; i < tracker.size(); i++) {
		objectPoints.push_back(toCv(tracker.getObjectPoint(i)));
		imagePoints.push_back(toCv(tracker.getImagePoint(i)));
	}
	
	// 2 guess for the rotation and translation of the face
	Mat cameraMatrix = calibration.getDistortedIntrinsics().getCameraMatrix();
	Mat distCoeffs = calibration.getDistCoeffs();
	Mat rvec, tvec;
	solvePnP(Mat(objectPoints),  Mat(imagePoints),
             cameraMatrix,  distCoeffs,
             rvec, tvec);
	
	// 3 reproject using guess, and fit to the actual image location
	vector<ofVec3f> fitWorldPoints;
	Mat cameraMatrixInv = cameraMatrix.inv();
	Mat rmat;
	Rodrigues(rvec, rmat);
	for(int i = 0; i < objectPoints.size(); i++) {
		Point2d imgImg = imagePoints[i];
		Point3d objObj = objectPoints[i];
		Point3d imgHom(imgImg.x, imgImg.y, 1.); // img->hom
		Point3d imgWor = (Point3f) Mat(cameraMatrixInv * Mat(imgHom)); // hom->wor
		Point3d objWor = (Point3d) Mat(tvec + rmat * Mat(objObj)); // obj->wor
		Point3d fitWor = intersectPointRay(objWor, imgWor); // scoot it over
		// if it was projected on the wrong side, flip it over
		if(fitWor.z < 0) {
			fitWor *= -1;
		}
		fitWorldPoints.push_back(toOf(fitWor));
		// convert down to image space coordinates
		//Point3d fitHom = (Point3d) Mat(cameraMatrix * Mat(fitWor)); // wor->hom
		//Point2d fitImg(fitHom.x / fitHom.z, fitHom.y / fitHom.z); // hom->img
	}
	
	// 4 use the resulting 3d points to build a mesh with normals
	physicalMesh = convertFromIndices(tracker.getMesh(fitWorldPoints));
   
	physicalMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	buildNormals(physicalMesh);
}


void testApp::setup() {
	ofSetVerticalSync(true);
    calibration.load("mbp-isight.yml");
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	light.setPosition(-500, 0, 0);
#ifdef TARGET_OSX
	ofSetDataPathRoot("../Resources/data/");
    syphonServer.setName("FaceOSC Camera");
#endif
	loadSettings();
}

void testApp::clearBundle() {
	bundle.clear();
}

template <>
void testApp::addMessage(string address, ofVec3f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	msg.addFloatArg(data.z);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, ofVec2f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, float data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, int data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addIntArg(data);
	bundle.addMessage(msg);
}

void testApp::sendBundle() {
	osc.sendBundle(bundle);
}

void testApp::update() {
	if(bPaused)
		return;
        
	videoSource->update();
	if(videoSource->isFrameNew()) {
		tracker.update(toCv(*videoSource));
        
		clearBundle();

		if(tracker.getFound()) {
            updatePhysicalMesh();
            
            storedMesh.clear();
            storedMesh = tracker.getObjectMesh();
            
            //storedTexture.clear();
            ofPixels pixelStore;
            cam.getTextureReference().readToPixels(pixelStore);
            ofImage storedImage;
            storedImage.setFromPixels(pixelStore);
            storedImage.setUseTexture(true);
            storedTexture = storedImage.getTextureReference();
            
			addMessage("/found", 1);

//			ofVec2f position = tracker.getPosition();
//			addMessage("/pose/position", position);
//			scale = tracker.getScale();
//			addMessage("/pose/scale", scale);
            ofVec3f orientation = tracker.getOrientation();
			addMessage("/pose/orientation", orientation);
//
//			addMessage("/gesture/mouth/width", tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH));
//			addMessage("/gesture/mouth/height", tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT));
//			addMessage("/gesture/eyebrow/left", tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT));
//			addMessage("/gesture/eyebrow/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT));
//			addMessage("/gesture/eye/left", tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS));
//			addMessage("/gesture/eye/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS));
//			addMessage("/gesture/jaw", tracker.getGesture(ofxFaceTracker::JAW_OPENNESS));
//			addMessage("/gesture/nostrils", tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE));
            
            if(bIncludeAllVertices){
                
                
                vector<ofVec3f> imagePoints = physicalMesh.getVertices();
                //vector<ofVec3f> imagePoints = tracker.getObjectPoints();
                
                ofxOscMessage msg1;
                msg1.setAddress("/vectors");
                for(int i = 0; i < imagePoints.size(); i++){
                    ofVec3f p = imagePoints.at(i);
                    
                    //p.normalize();
                    msg1.addFloatArg(p.x);
                    msg1.addFloatArg(p.y);
                    msg1.addFloatArg(p.z);
                    
                }
                bundle.addMessage(msg1);
                
                ofxOscMessage msg2;
                msg2.setAddress("/normals");
                //vector<ofVec2f> imagePoints = tracker.getImagePoints();
                vector<ofVec3f> normalPoints = physicalMesh.getNormals();
                for(int i = 0; i < physicalMesh.getNumNormals(); i++){
                    ofVec3f n = normalPoints.at(i);
                    n.normalize();
                    msg2.addFloatArg(n.x);
                    msg2.addFloatArg(n.y);
                    msg2.addFloatArg(n.z);
                }
                bundle.addMessage(msg2);
            }
		} 
        else {
			addMessage("/found", 0);
            
		}

		sendBundle();

		rotationMatrix = tracker.getRotationMatrix();
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
	//videoSource->draw(0, 0);

	if(tracker.getFound()) {
        
        //vector<ofIndexType> numIndices = physicalMesh.getIndices();
		//ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
        //ofDrawBitmapString(ofToString((ofIndexType) numIndices.at(200)), 10, 20);
        

		ofSetLineWidth(1);

		ofPushView();
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
        ofVec2f pos = tracker.getPosition();
		ofTranslate(pos.x-ofGetWidth()/2, pos.y-ofGetHeight()/2);
		applyMatrix(rotationMatrix);
		ofScale(-4,4,4);
        cam.getTextureReference().bind();
		tracker.getMeanObjectMesh().draw();
		cam.getTextureReference().unbind();
		ofPopView();
        
        
      //  calibration.getDistortedIntrinsics().loadProjectionMatrix();
//        ofEnableLighting();
//            light.enable();
//           // physicalMesh.draw();
//       cam.getTextureReference().bind();
//		tracker.getObjectMesh().draw();
//      //  physicalMesh.draw();
//	cam.getTextureReference().unbind();
//		ofDisableLighting();
	} else {
        
        
		ofDrawBitmapString("searching for face...", 10, 20);
        ofPushView();
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);

		ofScale(-4,4,4);
        storedTexture.bind();
		storedMesh.draw();
		storedTexture.unbind();
		ofPopView();
    
	}
    
	if(bPaused) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString( "paused", 10, 32);
	}

	if(!bUseCamera) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString("speed "+ofToString(movie.getSpeed()), ofGetWidth()-100, 20);
	}
    
#ifdef TARGET_OSX
    syphonServer.publishScreen();
#endif
}

void testApp::keyPressed(int key) {
	switch(key) {
		case 'r':
			tracker.reset();
			break;
		case 'p':
			bPaused = !bPaused;
			break;
        case 'x':
			exportPly = true;
			break;
		case OF_KEY_UP:
			movie.setSpeed(ofClamp(movie.getSpeed()+0.2, -16, 16));
			break;
		case OF_KEY_DOWN:
			movie.setSpeed(ofClamp(movie.getSpeed()-0.2, -16, 16));
			break;
	}
}

void testApp::setVideoSource(bool useCamera) {

	bUseCamera = useCamera;

	if(bUseCamera) {
		videoSource = &cam;
		sourceWidth = camWidth;
		sourceHeight = camHeight;
	}
	else {
		videoSource = &movie;
		sourceWidth = movieWidth;
		sourceHeight = movieHeight;
	}
}
