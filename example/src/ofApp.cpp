#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	ofDisableArbTex();
	ofBackground(ofColor::black);

	ofImage image;
	image.setUseTexture(false);
	if (!image.load("testcard.png"))
	{
		ofLogError("ofApp::setup") << "Could not load image!";
		return;
	}

	this->texture.enableMipmap();
	this->texture.loadData(image.getPixels());
	this->srcArea = ofRectangle(0, 0, this->texture.getWidth(), this->texture.getHeight());

	// Load warp settings from file if one exists.
	this->warpController.loadSettings("settings.json");
	if (this->warpController.getWarps().empty())
	{
		// Otherwise create warps from scratch.
		shared_ptr<ofxWarpBase> warp; 
		
		warp = this->warpController.buildWarp<ofxWarpPerspective>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 0.0f, 1.0f, 0.0f));
		
		warp = this->warpController.buildWarp<ofxWarpBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(1.0f, 0.0f, 0.0f, 1.0f));
		
		warp = this->warpController.buildWarp<ofxWarpPerspectiveBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 1.0f, 0.0f, 0.0f));
		
		warp = this->warpController.buildWarp<ofxWarpPerspectiveBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 1.0f, 1.0f, 0.0f));
	}
	
	this->useBeginEnd = false;
}

//--------------------------------------------------------------
void ofApp::exit()
{
	this->warpController.saveSettings("settings.json");
}

//--------------------------------------------------------------
void ofApp::update()
{
	ofSetWindowTitle(ofToString(ofGetFrameRate(), 2) + " FPS");
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(ofColor::black);

	if (this->texture.isAllocated())
	{
		for (auto warp : this->warpController.getWarps())
		{
			if (this->useBeginEnd)
			{
				warp->begin();

				auto bounds = warp->getBounds();
				this->texture.drawSubsection(bounds.x, bounds.y, bounds.width, bounds.height, this->srcArea.x, this->srcArea.y, this->srcArea.width, this->srcArea.height);

				warp->end();
			}
			else
			{
				warp->draw(this->texture, this->srcArea);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'f')
	{
		ofToggleFullscreen();
	}
	else if (key == 'a')
	{
		// Toggle drawing a random region of the image.
		if (this->srcArea.getWidth() != this->texture.getWidth() || this->srcArea.getHeight() != this->texture.getHeight())
			this->srcArea = ofRectangle(0, 0, this->texture.getWidth(), this->texture.getHeight());
		else {
			auto x1 = ofRandom(0, this->texture.getWidth() - 150);
			auto y1 = ofRandom(0, this->texture.getHeight() - 150);
			auto x2 = ofRandom(x1 + 150, this->texture.getWidth());
			auto y2 = ofRandom(y1 + 150, this->texture.getHeight());
			this->srcArea = ofRectangle(x1, y1, x2 - x1, y2 - y1);
		}
	}
	else if (key == ' ')
	{
		this->useBeginEnd ^= 1;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
