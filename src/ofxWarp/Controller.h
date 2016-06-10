#pragma once

#include "ofMain.h"
#include "WarpBase.h"

namespace ofxWarp
{
	class Controller
	{
	public:
		Controller();
		~Controller();

		//! read a settings json file and pass back a vector of Warps
		bool loadSettings(const std::string & filePath);
		//! write a settings json file
		bool saveSettings(const std::string & filePath);

		//! build and add a new warp of the specified type
		template<class Type>
		inline std::shared_ptr<Type> buildWarp()
		{
			auto warp = std::make_shared<Type>();
			this->warps.push_back(warp);

			return warp;
		}

		//! add the warp to the list
		bool addWarp(std::shared_ptr<WarpBase> warp);
		//! remove the warp from the list
		bool removeWarp(std::shared_ptr<WarpBase> warp);

		//! return the list of warps
		std::vector<std::shared_ptr<WarpBase>> & getWarps();

		//! handle mouseMoved events for multiple warps
		void onMouseMoved(ofMouseEventArgs & args);
		//! handle mousePressed events for multiple warps
		void onMousePressed(ofMouseEventArgs & args);
		//! handle mouseDragged events for multiple warps
		void onMouseDragged(ofMouseEventArgs & args);
		//! handle mouseReleased events for multiple warps
		void onMouseReleased(ofMouseEventArgs & args);

		//! handle keyPressed events for multiple warps
		void onKeyPressed(ofKeyEventArgs & args);
		//! handle keyReleased events for multiple warps
		void onKeyReleased(ofKeyEventArgs & args);

		//! handle windowResized events for multiple warps
		void onWindowResized(ofResizeEventArgs & args);

	protected:
		//! check all warps and select the closest control point
		void selectClosestControlPoint(const ofVec2f & pos);

	protected:
		std::vector<std::shared_ptr<WarpBase>> warps;

		size_t focusedIndex;
	};
}
