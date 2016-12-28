#include "Navigation.h"
#include "zoom.h"
#include "translationPan.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Navigation::Navigation()
	: m_updateInProgress(false)
	, zoomFactor(1.)
	, inTranslation(false)
{ 
}

Navigation::~Navigation()
{
}

void Navigation::startUpdate(glm::dvec2 currentMousePos, glm::vec3 centerOfRotation, double arcBallRadius)
{
	m_updateInProgress = true;
	
	m_mouseUpdateStartPos       = currentMousePos;
	m_centerOfRotation			= centerOfRotation;
	m_arcballRadius             = arcBallRadius;

	m_initView			= m_view;
	m_initInvView		= m_invView;

	m_initProjection = m_projection;
}

void Navigation::updateRotate(glm::dvec2 currentMousePos)
{
	static_assert(sizeof(glm::dmat4) == sizeof(double[16]), "GLM::dmat4.size != 16 * double.size");
	static_assert(sizeof(glm::dvec3) == sizeof(double[3]), "GLM::dvec3.size != 3 * double.size");

	// Transforming the mouse positions in the space [-1, 1]
	glm::dvec2 mouseInitPos, mouseFinalPos;
	mouseInitPos.x = glm::clamp(((m_mouseUpdateStartPos.x / ((m_screenWidth - 1.0) / 2.0)) - 1), -1.0, 1.0);
	mouseInitPos.y = glm::clamp(-((m_mouseUpdateStartPos.y / ((m_screenHeight - 1.0) / 2.0)) - 1), -1.0, 1.0);
	mouseFinalPos.x = glm::clamp(((currentMousePos.x / ((m_screenWidth  - 1.0) / 2.0)) - 1), -1.0, 1.0);
	mouseFinalPos.y = glm::clamp(-((currentMousePos.y / ((m_screenHeight - 1.0) / 2.0)) - 1), -1.0, 1.0);

	glm::dmat4 newViewMat = m_view;
	double (&_viewMat)[16]				= *reinterpret_cast<double (*)[16]>(&newViewMat);
	const double (&_center)[3]			= *reinterpret_cast<const double(*)[3]>(&m_centerOfRotation);
	const double (&_projMat)[16]		= *reinterpret_cast<const double(*)[16]>(&m_projection);
	const double (&_initViewMat)[16]    = *reinterpret_cast<const double(*)[16]>(&m_initView);
	const double (&_mouseInitPos)[2]	= *reinterpret_cast<const double(*)[2]>(&mouseInitPos);
	const double (&_mouseFinalPos)[2]	= *reinterpret_cast<const double(*)[2]>(&mouseFinalPos);

	doArcball(_viewMat, _center, _projMat, _initViewMat, _mouseInitPos, _mouseFinalPos, false, m_arcballRadius);

	setView(newViewMat);
}

void Navigation::endUpdate()
{
	m_updateInProgress = false;
}

void Navigation::startPan(glm::dvec3 currentViewPosUnderMouse)
{
	assert(!inTranslation);
	inTranslation = true;
	m_initView = m_view;
	m_initInvView = m_invView;
	m_initViewPos = currentViewPosUnderMouse;
}

void Navigation::updatePan(glm::dvec2 currentMousePos)
{
	glm::dvec2 currentMouseScreenPos;
	currentMouseScreenPos.x = glm::clamp(((currentMousePos.x / ((m_screenWidth - 1.0) / 2.0)) - 1), -1.0, 1.0);
	currentMouseScreenPos.y = glm::clamp(-((currentMousePos.y / ((m_screenHeight - 1.0) / 2.0)) - 1), -1.0, 1.0);

	assert(inTranslation);
	translationPan(
		&m_view[0][0],
		&m_invView[0][0],
		&m_projection[0][0],
		&m_invProjection[0][0],
		&m_initView[0][0],
		&m_initInvView[0][0],
		&m_initViewPos[0],
		&currentMouseScreenPos[0]
	);
}

void Navigation::endPan()
{
	assert(inTranslation);
	inTranslation = false;
}

void Navigation::zoomStep(int dir, glm::dvec2 center)
{
	if(!dir)
		return;

	auto const oldZoomFactor = zoomFactor;
	if(dir > 0) zoomFactor *= 1.25;
	if(dir < 0) zoomFactor /= 1.25;
	clampZoomFactor(zoomFactor, 16.);
	if(oldZoomFactor == zoomFactor)
		return;

	double initialCenter[2];
	zoomedPointToInitialPoint(
		initialCenter,
		&center[0],
		&m_invProjection[0][0],
		&m_initProjection[0][0]
	);

	double projectionCenter[2];
	zoomCenterToProjectionCenter(
		projectionCenter,
		zoomFactor,
		initialCenter,
		&center[0]
	);

	clampZoomProjectionCenter(
		projectionCenter,
		zoomFactor
	);

	zoomFactorProjectionCenter(
		&m_projection[0][0],
		&m_invProjection[0][0],
		&m_initProjection[0][0],
		&m_initInvProjection[0][0],
		zoomFactor,
		projectionCenter
	);
}

void Navigation::setProject(double fov, double aspectRatio, double zNear)
{
	m_initProjection = m_projection = glm::infinitePerspective(fov, aspectRatio, zNear);
	m_initInvProjection = m_invProjection = glm::inverse(m_projection);
}

void Navigation::setView(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up)
{
	m_view = glm::lookAt(eye, center, up);
	m_invView = glm::inverse(m_view);
}

void Navigation::setView(glm::dmat4 viewMat)
{
	m_view = viewMat;
	m_invView = glm::inverse(m_view);
}

void Navigation::setScreenSize(double width, double height)
{
	m_screenWidth  = width;
	m_screenHeight = height;
}

glm::dmat4 Navigation::getProjection() const
{
	return m_projection;
}

glm::dmat4 Navigation::getInvProjection() const
{
	return m_invProjection;
}

glm::dmat4 Navigation::getView() const
{
	return m_view;
}

glm::dmat4 Navigation::getInvView() const
{
	return m_invView;
}

glm::dvec3 Navigation::getPointViewCoord(glm::dvec2 screenCoord, double depth) const
{
	screenCoord[1] = m_screenHeight - screenCoord[1];
	glm::dvec4 NDC = glm::dvec4(screenCoord / glm::dvec2(m_screenWidth, m_screenHeight) * 2.0 - 1.0, 2 * depth - 1, 1.0);
	glm::dvec4 viewCoord(m_invProjection * NDC);

	if(std::fabs(viewCoord.w) > 0) 
		viewCoord /= viewCoord.w;

	return glm::dvec3(viewCoord);
}
