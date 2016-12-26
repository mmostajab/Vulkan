#include "translationPan.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

template<typename Matrix4, typename Vector3>
static Eigen::Vector3d applyHomogeneous(Matrix4 const & M, Vector3 const & v)
{
	Eigen::Vector4d v4 = M * v.homogeneous();
	return v4.topRows<3>() / v4(3);
}

template<typename Matrix4, typename Vector2>
static Eigen::ParametrizedLine<double, 3> makeViewRay(Matrix4 const & Pi, Vector2 const & s) {
	Eigen::Vector3d sp;
	sp << s, .75;
	auto vp0 = applyHomogeneous(Pi, sp);
	sp << s, .25;
	auto vp1 = applyHomogeneous(Pi, sp);
	return Eigen::ParametrizedLine<double, 3>(vp0, (vp1 - vp0).normalized());
};

void translationPan(
	double * viewMatrix,
	double * inverseViewMatrix,
	double const * projectionMatrix,
	double const * inverseProjectionMatrix,
	double const * initialViewMatrix,
	double const * initialInverseViewMatrix,
	double const * initialViewPoint,
	double const * currentScreenPosition
)
{
	Eigen::Map<Eigen::Matrix4d> Vc(viewMatrix), Vci(inverseViewMatrix);
	auto P = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(projectionMatrix));
	auto Pi = Eigen::Projective3d(Eigen::Map<Eigen::Matrix4d const>(inverseProjectionMatrix));
	auto Vi = Eigen::Affine3d(Eigen::Map<Eigen::Matrix4d const>(initialViewMatrix));
	auto Vii = Eigen::Affine3d(Eigen::Map<Eigen::Matrix4d const>(initialInverseViewMatrix));
	Eigen::Map<Eigen::Vector3d const> vi(initialViewPoint);
	Eigen::Map<Eigen::Vector2d const> sc(currentScreenPosition);

	Eigen::Vector2d si = applyHomogeneous(P, vi).topRows<2>();

	auto initialRay = makeViewRay(Pi, si);
	auto currentRay = makeViewRay(Pi, sc);

	Eigen::Hyperplane<double, 3> panningPlane(Eigen::Vector3d::UnitZ(), vi);
	auto initialIntersect = initialRay.intersectionPoint(panningPlane);
	auto currentIntersect = currentRay.intersectionPoint(panningPlane);

	Eigen::Vector3d translation = currentIntersect - initialIntersect;

	Vc = Vi.pretranslate(translation).matrix();
	Vci = Vii.translate(-translation).matrix();
}
