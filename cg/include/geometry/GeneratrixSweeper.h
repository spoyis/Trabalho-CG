#pragma once
#include "geometry/Generatrix.h"
#include "math/Matrix4x4.h"

namespace cg
{ // begin namespace cg



class GeneratrixSweeper {
public:
	GeneratrixSweeper() = default;
	~GeneratrixSweeper() { delete mesh; };
	auto get(){ return mesh; }
protected:
	TriangleMesh* mesh;
	template<typename T> 
	void boundaryCheck(T& data, T lo, T hi) {
		if (data < lo) data = lo;
		else if (data > hi) data = hi;
	}

	void initMeshData(TriangleMesh::Data& data, Generatrix& generatrix, long n_p, long n_steps, bool hasLid) {
		// TODO
		const auto lidTriangles = (generatrix.getAngle() == 360.0F) ? (n_p - 1) * 2 : (n_p - 2) * 2;
		// triangles resulting from sweeping
		const auto sweepTriangles = 2 * (n_p - 1) * (n_steps - 1);

		const auto n_triangles = hasLid ? lidTriangles + sweepTriangles : sweepTriangles;

		data.vertexCount = hasLid ? n_p * (n_steps + 2) + 2 : n_p * n_steps;
		data.vertices = new vec3f[data.vertexCount];
		data.vertexNormals = new vec3f[data.vertexCount];
		data.triangleCount = n_triangles;
		data.triangles = new TriangleMesh::Triangle[data.triangleCount];
	}

	void buildTriangles(TriangleMesh::Data& data, long n_steps, long n_p) {
		auto triangle = data.triangles;

		for (int g = 0; g < n_steps - 1; ++g)
		{
			auto i = g * n_p;
			auto j = i + n_p;

			for (int v = 0; v < n_p - 1; ++v, ++i, ++j)
			{
				auto k = i + 1;

				triangle++->setVertices(i, j, k);
				triangle++->setVertices(k, j, j + 1);
			}
		}


	}

	void buildLid(TriangleMesh::Data& data, Matrix<float, 4, 4> R, long n_p, long n_steps, bool np_offset) {

		const long lastVertex = n_p * n_steps;
		vec3f* generatrixPointer[2] = { &data.vertices[0], &data.vertices[lastVertex - n_p] };
		long start = lastVertex;
		vec3f normal = { 0,0,1 };

		auto sweepTriangles = 2 * (n_p - 1) * (n_steps - 1);
		auto triangle = data.triangles + sweepTriangles;
		
		for (long g = 0; g < 2; g++) 
		{
			long end = start + n_p;
			auto generatrix = generatrixPointer[g];

			auto minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
			auto maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
			for (long i = start; i < end; i++)
			{
				//min
				minX = std::min(minX, generatrix->x);
				minY = std::min(minY, generatrix->y);
				minZ = std::min(minZ, generatrix->z);
				//max
				maxX = std::max(maxX, generatrix->x);
				maxY = std::max(maxY, generatrix->y);
				maxZ = std::max(maxZ, generatrix->z);

				data.vertices[i] = *(generatrix++);
				data.vertexNormals[i] = normal;
			}
			data.vertices[end] = { 
				minX + (maxX - minX)/2,   // x --> menor ponto em x + wrapping box em x/2
				minY + (maxY - minY)/2,   // y --> menor ponto em y + wrapping box em y/2
				minZ + (maxZ - minZ)/2};  // z --> menor ponto em z + wrapping box em z/2

			generatrix = generatrixPointer[g];
			for (long i = 0; i < n_p - 2 + np_offset; i++)
			{
				auto index = generatrix++ - generatrixPointer[0];
				triangle++->setVertices(index, index + 1, end);
			}
			
			start = end + 1;
			normal = R * vec4f{ normal };
		}
	}

	auto toRadian(float angle) {
		constexpr auto pi = math::pi<float>();
		return pi * angle / 180;
	}
};



class SpiralSweeper : public GeneratrixSweeper
{ // begin class SpiralSweeper
public:
	SpiralSweeper(	Generatrix& generatrix, float w_e , float s_x,
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
		auto totalDelta_he =  r_e* delta_he;
		auto totalDelta_we = r_e * delta_we;
		auto d_e = (w_e - wrappingBox.b_x) / 2;
		auto theta_e = 360.0F * r_e;
		
		TriangleMesh::Data data;
		initMeshData(data, generatrix, n_p, n_steps, hasLid);

		vec3f S = { s_x, s_y, 1 };
		long index = 0;
		for (long k = 0; k < n_steps; k++) 
		{
			vec3f T = { d_e + totalDelta_we/n_re * k, totalDelta_he/n_re * k ,0.0F };

			constexpr auto pi = math::pi<float>();
			auto angle = ((pi * theta_e/n_re * k/ 2) / 180.0F) ;
			Quaternion<float> R = { 0,sin(angle),0, cos(angle)};

			auto trs = Matrix<float,4,4>::TRS({0,0,0}, R, S);
			auto normaltrs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { 1,1,1 });

			for (long i = 0; i < n_p; i++, index++) {
				data.vertices[index] = trs * (vec4f{ T } + vec4f(generatrix[i], 1));
				data.vertexNormals[index] = normaltrs * vec4f(generatrix[i]);
			}
			
		}

		// Rotação completa da varredura
		Quaternion<float> R = { 0,sin(toRadian(theta_e / 2)),0, cos(toRadian(theta_e / 2)) };
		auto trs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { 1,1,1 });

		buildTriangles(data, n_steps, n_p);
		if (hasLid) buildLid(data, trs, n_p, n_steps, generatrix.getAngle() == 360.0F);
		mesh = new TriangleMesh{ std::move(data) };
	}
}; // end class SpiralSweeper


class TwistSweeper : public GeneratrixSweeper
{ // begin class TwistSweeper
public:
	TwistSweeper(Generatrix& generatrix, float l_v,
		float o_wv, float o_hv, long n_sv, float s_bv, float s_ev, float r_v, bool hasLid)
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
		auto trs = Matrix<float, 4, 4>::TRS({ 0,0,0 }, R, { 1,1,1 });

		buildTriangles(data, n_steps, n_p);
		if (hasLid) buildLid(data, trs, n_p, n_steps, generatrix.getAngle() == 360.0F);
		mesh = new TriangleMesh{ std::move(data) };
	}
}; // end class TwistSweeper
} // end namespace cg