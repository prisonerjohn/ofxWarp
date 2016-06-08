#include "Controller.h"

namespace ofxWarping
{
	//--------------------------------------------------------------
	Controller::Controller()
	{
		ofAddListener(ofEvents().mouseMoved, this, &Controller::onMouseMoved);

	}
	
	//--------------------------------------------------------------
	Controller::~Controller()
	{
		this->warps.clear();
	}

	//--------------------------------------------------------------
	template<typename Type>
	shared_ptr<Type> Controller::addWarp<Type>()
	{
		auto warp = make_shared<Type>();
		this->warps.push_back(warp);

		return warp;
	}
	
	//--------------------------------------------------------------
	bool Controller::removeWarp(shared_ptr<Warp> warp)
	{
		auto it = find(this->warps.begin(), this->warps.end(), warp);
		if (it != this->warps.end())
		{
			this->warps.erase(it);
			return true;
		}

		return false;
	}

	//--------------------------------------------------------------
	vector<shared_ptr<Warp>> & Controller::getWarps()
	{
		return this->warps;
	}

	//--------------------------------------------------------------
	void Controller::selectClosestControlPoint(const vector<shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		size_t warpIdx = -1;
		size_t pointIdx = -1;
		auto distance = numeric_limits<float>::max();

		// Find warp and distance to closest control point.
		for (auto i = warps.size() - 1; i >= 0; --i)
		{
			float candidate;
			auto idx = warps[i]->findClosestControlPoint(pos, &candidate);
			if (candidate < distance)
			{
				distance = candidate;
				pointIdx = idx;
				warpIdx = i;
			}
		}

		// Select the closest control point and deselect all others.
		for (auto i = warps.size() - 1; i >= 0; --i)
		{
			if (i == warpIdx)
			{
				warps[i]->selectControlPoint(pointIdx);
			}
			else
			{
				warps[i]->deselectControlPoint();
			}
		}
	}

	//--------------------------------------------------------------
	void Controller::onMouseMoved(vector<shared_ptr<Warp>> & warps, ofMouseEventArgs & args)
	{
		// Find and select closest control point.
		this->selectClosestControlPoint(warps, args);

		return false;
	}

	//--------------------------------------------------------------
	void Controller::onMousePressed(vector<shared_ptr<Warp>> & warps, ofMouseEventArgs & args)
	{
		// Find and select closest control point.
		Warp::selectClosestControlPoint(warps, args);

		for (auto i = warps.size() - 1; i >= 0; --i)
		{
			if (warps[i]->onMousePressed(args))
			{
				return true;
			}
		}

		return false;
	}

	//--------------------------------------------------------------
	void Controller::onMouseDragged(vector<shared_ptr<Warp>> & warps, ofMouseEventArgs & args)
	{
		for (auto i = warps.size() - 1; i >= 0; --i)
		{
			if (warps[i]->onMouseDragged(args))
			{
				return true;
			}
		}

		return false;
	}

	//--------------------------------------------------------------
	void Controller::onMouseReleased(vector<shared_ptr<Warp>> & warps, ofMouseEventArgs & args)
	{
		return false;
	}

	//--------------------------------------------------------------
	void Controller::onKeyPressed(vector<shared_ptr<Warp>> & warps, ofKeyEventArgs & args)
	{
		for (auto warp : warps)
		{
			if (warp->onKeyPressed(args))
			{
				return true;
			}
		}

		return false;
	}

	//--------------------------------------------------------------
	void Controller::onKeyReleased(vector<shared_ptr<Warp>> & warps, ofKeyEventArgs & args)
	{
		return false;
	}

	//--------------------------------------------------------------
	void Controller::onWindowResized(vector<shared_ptr<Warp>> & warps, ofResizeEventArgs & args)
	{
		for (auto warp : warps)
		{
			warp->onWindowResized(args);
		}

		return false;
	}
}