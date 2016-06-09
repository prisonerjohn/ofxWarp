#include "Warp.h"

#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"

namespace ofxWarping
{
	//--------------------------------------------------------------
	Warp::Warp(Type type)
		: type(type)
		, editing(false)
		, dirty(true)
		, brightness(1.0f)
		, width(640.0f)
		, height(480.0f)
		, numControlsX(2)
		, numControlsY(2)
		, selectedIndex(-1)
		, selectedTime(0.0f)
		, luminance(0.5f)
		, gamma(1.0f)
		, exponent(2.0f)
		, edges(0.0f)
	{
		this->windowSize = ofVec2f(ofGetWidth(), ofGetHeight());
	}
	
	//--------------------------------------------------------------
	Warp::~Warp()
	{}

	//--------------------------------------------------------------
	Warp::Type Warp::getType() const
	{
		return this->type;
	}

	//--------------------------------------------------------------
	void Warp::serialize(nlohmann::json & json)
	{
		// Main parameters.
		json["type"] = this->type;
		json["brightness"] = this->brightness;

		// Warp parameters.
		{
			auto & jsonWarp = json["warp"];

			jsonWarp["columns"] = this->numControlsX;
			jsonWarp["rows"] = this->numControlsY;

			std::vector<std::string> points;
			for (auto & controlPoint : this->controlPoints)
			{
				std::ostringstream oss;
				oss << controlPoint;
				points.push_back(oss.str());
			}
			jsonWarp["control points"] = points;
		}

		// Blend parameters.
		{
			auto & jsonBlend = json["blend"];

			jsonBlend["exponent"] = this->exponent;

			std::ostringstream oss;
			oss << this->edges;
			jsonBlend["edges"] = oss.str();

			oss.str("");
			oss << this->gamma;
			jsonBlend["gamma"] = oss.str();

			oss.str("");
			oss << this->luminance;
			jsonBlend["luminance"] = oss.str();
		}
	}
	
	//--------------------------------------------------------------
	void Warp::deserialize(const nlohmann::json & json)
	{
		// Main parameters.
		int typeAsInt = json["type"];
		this->type = (Type)typeAsInt;
		this->brightness = json["brightness"];

		// Warp parameters.
		{
			const auto & jsonWarp = json["warp"];

			this->numControlsX = jsonWarp["columns"];
			this->numControlsY = jsonWarp["rows"];

			this->controlPoints.clear();
			for (const auto & jsonPoint : jsonWarp["control points"])
			{
				ofVec2f controlPoint;
				std::istringstream iss;
				iss.str(jsonPoint);
				iss >> controlPoint;
				this->controlPoints.push_back(controlPoint);
			}
		}

		// Blend parameters.
		{
			const auto & jsonBlend = json["blend"];

			this->exponent = jsonBlend["exponent"];

			std::istringstream iss;
			iss.str(jsonBlend["edges"]);
			iss >> this->edges;

			iss.str(jsonBlend["gamma"]);
			iss >> this->gamma;

			iss.str(jsonBlend["luminance"]);
			iss >> this->luminance;
		}

		this->dirty = true;
	}

	//--------------------------------------------------------------
	void Warp::setEditing(bool editing)
	{
		this->editing = editing;
	}
	
	//--------------------------------------------------------------
	void Warp::toggleEditing()
	{
		this->setEditing(!this->editing);
	}
	
	//--------------------------------------------------------------
	bool Warp::isEditing() const
	{
		return this->editing;
	}

	//--------------------------------------------------------------
	void Warp::setWidth(float width)
	{
		this->setSize(width, this->height);
	}

	//--------------------------------------------------------------
	float Warp::getWidth() const
	{
		return this->width;
	}

	//--------------------------------------------------------------
	void Warp::setHeight(float height)
	{
		this->setSize(this->width, height);
	}

	//--------------------------------------------------------------
	float Warp::getHeight() const
	{
		return this->height;
	}

	//--------------------------------------------------------------
	void Warp::setSize(float width, float height)
	{
		this->width = width;
		this->height = height;
		this->dirty = true;
	}
	
	//--------------------------------------------------------------
	void Warp::setSize(const ofVec2f & size)
	{
		this->setSize(size.x, size.y);
	}
	
	//--------------------------------------------------------------
	ofVec2f Warp::getSize() const
	{
		return ofVec2f(this->width, this->height);
	}

	//--------------------------------------------------------------
	ofRectangle Warp::getBounds() const
	{
		return ofRectangle(0, 0, this->width, this->height);
	}

	//--------------------------------------------------------------
	void Warp::setLuminance(float luminance)
	{
		this->luminance = ofVec3f(luminance);
	}
	
