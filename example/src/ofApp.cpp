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
	this->warps = ofxWarp::Warp::loadSettings("settings.json");
	if (this->warps.empty())
	{
		// Otherwise create warps from scratch.
		shared_ptr<ofxWarp::Warp> warp;

		warp = make_shared<ofxWarp::WarpPerspective>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 0.0f, 1.0f, 0.0f));
		this->warps.push_back(warp);

		warp = make_shared<ofxWarp::WarpBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(1.0f, 0.0f, 0.0f, 1.0f));
		this->warps.push_back(warp);

		warp = make_shared<ofxWarp::WarpPerspectiveBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 1.0f, 0.0f, 0.0f));
		this->warps.push_back(warp);

		warp = make_shared<ofxWarp::WarpPerspectiveBilinear>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 1.0f, 1.0f, 0.0f));
		this->warps.push_back(warp);
	}
	
	this->useBeginEnd = false;
}

//--------------------------------------------------------------
void ofApp::exit()
{
	ofxWarp::Warp::saveSettings(this->warps, "settings.json");
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
		for (auto warp : this->warps)
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
	ofxWarp::Warp::handleKeyPressed(this->warps, key);

	if (key == 'f')
	{
		ofToggleFullscreen();
	}
	else if (key == 'w')
	{
		for (auto warp : this->warps)
		{
			warp->toggleEditing();
		}
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
void ofApp::keyReleased(int key)
{
	if (!ofxWarp::Warp::handleKeyReleased(this->warps, key))
	{
		// Let the application perform its keyReleased handling here.
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
	if (!ofxWarp::Warp::handleMouseMoved(this->warps, ofVec2f(x, y))) 
	{
		// Let the application perform its mouseMoved handling here.
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	if (!ofxWarp::Warp::handleMouseDragged(this->warps, ofVec2f(x, y)))
	{
		// Let the application perform its mouseDragged handling here.
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	if (!ofxWarp::Warp::handleMousePressed(this->warps, ofVec2f(x, y)))
	{
		// Let the application perform its mousePressed handling here.
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	if (!ofxWarp::Warp::handleMouseReleased(this->warps, ofVec2f(x, y)))
	{
		// Let the application perform its mouseReleased handling here.
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	// Tell the warps our window has been resized, so they properly scale up or down.
	ofxWarp::Warp::handleWindowResized(this->warps, w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

