#include "Controller.h"

#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"

namespace ofxWarp
{
	//--------------------------------------------------------------
	Controller::Controller()
	{
		ofAddListener(ofEvents().mouseMoved, this, &Controller::onMouseMoved);
		ofAddListener(ofEvents().mousePressed, this, &Controller::onMousePressed);
		ofAddListener(ofEvents().mouseDragged, this, &Controller::onMouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &Controller::onMouseReleased);

		ofAddListener(ofEvents().keyPressed, this, &Controller::onKeyPressed);
		ofAddListener(ofEvents().keyReleased, this, &Controller::onKeyReleased);

		ofAddListener(ofEvents().windowResized, this, &Controller::onWindowResized);
	}
	
	//--------------------------------------------------------------
	Controller::~Controller()
	{
		ofRemoveListener(ofEvents().mouseMoved, this, &Controller::onMouseMoved);
		ofRemoveListener(ofEvents().mousePressed, this, &Controller::onMousePressed);
		ofRemoveListener(ofEvents().mouseDragged, this, &Controller::onMouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &Controller::onMouseReleased);

		ofRemoveListener(ofEvents().keyPressed, this, &Controller::onKeyPressed);
		ofRemoveListener(ofEvents().keyReleased, this, &Controller::onKeyReleased);

		ofRemoveListener(ofEvents().windowResized, this, &Controller::onWindowResized);
		
		this->warps.clear();
	}

	//--------------------------------------------------------------
	bool Controller::loadSettings(const std::string & filePath)
	{
		auto file = ofFile(filePath, ofFile::ReadOnly);
		if (!file.exists())
		{
			ofLogWarning("Warp::loadSettings") << "File not found at path " << filePath;
			return false;
		}

		this->warps.clear();

		nlohmann::json json;
		file >> json;
		for (auto & jsonWarp : json["warps"])
		{
			std::shared_ptr<WarpBase> warp;

			int typeAsInt = jsonWarp["type"];
			WarpBase::Type type = (WarpBase::Type)typeAsInt;
			switch (type)
			{
			case WarpBase::TYPE_BILINEAR:
				warp = std::make_shared<WarpBilinear>();
				break;

			case WarpBase::TYPE_PERSPECTIVE:
				warp = std::make_shared<WarpPerspective>();
				break;

			case WarpBase::TYPE_PERSPECTIVE_BILINEAR:
				warp = std::make_shared<WarpPerspectiveBilinear>();
				break;

			default:
				ofLogWarning("Warp::loadSettings") << "Unrecognized Warp type " << type;
			}

			if (warp)
			{
				warp->deserialize(jsonWarp);
				this->warps.push_back(warp);
			}
		}

		return true;
	}

	//--------------------------------------------------------------
	bool Controller::saveSettings(const std::string & filePath)
	{
		std::vector<nlohmann::json> jsonWarps;
		for (auto warp : this->warps)
		{
			nlohmann::json jsonWarp;
			warp->serialize(jsonWarp);
			jsonWarps.push_back(jsonWarp);
		}

		nlohmann::json json;
		json["warps"] = jsonWarps;

		auto file = ofFile(filePath, ofFile::WriteOnly);
		file << json;

		return true;
	}

	//--------------------------------------------------------------
	bool Controller::addWarp(std::shared_ptr<WarpBase> warp)
	{
		auto it = std::find(this->warps.begin(), this->warps.end(), warp);
		if (it == this->warps.end())
		{
			this->warps.push_back(warp);
			return true;
		}

		return false;
	}
	
	//--------------------------------------------------------------
	bool Controller::removeWarp(std::shared_ptr<WarpBase> warp)
	{
		auto it = std::find(this->warps.begin(), this->warps.end(), warp);
		if (it != this->warps.end())
		{
			this->warps.erase(it);
			return true;
		}

		return false;
	}

	//--------------------------------------------------------------
	std::vector<std::shared_ptr<WarpBase>> & Controller::getWarps()
	{
		return this->warps;
	}

	//--------------------------------------------------------------
	void Controller::selectClosestControlPoint(const ofVec2f & pos)
	{
		size_t warpIdx = -1;
		size_t pointIdx = -1;
		auto distance = std::numeric_limits<float>::max();

		// Find warp and distance to closest control point.
		for (int i = this->warps.size() - 1; i >= 0; --i)
		{
			float candidate;
			auto idx = this->warps[i]->findClosestControlPoint(pos, &candidate);
			if (candidate < distance)
			{
				distance = candidate;
				pointIdx = idx;
				warpIdx = i;
			}
		}

		// Select the closest control point and deselect all others.
		for (int i = this->warps.size() - 1; i >= 0; --i)
		{
			if (i == warpIdx)
			{
				this->warps[i]->selectControlPoint(pointIdx);
			}
			else
			{
				this->warps[i]->deselectControlPoint();
			}
		}
	}

	//--------------------------------------------------------------
	void Controller::onMouseMoved(ofMouseEventArgs & args)
	{
		// Find and select closest control point.
		this->selectClosestControlPoint(args);
	}

	//--------------------------------------------------------------
	void Controller::onMousePressed(ofMouseEventArgs & args)
	{
		// Find and select closest control point.
		this->selectClosestControlPoint(args);

		for (int i = this->warps.size() - 1; i >= 0; --i)
		{
			if (this->warps[i]->onMousePressed(args))
			{
				break;
			}
		}
	}

	//--------------------------------------------------------------
	void Controller::onMouseDragged(ofMouseEventArgs & args)
	{
		for (int i = this->warps.size() - 1; i >= 0; --i)
		{
			if (this->warps[i]->onMouseDragged(args))
			{
				break;
			}
		}
	}

	//--------------------------------------------------------------
	void Controller::onMouseReleased(ofMouseEventArgs & args)
	{}

	//--------------------------------------------------------------
	void Controller::onKeyPressed(ofKeyEventArgs & args)
	{
		if (args.key == 'w')
		{
			for (auto warp : this->warps)
			{
				warp->toggleEditing();
			}
		}

		for (auto warp : this->warps)
		{
			if (warp->onKeyPressed(args.key))
			{
				break;
			}
		}
	}

	//--------------------------------------------------------------
	void Controller::onKeyReleased(ofKeyEventArgs & args)
	{}

	//--------------------------------------------------------------
	void Controller::onWindowResized(ofResizeEventArgs & args)
	{
		for (auto warp : this->warps)
		{
			warp->onWindowResized(args.width, args.height);
		}
	}
}