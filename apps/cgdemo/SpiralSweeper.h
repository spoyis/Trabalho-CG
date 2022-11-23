#pragma once
#include "GeneratrixSweeper.h"
namespace cg 
{ // begin namespace cg
	class SpiralSweeper : public GeneratrixSweeper
	{ // begin class SpiralSweeper
	public:
		SpiralSweeper(Generatrix& generatrix, float w_e, float s_x,
			float s_y, float r_e, long n_se,
			float delta_he, float delta_we, bool hasLid)
		{
			Generatrix::WrappingBox wrappingBox = generatrix.getWrappingBox();
			wrappingBox.b_x *= s_x;
			wrappingBox.b_y *= s_y;
			boundaryCheck(w_e, wrappingBox.b_x * 2, 100.0F);
			boundaryCheck(delta_he, r_e > 1 ? wrappingBox.b_y : 0.0F, 10.0F);
			boundaryCheck(delta_we, r_e > 1 ? wrappingBox.b_x : 0.0F, 10.0F);
			boundaryCheck(s_x, 0.0F, FLT_MAX);
			boundaryCheck(s_y, 0.0F, FLT_MAX);
			boundaryCheck(r_e, 0.1F, 100.0F);
			boundaryCheck(n_se, 3L, 40L);

			if (!delta_he and !delta_we) hasLid = false;

			long n_re = std::max((int)floor(r_e + 0.5F), 1) * n_se;
			long n_steps = n_re + 1;
			auto n_p = generatrix.size() + generatrix.isClosed();
			auto totalDelta_he = r_e * delta_he;
			auto totalDelta_we = r_e * delta_we;
			auto d_e = (w_e - wrappingBox.b_x) / 2;
			auto theta_e = 360.0F * r_e;

			TriangleMesh::Data data;
			initMeshData(data, generatrix, n_p, n_steps, hasLid);

			vec3f S = { s_x, s_y, 1 };
			long index = 0;
			for (long k = 0; k < n_steps; k++)
			{
				vec3f T = { d_e + totalDelta_we / n_re * k, totalDelta_he / n_re * k ,0.0F };

				constexpr auto pi = math::pi<float>();
				auto angle = ((pi * theta_e / n_re * k / 2) / 180.0F);
				Quaternion<float> R = { 0,sin(angle),0, cos(angle) };

				auto trs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, S);
				auto normaltrs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { 1,1,1 });

				for (long i = 0; i < n_p; i++, index++) {
					data.vertices[index] = trs * (vec4f{ T } + vec4f(generatrix[i], 1));
					data.vertexNormals[index] = normaltrs * vec4f(generatrix[i]);
				}

			}

			// Rotação completa da varredura
			Quaternion<float> R = { 0,sin(toRadian(theta_e / 2)),0, cos(toRadian(theta_e / 2)) };
			auto trs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { -1,-1,-1 });

			buildTriangles(data, n_steps, n_p);
			if (hasLid) buildLid(data, trs, n_p, n_steps, generatrix.size());
			mesh = new TriangleMesh{ std::move(data) };
		}
	}; // end class SpiralSweeper
} // end namespace cg