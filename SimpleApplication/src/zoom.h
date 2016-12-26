#pragma once

/**
Computes a zoomed projection matrix from the initial matrix, a zoom factor and a projection center.
All matrices are 4x4 matrices in column-major storage (16 element arrays). All pointer arguments must be non-null.
@param[out] projectionMatrix resulting projection matrix.
@param[out] projectionMatrix resulting inverse projection matrix.
@param initialProjectionMatrix initial (unzoomed) projection matrix.
@param initialInverseProjectionMatrix inverse of initial (unzoomed) projection matrix.
@param factor zoom factor relative to initial projection.
@param projectionCenter center of zoomed area in initial projection screen space. Pointer to 2-component array.
*/
void zoomFactorProjectionCenter(
	double * projectionMatrix,
	double * inverseProjectionMatrix,
	double const * initialProjectionMatrix,
	double const * initialInverseProjectionMatrix,
	double factor,
	double const * projectionCenter
);

/**
Computes a centered zoomed projection matrix from the initial matrix and a zoom factor.
All matrices are 4x4 matrices in column-major storage (16 element arrays). All pointer arguments must be non-null.
@param[out] projectionMatrix resulting projection matrix.
@param[out] inverseProjectionMatrix resulting inverse projection matrix.
@param initialProjectionMatrix initial (unzoomed) projection matrix.
@param initialInverseProjectionMatrix inverse of initial (unzoomed) projection matrix.
@param factor zoom factor relative to initial projection.
*/
void zoomFactor(
	double * projectionMatrix,
	double * inverseProjectionMatrix,
	double const * initialProjectionMatrix,
	double const * initialInverseProjectionMatrix,
	double factor
);

/**
Reprojects a position in current (zoomed) screen space to a position in the initial (unzoomed) screen space.
All matrices are 4x4 matrices in column-major storage (16 element arrays). All pointer arguments must be non-null.
@param[out] pointInInitialSpace resulting point in initial projection screen space. Pointer to 2-component array.
@param pointInZoomedSpace point in current projection screen space. Pointer to 2-component array.
@param currentInverseProjectionMatrix current (zoomed) inverse projection matrix.
@param initialProjectionMatrix initial (unzoomed) projection matrix.
*/
void zoomedPointToInitialPoint(
	double * pointInInitialSpace,
	double const * pointInZoomedSpace,
	double const * currentInverseProjectionMatrix,
	double const * initialProjectionMatrix
);

/**
Clamps a zoom factor to a range of [1, maxFactor].
@param[in, out] factor zoom factor to be clamped.
@param maxFactor maximum zoom factor.
*/
void clampZoomFactor(
	double & factor,
	double maxFactor = 16.
);

/**
Repositions a projection center so that the zoomed view is entirely within the initial view.
@param[in, out] projectionCenter center of zoomed area in initial projection screen space to be clamped. Non-null pointer to 2-component array.
@param factor zoom factor. Must be greater than or equal to 1.
*/
void clampZoomProjectionCenter(
	double * projectionCenter,
	double factor
);

/**
Calculates a zoom factor and a projection center from a screen-space rectangle.
All pointer arguments must be non-null.
@param[out] factor resulting zoom factor.
@param[out] projectionCenter resulting projection center. Pointer to 2-component array.
@param startRect first corner of rectangle in initial projection screen space. Pointer to 2-component array.
@param endRect second (opposing) corner of rectangle in initial projection screen space. Pointer to 2-component array.
*/
void zoomRectangleToFactorProjectionCenter(
	double & factor,
	double * projectionCenter,
	double const * startRect,
	double const * endRect
);

/**
Calculates a the projection center for a zoom centered around a screen-space point.
All pointer arguments must be non-null.
@param[out] projectionCenter resulting projection center.
@param factor zoom factor.
@param zoomCenter center of zoom in initial projection screen space. Pointer to 2-component array.
@param zoomCenterScreen center of zoom in current projection screen space. Pointer to 2-component array.
*/
void zoomCenterToProjectionCenter(
	double * projectionCenter,
	double factor,
	double const * zoomCenter,
	double const * zoomCenterScreen
);
