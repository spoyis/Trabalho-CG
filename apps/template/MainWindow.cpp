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


/////////////////////////////////////////////////////////////////////
//
// MainWindow implementation
// ==========
MainWindow::MainWindow(int width, int height) :
    Base{ "Ds template", width, height }
{
    // Put your code here. Example:
    _lineColor = cg::Color::yellow;
    _meshColor = cg::Color::blue;
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

void
MainWindow::renderScene()
{
    // Put your scene rendering code here. Example:
    using namespace cg;

    auto g3 = this->g3();

    g3->setLineColor(_lineColor);

    ArchGeneratrix g(5, 180);
    g3->drawGeneratrix(g);


    if (_showGround)
        g3->drawXZPlane(8, 1);
}
//Severity	Code	Description	Project	File	Line	Suppression State
//Error	LNK2019	unresolved external symbol 
//"public: void __cdecl cg::GLGraphics3::drawPolygonGeneratrix(long)" (? drawPolygonGeneratrix@GLGraphics3@cg@@QEAAXJ@Z) referenced in function "private: virtual void __cdecl MainWindow::renderScene(void)" (? renderScene@MainWindow@@EEAAXXZ)	myapp	C : \Users\spOy\source\repos\trab_CG\apps\template\build\vs2022\MainWindow.obj	1

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
}
