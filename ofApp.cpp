#include "ofApp.h"


//  (c) Troy Perez - November 2 2022

// Intersect Ray with Plane  (wrapper on glm::intersect*
//

bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		setIntersectionPoint(r.evalPoint(dist));

		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1]
			&& point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

//--------------------------------------------------------------
//converts the current point on the plane to a pixel on texture map
//returns the color from the texture
ofColor Plane::textureMap(glm::vec3 p) {
	ofColor tex = ofColor(0);
	//ground plane
	if (normal == glm::vec3(0, 1, 0)) {
		float x = getIntersectionPoint().x - position.x;
		float y = getIntersectionPoint().z - position.z;

		float u = ofMap(x, position.x - getWidth() / 2, position.x + getWidth() / 2, 0, floortiles);
		float v = ofMap(y, position.z - getHeight() / 2, position.z + getHeight() / 2, 0, floortiles);

		int i = u * image.getWidth() - .5;
		int j = v * image.getHeight() - .5;

		if (i > 0 && j > 0) {
			tex = image.getColor(fmod(i, image.getWidth()), fmod(j, image.getHeight()));
		}
	}
	//wall plane
	else if (normal == glm::vec3(0, 0, 1)) {
		float x = getIntersectionPoint().x - position.x;
		float y = getIntersectionPoint().y - position.y;

		float u = ofMap(x, position.x - getWidth() / 2, position.x + getWidth() / 2, 0, walltiles);
		float v = ofMap(y, position.y + getHeight() / 2, position.y - getHeight() / 2, 0, walltiles);

		int i = u * image.getWidth() - .5;
		int j = v * image.getHeight() - .5;

		if (i > 0 && j > 0) {
			tex = image.getColor(fmod(i, image.getWidth()), fmod(j, image.getHeight()));
		}
	}
	return tex;
}

//--------------------------------------------------------------
//converts the point to a pixel on the texture specular map
//returns the specular color from the texture
ofColor Plane::specularTextureMap(glm::vec3 p) {
	ofColor tex = ofColor(0);
	//ground plane
	if (normal == glm::vec3(0, 1, 0)) {
		float x = getIntersectionPoint().x - position.x;
		float y = getIntersectionPoint().z - position.z;

		float u = ofMap(x, position.x - getWidth() / 2, position.x + getWidth() / 2, 0, floortiles);
		float v = ofMap(y, position.z - getHeight() / 2, position.z + getHeight() / 2, 0, floortiles);

		int i = u * imageSpec.getWidth() - .5;
		int j = v * imageSpec.getHeight() - .5;

		if (i > 0 && j > 0) {
			tex = imageSpec.getColor(fmod(i, imageSpec.getWidth()), fmod(j, imageSpec.getHeight()));
		}
	}
	//wall plane
	else if (normal == glm::vec3(0, 0, 1)) {
		float x = getIntersectionPoint().x - position.x;
		float y = getIntersectionPoint().y - position.y;

		float u = ofMap(x, position.x - getWidth() / 2, position.x + getWidth() / 2, 0, walltiles);
		float v = ofMap(y, position.y - getHeight() / 2, position.y + getHeight() / 2, 0, walltiles);

		int i = u * imageSpec.getWidth() - .5;
		int j = v * imageSpec.getHeight() - .5;

		if (i > 0 && j > 0) {
			tex = imageSpec.getColor(fmod(i, imageSpec.getWidth()), fmod(j, imageSpec.getHeight()));
		}
	}
	return tex;
}



//--------------------------------------------------------------
void ofApp::setup() {
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);

	groundTexture.load("wood_floor.jpg");
	groundTextureSpecular.load("wood_floor_spec.jpg");

	wallTexture.load("bricks_wall.jpg");
	wallTextureSpecular.load("bricks_wall_spec.jpg");

	gui.setup();
	gui.add(intensity.setup("Light intensity", .2, .05, 1));
	gui.add(power.setup("Phong p", 100, 10, 10000));
	bHide = true;

	theCam = &mainCam;
	mainCam.setDistance(10);
	mainCam.setNearClip(.1);
	previewCam.setFov(90);
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(glm::vec3(0, 0, -1));


	scene.clear();

	scene.push_back(new Plane(glm::vec3(-1, -3, 0), glm::vec3(0, 1, 0), ofColor::darkBlue, 12, 10));				//ground plane

	scene.push_back(new Plane(glm::vec3(-1, 2, -5), glm::vec3(0, 0, 1), ofColor::darkGray, 20, 10));	        	//wall plane

	scene.push_back(new Sphere(glm::vec3(-3, -1.5, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.75, -1.6, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.5, -1, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.75, -.9, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.5, -1.5, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-3.1, -1.25, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-3, -1, 1), .1, ofColor::darkRed));

	scene.push_back(new Sphere(glm::vec3(-2.45, -1.25, 1), .1, ofColor::darkRed));											//purple sphere


	scene.push_back(new Sphere(glm::vec3(-2.5, -1.7, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.5, -1.9, 1), .1, ofColor::darkRed));											//purple sphere

	scene.push_back(new Sphere(glm::vec3(-2.5, -2.1, 1), .1, ofColor::darkRed));											//purple sphere


	//scene.push_back(new Sphere(glm::vec3(-1, -1.5, 2), .5, ofColor::blue));												//blue sphere

	//scene.push_back(new Sphere(glm::vec3(-.5, -1.5, 0), .5, ofColor::darkGreen));											//green sphere


	light.clear();

	light.push_back(new Light(glm::vec3(100, 150, 150), .2));			//top right light

	light.push_back(new Light(glm::vec3(-20, 30, 45), .2));		//top left light

	light.push_back(new Light(glm::vec3(-5, -1, 20), .2));				//bottom light


	scene[0]->setImage(groundTexture);
	scene[0]->setImageSpec(groundTextureSpecular);


	scene[1]->setImage(wallTexture);
	scene[1]->setImageSpec(wallTextureSpecular);


	cout << "h to toggle GUI" << endl;
	cout << "t to start ray tracer" << endl;
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	ofSetDepthTest(true);

	theCam->begin();

	//draw all scene objects
	for (int i = 0; i < scene.size(); i++) {
		ofColor color = scene[i]->diffuseColor;
		ofSetColor(color);
		scene[i]->draw();
	}


	//draw all lights
	for (int i = 0; i < light.size(); i++) {
		light[i]->setIntensity(intensity);
		light[i]->draw();
	}

	theCam->end();


	if (!bHide) {
		ofSetDepthTest(false);
		gui.draw();
	}

	//draw render
	if (drawImage) {
		ofSetColor(ofColor::white);
		image.draw(0, 0);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		break;
	case 't':
		drawImage = false;
		rayTrace();
		drawImage = true;
		break;
	case 'h':
		bHide = !bHide;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}


