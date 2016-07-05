#pragma once

#include "WarpBilinear.h"
#include "WarpPerspective.h"

namespace ofxWarp
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

		//! return the coordinates of the specified control point
		virtual glm::vec2 getControlPoint(size_t index) const override;
		//! set the coordinates of the specified control point
		virtual void setControlPoint(size_t index, const glm::vec2 & pos) override;
		//! move the specified control point
		virtual void moveControlPoint(size_t index, const glm::vec2 & shift) override;
		//! select one of the control points
		virtual void selectControlPoint(size_t index) override;
		//! deselect the selected control point
		virtual void deselectControlPoint() override;

		virtual void rotateClockwise() override;
		virtual void rotateCounterclockwise() override;

		virtual bool handleCursorDown(const glm::vec2 & pos) override;
		virtual bool handleCursorDrag(const glm::vec2 & pos) override;

		virtual bool handleWindowResize(int width, int height) override;

	protected:
		//! draw a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) override;

		//! return whether or not the control point is one of the 4 corners and should be treated as a perspective control point
		bool isCorner(size_t index) const;
		//! convert the control point index to the appropriate perspective warp index
		size_t convertIndex(size_t index) const;

	protected:
		std::shared_ptr<WarpPerspective> warpPerspective;
	};
}