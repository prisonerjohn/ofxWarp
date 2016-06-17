#include "WarpBilinear.h"

namespace ofxWarp
{
	//--------------------------------------------------------------
	WarpBilinear::WarpBilinear(const ofFbo::Settings & fboSettings)
		: WarpBase(TYPE_BILINEAR)
		, fboSettings(fboSettings)
		, linear(false)
		, adaptive(true)
		, corners(0.0f, 0.0f, 1.0f, 1.0f)
		, resolutionX(0)
		, resolutionY(0)
		, resolution(16)  // higher value is coarser mesh
	{
		this->reset();

		this->shader.load(WarpBase::shaderPath + "WarpBilinear");
	}

	//--------------------------------------------------------------
	WarpBilinear::~WarpBilinear()
	{}

	//--------------------------------------------------------------
	void WarpBilinear::serialize(nlohmann::json & json)
	{
		WarpBase::serialize(json);

		json["resolution"] = this->resolution;
		json["linear"] = this->linear;
		json["adaptive"] = this->adaptive;
	}

	//--------------------------------------------------------------
	void WarpBilinear::deserialize(const nlohmann::json & json)
	{
		WarpBase::deserialize(json);

		this->resolution = json["resolution"];
		this->linear = json["linear"];
		this->adaptive = json["adaptive"];
	}

	//--------------------------------------------------------------
	void WarpBilinear::setSize(float width, float height)
	{
		WarpBase::setSize(width, height);
		this->fbo.clear();
	}

	//--------------------------------------------------------------
	void WarpBilinear::setFboSettings(const ofFbo::Settings & fboSettings)
	{
		this->fboSettings = fboSettings;
		this->fbo.clear();
	}

	//--------------------------------------------------------------
	void WarpBilinear::setLinear(bool linear)
	{
		this->linear = linear;
		this->dirty = true;
	}

	//--------------------------------------------------------------
	bool WarpBilinear::getLinear() const
	{
		return this->linear;
	}

	//--------------------------------------------------------------
	void WarpBilinear::setAdaptive(bool adaptive)
	{
		this->adaptive = adaptive;
		this->dirty = true;
	}

	//--------------------------------------------------------------
	bool WarpBilinear::getAdaptive() const
	{
		return this->adaptive;
	}

	//--------------------------------------------------------------
	void WarpBilinear::increaseResolution()
	{
		if (this->resolution < 64)
		{
			this->resolution += 4;
			this->dirty = true;
		}
	}

	//--------------------------------------------------------------
	void WarpBilinear::decreaseResolution()
	{
		if (this->resolution > 4)
		{
			this->resolution -= 4;
			this->dirty = true;
		}
	}

	//--------------------------------------------------------------
	int WarpBilinear::getResolution() const
	{
		return this->resolution;
	}

	//--------------------------------------------------------------
	void WarpBilinear::reset()
	{
		this->controlPoints.clear();
		for (auto x = 0; x < this->numControlsX; ++x) 
		{
			for (auto y = 0; y < this->numControlsY; ++y) 
			{
				this->controlPoints.push_back(ofVec2f(x / float(this->numControlsX - 1), y / float(this->numControlsY - 1)));
			}
		}

		this->dirty = true;
	}

	//--------------------------------------------------------------
	void WarpBilinear::begin()
	{
		this->setupFbo();

		this->fbo.begin();
	}

	//--------------------------------------------------------------
	void WarpBilinear::end()
	{
		this->fbo.end();

		// Draw flipped.
		auto srcBounds = ofRectangle(0.0f, 0.0f, this->fbo.getWidth(), this->fbo.getHeight());
		this->draw(this->fbo.getTexture(), srcBounds, this->getBounds());
	}

