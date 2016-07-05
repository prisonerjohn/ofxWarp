#include "WarpPerspectiveBilinear.h"

namespace ofxWarp
{
	//--------------------------------------------------------------
	WarpPerspectiveBilinear::WarpPerspectiveBilinear(const ofFbo::Settings & fboSettings)
		: WarpBilinear(fboSettings)
	{
		this->type = TYPE_PERSPECTIVE_BILINEAR;

		// Create child WarpPerspective.
		this->warpPerspective = std::make_shared<WarpPerspective>();
	}
	
	//--------------------------------------------------------------
	WarpPerspectiveBilinear::~WarpPerspectiveBilinear()
	{}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::serialize(nlohmann::json & json)
	{
		WarpBilinear::serialize(json);
		
		std::vector<std::string> corners;
		for (auto i = 0; i < 4; ++i)
		{
			const auto corner = this->warpPerspective->getControlPoint(i);
			std::ostringstream oss;
			oss << corner;
			corners.push_back(oss.str());
		}
		json["corners"] = corners;
	}
	
	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::deserialize(const nlohmann::json & json)
	{
		WarpBilinear::deserialize(json);

		auto i = 0;
		for (const auto & jsonPoint : json["corners"])
		{
			glm::vec2 corner;
			std::istringstream iss;
			iss.str(jsonPoint);
			iss >> corner;
			this->warpPerspective->setControlPoint(i, corner);

			++i;
		}
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::setEditing(bool editing)
	{
		WarpBase::setEditing(editing);
		this->warpPerspective->setEditing(this->editing);
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::setSize(float width, float height)
	{
		// Make content size compatible with WarpBilinear's windowSize.
		this->warpPerspective->setSize(this->windowSize);

		WarpBilinear::setSize(width, height);
	}

	//--------------------------------------------------------------
	glm::vec2 WarpPerspectiveBilinear::getControlPoint(size_t index) const
	{
		// Depending on index, return perspective or bilinear control point.
		if (this->isCorner(index)) 
		{
			// Perspective: simply return one of the corners.
			return this->warpPerspective->getControlPoint(this->convertIndex(index));
		}
		else 
		{
			// Bilinear: transform control point from warped space to normalized screen space.
			auto cp = WarpBase::getControlPoint(index) * this->warpPerspective->getSize();
			auto pt = this->warpPerspective->getTransform() * glm::vec4(cp.x, cp.y, 0.0f, 1.0f);

			if (pt.w != 0) pt.w = 1.0f / pt.w;
			pt *= pt.w;

			return glm::vec2(pt.x, pt.y) / this->windowSize;
		}
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::setControlPoint(size_t index, const glm::vec2 & pos)
	{
		// Depending on index, set perspective or bilinear control point.
		if (this->isCorner(index)) 
		{
			// Perspective: simply set the control point.
			this->warpPerspective->setControlPoint(this->convertIndex(index), pos);
		}
		else 
		{
			// Bilinear:: transform control point from normalized screen space to warped space.
			auto cp = pos * this->windowSize;
			auto pt = this->warpPerspective->getTransformInverted() * glm::vec4(cp.x, cp.y, 0.0f, 1.0f);

			if (pt.w != 0) pt.w = 1.0f / pt.w;
			pt *= pt.w;

			WarpBase::setControlPoint(index, glm::vec2(pt.x, pt.y) / this->warpPerspective->getSize());
		}
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::moveControlPoint(size_t index, const glm::vec2 & shift)
	{
		// Depending on index, move perspective or bilinear control point.
		if (this->isCorner(index)) 
		{
			// Perspective: simply move the control point.
			this->warpPerspective->moveControlPoint(this->convertIndex(index), shift);
		}
		else {
			// Bilinear: transform control point from normalized screen space to warped space.
			auto pt = getControlPoint(index);
			this->setControlPoint(index, pt + shift);
		}
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::selectControlPoint(size_t index)
	{
		// Depending on index, select perspective or bilinear control point.
		if (this->isCorner(index)) 
		{
			this->warpPerspective->selectControlPoint(this->convertIndex(index));
		}
		else 
		{
			this->warpPerspective->deselectControlPoint();
		}

		// Always select bilinear control point, which we use to keep track of editing.
		WarpBase::selectControlPoint(index);
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::deselectControlPoint()
	{
		this->warpPerspective->deselectControlPoint();
		WarpBase::deselectControlPoint();
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::rotateClockwise()
	{
		this->warpPerspective->rotateClockwise();
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::rotateCounterclockwise()
	{
		this->warpPerspective->rotateCounterclockwise();
	}

	//--------------------------------------------------------------
	bool WarpPerspectiveBilinear::handleCursorDown(const glm::vec2 & pos)
	{
		if (!this->editing || this->selectedIndex >= this->controlPoints.size()) return false;

		// Depending on selected control point, let perspective or bilinear warp handle it.
		if (this->isCorner(this->selectedIndex)) 
		{
			return this->warpPerspective->handleCursorDown(pos);
		}
		return WarpBase::handleCursorDown(pos);
	}
			
	//--------------------------------------------------------------
	bool WarpPerspectiveBilinear::handleCursorDrag(const glm::vec2 & pos)
	{
		if (!this->editing || this->selectedIndex >= this->controlPoints.size()) return false;

		// Depending on selected control point, let perspective or bilinear warp handle it.
		if (this->isCorner(this->selectedIndex))
		{
			return this->warpPerspective->handleCursorDrag(pos);
		}
		return WarpBase::handleCursorDrag(pos);
	}

	//--------------------------------------------------------------
	bool WarpPerspectiveBilinear::handleWindowResize(int width, int height)
	{
		// Make content size compatible with WarpBilinear's windowSize.
		this->warpPerspective->setSize(width, height);

		auto handled = this->warpPerspective->handleWindowResize(width, height);
		handled |= WarpBilinear::handleWindowResize(width, height);

		return handled;
	}

	//--------------------------------------------------------------
	void WarpPerspectiveBilinear::drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds)
	{
		ofPushMatrix();
		{
			// Apply Perspective transform.
			ofMultMatrix(this->warpPerspective->getTransform());

			// Draw Bilinear warp.
			WarpBilinear::drawTexture(texture, srcBounds, dstBounds);
		}
		ofPopMatrix();
	}

	//--------------------------------------------------------------
	bool WarpPerspectiveBilinear::isCorner(size_t index) const
	{
		auto numControls = (this->numControlsX * this->numControlsY);

		return (index == 0 || index == (numControls - this->numControlsY) || index == (numControls - 1) || index == (this->numControlsY - 1));
	}

	//--------------------------------------------------------------
	size_t WarpPerspectiveBilinear::convertIndex(size_t index) const
	{
		auto numControls = (this->numControlsX * this->numControlsY);

		if (index == 0)
		{
			return 0;
		}
		if (index == (numControls - this->numControlsY))
		{
			return 2;
		}
		if (index == (numControls - 1))
		{
			return 3;
		}
		if (index == (this->numControlsY - 1))
		{
			return 1;
		}
		
		return index;
	}
}