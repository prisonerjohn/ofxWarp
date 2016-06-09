#pragma once

#include "WarpBilinear.h"
#include "WarpPerspective.h"

namespace ofxWarping
{
	class WarpPerspectiveBilinear
		: public WarpBilinear
	{
	public:
		WarpPerspectiveBilinear(const ofFbo::Settings & fboSettings = ofFbo::Settings());
		virtual ~WarpPerspectiveBilinear();

		virtual void serialize(nlohmann::json & json) override;
		virtual void deserialize(const nlohmann::json & json) override;

		virtual void setEditing(bool editing) override;

		virtual void setSize(float width, float height) override;

		//! returns the coordinates of the specified control point
		virtual const ofVec2f & getControlPoint(size_t index) const override;
		//! sets the coordinates of the specified control point
		virtual void setControlPoint(size_t index, const ofVec2f & pos) override;
		//! moves the specified control point
		virtual void moveControlPoint(size_t index, const ofVec2f & shift) override;
		//! select one of the control points
		virtual void selectControlPoint(size_t index) override;
		//! deselect the selected control point
		virtual void deselectControlPoint() override;

		virtual bool onMouseMoved(const ofVec2f & pos) override;
		virtual bool onMousePressed(const ofVec2f & pos) override;
		virtual bool onMouseDragged(const ofVec2f & pos) override;

		virtual bool onKeyPressed(int key) override;

		virtual bool onWindowResized(int width, int height) override;

	protected:
		//! draws a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) override;

		//! returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point
		bool isCorner(size_t index) const;
		//! converts the control point index to the appropriate perspective warp index
		size_t convertIndex(size_t index) const;

	protected:
		std::shared_ptr<WarpPerspective> warpPerspective;
	};
}