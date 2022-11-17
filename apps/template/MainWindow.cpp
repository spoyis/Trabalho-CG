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
// OVERVIEW: MainWindow.cpp
// ========
// Source file for cg template window.
//
// Author: Paulo Pagliosa
// Last revision: 07/11/2022

#include "MainWindow.h"
#include "geometry/GeneratrixSweeper.h"
MainWindow* window;
/////////////////////////////////////////////////////////////////////
//
// MainWindow implementation
// ==========
MainWindow::MainWindow(int width, int height) :
    Base{ "Ds template", width, height }
{
    // Put your code here. Example:
    _lineColor = cg::Color::black;
    _meshColor = cg::Color::white;
    _radius = 1;
    _speed = 0.01f;
}

void
MainWindow::initialize()
{
    // Put your OpenGL initialization code here. Example:
    Base::initialize();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
}

void
MainWindow::update()
{
    // Put your scene update code here. Example:
    static float time{};

    if (_animate)
        _radius = 1 + cosf(_speed * (time += deltaTime()) * 0.5f);
}


// spiral sweeper
float w_e = 1, s_x = 1, s_y = 1, r_e = 2;
long n_se = 40;
float delta_he = 5, delta_we = 2;
bool hasLid = false;
// generatrix
float angle = 260;
long points = 5;

// imgui
static int selectedGeneratrix = 1;
static int selectedSweepType = 0;

float l_v = 1, o_wv = 0, o_hv = 0;
long n_sv = 20;
float s_bv = 2, s_ev = 2, r_v = 0;
bool closed = true;

void
MainWindow::renderScene()
{
    // Put your scene rendering code here. Example:
    using namespace cg;

    auto g3 = this->g3();

    g3->setLineColor(_lineColor);
    g3->setMeshColor(_meshColor);
    Generatrix* g;
    ArchGeneratrix garch = ArchGeneratrix(points, angle, closed);
    PolygonGeneratrix gpoly = PolygonGeneratrix(points);
    if (selectedGeneratrix) g = &garch; else g = &gpoly;
    //g3->drawGeneratrix(*g);

    TriangleMesh* mesh;
    
    SpiralSweeper spiralSweep(*g, w_e, s_x, s_y, r_e, n_se, delta_he, delta_we, hasLid);
    TwistSweeper twistsweep(*g, l_v, o_wv, o_hv, n_sv, s_bv, s_ev, r_v, hasLid);
    mesh = selectedSweepType ? spiralSweep.get() : twistsweep.get();
    

    g3->drawMesh(*mesh);

    
    for(long step = 0; step < 40; step++)
    for (long i = 0; i < g->size(); i++) {
        //g3->drawLine(mesh->data().vertices[step * g.size() + i], mesh->data().vertices[step * g.size() + i + 1]);
    }

    if (_showGround)
        g3->drawXZPlane(8, 1);
}

bool
MainWindow::keyInputEvent(int key, int action, int mods)
{
    // Put your key event handler code here. Example:
    if (action != GLFW_RELEASE && mods == GLFW_MOD_ALT)
        switch (key)
        {
        case GLFW_KEY_P:
            _animate ^= true;
            return true;
        }
    return Base::keyInputEvent(key, action, mods);
}

void
MainWindow::gui()
{
    // Put your gui code here. Example:
    ImGui::SetNextWindowSize({ 360, 180 });
    ImGui::Begin("Template GUI");
    ImGui::ColorEdit3("Line Color", (float*)&_lineColor);
    ImGui::ColorEdit3("Mesh Color", (float*)&_meshColor);
    ImGui::Separator();
    ImGui::Checkbox("Animate", &_animate);
    ImGui::SliderFloat("Speed", &_speed, 0.001f, 0.01f);
    ImGui::Checkbox("Show Ground", &_showGround);
    ImGui::Separator();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        deltaTime(),
        ImGui::GetIO().Framerate);
    ImGui::End();


    static const char* generatrixTypes[] = { "Poligon", "Arch" };
    static const char* sweepTypes[] = { "Twist", "Spiral" };

    ImGui::SetNextWindowSize({ 360, 270 });
    ImGui::Begin("Sweep menu");
    
    ImGui::Combo("Generatrix type", &selectedGeneratrix, generatrixTypes, IM_ARRAYSIZE(generatrixTypes));
    if (selectedGeneratrix == 1)
    {
        ImGui::Checkbox("Closed arch", &closed);
        ImGui::SliderFloat("angle", &angle, 1.0f, 360.0f);
    }
    ImGui::SliderInt("segments", (int*) &points, 2.0, 30);

    ImGui::Separator();
    ImGui::Combo("Sweep type", &selectedSweepType, sweepTypes, IM_ARRAYSIZE(sweepTypes));
    ImGui::Checkbox("has lid", &hasLid);
    if(selectedSweepType == 1){
        
        ImGui::SliderFloat("Initial length", &w_e, 2.0f, 100.0f);
        ImGui::SliderFloat("x scale", &s_x, 0.2f, 10.0f);
        ImGui::SliderFloat("y scale", &s_y, 0.2f, 10.0f);
        ImGui::SliderFloat("rotations", &r_e, 0.5f, 20.0f);
        ImGui::SliderInt("segments per rotation", (int*) &n_se, 3, 40);
        ImGui::SliderFloat("height variation", &delta_he, 0.0f, 10.0f);
        ImGui::SliderFloat("length variation", &delta_we, 0.0f, 10.0f);
    }
    else {
		  ImGui::SliderFloat("Length", &l_v, 1.0f, 50.0f);
		  ImGui::SliderFloat("Horizontal shift", &o_wv, -5.0f, 5.0f);
		  ImGui::SliderFloat("Vertival shift", &o_hv, -5.0f, 5.0f);
		  ImGui::SliderFloat("Initial scale", &s_bv, 0.1f, 5.0f);
		  ImGui::SliderFloat("Final scale", &s_ev, 0.1f, 5.0f);
		  ImGui::SliderFloat("Twist", &r_v, -2.0f, 2.0f);
		  ImGui::SliderInt("segments per rotation", (int*)&n_sv, 1, 50);
    }

    ImGui::End();
}
