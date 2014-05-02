#include "cinder/app/AppBasic.h"
#include <list>
#include <boost/bind.hpp>
#include <memory>
#include <cmath>
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include <cassert>

using namespace ci;
using namespace ci::app;
using namespace std;

class RigidObject {
public:
	void push(Vec2f dir, float force) {

	}
private:
	Vec2f _position;
	float _weight;
};

class Physics {
public:
	void addRigidObject(RigidObject&& ro) {
		_rigidObjects.push_back(ro);
	}
private:
	std::vector<RigidObject> _rigidObjects;
};

// Parking space
class ParkingSpace {
public:
	ParkingSpace(float w, float l, Vec2f loc, Vec2f dir)
		: _width(w)
		, _length(l)
		, _location(loc)
		, _entryDir(dir.normalized())
	{ }

	void draw();
private:
	float _width;	 // width
	float _length;
	Vec2f _location; // entrance middle point
	Vec2f _entryDir; // entrance direction
};

void ParkingSpace::draw()
{
	gl::color(1.0f, 1.0f, 1.0f);

	Vec2f perpVec = _entryDir * (_width / 2.f);
	perpVec.rotate(static_cast<float>(M_PI / 2.));
	
	Vec2f leftStart = _location + perpVec;
	Vec2f leftEnd = leftStart - (_entryDir * _length);
	Vec2f rightStart = _location - perpVec;
	Vec2f rightEnd = rightStart - (_entryDir * _length);
	gl::drawLine(leftStart, leftEnd);
	gl::drawLine(rightStart, rightEnd);
	gl::drawLine(leftStart, rightStart);
}

// Car park is the actual map
class CarPark {
public:
	void addSingleParkingSpace(ParkingSpace&& ps) {
		_spaces.push_back(ps);
	}

	void addParkingSpaces(Vec2f from, Vec2f direction, size_t count, bool entryFromLeft = true, float width = 30.f, float length = 75.f) {
		direction.normalize();
		Vec2f entryVec = direction;
		entryVec.rotate(static_cast<float>(entryFromLeft ? M_PI / 2.f : -M_PI / 2.f));

		for (size_t i = 0; i < count; ++i) {			
			addSingleParkingSpace(ParkingSpace(width, length, from, entryVec));
			from += direction * width;
		}
	}

	void addDoubleOpenedParkingSpaces(Vec2f from, Vec2f direction, size_t count, float width = 30.f, float length = 75.f) {
		addParkingSpaces(from, direction, count, true, width, length);
		addParkingSpaces(from, direction, count, false, width, length);
	}

	void draw();
private:
	std::vector<ParkingSpace> _spaces;
};


void CarPark::draw()
{
	std::for_each(_spaces.begin(), _spaces.end(), std::mem_fn(&ParkingSpace::draw));
}



class Driver;
// Car
class Car {
public:
	Car() {
		if (!_image) {
			_image = gl::Texture(loadImage(boost::filesystem::path("Cossie-GTA1.png")));
		}
	}

	// v in [-1., 1.], acceleration from backward to forward
	void accelerate(float v) {

	}

	// v in [-1., 1.], direction from left to right
	void turn(float v) {

	}

	void setDriver(Driver* d) {
		_driver = d;
	}

	void draw() {
		if (_image) {
			gl::draw(_image, Rectf(50, 50, 50 + 189 * (25. / 108), 50 + 108 * (25. / 108)));
		}
	}
private:
	static gl::Texture	_image;
	Driver*				_driver = nullptr;
};

gl::Texture Car::_image;

// Driver drives the car
class Driver {
public:
	void drive(Car& car) {
		car.setDriver(this);
	}
};

class PlayerDriver : public Driver {
public:
};

class AIDriver : public Driver {
public:
};

CarPark	createTestCarPark() {
	CarPark cp;
	cp.addDoubleOpenedParkingSpaces(Vec2f(500., 100.), Vec2f(0., 1.), 15);
	cp.addDoubleOpenedParkingSpaces(Vec2f(200., 100.), Vec2f(0., 1.), 15);
	return cp;
}

// We'll create a new Cinder Application by deriving from the AppBasic class
class BasicApp : public AppBasic {
public:
	BasicApp()
	{

	}

	void	setup() override;

	void prepareSettings(Settings *settings) override {

		settings->setWindowSize(800, 600);
		settings->setFrameRate(60.0f);
	}

	void keyDown(KeyEvent event);
	void draw();
private:
	PlayerDriver		_player;
	CarPark				_carpark;
	std::vector<Car>	_cars;
};


void BasicApp::keyDown(KeyEvent event)
{
	if (event.getChar() == 'f')
		setFullScreen(!isFullScreen());
	if (event.getCode() == KeyEvent::KEY_LEFT) {
		console() << "left pressed" << std::endl;
	}
}



void BasicApp::draw()
{
	gl::clear(Color(0.1f, 0.1f, 0.15f));
	_carpark.draw();
	std::for_each(_cars.begin(), _cars.end(), std::mem_fn(&Car::draw));
}

void BasicApp::setup()
{
	_carpark = createTestCarPark();
	Car testCar;
	//AIDriver ai;
	_player.drive(testCar);
	_cars.push_back(testCar);
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(BasicApp, RendererGl)