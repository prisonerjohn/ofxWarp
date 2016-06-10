#pragma once

#include "ofMain.h"
#include "Warp.h"

namespace ofxWarping
{
	class Controller
	{
	public:
		Controller();
		~Controller();

		template<typename Type>
		shared_ptr<Type> addWarp<Type>();
		bool removeWarp(shared_ptr<Warp> warp);

		vector<shared_ptr<Warp>> & getWarps();

		//! checks all warps and selects the closest control point
		void selectClosestControlPoint(const ofVec2f & pos);

		//! handles mouseMoved events for multiple warps
		void onMouseMoved(ofMouseEventArgs & args);
		//! handles mousePressed events for multiple warps
		void onMousePressed(ofMouseEventArgs & args);
		//! handles mouseDragged events for multiple warps
		void onMouseDragged(ofMouseEventArgs & args);
		//! handles mouseReleased events for multiple warps
		void onMouseReleased(ofMouseEventArgs & args);

		//! handles keyPressed events for multiple warps
		void onKeyPressed(ofKeyEventArgs & args);
		//! handles keyReleased events for multiple warps
		void onKeyReleased(ofKeyEventArgs & args);

		//! handles windowResized events for multiple warps
		void onWindowResized(ofResizeEventArgs & args);

	protected:
		vector<shared_ptr<Warp>> warps;
	};
}
