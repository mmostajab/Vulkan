#pragma once

void translationPan(
	double * viewMatrix,
	double * inverseViewMatrix,
	double const * projectionMatrix,
	double const * inverseProjectionMatrix,
	double const * initialViewMatrix,
	double const * initialInverseViewMatrix,
	double const * initialViewPoint,
	double const * currentScreenPosition
);
