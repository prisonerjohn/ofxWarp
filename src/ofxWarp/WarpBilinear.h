#pragma once

#include "WarpBase.h"

namespace ofxWarp
{
	class WarpBilinear
		: public WarpBase
	{
	public:
		WarpBilinear(const ofFbo::Settings & fboSettings = ofFbo::Settings());
		virtual ~WarpBilinear();

		virtual void serialize(nlohmann::json & json) override;
		virtual void deserialize(const nlohmann::json & json) override;

		virtual void setSize(float width, float height) override;

		void setFboSettings(const ofFbo::Settings & fboSettings);
		
		//! set whether the mesh is linear (or curved)
		void setLinear(bool linear);
		//! return whether the mesh is linear (or curved)
		bool getLinear() const;

		//! set whether the mesh resolution is adaptive to the window size
		void setAdaptive(bool adaptive);
		//! return whether the mesh resolution is adaptive to the window size
		bool getAdaptive() const;

		//! increase the mesh resolution
		void increaseResolution();
		//! decrease the mesh resolution
		void decreaseResolution();

		//! reset control points to undistorted image
		virtual void reset() override;
		//! setup the warp before drawing its contents
		virtual void begin() override;
		//! restore the warp after drawing
		virtual void end() override;

		//! set the number of horizontal control points for this warp
		void setNumControlsX(int n);
		//! set the number of vertical control points for this warp
		void setNumControlsY(int n);

		void setCorners(float left, float top, float right, float bottom);

		virtual void rotateClockwise() override;
		virtual void rotateCounterclockwise() override;

		virtual void flipHorizontal() override;
		virtual void flipVertical() override;

	protected:
		//! draw a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) override;
		//! draw the warp's controls interface
		virtual void drawControls() override;

		//! set up the frame buffer
		void setupFbo();
		//! set up the shader and vertex buffer
		void setupVbo();
		//! set up the vbo mesh
		void setupMesh(int resolutionX = 36, int resolutionY = 36);
		//! update the vbo mesh based on the control points
		void updateMesh();
		//!	return the specified control point, values for col and row are clamped to prevent errors.
		ofVec2f getPoint(int col, int row) const;
		//! perform fast Catmull-Rom interpolation, and return the interpolated value at t
		ofVec2f cubicInterpolate(const std::vector<ofVec2f> & knots, float t) const;
		//!
		ofRectangle getMeshBounds() const;

	protected:
		ofFbo fbo;
		ofFbo::Settings fboSettings;
		ofVbo vbo;
		ofShader shader;

		//! linear or curved interpolation
		bool linear;

		bool adaptive;

		//! texture coordinates of corners
		ofVec4f corners;

		//! detail of the generated mesh (multiples of 5 seem to work best)
		int resolution;

		//! number of horizontal quads
		int resolutionX;
		//! number of vertical quads
		int resolutionY;

	private:
		//! greatest common divisor using Euclidian algorithm (from: http://en.wikipedia.org/wiki/Greatest_common_divisor)
		inline int gcd(int a, int b) const
		{
			if (b == 0)
				return a;
			else
				return gcd(b, a % b);
		};
	};
}
