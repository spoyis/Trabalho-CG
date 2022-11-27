#pragma once
#include "graph/PrimitiveProxy.h"
#include "SpiralSweeper.h"
#include "TwistSweeper.h"

/////////////////////////////////////////////////////////////////////
//
// SweeperProxy: triangle mesh proxy class
// =================
namespace cg::graph
{

class SweeperProxy final : public PrimitiveProxy
{
public:
    static auto New(const std::string& meshName)
    {
        ArchGeneratrix garch = ArchGeneratrix(5, 260, true);
        SpiralSweeper* spiralSweep = new SpiralSweeper(garch, 1, 1, 1, 2, 40, 5, 2, false, false);

        return new SweeperProxy{ *spiralSweep->get(), meshName };
    }

    const char* const meshName() const
    {
        return _meshName.c_str();
    }

    // creates triangle mesh
    TriangleMesh* getMesh() {
        Generatrix* g;
        ArchGeneratrix garch = ArchGeneratrix(points, angle, closed);
        PolygonGeneratrix gpoly = PolygonGeneratrix(points);
        if (selectedGeneratrix) g = &garch; else g = &gpoly;

        TriangleMesh* mesh;

        SpiralSweeper* spiralSweep;
        TwistSweeper* twistsweep;
        if (selectedSweepType)
        {
            spiralSweep = new SpiralSweeper(*g, w_e, s_x, s_y, r_e, n_se, delta_he, delta_we, hasLid, useComputeNormals);
            mesh = spiralSweep->get();
        }
        else
        {
            twistsweep = new TwistSweeper(*g, l_v, o_wv, o_hv, n_sv, s_bv, s_ev, r_v, hasLid, useComputeNormals);
            mesh = twistsweep->get();
        }

        return mesh;
    }

    // called every time the user changes one of the variables on the gui
    void renewMesh()
    {
        TriangleMesh* mesh = getMesh();
        ((TriangleMeshMapper*)PrimitiveProxy::mapper())->setMesh(*mesh);
    }

    // variables pertaining to each generatrix type and sweeper.
    float w_e = 1, s_x = 1, s_y = 1, r_e = 2;
    long n_se = 40;
    float delta_he = 5, delta_we = 2;
    bool hasLid = false;
    bool useComputeNormals = false;
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

    SweeperProxy(const TriangleMesh& mesh, const std::string& meshName) : PrimitiveProxy{ *new TriangleMeshMapper{mesh} },
        _meshName{ meshName }
    {
        // do nothing
    }

}; // TriangleMeshProxy

}