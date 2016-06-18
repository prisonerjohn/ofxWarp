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

	// Load warp settings from file if one exists.
	this->warpController.loadSettings("settings.json");
	if (this->warpController.getWarps().empty())
	{
		// Otherwise create warps from scratch.
		shared_ptr<ofxWarpBase> warp; 
		
		warp = this->warpController.buildWarp<ofxWarpPerspective>();
		warp->setSize(this->texture.getWidth(), this->texture.getHeight());
		warp->setEdges(ofVec4f(0.0f, 0.0f, 1.0f, 1.0f));
		
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

	this->srcAreas.resize(this->warpController.getNumWarps());

	// Start with full area mode.
	this->areaMode = -1;
	this->keyPressed('a');
	
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
	ofSetWindowTitle(ofToString(ofGetFrameRate(), 2) + " FPS :: " + areaName + " :: " + (this->useBeginEnd ? "begin()/end()" : "draw()"));
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(ofColor::black);

	if (this->texture.isAllocated())
	{
		for (auto i = 0; i < this->warpController.getNumWarps(); ++i)
		{
			auto warp = this->warpController.getWarp(i);
			if (this->useBeginEnd)
			{
				warp->begin();

				auto bounds = warp->getBounds();
				this->texture.drawSubsection(bounds.x, bounds.y, bounds.width, bounds.height, this->srcAreas[i].x, this->srcAreas[i].y, this->srcAreas[i].width, this->srcAreas[i].height);

				warp->end();
			}
			else
			{
				warp->draw(this->texture, this->srcAreas[i]);
			}
		}
	}

	ostringstream oss;
	oss << ofToString(ofGetFrameRate(), 2) << " fps" << endl;
	oss << "[a]rea mode: " << areaName << endl;
	oss << "[d]raw mode: " << (this->useBeginEnd ? "begin()/end()" : "draw()") << endl;
	oss << "[w]arp edit: " << (this->warpController.getWarp(0)->isEditing() ? "on" : "off");
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(oss.str(), 10, 20);
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
		this->areaMode = (this->areaMode + 1) % 3;
		if (this->areaMode == 0)
		{
			// Draw the full image for each warp.
			auto area = ofRectangle(0, 0, this->texture.getWidth(), this->texture.getHeight());
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i)
			{
				this->srcAreas[i] = area;
			}

			this->areaName = "full";
		}
		else if (this->areaMode == 1)
		{			
			// Draw a corner region of the image so that all warps make up the entire image.
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i)
			{
				static const auto overlap = 10.0f;
				if (i == 0)
				{
					// Top-left.
					this->srcAreas[i] = ofRectangle(0, 0, this->texture.getWidth() * 0.5f + overlap, this->texture.getHeight() * 0.5f + overlap);
				}
				else if (i == 1)
				{
					// Top-right.
					this->srcAreas[i] = ofRectangle(this->texture.getWidth() * 0.5f - overlap, 0, this->texture.getWidth() * 0.5f + overlap, this->texture.getHeight() * 0.5f + overlap);
				}
				else if (i == 2)
				{
					// Bottom-right.
					this->srcAreas[i] = ofRectangle(this->texture.getWidth() * 0.5f - overlap, this->texture.getHeight() * 0.5f - overlap, this->texture.getWidth() * 0.5f + overlap, this->texture.getHeight() * 0.5f + overlap);
				}
				else
				{
					// Bottom-left.
					this->srcAreas[i] = ofRectangle(0, this->texture.getHeight() * 0.5f - overlap, this->texture.getWidth() * 0.5f + overlap, this->texture.getHeight() * 0.5f + overlap);
				}
			}

			this->areaName = "corners";
		}
		else
		{			
			// Draw a random region of the image for each warp.
			auto x1 = ofRandom(0, this->texture.getWidth() - 150);
			auto y1 = ofRandom(0, this->texture.getHeight() - 150);
			auto x2 = ofRandom(x1 + 150, this->texture.getWidth());
			auto y2 = ofRandom(y1 + 150, this->texture.getHeight());
			auto area = ofRectangle(x1, y1, x2 - x1, y2 - y1);
			for (auto i = 0; i < this->warpController.getNumWarps(); ++i)
			{
				this->srcAreas[i] = area;
			}

			this->areaName = "random";
		}
	}
	else if (key == 'd')
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
