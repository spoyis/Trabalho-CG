#pragma once
#include "Generatrix.h"
#include "iostream"
#include "math/Matrix4x4.h"

namespace cg
{ // begin namespace cg

	class GeneratrixSweeper {
	public:
		GeneratrixSweeper() = default;
		~GeneratrixSweeper() { delete mesh; };
		auto get() { return mesh; }
	protected:
		TriangleMesh* mesh;

		/* AUXILIARY METHODS FOR SWEEPING 
		
		   THESE METHODS ARE USED IN BOTH SWEEP TYPES
		   SO WE IMPLEMENT THEM IN THE PARENT CLASS.
		*/
		template<typename T>
		void boundaryCheck(T& data, T lo, T hi) {
			if (data < lo) data = lo;
			else if (data > hi) data = hi;
		}

		void initMeshData(TriangleMesh::Data& data, Generatrix& generatrix, long n_p, long n_steps, bool hasLid) {
			// triangles belonging to the lid
			const auto lidTriangles = generatrix.size() * 2;
			// triangles resulting from sweeping
			const auto sweepTriangles = 2 * (n_p - 1) * (n_steps - 1);
			// total number of triangles
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

		// Creates lid vertices AND triangles
		void buildLid(TriangleMesh::Data& data, Matrix<float, 4, 4> R, long n_p, long n_steps, long generatrixSize) {

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
					minX + (maxX - minX) / 2,   // x --> menor ponto em x + wrapping box em x/2
					minY + (maxY - minY) / 2,   // y --> menor ponto em y + wrapping box em y/2
					minZ + (maxZ - minZ) / 2 };  // z --> menor ponto em z + wrapping box em z/2

				generatrix = generatrixPointer[g];
				for (long i = 0; i < generatrixSize; i++)
				{
					auto index = generatrix++ - generatrixPointer[0];
					if (i != generatrixSize - 1)
						triangle++->setVertices(index, index + 1, end);
					else triangle++->setVertices(index, generatrixPointer[g] - generatrixPointer[0], end);
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
} // end namespace cg