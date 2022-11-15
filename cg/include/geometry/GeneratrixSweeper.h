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

		long n_re = floor(r_e + 0.5F) * n_se;
		long n_steps = n_re + 1;
		auto n_p = generatrix.size() + 1;
		auto totalDelta_he =  r_e* delta_he;
		auto totalDelta_we = r_e * delta_we;
		auto d_e = (w_e - wrappingBox.b_x) / 2;
		auto theta_e = 360.0F * r_e;
		
		auto lidTriangles = (generatrix.getAngle() == 360.0F) ? n_p - 1 : n_p - 2;
		auto sweepTriangles = 2 * (n_p - 1) * (n_steps - 1);
		auto n_triangles = hasLid ? lidTriangles + sweepTriangles : sweepTriangles;
		TriangleMesh::Data data; 

		data.vertexCount = hasLid ? n_p * (n_steps + 2) + 2 : n_p * n_steps; 
		data.vertices = new vec3f[data.vertexCount]; 
		data.vertexNormals = new vec3f[data.vertexCount]; 
		data.triangleCount = n_triangles;
		data.triangles = new TriangleMesh::Triangle[data.triangleCount]; 

		
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

		mesh =  new TriangleMesh{ std::move(data) };
	}


private:
	


}; // end class SpiralSweeper
} // end namespace cg