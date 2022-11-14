#pragma once
#include "geometry/MeshSweeper.h"

namespace cg
{ // begin namespace cg
	class Generatrix {
	public:
		Generatrix(long sz) {
			this->maxSize = sz;
			vertexArr = new vec3f[maxSize];
		}

		~Generatrix() {
			delete vertexArr;
		}

		void add(float x, float y, float z) {
#ifdef _DEBUG
			if (occupied == maxSize)
			{
				std::cout << "MANO CÊ FEZ DODOI, ALOCOU ESPACO INSUFICIENTE NA GERATRIZ\n";
				return;
			}
#endif // DEBUG

			vertexArr[occupied++].set(x, y, z);
		}

		auto& operator[](long index) {
			return vertexArr[index];
		}

		auto get() {
			return vertexArr;
		}

		auto size() { return occupied; }
	private:
		long occupied{ 0 };
		long maxSize;
		vec3f* vertexArr;
	};

	class PolygonGeneratrix : public Generatrix {
	public:
		PolygonGeneratrix(long dim) : Generatrix(dim > 2 ? dim : 3) {
			if (dim <= 2) dim = 3;

			float x = 0, y = 1;
			add(x, y, 0);
			{
				constexpr auto pi = math::pi<float>();
				auto angleStep = pi / dim * 2;
				auto cosStep = cos(angleStep);
				auto sinStep = sin(angleStep);

				for (long i = 1; i < dim; i++)
				{
					auto nextX = cosStep * x - sinStep * y;
					auto nextY = sinStep * x + cosStep * y;

					x = nextX;
					y = nextY;
					add(x, y, 0);
				}
			}
		}
	};

	class ArchGeneratrix : public Generatrix {
	public:
		ArchGeneratrix(long segments, float angle) : Generatrix(angle == 360 ? segments : segments + 1) {
			if (angle < 0) angle = 1.0F;
			if (angle > 360) angle = 360.0F;
			auto vertices = (angle == 360.0F) ? segments : segments + 1;

			constexpr auto pi = math::pi<float>();
			angle = (pi * angle) / 180.0F;

			auto angleStep = angle / segments;
			auto cosStep = cos(angleStep);
			auto sinStep = sin(angleStep);

			auto x = sin(angle / 2.0);
			auto y = cos(angle / 2.0);
			while (vertices--) {
				add(x, y, 0);
				auto nextX = cosStep * x - sinStep * y;
				auto nextY = sinStep * x + cosStep * y;
				x = nextX;
				y = nextY;
			}
		}
	};

} // end namespace cg
