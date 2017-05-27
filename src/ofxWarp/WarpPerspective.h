#pragma once

#include "WarpBase.h"

namespace ofxWarp
{
	class WarpPerspective
		: public WarpBase
	{
	public:
		WarpPerspective();
		virtual ~WarpPerspective();

		const glm::mat4 & getTransform();
		const glm::mat4 & getTransformInverted();

		//! reset control points to undistorted image
		virtual void reset(const glm::vec2 & scale = glm::vec2(1.0f), const glm::vec2 & offset = glm::vec2(0.0f)) override;
		//! setup the warp before drawing its contents
		virtual void begin() override;
		//! restore the warp after drawing
		virtual void end() override;

		virtual void rotateClockwise() override;
		virtual void rotateCounterclockwise() override;

		virtual void flipHorizontal() override;
		virtual void flipVertical() override;

	protected:
		//! draw a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) override;
		//! draw the warp's controls interface
		virtual void drawControls() override;

		glm::mat4 getPerspectiveTransform(const glm::vec2 src[4], const glm::vec2 dst[4]) const;
		void gaussianElimination(float * input, int n) const;

	protected:
		glm::vec2 srcPoints[4];
		glm::vec2 dstPoints[4];

		glm::mat4 transform;
		glm::mat4 transformInverted;

		ofShader shader;
		ofVboMesh quadMesh;
	};
}