	//--------------------------------------------------------------
	void WarpBilinear::drawTexture(const ofTexture & texture, const ofRectangle & srcBounds, const ofRectangle & dstBounds)
	{
		// Clip against bounds.
		auto srcClip = srcBounds;
		auto dstClip = dstBounds;
		this->clip(srcClip, dstClip);

		// Set corner texture coordinates.
		if (texture.getTextureData().textureTarget == GL_TEXTURE_RECTANGLE_ARB)
		{
			this->setCorners(srcClip.getMinX(), srcClip.getMinY(), srcClip.getMaxX(), srcClip.getMaxY());
		}
		else
		{
			this->setCorners(srcClip.getMinX() / texture.getWidth(), srcClip.getMinY() / texture.getHeight(), srcClip.getMaxX() / texture.getWidth(), srcClip.getMaxY() / texture.getHeight());
		}

		this->setupVbo();

		auto currentColor = ofGetStyle().color;
		ofPushStyle();
		{
			auto wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
			ofDisableDepthTest();
			
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			
			// Adjust brightness.
			if (this->brightness < 1.0f)
			{
				currentColor *= this->brightness;
				ofSetColor(currentColor);
			}

			this->shader.begin();
			{
				this->shader.setUniformTexture("uTexture", texture, 1);
				this->shader.setUniform4f("uExtends", ofVec4f(this->width, this->height, this->width / float(this->numControlsX - 1), this->height / float(this->numControlsY - 1)));
				this->shader.setUniform3f("uLuminance", this->luminance);
				this->shader.setUniform3f("uGamma", this->gamma);
				this->shader.setUniform4f("uEdges", this->edges);
				this->shader.setUniform4f("uCorners", this->corners);
				this->shader.setUniform1f("uExponent", this->exponent);
				this->shader.setUniform1i("uEditing", this->editing);

				this->vbo.drawElements(GL_TRIANGLES, this->vbo.getNumIndices());
			}
			this->shader.end();

			if (wasDepthTest)
			{
				ofEnableDepthTest();
			}
		}
		ofPopStyle();
	}

	//--------------------------------------------------------------
	void WarpBilinear::drawControls()
	{
		if (this->editing && this->selectedIndex < this->controlPoints.size())
		{
			// Draw control points.
			for (auto i = 0; i < this->controlPoints.size(); ++i)
			{
				this->queueControlPoint(this->getControlPoint(i) * this->windowSize, i == this->selectedIndex);
			}

			this->drawControlPoints();
		}
	}

	//--------------------------------------------------------------
	void WarpBilinear::setupFbo()
	{
		if (!this->fbo.isAllocated() || this->fbo.getWidth() != this->width || this->fbo.getHeight() != this->height)
		{
			this->fboSettings.width = this->width;
			this->fboSettings.height = this->height;
			this->fbo.allocate(this->fboSettings);
		}
	}

	//--------------------------------------------------------------
	void WarpBilinear::setupVbo()
	{
		if (this->dirty)
		{
			if (this->adaptive)
			{
				// Determine a suitable mesh resolution based on the dimensions of the window
				// and the size of the mesh in pixels.
				auto meshBounds = this->getMeshBounds();
				this->setupMesh(meshBounds.getWidth() / this->resolution, meshBounds.getHeight() / this->resolution);
			}
			else
			{
				// Use a fixed mesh resolution.
				this->setupMesh(this->width / this->resolution, this->height / this->resolution);
			}
			this->updateMesh();
		}
	}

	//--------------------------------------------------------------
	void WarpBilinear::setupMesh(int resolutionX, int resolutionY)
	{
		// Convert from number of quads to number of vertices.
		++resolutionX;
		++resolutionY;

		// Find a value for resolutionX and resolutionY that can be evenly divided by numControlsX and numControlsY.
		if (this->numControlsX < resolutionX) {
			int dx = (resolutionX - 1) % (this->numControlsX - 1);
			if (dx >= (this->numControlsX / 2))
			{
				dx -= (this->numControlsX - 1);
			}
			resolutionX -= dx;
		}
		else {
			resolutionX = this->numControlsX;
		}

		if (this->numControlsY < resolutionY) {
			int dy = (resolutionY - 1) % (this->numControlsY - 1);
			if (dy >= (this->numControlsY / 2)) 
			{
				dy -= (this->numControlsY - 1);
			}
			resolutionY -= dy;
		}
		else {
			resolutionY = this->numControlsY;
		}

		this->resolutionX = resolutionX;
		this->resolutionY = resolutionY;

		int numVertices = (resolutionX * resolutionY);
		int numTriangles = 2 * (resolutionX - 1) * (resolutionY - 1);
		int numIndices = numTriangles * 3;

		// Build the static data.
		int i = 0;
		int j = 0;

		std::vector<ofIndexType> indices(numIndices);
		std::vector<ofVec2f> texCoords(numVertices);

		for (int x = 0; x < resolutionX; ++x) 
		{
			for (int y = 0; y < resolutionY; ++y) 
			{
				// Index.
				if (((x + 1) < resolutionX) && ((y + 1) < resolutionY)) 
				{
					indices[i++] = (x + 0) * resolutionY + (y + 0);
					indices[i++] = (x + 1) * resolutionY + (y + 0);
					indices[i++] = (x + 1) * resolutionY + (y + 1);

					indices[i++] = (x + 0) * resolutionY + (y + 0);
					indices[i++] = (x + 1) * resolutionY + (y + 1);
					indices[i++] = (x + 0) * resolutionY + (y + 1);
				}

				// Tex Coord.
				float tx = ofLerp(this->corners.x, this->corners.z, x / (float)(this->resolutionX - 1));
				float ty = ofLerp(this->corners.y, this->corners.w, y / (float)(this->resolutionY - 1));
				texCoords[j++] = ofVec2f(tx, ty);
			}
		}

		// Build placeholder data.
		std::vector<ofVec3f> positions(this->resolutionX * this->resolutionY);

		// Build mesh.
		this->vbo.clear();
		this->vbo.setVertexData(positions.data(), positions.size(), GL_STATIC_DRAW);
		this->vbo.setTexCoordData(texCoords.data(), texCoords.size(), GL_STATIC_DRAW);
		this->vbo.setIndexData(indices.data(), indices.size(), GL_STATIC_DRAW);

		this->dirty = true;
	}