	//--------------------------------------------------------------
	void Warp::setLuminance(float red, float green, float blue)
	{
		this->luminance = ofVec3f(red, green, blue);
	}
	
	//--------------------------------------------------------------
	const ofVec3f & Warp::getLuminance() const
	{
		return this->luminance;
	}

	//--------------------------------------------------------------
	void Warp::setGamma(float gamma)
	{
		this->gamma = ofVec3f(gamma);
	}
	
	//--------------------------------------------------------------
	void Warp::setGamma(float red, float green, float blue)
	{
		this->gamma = ofVec3f(red, green, blue);
	}
	
	//--------------------------------------------------------------
	const ofVec3f & Warp::getGamma() const
	{
		return this->gamma;
	}

	//--------------------------------------------------------------
	void Warp::setExponent(float exponent)
	{
		this->exponent = exponent;
	}
	
	//--------------------------------------------------------------
	float Warp::getExponent() const
	{
		return this->exponent;
	}

	//--------------------------------------------------------------
	void Warp::setEdges(float left, float top, float right, float bottom)
	{
		this->setEdges(ofVec4f(left, top, right, bottom));
	}
	
	//--------------------------------------------------------------
	void Warp::setEdges(const ofVec4f & edges)
	{
		this->edges.x = ofClamp(edges.x * 0.5f, 0.0f, 1.0f);
		this->edges.y = ofClamp(edges.y * 0.5f, 0.0f, 1.0f);
		this->edges.z = ofClamp(edges.z * 0.5f, 0.0f, 1.0f);
		this->edges.w = ofClamp(edges.w * 0.5f, 0.0f, 1.0f);
	}
	
	//--------------------------------------------------------------
	ofVec4f Warp::getEdges() const
	{
		return this->edges * 2.0f;
	}

	//--------------------------------------------------------------
	void Warp::draw(const ofTexture & texture)
	{
		this->draw(texture, ofRectangle(0, 0, texture.getWidth(), texture.getHeight()), this->getBounds());
	}
	
	//--------------------------------------------------------------
	void Warp::draw(const ofTexture & texture, const ofRectangle & srcBounds)
	{
		this->draw(texture, srcBounds, this->getBounds());
	}

	//--------------------------------------------------------------
	void Warp::draw(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds)
	{
		this->drawTexture(texture, srcBounds, dstBounds);
		this->drawControls();
	}
	
	//--------------------------------------------------------------
	bool Warp::clip(ofRectangle & srcBounds, ofRectangle & dstBounds) const
	{
		bool clipped = false;

		ofVec4f srcVec = ofVec4f(srcBounds.getMinX(), srcBounds.getMinY(), srcBounds.getMaxX(), srcBounds.getMaxY());
		ofVec4f dstVec = ofVec4f(dstBounds.getMinX(), dstBounds.getMinY(), dstBounds.getMaxX(), dstBounds.getMaxY());
		
		float x1 = dstVec.x / this->width;
		float x2 = dstVec.z / this->width;
		float y1 = dstVec.y / this->height;
		float y2 = dstVec.w / this->height;

		if (x1 < 0.0f) 
		{
			dstVec.x = 0.0f;
			srcVec.x -= (x1 * srcBounds.getWidth());
			clipped = true;
		}
		else if (x1 > 1.0f) 
		{
			dstVec.x = this->width;
			srcVec.x -= ((1.0f / x1) * srcBounds.getWidth());
			clipped = true;
		}

		if (x2 < 0.0f) 
		{
			dstVec.z = 0.0f;
			srcVec.z -= static_cast<int32_t>(x2 * srcBounds.getWidth());
			clipped = true;
		}
		else if (x2 > 1.0f) {
			dstVec.z = this->width;
			srcVec.z -= ((1.0f / x2) * srcBounds.getWidth());
			clipped = true;
		}

		if (y1 < 0.0f) 
		{
			dstVec.y = 0.0f;
			srcVec.y -= (y1 * srcBounds.getHeight());
			clipped = true;
		}
		else if (y1 > 1.0f) 
		{
			dstVec.y = this->height;
			srcVec.y -= static_cast<int32_t>((1.0f / y1) * srcBounds.getHeight());
			clipped = true;
		}

		if (y2 < 0.0f) {
			dstVec.w = 0.0f;
			srcVec.w -= (y2 * srcBounds.getHeight());
			clipped = true;
		}
		else if (y2 > 1.0f) {
			dstVec.w = this->height;
			srcVec.w -= ((1.0f / y2) * srcBounds.getHeight());
			clipped = true;
		}

		srcBounds.set(srcVec.x, srcVec.y, srcVec.z - srcVec.x, srcVec.w - srcVec.y);
		dstBounds.set(dstVec.x, dstVec.y, dstVec.z - dstVec.x, dstVec.w - dstVec.y);

		return clipped;
	}

