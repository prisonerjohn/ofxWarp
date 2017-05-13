#pragma once

#include "ofMain.h"

namespace ofxWarp
{
	class WarpBase
	{
	public:
		typedef enum 
		{
			TYPE_UNKNOWN,
			TYPE_BILINEAR,
			TYPE_PERSPECTIVE,
			TYPE_PERSPECTIVE_BILINEAR
		} Type;

		WarpBase(Type type = TYPE_UNKNOWN);
		virtual ~WarpBase();

		//! returns the type of the warp
		Type getType() const;

		virtual void serialize(nlohmann::json & json);
		virtual void deserialize(const nlohmann::json & json);

		virtual void setEditing(bool editing);
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
		virtual void setSize(const glm::vec2 & size);
		//! get the width and height of the content in pixels
		glm::vec2 getSize() const;
		//! get the rectangle of the content in pixels
		ofRectangle getBounds() const;

		//! set the brightness value of the texture (values between 0 and 1)
		void setBrightness(float brightness);
		//! return the brightness value of the texture (values between 0 and 1)
		float getBrightness() const;

		//! set the luminance value for all color channels, used for edge blending (0.5 = linear)
		void setLuminance(float luminance);
		//! set the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		void setLuminance(float red, float green, float blue);
		//! set the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		void setLuminance(const glm::vec3 & rgb);
		//! returns the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		const glm::vec3 & getLuminance() const;

		//! set the gamma curve value for all color channels
		void setGamma(float gamma);
		//! set the gamma curve value for the red, green and blue channels
		void setGamma(float red, float green, float blue);
		//! set the gamma curve value for the red, green and blue channels
		void setGamma(const glm::vec3 & rgb);
		//! return the gamma curve value for the red, green and blue channels
		const glm::vec3 & getGamma() const;

		//! set the edge blending curve exponent  (1.0 = linear, 2.0 = quadratic)
		void setExponent(float exponent);
		//! return the edge blending curve exponent (1.0 = linear, 2.0 = quadratic)
		float getExponent() const;

		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		void setEdges(float left, float top, float right, float bottom);
		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		void setEdges(const glm::vec4 & edges);
		//! return the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		glm::vec4 getEdges() const;

		//! reset control points to undistorted image
		virtual void reset() = 0;
		//! setup the warp before drawing its contents
		virtual void begin() = 0;
		//! restore the warp after drawing
		virtual void end() = 0;

		//! draw a warped texture
		void draw(const ofTexture & texture);
		//! draw a specific area of a warped texture
		void draw(const ofTexture & texture, const ofRectangle & srcBounds);
		//! draw a specific area of a warped texture to a specific region
		void draw(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds);

		//! adjust both the source and destination rectangles so that they are clipped against the warp's content
		bool clip(ofRectangle & srcBounds, ofRectangle & dstBounds) const;

		//! return the coordinates of the specified control point
		virtual glm::vec2 getControlPoint(size_t index) const;
		//! set the coordinates of the specified control point
		virtual void setControlPoint(size_t index, const glm::vec2 & pos);
		//! move the specified control point
		virtual void moveControlPoint(size_t index, const glm::vec2 & shift);
		//! get the number of control points
		virtual size_t getNumControlPoints() const;
		//! get the index of the currently selected control point
		virtual size_t getSelectedControlPoint() const;
		//! select one of the control points
		virtual void selectControlPoint(size_t index);
		//! deselect the selected control point
		virtual void deselectControlPoint();
		//! return the index of the closest control point, as well as the distance in pixels
		virtual size_t findClosestControlPoint(const glm::vec2 & pos, float * distance) const;

		//! return the number of control points columns
		size_t getNumControlsX() const;
		//! return the number of control points rows
		size_t getNumControlsY() const;

		virtual void rotateClockwise() = 0;
		virtual void rotateCounterclockwise() = 0;

		virtual void flipHorizontal() = 0;
		virtual void flipVertical() = 0;

		virtual bool handleCursorDown(const glm::vec2 & pos);
		virtual bool handleCursorDrag(const glm::vec2 & pos);

		virtual bool handleWindowResize(int width, int height);

		static void setShaderPath(const std::filesystem::path shaderPath);

	protected:
		//! draw a specific area of a warped texture to a specific region
		virtual void drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds) = 0;
		//! draw the warp's controls interface
		virtual void drawControls() = 0;
		
		//! draw a control point in the preset color
		void queueControlPoint(const glm::vec2 & pos, bool selected = false, bool attached = false);
		//! draw a control point in the specified color
		void queueControlPoint(const glm::vec2 & pos, const ofFloatColor & color, float scale = 1.0f);

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

		glm::vec2 windowSize;

		float brightness;

		size_t numControlsX;
		size_t numControlsY;
		std::vector<glm::vec2> controlPoints;

		size_t selectedIndex;
		float selectedTime;
		glm::vec2 selectedOffset;

		glm::vec3 luminance;
		glm::vec3 gamma;
		float exponent;
		glm::vec4 edges;

		static const int MAX_NUM_CONTROL_POINTS = 1024;

		static std::filesystem::path shaderPath;

	private:
		typedef enum
		{
			INSTANCE_POS_SCALE_ATTRIBUTE = 5,
			INSTANCE_COLOR_ATTRIBUTE = 6
		} Attribute;

		typedef struct ControlData
		{
			glm::vec2 pos;
			float scale;
			float dummy;
			ofFloatColor color;

			ControlData() 
			{}

			ControlData(const glm::vec2 & pos, const ofFloatColor & color, float scale)
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