	// Mapped buffer seems to be a *tiny* bit faster.
#define USE_MAPPED_BUFFER 1

	//--------------------------------------------------------------
	void WarpBilinear::updateMesh()
	{
		if (!this->vbo.getIsAllocated() || !this->dirty) return;
		
		ofVec2f pt;
		float u, v;
		int col, row;

		std::vector<ofVec2f> cols, rows;

#if USE_MAPPED_BUFFER
		auto vertexBuffer = this->vbo.getVertexBuffer();
		auto mappedMesh = (ofVec3f *)vertexBuffer.map(GL_WRITE_ONLY);
#else
		std::vector<ofVec3f> positions(this->resolutionX * this->resolutionY);
		auto index = 0;
#endif

		for (auto x = 0; x < this->resolutionX; ++x) 
		{
			for (auto y = 0; y < this->resolutionY; ++y) 
			{
				// Transform coordinates to [0..numControls]
				u = x * (this->numControlsX - 1) / (float)(this->resolutionX - 1);
				v = y * (this->numControlsY - 1) / (float)(this->resolutionY - 1);

				// Determine col and row.
				col = (int)u;
				row = (int)v;

				// Normalize coordinates to [0..1]
				u -= col;
				v -= row;

				if (this->linear) 
				{
					// Perform linear interpolation.
					auto p1 = (1.0f - u) * this->getPoint(col, row) + u * this->getPoint(col + 1, row);
					auto p2 = (1.0f - u) * this->getPoint(col, row + 1) + u * this->getPoint(col + 1, row + 1);
					pt = ((1.0f - v) * p1 + v * p2) * this->windowSize;
				}
				else {
					// Perform bicubic interpolation.
					rows.clear();
					for (int i = -1; i < 3; ++i) 
					{
						cols.clear();
						for (int j = -1; j < 3; ++j) 
						{
							cols.push_back(this->getPoint(col + i, row + j));
						}
						rows.push_back(this->cubicInterpolate(cols, v));
					}
					pt = this->cubicInterpolate(rows, u) * this->windowSize;
				}

#if USE_MAPPED_BUFFER
				*mappedMesh++ = ofVec3f(pt.x, pt.y, 0.0f);
#else
				positions[index++] = ofVec3f(pt.x, pt.y, 0.0f);
#endif
			}
		}

#if USE_MAPPED_BUFFER
		vertexBuffer.unmap();
#else
		this->vbo.updateVertexData(positions.data(), positions.size());
#endif

		this->dirty = false;
	}

