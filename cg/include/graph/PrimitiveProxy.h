//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2022 Paulo Pagliosa.                              |
//|                                                                 |
//| This software is provided 'as-is', without any express or       |
//| implied warranty. In no event will the authors be held liable   |
//| for any damages arising from the use of this software.          |
//|                                                                 |
//| Permission is granted to anyone to use this software for any    |
//| purpose, including commercial applications, and to alter it and |
//| redistribute it freely, subject to the following restrictions:  |
//|                                                                 |
//| 1. The origin of this software must not be misrepresented; you  |
//| must not claim that you wrote the original software. If you use |
//| this software in a product, an acknowledgment in the product    |
//| documentation would be appreciated but is not required.         |
//|                                                                 |
//| 2. Altered source versions must be plainly marked as such, and  |
//| must not be misrepresented as being the original software.      |
//|                                                                 |
//| 3. This notice may not be removed or altered from any source    |
//| distribution.                                                   |
//|                                                                 |
//[]---------------------------------------------------------------[]
//
// OVERVIEW: PrimitiveProxy.h
// ========
// Class definition for primitive proxy.
//
// Author: Paulo Pagliosa
// Last revision: 24/01/2022

#ifndef __PrimitiveProxy_h
#define __PrimitiveProxy_h

#include "graph/ComponentProxy.h"
#include "graphics/Actor.h"
#include "graphics/TriangleMeshMapper.h"
#include "geometry/GeneratrixSweeper.h"

namespace cg
{ // begin namespace cg

namespace graph
{ // begin namespace graph


/////////////////////////////////////////////////////////////////////
//
// PrimitiveProxy: primitive proxy class
// ==============
class PrimitiveProxy: public ComponentProxy<PrimitiveMapper>
{
public:
  static auto New(const PrimitiveMapper& mapper)
  {
    return new PrimitiveProxy{mapper};
  }

  PrimitiveMapper* mapper() const
  {
    return _object;
  }

  Actor* actor() const
  {
    return _actor;
  }

protected:
  Reference<Actor> _actor;

  PrimitiveProxy(const PrimitiveMapper& mapper):
    ComponentProxy<PrimitiveMapper>{"Primitive", mapper}
  {
    // do nothing
  }

  void afterAdded() override;
  void beforeRemoved() override;
  void update() override;
  void setVisible(bool value) override;

}; // PrimitiveProxy

inline auto
asPrimitive(Component* component)
{
  return dynamic_cast<PrimitiveProxy*>(component);
}


/////////////////////////////////////////////////////////////////////
//
// TriangleMeshProxy: triangle mesh proxy class
// =================
class TriangleMeshProxy: public PrimitiveProxy
{
public:
  static auto New(const TriangleMesh& mesh, const std::string& meshName)
  {
    return new TriangleMeshProxy{mesh, meshName};
  }

  const char* const meshName() const
  {
    return _meshName.c_str();
  }

  void setMesh(const TriangleMesh& mesh, const std::string& meshName)
  {
    ((TriangleMeshMapper*)PrimitiveProxy::mapper())->setMesh(mesh);
    _meshName = meshName;
  }

protected:
  std::string _meshName;

  TriangleMeshProxy(const TriangleMesh& mesh, const std::string& meshName):
    PrimitiveProxy{*new TriangleMeshMapper{mesh}},
    _meshName{meshName}
  {
    // do nothing
  }

}; // TriangleMeshProxy


/////////////////////////////////////////////////////////////////////
//
// SweeperProxy: triangle mesh proxy class
// =================
class SweeperProxy final : public TriangleMeshProxy
{
public:
    static auto New(const std::string& meshName)
    {
        ArchGeneratrix garch = ArchGeneratrix(5, 260, true);
        SpiralSweeper* spiralSweep = new SpiralSweeper(garch, 1, 1, 1, 2, 40, 5, 2,false);
         
        return new SweeperProxy{ *spiralSweep->get(), meshName};
    }

    const char* const meshName() const
    {
        return _meshName.c_str();
    }

    TriangleMesh* getMesh() {
        Generatrix* g;
        ArchGeneratrix garch = ArchGeneratrix(points, angle, closed);
        PolygonGeneratrix gpoly = PolygonGeneratrix(points);
        if (selectedGeneratrix) g = &garch; else g = &gpoly;
        //g3->drawGeneratrix(*g);

        TriangleMesh* mesh;

        SpiralSweeper* spiralSweep;
        TwistSweeper* twistsweep;
        if (selectedSweepType)
        {
            spiralSweep = new SpiralSweeper(*g, w_e, s_x, s_y, r_e, n_se, delta_he, delta_we, hasLid);
            mesh = spiralSweep->get();
        }
        else
        {
            twistsweep = new TwistSweeper(*g, l_v, o_wv, o_hv, n_sv, s_bv, s_ev, r_v, hasLid);
            mesh = twistsweep->get();
        }

        return mesh;
    }

    void renewMesh()
    {
        TriangleMesh* mesh = getMesh();
        ((TriangleMeshMapper*)PrimitiveProxy::mapper())->setMesh(*mesh);
    }

    float w_e = 1, s_x = 1, s_y = 1, r_e = 2;
    long n_se = 40;
    float delta_he = 5, delta_we = 2;
    bool hasLid = false;
    // generatrix
    float angle = 260;
    long points = 5;

    // imgui
    int selectedGeneratrix = 1;
    int selectedSweepType = 1;

    float l_v = 1, o_wv = 0, o_hv = 0;
    long n_sv = 20;
    float s_bv = 2, s_ev = 2, r_v = 0;
    bool closed = true;

private:
    std::string _meshName;

    SweeperProxy(const TriangleMesh& mesh, const std::string& meshName) : TriangleMeshProxy(mesh, meshName)
    {
        // do nothing
    }

}; // TriangleMeshProxy
} // end namepace graph

} // end namespace cg

#endif // __PrimitiveProxy_h
