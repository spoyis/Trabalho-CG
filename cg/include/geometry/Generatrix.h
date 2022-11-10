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

		auto operator[](long index) {
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
		ArchGeneratrix(long cuts, int angle) : Generatrix(1) {
			//	if angle < 0 angle = 1;
		}
	};

} // end namespace cg