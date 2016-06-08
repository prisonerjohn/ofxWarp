#pragma once

#include "ofMain.h"

namespace ofxWarping
{
	class Warp
	{
	public:
		typedef enum 
		{
			TYPE_UNKNOWN,
			TYPE_BILINEAR,
			TYPE_PERSPECTIVE,
			TYPE_PERSPECTIVE_BILINEAR
		} Type;

		Warp(Type type = TYPE_UNKNOWN);
		virtual ~Warp();

		//! returns the type of the warp
		Type getType() const;

		virtual void serialize(nlohmann::json & json);
		virtual void deserialize(const nlohmann::json & json);

		void setEditing(bool editing);
		void toggleEditing();
		bool isEditing() const;

		//! set the width of the content in pixels
		void setWidth(float width);
		//! get the width of the content in pixels
		float getWidth() const;

		//! set the height of the content in pixels
		void setHeight(float height);
		//! get the height of the content in pixels
		float getHeight() const;

		//! set the width and height of the content in pixels
		void setSize(float width, float height);
		//! set the width and height of the content in pixels
		void setSize(const ofVec2f & size);
		//! get the width and height of the content in pixels
		ofVec2f getSize() const;
		//! get the rectangle of the content in pixels
		ofRectangle getBounds() const;

		//! set the luminance value for all color channels, used for edge blending (0.5 = linear)
		void setLuminance(float luminance);
		//! set the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		void setLuminance(float red, float green, float blue);
		//! returns the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		const ofVec3f & getLuminance() const;

		//! set the gamma curve value for all color channels
		void setGamma(float gamma);
		//! set the gamma curve value for the red, green and blue channels
		void setGamma(float red, float green, float blue);
		//! returns the gamma curve value for the red, green and blue channels
		const ofVec3f & getGamma() const;

		//! set the edge blending curve exponent  (1.0 = linear, 2.0 = quadratic)
		void setExponent(float exponent);
		//! returns the edge blending curve exponent (1.0 = linear, 2.0 = quadratic)
		float getExponent() const;

		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		void setEdges(float left, float top, float right, float bottom);
		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		void setEdges(const ofVec4f & edges);
		//! returns the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		ofVec4f getEdges() const;

		//! reset control points to undistorted image
		virtual void reset() = 0;
		//! setup the warp before drawing its contents
		virtual void begin() = 0;
		//! restore the warp after drawing
		virtual void end() = 0;

		//! draws a warped texture
		void draw(const ofTexture & texture);
		//! draws a specific area of a warped texture
		void draw(const ofTexture & texture, const ofRectangle & srcBounds);
		//! draws a specific area of a warped texture to a specific region
		virtual void draw(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) = 0;

		//! adjusts both the source and destination rectangles so that they are clipped against the warp's content
		bool clip(ofRectangle & srcBounds, ofRectangle & dstBounds) const;

		//! returns the coordinates of the specified control point
		virtual const ofVec2f & getControlPoint(size_t index) const;
		//! sets the coordinates of the specified control point
		virtual void setControlPoint(size_t index, const ofVec2f & pos);
		//! moves the specified control point
		virtual void moveControlPoint(size_t index, const ofVec2f & shift);
		//! get the number of control points
		virtual size_t getNumControlPoints() const;
		//! get the index of the currently selected control point
		virtual size_t getSelectedControlPoint() const;
		//! select one of the control points
		virtual void selectControlPoint(size_t index);
		//! deselect the selected control point
		virtual void deselectControlPoint();
		//! returns the index of the closest control point, as well as the distance in pixels
		virtual size_t findClosestControlPoint(const ofVec2f & pos, float * distance) const;

		virtual bool onMouseMoved(const ofVec2f & pos);
		virtual bool onMousePressed(const ofVec2f & pos);
		virtual bool onMouseDragged(const ofVec2f & pos);
		virtual bool onMouseReleased(const ofVec2f & pos);

		virtual bool onKeyPressed(int key);
		virtual bool onKeyReleased(int key);

		virtual bool onWindowResized(int width, int height);

		//! checks all warps and selects the closest control point
		static void selectClosestControlPoint(const vector<shared_ptr<Warp>> & warps, const ofVec2f & pos);

		//! handles mouseMoved events for multiple warps
		static bool handleMouseMoved(vector<shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mousePressed events for multiple warps
		static bool handleMousePressed(vector<shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mouseDragged events for multiple warps
		static bool handleMouseDragged(vector<shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mouseReleased events for multiple warps
		static bool handleMouseReleased(vector<shared_ptr<Warp>> & warps, const ofVec2f & pos);

		//! handles keyPressed events for multiple warps
		static bool handleKeyPressed(vector<shared_ptr<Warp>> & warps, int key);
		//! handles keyReleased events for multiple warps
		static bool handleKeyReleased(vector<shared_ptr<Warp>> & warps, int key);

		//! handles windowResized events for multiple warps
		static bool handleWindowResized(vector<shared_ptr<Warp>> & warps, int width, int height);

		//! read a settings json file and pass back a vector of Warps
		static vector<shared_ptr<Warp>> loadSettings(const string & filePath);
		//! write a settings json file
		static void saveSettings(const vector<shared_ptr<Warp>> & warps, const string & filePath);

	protected:
		//! draw the warp's editing interface
		virtual void drawInterface(bool controls = true) = 0;
		//! draw the control points
		void drawControlPoints();

	protected:
		Type type;

		bool editing;
		bool dirty;

		float width;
		float height;

		ofVec2f windowSize;

		float brightness;

		size_t numControlsX;
		size_t numControlsY;
		vector<ofVec2f> controlPoints;

		size_t selectedIndex;
		float selectedTime;
		ofVec2f selectedOffset;

		ofVec3f luminance;
		ofVec3f gamma;
		float exponent;
		ofVec4f edges;
	};
}