	//--------------------------------------------------------------
	const ofVec2f & Warp::getControlPoint(size_t index) const
	{
		if (index >= this->controlPoints.size()) return ofVec2f::zero();

		return this->controlPoints[index];
	}

	//--------------------------------------------------------------
	void Warp::setControlPoint(size_t index, const ofVec2f & pos)
	{
		if (index >= this->controlPoints.size()) return;

		this->controlPoints[index] = pos;
		this->dirty = true;
	}

	//--------------------------------------------------------------
	void Warp::moveControlPoint(size_t index, const ofVec2f & shift)
	{
		if (index >= this->controlPoints.size()) return;

		this->controlPoints[index] += shift;
		this->dirty = true;
	}

	//--------------------------------------------------------------
	size_t Warp::getNumControlPoints() const
	{
		return this->controlPoints.size();
	}

	//--------------------------------------------------------------
	size_t Warp::getSelectedControlPoint() const
	{
		return this->selectedIndex;
	}

	//--------------------------------------------------------------
	void Warp::selectControlPoint(size_t index)
	{
		if (index >= this->controlPoints.size() || index == this->selectedIndex) return;

		this->selectedIndex = index;
		this->selectedTime = ofGetElapsedTimef();
	}

	//--------------------------------------------------------------
	void Warp::deselectControlPoint()
	{
		this->selectedIndex = -1;
	}

	//--------------------------------------------------------------
	size_t Warp::findClosestControlPoint(const ofVec2f & pos, float * distance) const
	{
		size_t index;
		auto minDistance = std::numeric_limits<float>::max();

		for (auto i = 0; i < this->controlPoints.size(); ++i)
		{
			auto candidate = pos.distance(this->getControlPoint(i) * this->windowSize);
			if (candidate < minDistance)
			{
				minDistance = candidate;
				index = i;
			}
		}

		*distance = minDistance;
		return index;
	}

	//--------------------------------------------------------------
	void Warp::queueControlPoint(const ofVec2f & pos, bool selected, bool attached)
	{
		if (selected && attached) 
		{
			queueControlPoint(pos, ofFloatColor(0.0f, 0.8f, 0.0f));
		}
		else if (selected) 
		{
			auto scale = 0.9f + 0.2f * sinf(6.0f * (ofGetElapsedTimef() - this->selectedTime));
			queueControlPoint(pos, ofFloatColor(0.9f, 0.9f, 0.9f), scale);
		}
		else if (attached) 
		{
			queueControlPoint(pos, ofFloatColor(0.0f, 0.4f, 0.0f));
		}
		else 
		{
			queueControlPoint(pos, ofFloatColor(0.4f, 0.4f, 0.4f));
		}
	}
	
	//--------------------------------------------------------------
	void Warp::queueControlPoint(const ofVec2f & pos, const ofFloatColor & color, float scale)
	{
		if (this->controlData.size() < MAX_NUM_CONTROL_POINTS)
		{
			this->controlData.emplace_back(ControlData(pos, color, scale));
		}
	}

	//--------------------------------------------------------------
	void Warp::setupControlPoints()
	{
		if (this->controlMesh.getVertices().empty())
		{
			// Set up the vbo mesh.
			ofPolyline unitCircle;
			unitCircle.arc(ofVec3f::zero(), 1.0f, 1.0f, 0.0f, 360.0f, 18);
			const auto & circlePoints = unitCircle.getVertices();
			static const auto radius = 15.0f;
			static const auto halfVec = ofVec2f(0.5f);
			this->controlMesh.clear();
			this->controlMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			this->controlMesh.setUsage(GL_STATIC_DRAW);
			this->controlMesh.addVertex(ofVec2f::zero());
			this->controlMesh.addTexCoord(halfVec);
			for (auto & pt : circlePoints)
			{
				this->controlMesh.addVertex(pt * radius);
				this->controlMesh.addTexCoord(pt * 0.5f + halfVec);
			}

			// Set up per-instance data to the vbo.
			std::vector<ControlData> instanceData;
			instanceData.resize(MAX_NUM_CONTROL_POINTS);

			this->controlMesh.getVbo().setAttributeData(INSTANCE_POS_SCALE_ATTRIBUTE, (float *)&instanceData[0].pos, 4, instanceData.size(), GL_STREAM_DRAW, sizeof(ControlData));
			this->controlMesh.getVbo().setAttributeDivisor(INSTANCE_POS_SCALE_ATTRIBUTE, 1);
			this->controlMesh.getVbo().setAttributeData(INSTANCE_COLOR_ATTRIBUTE, (float *)&instanceData[0].color, 4, instanceData.size(), GL_STREAM_DRAW, sizeof(ControlData));
			this->controlMesh.getVbo().setAttributeDivisor(INSTANCE_COLOR_ATTRIBUTE, 1);
		}

		if (!this->controlShader.isLoaded())
		{
			// Load the shader.
			this->controlShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/ofxWarping/ControlPoint.vert");
			this->controlShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/ofxWarping/ControlPoint.frag");
			this->controlShader.bindAttribute(INSTANCE_POS_SCALE_ATTRIBUTE, "iPositionScale");
			this->controlShader.bindAttribute(INSTANCE_COLOR_ATTRIBUTE, "iColor");
			this->controlShader.bindDefaults();
			this->controlShader.linkProgram();
		}
	}