//--------------------------------------------------------------
void ofApp::rayTrace() {

	cout << "drawing..." << endl;

	ofColor closest;
	float distance = FLT_MIN;
	float close = FLT_MAX;
	closestIndex = 0;
	for (int i = 0; i < image.getWidth(); i++) {
		for (int j = 0; j < image.getHeight(); j++) {
			background = true;																//reset variables every pixel
			distance = FLT_MIN;
			close = FLT_MAX;
			closestIndex = 0;

			float u = (i + .5) / image.getWidth();
			float v = 1 - (j + .5) / image.getHeight();

			Ray r = renderCam.getRay(u, v);
			for (int k = 0; k < scene.size(); k++) {
				if (scene[k]->intersect(r, scene[k]->intersectionPoint, glm::vec3(0, 1, 0))) {
					background = false;														//if intersected with scene object, pixel is not background

					distance = glm::distance(r.p, scene[k]->position);						//calculate distance of intersection
					if (distance < close)													//if current object is closest to viewplane
					{
						closestIndex = k;													//save index of closest object
						close = distance;													//set threshold to new closest distance
					}
				}
			}
			if (!background) {
				//get diffuse and specular
				ofColor diffuse = scene[closestIndex]->getDiffuse(r.evalPoint(close));
				ofColor specular = scene[closestIndex]->getSpecular(r.evalPoint(close));

				//add shading contribution
				closest = shade(r.evalPoint(close), scene[closestIndex]->getNormal(glm::vec3(0, 0, 0)), diffuse, close, specular, power, r);
				image.setColor(i, j, closest);
			}
			else if (background) {
				image.setColor(i, j, ofColor::black);
			}
		}
	}

	image.save("output.png");
	image.load("output.png");

	cout << "render saved" << endl;
}





//--------------------------------------------------------------
//adds shading contribution
//calculates shadows
//returns shaded color
ofColor ofApp::shade(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, const ofColor specular, float power, Ray r) {
	ofColor shaded = (0, 0, 0);
	glm::vec3 p1 = p;

	//loop through all lights
	for (int i = 0; i < light.size(); i++) {
		blocked = false;

		//test for shadows
		if (closestIndex < 2) {								//if the closest object is one of the planes

			for (int k = 0; k < 2; k++) {
				if (scene[k]->intersect(r, p1, glm::vec3(0, 1, 0))) {													//check if current point intersected with ground plane

					Ray shadowRay = Ray(scene[k]->getIntersectionPoint(), light[i]->position - scene[k]->getIntersectionPoint());

					//check all sphere objects
					for (int j = 2; j < scene.size(); j++) {
						if (scene[j]->intersect(shadowRay, p1, scene[j]->getNormal(p1))) {
							blocked = true;
						}
					}
				}
			}
		}
		if (!blocked) {
			//add shading contribution for current light
			shaded += phong(p, norm, diffuse, specular, power, distance, r, *light[i]);
		}
	}
	return shaded;
}

//--------------------------------------------------------------
//calculates all shading including:
// lambert
// phong
// ambient
//returns shaded color
ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power, float distance, Ray r, Light light) {
	ofColor phong = ofColor(0, 0, 0);
	glm::vec3 h = glm::vec3(0);

	glm::vec3 l = glm::normalize(light.position - p);
	glm::vec3 v = glm::normalize(renderCam.position - p);
	h = glm::normalize(l + v);

	float distance1 = glm::distance(light.position, p);


	phong += (ambient(diffuse)) + (lambert(p, norm, diffuse, distance1, r, light)) + (specular * (light.intensity / distance1 * distance1) * glm::pow(glm::max(zero, glm::dot(norm, h)), power));

	return phong;
}

// --------------------------------------------------------------
//calculates ambient shading
//returns shaded color
ofColor ofApp::ambient(const ofColor diffuse) {
	ofColor ambient = ofColor(0, 0, 0);
	ambient = .05 * diffuse;
	return ambient;
}


//--------------------------------------------------------------
//calculates lambert shading
//returns shaded color
ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, Light light) {
	ofColor lambert = ofColor(0, 0, 0);
	float distance1 = glm::distance(light.position, p);

	glm::vec3 l = glm::normalize(light.position - p);
	lambert += diffuse * (light.intensity / distance1 * distance1) * (glm::max(zero, glm::dot(norm, l)));

	return lambert;
}