	//--------------------------------------------------------------
	ofVec2f WarpBilinear::getPoint(int col, int row) const
	{
		auto maxCol = this->numControlsX - 1;
		auto maxRow = this->numControlsY - 1;

		// Here's the magic: extrapolate points beyond the edges.
		if (col < 0)
		{
			return (2.0f * getPoint(0, row) - getPoint(0 - col, row));
		}
		if (row < 0)
		{
			return (2.0f * getPoint(col, 0) - getPoint(col, 0 - row));
		}
		if (col > maxCol)
		{
			return (2.0f * getPoint(maxCol, row) - getPoint(2 * maxCol - col, row));
		}
		if (row > maxRow)
		{
			return (2.0f * getPoint(col, maxRow) - getPoint(col, 2 * maxRow - row));
		}

		// Points on the edges or within the mesh can simply be looked up.
		auto idx = (col * this->numControlsY) + row;
		return this->controlPoints[idx];
	}

	//--------------------------------------------------------------
	// From http://www.paulinternet.nl/?page=bicubic : fast catmull-rom calculation
	ofVec2f WarpBilinear::cubicInterpolate(const std::vector<ofVec2f> & knots, float t) const
	{
		assert(knots.size() >= 4);

		return (knots[1] + 0.5f * t * (knots[2] - knots[0] + t * (2.0f * knots[0] - 5.0f * knots[1] + 4.0f * knots[2] - knots[3] + t * (3.0f * (knots[1] - knots[2]) + knots[3] - knots[0]))));
	}

	//--------------------------------------------------------------
	void WarpBilinear::setNumControlsX(int n)
	{
		// There should be a minimum of 2 control points.
		n = MAX(2, n);

		// Prevent overflow.
		if ((n * this->numControlsY) > MAX_NUM_CONTROL_POINTS) return;

		// Create a list of new points.
		std::vector<ofVec2f> tempPoints(n * this->numControlsY);

		// Perform spline fitting.
		for (auto row = 0; row < this->numControlsY; ++row) {
			if (this->linear) 
			{
				// Construct piece-wise linear spline.
				ofPolyline polyline;
				for (auto col = 0; col < this->numControlsX; ++col)
				{
					polyline.lineTo(getPoint(col, row));
				}

				// Calculate position of new control points.
				auto step = 1.0f / (n - 1);
				for (auto col = 0; col < n; ++col)
				{
					auto idx = (col * this->numControlsY) + row;
					tempPoints[idx] = polyline.getPointAtPercent(col * step);
				}
			}
			else 
			{
				// Construct piece-wise catmull-rom spline.
				ofPolyline polyline;
				for (auto col = 0; col < this->numControlsX; ++col)
				{
					auto p0 = getPoint(col - 1, row);
					auto p1 = getPoint(col, row);
					auto p2 = getPoint(col + 1, row);
					auto p3 = getPoint(col + 2, row);

					// Control points according to an optimized Catmull-Rom implementation
					auto b1 = p1 + (p2 - p0) / 6.0f;
					auto b2 = p2 - (p3 - p1) / 6.0f;

					if (col == 0)
					{
						polyline.lineTo(p1);
					}
					
					polyline.curveTo(p1);

					if (col < (this->numControlsX - 1)) 
					{
						polyline.curveTo(b1); 
						polyline.curveTo(b2);
					}
					else
					{
						polyline.lineTo(p1);
					}
				}

				// Calculate position of new control points.
				auto step = 1.0f / (n - 1);
				for (auto col = 0; col < n; ++col)
				{
					auto idx = (col * this->numControlsY) + row; 
					tempPoints[idx] = polyline.getPointAtPercent(col * step);
				}
			}
		}

		// Save new control points.
		this->controlPoints = tempPoints;
		this->numControlsX = n;

		// Find new closest control point.
		float distance;
		this->selectedIndex = this->findClosestControlPoint(ofVec2f(ofGetMouseX(), ofGetMouseY()), &distance);

		this->dirty = true;
	}