	//--------------------------------------------------------------
	void Warp::drawControlPoints()
	{
		this->setupControlPoints();

		if (!this->controlData.empty())
		{
			this->controlMesh.getVbo().updateAttributeData(INSTANCE_POS_SCALE_ATTRIBUTE, (float *)&this->controlData[0].pos, this->controlData.size());
			this->controlMesh.getVbo().updateAttributeData(INSTANCE_COLOR_ATTRIBUTE, (float *)&this->controlData[0].color, this->controlData.size());
		
			this->controlShader.begin();
			{
				this->controlMesh.drawInstanced(OF_MESH_FILL, this->controlData.size());
			}
			this->controlShader.end();
		}

		this->controlData.clear();
	}

	//--------------------------------------------------------------
	bool Warp::onMouseMoved(const ofVec2f & pos)
	{
		return false;
	}
	
	//--------------------------------------------------------------
	bool Warp::onMousePressed(const ofVec2f & pos)
	{
		if (!this->editing || this->selectedIndex >= this->controlPoints.size()) return false;

		// Calculate offset by converting control point from normalized to screen space.
		ofVec2f screenPoint = (this->getControlPoint(this->selectedIndex) * this->windowSize);
		this->selectedOffset = pos - screenPoint;

		return true;
	}
	
	//--------------------------------------------------------------
	bool Warp::onMouseDragged(const ofVec2f & pos)
	{
		if (!this->editing || this->selectedIndex >= this->controlPoints.size()) return false;

		// Set control point in normalized space.
		ofVec2f screenPoint = pos - this->selectedOffset;
		this->setControlPoint(this->selectedIndex, screenPoint / this->windowSize);

		this->dirty = true;

		return true;
	}
	
	//--------------------------------------------------------------
	bool Warp::onMouseReleased(const ofVec2f & pos)
	{
		return false;
	}

	//--------------------------------------------------------------
	bool Warp::onKeyPressed(int key)
	{
		if (!this->editing || this->selectedIndex >= this->controlPoints.size()) return false;

		if (key == OF_KEY_TAB)
		{
			// Select the next of previous (+ SHIFT) control point.
			size_t nextIndex;
			if (ofGetKeyPressed(OF_KEY_SHIFT))
			{
				if (this->selectedIndex == 0)
				{
					nextIndex = this->controlPoints.size() - 1;
				}
				else
				{
					nextIndex = this->selectedIndex - 1;
				}
			}
			else
			{
				nextIndex = (this->selectedIndex + 1) % this->controlPoints.size();
			}
			this->selectControlPoint(nextIndex);
		}
		else if (key == OF_KEY_UP || key == OF_KEY_DOWN || key == OF_KEY_LEFT || key == OF_KEY_RIGHT)
		{
			auto step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
			auto shift = ofVec2f::zero();
			if (key == OF_KEY_UP)
			{
				shift.y = -step / this->windowSize.y;
			}
			if (key == OF_KEY_DOWN)
			{
				shift.y = step / this->windowSize.y;
			}
			if (key == OF_KEY_LEFT)
			{
				shift.x = -step / this->windowSize.x;
			}
			else
			{
				shift.x = step / this->windowSize.x;
			}
			this->moveControlPoint(this->selectedIndex, shift);
		}
		else if (key == '-')
		{
			this->brightness = MAX(0.0f, this->brightness - 0.01f);
		}
		else if (key == '+')
		{
			this->brightness = MIN(1.0f, this->brightness + 0.01f);
		}
		else if (key == 'r')
		{
			this->reset();
		}
		else
		{
			return false;
		}

		return true;
	}
	
	//--------------------------------------------------------------
	bool Warp::onKeyReleased(int key)
	{
		return false;
	}

