#pragma once

void doArcball(
	double * viewMatrix,              // resulting viewMatrix
	double const * rotationCenter,    // center of rotation in view space
	double const * projectionMatrix,  // current camera projection matrix
	double const * intialViewMatrix,  // initial camera view matrix (scale/shear/projection free, pure translation and rotation)
	//double const * currentViewMatrix, // current camera view matrix (scale/shear/projection free, pure translation and rotation)
	double const * initialMouse,      // initial mouse position in screen space (-1,1)
	double const * currentMouse,      // current mouse position in screen space
	bool screenSpaceRadius,           // arcball radius in screen space or view space
	double radius                     // arcball radius (in view space)
);
