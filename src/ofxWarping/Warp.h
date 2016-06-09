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
		virtual void setWidth(float width);
		//! get the width of the content in pixels
		float getWidth() const;

		//! set the height of the content in pixels
		virtual void setHeight(float height);
		//! get the height of the content in pixels
		float getHeight() const;

		//! set the width and height of the content in pixels
		virtual void setSize(float width, float height);
		//! set the width and height of the content in pixels
		virtual void setSize(const ofVec2f & size);
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
		void draw(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds);

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
		static void selectClosestControlPoint(const std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos);

		//! handles mouseMoved events for multiple warps
		static bool handleMouseMoved(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mousePressed events for multiple warps
		static bool handleMousePressed(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mouseDragged events for multiple warps
		static bool handleMouseDragged(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos);
		//! handles mouseReleased events for multiple warps
		static bool handleMouseReleased(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos);

		//! handles keyPressed events for multiple warps
		static bool handleKeyPressed(std::vector<std::shared_ptr<Warp>> & warps, int key);
		//! handles keyReleased events for multiple warps
		static bool handleKeyReleased(std::vector<std::shared_ptr<Warp>> & warps, int key);

		//! handles windowResized events for multiple warps
		static bool handleWindowResized(std::vector<std::shared_ptr<Warp>> & warps, int width, int height);

		//! read a settings json file and pass back a vector of Warps
		static std::vector<std::shared_ptr<Warp>> loadSettings(const std::string & filePath);
		//! write a settings json file
		static void saveSettings(const std::vector<std::shared_ptr<Warp>> & warps, const std::string & filePath);

	protected:
		//! draws a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) = 0;
		//! draw the warp's controls interface
		virtual void drawControls() = 0;
		
		//! draw a control point in the preset color
		void queueControlPoint(const ofVec2f & pos, bool selected = false, bool attached = false);
		//! draw a control point in the specified color
		void queueControlPoint(const ofVec2f & pos, const ofFloatColor & color, float scale = 1.0f);

		//! setup the control points instanced vbo
		void setupControlPoints();
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
		std::vector<ofVec2f> controlPoints;

		size_t selectedIndex;
		float selectedTime;
		ofVec2f selectedOffset;

		ofVec3f luminance;
		ofVec3f gamma;
		float exponent;
		ofVec4f edges;

		static const int MAX_NUM_CONTROL_POINTS = 1024;

	private:
		typedef enum
		{
			INSTANCE_POS_SCALE_ATTRIBUTE = 5,
			INSTANCE_COLOR_ATTRIBUTE = 6
		} Attribute;

		typedef struct ControlData
		{
			ofVec2f pos;
			float scale;
			float dummy;
			ofFloatColor color;

			ControlData() 
			{}

			ControlData(const ofVec2f & pos, const ofFloatColor & color, float scale)
				: pos(pos)
				, color(color)
				, scale(scale)
			{}
		} ControlData;

		std::vector<ControlData> controlData;
		ofVboMesh controlMesh;
		ofShader controlShader;
	};
}