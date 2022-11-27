#pragma once
#include "GeneratrixSweeper.h"
namespace cg
{// begin namespace cg


class TwistSweeper : public GeneratrixSweeper
{ // begin class TwistSweeper
public:
	TwistSweeper(Generatrix& generatrix, float l_v,
		float o_wv, float o_hv, long n_sv, float s_bv, float s_ev, float r_v, bool hasLid, bool useComputeNormals)
	{
		Generatrix::WrappingBox wrappingBox = generatrix.getWrappingBox();
		wrappingBox.b_x *= s_bv;
		wrappingBox.b_y *= s_bv;
		boundaryCheck(l_v, 1.0F, 50.0F);
		boundaryCheck(o_wv, -wrappingBox.b_x, wrappingBox.b_x);
		boundaryCheck(o_hv, -wrappingBox.b_y, wrappingBox.b_y);
		boundaryCheck(n_sv, 1L, 50L);
		boundaryCheck(s_bv, 0.1F, 5.0F);
		boundaryCheck(s_ev, 0.1F, 5.0F);
		boundaryCheck(r_v, -2.0F, 2.0F);

		long n_steps = n_sv + 1;
		auto n_p = generatrix.size() + generatrix.isClosed();
		auto s_v = s_bv + (s_ev - s_bv) / n_sv * n_steps;
		auto theta_v = 360.0F * r_v;

		TriangleMesh::Data data;
		initMeshData(data, generatrix, n_p, n_steps, hasLid);

		long index = 0;
		for (long k = 0; k < n_steps; k++)
		{
			auto scale = s_bv + (s_ev - s_bv) / n_sv * k;
			vec3f S = { scale, scale, 1 };
			vec3f T = { o_wv, o_hv, l_v / n_sv * k };

			constexpr auto pi = math::pi<float>();
			auto angle = ((pi * theta_v / n_sv * k / 2) / 180.0F);
			Quaternion<float> R = { 0,0,sin(angle), cos(angle) };

			auto trs = Matrix<float, 4, 4>::TRS(T, R, S);
			auto normaltrs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { 1,1,1 });

			for (long i = 0; i < n_p; i++, index++) {
				data.vertices[index] = (trs * vec4f(generatrix[i], 1));
				data.vertexNormals[index] = normaltrs * vec4f(generatrix[i]);
			}
		}

		Quaternion<float> R = { 0,0,sin(toRadian(theta_v / 2)), cos(toRadian(theta_v / 2)) };
		auto trs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { -1,-1,-1 });

		buildTriangles(data, n_steps, n_p);
		if (hasLid) buildLid(data, trs, n_p, n_steps, generatrix.size());
		mesh = new TriangleMesh{ std::move(data) };
		if(useComputeNormals)mesh->computeNormals();
	}
}; // end class TwistSweeper
} // end namespace cg