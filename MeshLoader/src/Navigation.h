#pragma once

#include "arcball.h"

#include <glm/glm.hpp>

class Navigation {
public:
	Navigation();
	~Navigation();

	void startUpdate(glm::dvec2 currentMousePos, glm::vec3 centerOfRotation, double arcBallRadius);
	void updateRotate(glm::dvec2 mouseFinalPos);
	void endUpdate();

	void startPan(glm::dvec3 currentViewPosUnderMouse);
	void updatePan(glm::dvec2 currentMousePos);
	void endPan();

	void zoomStep(int dir, glm::dvec2 center = glm::dvec2(0));

	void setProject(double fov, double aspectRatio, double zNear);

	void setView(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up);
	void setView(glm::dmat4 viewMat);

	void setScreenSize(double width, double height);

	glm::dmat4 getProjection() const;
	glm::dmat4 getInvProjection() const;
	glm::dmat4 getView() const;
	glm::dmat4 getInvView() const;

	glm::dvec3 getPointViewCoord(glm::dvec2 screenCoord, double depth) const;

private:
	glm::dmat4 m_projection, m_invProjection, m_initProjection, m_initInvProjection;
	glm::dmat4 m_view, m_invView;
	double     m_screenWidth, m_screenHeight;

	bool       m_updateInProgress;
	glm::dvec2 m_mouseUpdateStartPos;
	
	// =============================
	// == Rotation related
	// =============================
	glm::dvec3 m_centerOfRotation;
	double     m_arcballRadius;

	// Zoom status
	double zoomFactor;

	// Translation "Pan" status
	bool inTranslation;
	glm::dmat4 m_initView, m_initInvView;
	glm::dvec3 m_initViewPos;
};