	//--------------------------------------------------------------
	void WarpBilinear::setNumControlsY(int n)
	{
		// There should be a minimum of 2 control points.
		n = MAX(2, n);

		// Prevent overflow.
		if ((this->numControlsX * n) > MAX_NUM_CONTROL_POINTS) return;

		// Create a list of new points.
		std::vector<ofVec2f> tempPoints(this->numControlsX * n);

		// Perform spline fitting
		for (auto col = 0; col < this->numControlsX; ++col) {
			if (this->linear) 
			{
				// Construct piece-wise linear spline.
				ofPolyline polyline;
				for (auto row = 0; row < this->numControlsY; ++row)
				{
					polyline.lineTo(getPoint(col, row));
				}

				// Calculate position of new control points.
				float step = 1.0f / (n - 1);
				for (auto row = 0; row < n; ++row) 
				{
					auto idx = (col * n) + row; 
					tempPoints[idx] = polyline.getPointAtPercent(row * step);
				}
			}
			else 
			{
				// Construct piece-wise catmull-rom spline.
				ofPolyline polyline;
				for (auto row = 0; row < this->numControlsY; ++row)
				{
					auto p0 = getPoint(col, row - 1);
					auto p1 = getPoint(col, row);
					auto p2 = getPoint(col, row + 1);
					auto p3 = getPoint(col, row + 2);

					// Control points according to an optimized Catmull-Rom implementation
					auto b1 = p1 + (p2 - p0) / 6.0f;
					auto b2 = p2 - (p3 - p1) / 6.0f;

					if (row == 0)
					{
						polyline.lineTo(p1);
					}

					polyline.curveTo(p1);

					if (row < (this->numControlsY - 1)) 
					{
						polyline.curveTo(b1);
						polyline.curveTo(b2);
					}
					else
					{
						polyline.lineTo(p1);
					}
				}

				// Calculate position of new control points.
				auto step = 1.0f / (n - 1);
				for (auto row = 0; row < n; ++row) 
				{
					auto idx = (col * n) + row;
					tempPoints[idx] = polyline.getPointAtPercent(row * step);
				}
			}
		}

		// Save new control points.
		this->controlPoints = tempPoints;
		this->numControlsY = n;

		// Find new closest control point.
		float distance;
		this->selectedIndex = this->findClosestControlPoint(ofVec2f(ofGetMouseX(), ofGetMouseY()), &distance);

		this->dirty = true;
	}

	//--------------------------------------------------------------
	ofRectangle WarpBilinear::getMeshBounds() const
	{
		auto min = ofVec2f(1.0f);
		auto max = ofVec2f(0.0f);

		for (auto & pt : this->controlPoints) 
		{
			min.x = MIN(pt.x, min.x);
			min.y = MIN(pt.y, min.y);
			max.x = MAX(pt.x, max.x);
			max.y = MAX(pt.y, min.y);
		}

		return ofRectangle(min * this->windowSize, max * this->windowSize);
	}

	//--------------------------------------------------------------
	void WarpBilinear::setCorners(float left, float top, float right, float bottom)
	{
		this->dirty |= (left != this->corners.x || top != this->corners.y || right != this->corners.z || bottom != this->corners.w);
		if (!this->dirty) return;

		this->corners = ofVec4f(left, top, right, bottom);
	}

	//--------------------------------------------------------------
	void WarpBilinear::rotateClockwise()
	{
		ofLogWarning("WarpBilinear::rotateClockwise") << "Not implemented!";
	}

	//--------------------------------------------------------------
	void WarpBilinear::rotateCounterclockwise()
	{
		ofLogWarning("WarpBilinear::rotateCounterclockwise") << "Not implemented!";
	}

	//--------------------------------------------------------------
	void WarpBilinear::flipHorizontal()
	{
		std::vector<ofVec2f> flippedPoints;
		for (int x = this->numControlsX - 1; x >= 0; --x)
		{
			for (int y = 0; y < this->numControlsY; ++y)
			{
				auto i = (x * this->numControlsY + y);
				flippedPoints.push_back(this->controlPoints[i]);
			}
		}
		this->controlPoints = flippedPoints;
		this->dirty = true;

		// Find new closest control point.
		float distance;
		this->selectedIndex = this->findClosestControlPoint(ofVec2f(ofGetMouseX(), ofGetMouseY()), &distance);
	}

	//--------------------------------------------------------------
	void WarpBilinear::flipVertical()
	{
		std::vector<ofVec2f> flippedPoints;
		for (int x = 0; x < this->numControlsX; ++x)
		{
			for (int y = this->numControlsY - 1; y >= 0; --y)
			{
				auto i = (x * this->numControlsY + y);
				flippedPoints.push_back(this->controlPoints[i]);
			}
		}
		this->controlPoints = flippedPoints;
		this->dirty = true;

		// Find new closest control point.
		float distance;
		this->selectedIndex = this->findClosestControlPoint(ofVec2f(ofGetMouseX(), ofGetMouseY()), &distance);
	}
}