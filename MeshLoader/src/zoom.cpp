#include "zoom.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

void zoomFactor(
	double * projectionMatrix,
	double * inverseProjectionMatrix,
	double const * initialProjectionMatrix,
	double const * initialInverseProjectionMatrix,
	double factor
)
{
	double zero[] = { 0., 0. };
	zoomFactorProjectionCenter(projectionMatrix, inverseProjectionMatrix, initialProjectionMatrix, initialInverseProjectionMatrix, factor, zero);
}

void zoomFactorProjectionCenter(
	double * projectionMatrix,
	double * inverseProjectionMatrix,
	double const * initialProjectionMatrix,
	double const * initialInverseProjectionMatrix,
	double factor,
	double const * projectionCenter
)
{
	auto s = factor;
	auto si = 1 / s;
	Eigen::Map<Eigen::Vector2d const> c(projectionCenter);

	{
		Eigen::Map<Eigen::Matrix4d> Pc(projectionMatrix);
		Eigen::Vector3d c3, d3;
		c3 << -c, 0;
		d3 << s, s, 1;

		auto Pn = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(initialProjectionMatrix));
		Pn.pretranslate(c3);
		Pn.prescale(d3);
		Pc = Pn.matrix();
	}

	{
		Eigen::Map<Eigen::Matrix4d> Pci(inverseProjectionMatrix);
		Eigen::Vector3d c3, d3;
		c3 << c, 0;
		d3 << si, si, 1;

		auto Pn = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(initialInverseProjectionMatrix));
		Pn.translate(c3);
		Pn.scale(d3);
		Pci = Pn.matrix();
	}
}

void zoomedPointToInitialPoint(
	double * pointInInitialSpace,
	double const * pointInZoomedSpace,
	double const * currentInverseProjectionMatrix,
	double const * initialProjectionMatrix
)
{
	Eigen::Vector4d pc;
	pc << Eigen::Map<Eigen::Vector2d const>(pointInZoomedSpace), 0., 1.;

	auto Pci = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(currentInverseProjectionMatrix));
	auto Pi = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(initialProjectionMatrix));

	Eigen::Vector4d pi4 = Pi * Pci * pc;

	Eigen::Map<Eigen::Vector2d> pi(pointInInitialSpace);
	pi = pi4.topRows<2>() / pi4(3);
}

void clampZoomFactor(
	double & factor,
	double maxFactor
)
{
	factor = std::fmin(std::fmax(factor, 1.), maxFactor);
}

void clampZoomProjectionCenter(
	double * zoomCenter,
	double factor
)
{
	Eigen::Map<Eigen::Vector2d> center(zoomCenter);
	auto invFactor = 1. / factor;
	center = center.cwiseMax(invFactor - 1).cwiseMin(1 - invFactor);
}

void zoomRectangleToFactorProjectionCenter(
	double & factor,
	double * projectionCenter,
	double const * startRect,
	double const * endRect
)
{
	Eigen::Map<Eigen::Vector2d const> p0(startRect), p1(endRect);
	factor = 2. / (p1 - p0).cwiseAbs().maxCoeff();

	Eigen::Map<Eigen::Vector2d> c(projectionCenter);
	c = .5 * (p0 + p1);
}

void zoomCenterToProjectionCenter(
	double * projectionCenter,
	double factor,
	double const * zoomCenter,
	double const * zoomCenterScreen
)
{
	Eigen::Map<Eigen::Vector2d> c(projectionCenter);
	Eigen::Map<Eigen::Vector2d const> c0(zoomCenter);
	Eigen::Map<Eigen::Vector2d const> c1(zoomCenterScreen);
	c = c0 - c1 / factor;
}
