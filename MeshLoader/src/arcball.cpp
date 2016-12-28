#include "arcball.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <iostream>

static Eigen::Vector3d mapToSphere(Eigen::Vector2d const p, double screenSpaceRadius) {
	double l = sqrt( p[0] * p[0] + p[1] * p[1] );
	if (l <= screenSpaceRadius) {
		return Eigen::Vector3d(p[0], p[1], sqrt(screenSpaceRadius * screenSpaceRadius - (p[0] * p[0]  + p[1] * p[1] )));
	}
	else {
		return screenSpaceRadius / l * Eigen::Vector3d(p[0], p[1], 0.0);
	}
}

static Eigen::Vector3d mapToSphere(
	Eigen::Matrix4d invProjMatrix, 
	Eigen::Vector2d const p, 
	Eigen::Vector3d viewSphereCenter, 
	double viewSpaceRadius) {

	Eigen::Vector4d viewP = invProjMatrix * Eigen::Vector4d(p(0), p(1), -1.0, 1.0);
	//viewP(0) /= viewP(3);
	//viewP(1) /= viewP(3);
	//viewP(2) /= viewP(3);

	Eigen::Vector3d rayOrigin(0.0, 0.0, 0.0);
	Eigen::Vector3d rayDir(viewP(0), viewP(1), viewP(2));
	rayDir.normalize();

	Eigen::Vector3d CO = rayOrigin - viewSphereCenter;

	double a = 1.0;
	double bPrime = CO.dot(rayDir);
	double c = CO.dot(CO) - viewSpaceRadius * viewSpaceRadius;

	double delta = bPrime * bPrime - a * c;

	if (delta < 0.0) {
		double t = -CO.z() / rayDir.z();
		Eigen::Vector3d point = rayOrigin + t * rayDir;
		Eigen::Vector3d dir = point - viewSphereCenter;
		dir.normalize();
		return viewSphereCenter + dir * viewSpaceRadius;
	}

	double root[2] = { (-bPrime - sqrt(delta)) / a, (-bPrime + sqrt(delta)) / a };

	if (root[0] >= 0.0) {
		Eigen::Vector3d intersectionPoint = rayOrigin + root[0] * rayDir;
		return intersectionPoint;
	}
	else if (root[1] >= 0.0) {
		Eigen::Vector3d intersectionPoint = rayOrigin + root[1] * rayDir;
		return intersectionPoint;
	}
	else {
		double t = -CO.z() / rayDir.z();
		Eigen::Vector3d point = rayOrigin + t * rayDir;
		Eigen::Vector3d dir = point - viewSphereCenter;
		dir.normalize();
		return viewSphereCenter + dir * viewSpaceRadius;
	}
}

//static Eigen::Vector3d 

void doArcball(
	double * _viewMatrix, 
	double const * _rotationCenter, 
	double const * _projectionMatrix, 
	double const * _initialViewMatrix, 
	//double const * _currentViewMatrix,
	double const * _initialMouse, 
	double const * _currentMouse, 
	bool screenSpaceRadius,
	double radius)
{
	Eigen::Map<Eigen::Vector3d const> rotationCenter(_rotationCenter);
	Eigen::Map<Eigen::Matrix4d const> initialViewMatrix(_initialViewMatrix);
	//Eigen::Map<Eigen::Matrix4d const> currentViewMatrix(_currentViewMatrix);
	Eigen::Map<Eigen::Matrix4d const> projectionMatrix(_projectionMatrix);
	Eigen::Map<Eigen::Matrix4d> viewMatrix(_viewMatrix);

	Eigen::Vector2d initialMouse(_initialMouse);
	Eigen::Vector2d currentMouse(_currentMouse);
	
	Eigen::Quaterniond q;
	Eigen::Vector3d Pa, Pc;
	if (screenSpaceRadius) {
		double aspectRatio = projectionMatrix(1, 1) / projectionMatrix(0, 0);

		initialMouse(0) *= aspectRatio;
		currentMouse(0) *= aspectRatio;

		Pa = mapToSphere(initialMouse, radius);
		Pc = mapToSphere(currentMouse, radius);

		q.setFromTwoVectors(Pa - rotationCenter, Pc - rotationCenter);
	}
	else {
		Pa = mapToSphere(projectionMatrix.inverse(), initialMouse, rotationCenter, radius);
		Pc = mapToSphere(projectionMatrix.inverse(), currentMouse, rotationCenter, radius);

		Eigen::Vector3d a = Pa - rotationCenter, b = Pc - rotationCenter;

#if 0
		std::cout << "Mouse Initial Radius = " << sqrt(a.dot(a)) << " Current Radius = " << sqrt(b.dot(b)) << std::endl;
#endif

		q.setFromTwoVectors(a, b);
	}	

	Eigen::Matrix4d qMat4;
	qMat4.setIdentity();
	qMat4.topLeftCorner<3, 3>() = q.toRotationMatrix();

	Eigen::Matrix4d translate;
	translate.setIdentity();
	translate.topRightCorner<3, 1>() = -rotationCenter;

	Eigen::Matrix4d invTranslate;
	invTranslate.setIdentity();
	invTranslate.topRightCorner<3, 1>() = rotationCenter;
	
    viewMatrix = invTranslate * qMat4 * translate * initialViewMatrix;
}

