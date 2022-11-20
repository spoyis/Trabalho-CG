#pragma once
#include "geometry/TriangleMesh.h"
#include <string>
#include <fstream>

namespace cg
{ // begin of namespace cg

	class MeshWriter
	{
	public:
		MeshWriter(const TriangleMesh::Data& data);
		~MeshWriter();

		void writeMesh(std::string& filename);

	private:
		const TriangleMesh::Data& data;
	};

	MeshWriter::MeshWriter(const TriangleMesh::Data& data) : data{ data }
	{
	}

	MeshWriter::~MeshWriter()
	{
	}

	void MeshWriter::writeMesh(std::string& filename) {
		
		std::string path = "../../assets/meshes/";
		std::string extension = ".obj";
		std::string file;
		file += path;
		file += filename;
		file += extension;
		std::cout << "TRYING " << file << '\n';
		fflush(stdout);
		std::ofstream out(file);
		//std::cout << "YAY\n";

		auto vertices = this->data.vertices;
		auto vertexCount = this->data.vertexCount;
		auto triangles = this->data.triangles;
		auto triangleCount = this->data.triangleCount;

		while (vertexCount--) {
			out << "v " << vertices->x << " " << vertices->y << " " << vertices->z << std::endl;
			vertices++;
		}

		while (triangleCount--) {
			out << "f " << triangles->v[0] + 1 << " " << triangles->v[1] + 1 << " " << triangles->v[2] + 1 << std::endl;
			triangles++;
		}

		out.close();
	}

} // end of namespace cg
