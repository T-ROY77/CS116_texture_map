
//  
//  (c) Troy Perez - November 2 2022
//
#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include <glm/gtx/intersect.hpp>

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual glm::vec3 getNormal(const glm::vec3& p) { return glm::vec3(0); }
	virtual glm::vec3 getIntersectionPoint() { return glm::vec3(1); }
	virtual void setImage(ofImage i) {}
	virtual void setImageSpec(ofImage i) {}
	virtual ofColor getDiffuse(glm::vec3 p) { return diffuseColor; }
	virtual ofColor getSpecular(glm::vec3 p) { return specularColor; }
	virtual float getWidth() { return width; }
	virtual float getHeight() { return height; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 intersectionPoint;
	float width;
	float height;
	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	ofImage image;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		bool intersect = (glm::intersectRaySphere(ray.p, glm::normalize(ray.d), position, radius, point, normal));
		setNormal(normal);
		intersectionPoint = point;
		return intersect;
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	void setNormal(const glm::vec3& p) { normal = p; }

	glm::vec3 getNormal(const glm::vec3& p) { return glm::normalize(normal); }

	ofColor getDiffuse(glm::vec3 p) { return diffuseColor; }

	glm::vec3 normal;

	float radius = 1.0;
};


class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) { position = p; intensity = i; }
	Light() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofSetColor(ofColor::gray);
		ofDrawSphere(position, radius);
	}
	void setIntensity(float i) {
		intensity = i;
	}
	float radius = .5;
	float intensity = 0.0;
};


//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	void draw() { }

};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse,
		float w, float h) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	float sdf(const glm::vec3& p);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }
	glm::vec3 getIntersectionPoint() { return this->intersectionPoint; }
	float getWidth() { return width; }
	float getHeight() { return height; }
	ofColor textureMap(glm::vec3 p);
	ofColor specularTextureMap(glm::vec3 p);

	ofColor getDiffuse(glm::vec3 p) {
		if (hasTexture) {
			return textureMap(p);
		}
		else {
			return diffuseColor;
		}
	}

	ofColor getSpecular(glm::vec3 p) {
		if (hasTextureSpecular) {
			return specularTextureMap(p);
		}
		else {
			return specularColor;
		}
	}

	void setImage(ofImage i) {
		image = i;
		hasTexture = true;
	}
	void setImageSpec(ofImage i) {
		imageSpec = i;
		hasTextureSpecular = true;
	}
	void setIntersectionPoint(const glm::vec3& p) { intersectionPoint = p; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.draw();
	}
	ofPlanePrimitive plane;
	glm::vec3 normal;
	float width;
	float height;
	glm::vec3 intersectionPoint;
	ofImage image;
	ofImage imageSpec;

	bool hasTexture = false;
	bool hasTextureSpecular = false;

	int floortiles = 3;
	int walltiles = 3;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	void drawGrid();
	void drawAxis(glm::vec3 position);
	ofColor ambient(ofColor diffuse);
	ofColor lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, Light light);
	ofColor phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power, float distance, Ray r, Light light);
	ofColor shade(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, const ofColor specular, float power, Ray r);
	ofColor textureMap(glm::vec3 p);

	const float zero = 0.0;

	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera* theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image through
	//
	RenderCam renderCam;
	ofImage image;


	ofImage groundTexture;
	ofImage groundTextureSpecular;

	ofImage wallTexture;
	ofImage wallTextureSpecular;


	//object vectors
	//
	vector<SceneObject*> scene;
	vector<Light*> light;

	int imageWidth = 1200;
	int imageHeight = 800;

	int closestIndex = 0;

	//state variables
	//
	bool drawImage = false;
	bool trace = false;
	bool background = true;
	bool blocked = false;
	bool texture = false;

	//GUI
	//
	ofxFloatSlider power;
	ofxFloatSlider intensity;
	ofxPanel gui;

};