	//--------------------------------------------------------------
	bool Warp::onWindowResized(int width, int height)
	{
		this->windowSize = ofVec2f(width, height);
		this->dirty = true;

		return true;
	}

	//--------------------------------------------------------------
	void Warp::selectClosestControlPoint(const std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		size_t warpIdx = -1;
		size_t pointIdx = -1;
		auto distance = std::numeric_limits<float>::max();

		// Find warp and distance to closest control point.
		for (int i = warps.size() - 1; i >= 0; --i)
		{
			float candidate;
			auto idx = warps[i]->findClosestControlPoint(pos, &candidate);
			if (candidate < distance)
			{
				distance = candidate;
				pointIdx = idx;
				warpIdx = i;
			}
		}

		// Select the closest control point and deselect all others.
		for (int i = warps.size() - 1; i >= 0; --i)
		{
			if (i == warpIdx)
			{
				warps[i]->selectControlPoint(pointIdx);
			}
			else
			{
				warps[i]->deselectControlPoint();
			}
		}
	}

	//--------------------------------------------------------------
	bool Warp::handleMouseMoved(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		// Find and select closest control point.
		Warp::selectClosestControlPoint(warps, pos);

		return false;
	}
	
	//--------------------------------------------------------------
	bool Warp::handleMousePressed(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		// Find and select closest control point.
		Warp::selectClosestControlPoint(warps, pos);

		for (int i = warps.size() - 1; i >= 0; --i)
		{
			if (warps[i]->onMousePressed(pos))
			{
				return true;
			}
		}

		return false;
	}
	
	//--------------------------------------------------------------
	bool Warp::handleMouseDragged(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		for (int i = warps.size() - 1; i >= 0; --i)
		{
			if (warps[i]->onMouseDragged(pos))
			{
				return true;
			}
		}

		return false;
	}
	
	//--------------------------------------------------------------
	bool Warp::handleMouseReleased(std::vector<std::shared_ptr<Warp>> & warps, const ofVec2f & pos)
	{
		return false;
	}

	//--------------------------------------------------------------
	bool Warp::handleKeyPressed(std::vector<std::shared_ptr<Warp>> & warps, int key)
	{
		for (auto warp : warps)
		{
			if (warp->onKeyPressed(key))
			{
				return true;
			}
		}

		return false;
	}
	
	//--------------------------------------------------------------
	bool Warp::handleKeyReleased(std::vector<std::shared_ptr<Warp>> & warps, int key)
	{
		return false;
	}

	//--------------------------------------------------------------
	bool Warp::handleWindowResized(std::vector<std::shared_ptr<Warp>> & warps, int width, int height)
	{
		for (auto warp : warps)
		{
			warp->onWindowResized(width, height);
		}

		return false;
	}

	//--------------------------------------------------------------
	std::vector<std::shared_ptr<Warp>> Warp::loadSettings(const std::string & filePath)
	{
		std::vector<std::shared_ptr<Warp>> warps;

		auto file = ofFile(filePath, ofFile::ReadOnly);
		if (!file.exists())
		{
			ofLogWarning("Warp::loadSettings") << "File not found at path " << filePath;
			return warps;
		}

		nlohmann::json json;
		file >> json;
		for (auto & jsonWarp : json["warps"])
		{
			std::shared_ptr<Warp> warp;

			int typeAsInt = jsonWarp["type"];
			Type type = (Type)typeAsInt;
			switch (type)
			{
			case TYPE_BILINEAR:
				warp = std::make_shared<WarpBilinear>();
				break;

			case TYPE_PERSPECTIVE:
				warp = std::make_shared<WarpPerspective>();
				break;

			case TYPE_PERSPECTIVE_BILINEAR:
				warp = std::make_shared<WarpPerspectiveBilinear>();
				break;

			default:
				ofLogWarning("Warp::loadSettings") << "Unrecognized Warp type " << type;
			}

			if (warp)
			{
				warp->deserialize(jsonWarp);
				warps.push_back(warp);
			}
		}

		return warps;
	}
	
	//--------------------------------------------------------------
	void Warp::saveSettings(const std::vector<std::shared_ptr<Warp>> & warps, const std::string & filePath)
	{
		std::vector<nlohmann::json> jsonWarps;
		for (auto warp : warps)
		{
			nlohmann::json jsonWarp;
			warp->serialize(jsonWarp);
			jsonWarps.push_back(jsonWarp);
		}
		
		nlohmann::json json;
		json["warps"] = jsonWarps;

		auto file = ofFile(filePath, ofFile::WriteOnly);
		file << json;
	}
}