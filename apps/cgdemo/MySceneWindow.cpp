//[]---------------------------------------------------------------[]
//|                                                                 |
//| Copyright (C) 2020, 2022 Paulo Pagliosa.                        |
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
// OVERVIEW: SceneWindowBase.cpp
// ========
// Source file for generic graph scene window.
//
// Author: Paulo Pagliosa
// Last revision: 05/08/2022

#include "MySceneWindow.h"
#include "graphics/Assets.h"
#include "MeshWriter.h"
#include <cassert>
#include <typeinfo>
#include <unordered_map>

namespace cg
{ // begin namespace cg

    namespace graph
    { // begin namespace graph


    /////////////////////////////////////////////////////////////////////
    //
    // MySceneWindow implementation
    // ===========
        SceneBase*
            MySceneWindow::makeScene()
        {
            auto scene = Scene::New();

            SceneObjectBuilder::setScene(*scene);
            _currentNode = scene;
            return scene;
        }

        void MySceneWindow::inspectSweeper(MySceneWindow&, SweeperProxy& proxy)
        {
            static const char* generatrixTypes[] = { "Poligon", "Arch" };
            static const char* sweepTypes[] = { "Twist", "Spiral" };

            auto primitive = proxy.mapper()->primitive();
            auto material = primitive->material();

            ImGui::inputText("Material", material->name());
            if (ImGui::BeginDragDropTarget())
            {
                if (auto* payload = ImGui::AcceptDragDropPayload("Material"))
                {
                    auto mit = *(MaterialMapIterator*)payload->Data;

                    assert(mit->second != nullptr);
                    primitive->setMaterial(material = mit->second);
                }
                ImGui::EndDragDropTarget();
            }
            inspectMaterial(*material);
            proxy.actor()->visible = proxy.sceneObject()->visible();
            {
                bool changed = false;
                ImGui::Separator();
                changed |= ImGui::Combo("Generatrix type", &proxy.selectedGeneratrix, generatrixTypes, IM_ARRAYSIZE(generatrixTypes));
                if (proxy.selectedGeneratrix == 1)
                {
                    changed |= ImGui::Checkbox("Closed arch", &proxy.closed);
                    changed |= ImGui::SliderFloat("angle", &proxy.angle, 1.0f, 360.0f);
                }
                changed |= ImGui::SliderInt("segments", (int*)&proxy.points, 2.0, 30);

                ImGui::Separator();
                changed |= ImGui::Combo("Sweep type", &proxy.selectedSweepType, sweepTypes, IM_ARRAYSIZE(sweepTypes));
                changed |= ImGui::Checkbox("has lid", &proxy.hasLid);
                changed |= ImGui::Checkbox("use computeNormals", &proxy.useComputeNormals);
                if (proxy.selectedSweepType == 1) {

                    changed |= ImGui::SliderFloat("Initial length", &proxy.w_e, 2.0f, 100.0f);
                    changed |= ImGui::SliderFloat("x scale", &proxy.s_x, 0.2f, 10.0f);
                    changed |= ImGui::SliderFloat("y scale", &proxy.s_y, 0.2f, 10.0f);
                    changed |= ImGui::SliderFloat("rotations", &proxy.r_e, 0.1f, 20.0f);
                    changed |= ImGui::SliderInt("segments per rotation", (int*)&proxy.n_se, 3, 40);
                    changed |= ImGui::SliderFloat("height variation", &proxy.delta_he, 0.0f, 10.0f);
                    changed |= ImGui::SliderFloat("length variation", &proxy.delta_we, 0.0f, 10.0f);
                }
                else {
                    changed |= ImGui::SliderFloat("Length", &proxy.l_v, 1.0f, 50.0f);
                    changed |= ImGui::SliderFloat("Horizontal shift", &proxy.o_wv, -5.0f, 5.0f);
                    changed |= ImGui::SliderFloat("Vertival shift", &proxy.o_hv, -5.0f, 5.0f);
                    changed |= ImGui::SliderFloat("Initial scale", &proxy.s_bv, 0.1f, 5.0f);
                    changed |= ImGui::SliderFloat("Final scale", &proxy.s_ev, 0.1f, 5.0f);
                    changed |= ImGui::SliderFloat("Twist", &proxy.r_v, -2.0f, 2.0f);
                    changed |= ImGui::SliderInt("segments per rotation", (int*)&proxy.n_sv, 1, 50);
                }
                if (changed) {
                    proxy.renewMesh();
                }
                bool buttonPress = false;
                static char textBoxStr[50] = "teste";
                buttonPress = ImGui::Button("Save mesh");
                ImGui::SameLine();
                ImGui::InputText("mesh name", textBoxStr, IM_ARRAYSIZE(textBoxStr));

                if (buttonPress) {
                    std::string name(textBoxStr);
                    TriangleMesh* mesh = proxy.getMesh();
                    MeshWriter meshWriter(mesh->data());
                    meshWriter.writeMesh(name);
                }
            }
        }

        void
            MySceneWindow::setScene(Scene& scene)
        {
            SceneObjectBuilder::setScene(scene);
            _currentNode = &scene;
            editor()->setScene(scene);
            _viewMode = ViewMode::Editor;
        }

        void
            MySceneWindow::renderScene()
        {
            // do nothing
        }

        void
            MySceneWindow::drawComponents(const SceneObject& object)
        {
            auto editor = this->editor();
            const auto& components = object.components();

            // Iterate the object components skipping its transform
            for (auto end = components.end(), cit = ++components.begin(); cit != end;)
            {
                Component* c{ *cit++ };

                if (auto proxy = graph::asLight(c))
                    editor->drawLight(*proxy->light());
                else if (auto proxy = graph::asCamera(c))
                {
                    editor->drawCamera(*proxy->camera());
                    preview(*proxy->camera());
                }
                else if (auto proxy = graph::asPrimitive(c))
                {
                    auto p = proxy->mapper()->primitive();

                    assert(p != nullptr);
                    if (auto mesh = p->tesselate())
                    {
                        editor->setPolygonMode(GLGraphics3::LINE);
                        editor->setMeshColor(_selectedWireframeColor);
                        editor->drawMesh(*mesh, p->localToWorldMatrix(), p->normalMatrix());
                    }
                }
            }
        }

        void
            MySceneWindow::drawSelectedObject(const SceneObject& object)
        {
            if (!object.visible())
                return;
            drawComponents(object);
            for (auto& child : object.children())
                drawSelectedObject(*child);
        }

        void
            MySceneWindow::render()
        {
            if (_viewMode != ViewMode::Editor)
            {
                renderScene();
                return;
            }
            SceneWindowBase::render();
            if (auto object = _currentNode.as<SceneObject>())
            {
                const auto t = object->transform();

                drawSelectedObject(*object);
                editor()->drawTransform(t->position(), t->rotation());
            }
        }

        void
            MySceneWindow::createObjectMenu()
        {
            // do nothing
        }

        inline void
            MySceneWindow::createObjectButton()
        {
            if (ImGui::Button("Create###CreateObjectButton"))
                ImGui::OpenPopup("CreateObjectPopup");
            if (ImGui::BeginPopup("CreateObjectPopup"))
            {
                if (ImGui::MenuItem("Empty Object"))
                    createEmptyObject();
                ImGui::Separator();
                createObjectMenu();
                if (ImGui::BeginMenu("Light"))
                {
                    if (ImGui::MenuItem("Directional Light"))
                        createLightObject(Light::Type::Directional);
                    if (ImGui::MenuItem("Point Light"))
                        createLightObject(Light::Type::Point);
                    if (ImGui::MenuItem("Spotlight"))
                        createLightObject(Light::Type::Spot);
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Camera"))
                    createCameraObject();
                ImGui::EndPopup();
            }
        }

        inline bool
            MySceneWindow::treeNode(SceneNode node, ImGuiTreeNodeFlags flags)
        {
            if (node == _currentNode)
                flags |= ImGuiTreeNodeFlags_Selected;

            auto open = ImGui::TreeNodeEx(node, flags, node->name());

            if (ImGui::IsItemClicked())
                _currentNode = node;
            return open;
        }

        inline bool
            MySceneWindow::deleteObjectPopup(SceneObject& object)
        {
            auto deleted = false;

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete###DeleteObject"))
                {
                    auto parent = object.parent();

                    if (parent->parent() == nullptr)
                        _currentNode = parent->scene();
                    else
                        _currentNode = parent;
                    parent->removeChild(&object);
                    deleted = true;
                }
                ImGui::EndPopup();
            }
            return deleted;
        }

        namespace
        { // begin namespace

            bool
                dropSceneObject(SceneObject& target)
            {
                if (ImGui::BeginDragDropTarget() && target.movable())
                {
                    if (auto* payload = ImGui::AcceptDragDropPayload("SceneObject"))
                    {
                        auto object = *(SceneObject**)payload->Data;

                        if (&target != object)
                        {
                            object->setParent(&target);
                            return true;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                return false;
            }

        } // end namespace

        bool
            MySceneWindow::objectHierarchy(SceneObject& object)
        {
            for (auto& child : object.children())
            {
                ImGuiTreeNodeFlags flags{ ImGuiTreeNodeFlags_OpenOnArrow };
                auto hasChildren = child.childrenCount() > 0;

                if (!hasChildren)
                    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                auto open = treeNode(&child, flags);
                auto movable = child.movable() && !deleteObjectPopup(child);

                if (movable && ImGui::BeginDragDropSource())
                {
                    auto* data = &child;

                    ImGui::Text(child.name());
                    ImGui::SetDragDropPayload("SceneObject", &data, sizeof(SceneObject*));
                    ImGui::EndDragDropSource();
                }

                auto dropped = dropSceneObject(child);

                if (open)
                {
                    if (movable)
                        dropped |= objectHierarchy(child);
                    if (hasChildren)
                        ImGui::TreePop();
                }
                if (dropped)
                    return true;
            }
            return false;
        }

        void
            MySceneWindow::hierarchyWindow(const char* title)
        {
            if (!_showHierarchy)
                return;
            assert(title != nullptr);
            ImGui::Begin(title);
            createObjectButton();
            ImGui::Separator();
            if (treeNode(_scene.get(), ImGuiTreeNodeFlags_OpenOnArrow))
            {
                auto root = _scene->root();

                dropSceneObject(*root);
                objectHierarchy(*root);
                ImGui::TreePop();
            }
            ImGui::End();
        }

        void
            MySceneWindow::inspectTransform(Transform& transform)
        {
            vec3f temp;

            temp = transform.localPosition();
            if (ImGui::dragVec3("Position", temp))
                transform.setLocalPosition(temp);
            temp = transform.localEulerAngles();
            if (ImGui::dragVec3("Rotation", temp))
                transform.setLocalEulerAngles(temp);
            temp = transform.localScale();
            if (ImGui::dragVec3("Scale", temp, 0.001f, math::Limits<float>::inf()))
                transform.setLocalScale(temp);
        }

        void
            MySceneWindow::inspectComponent(Component& component)
        {
            auto typeName = component.typeName();
            auto notDelete{ true };
            auto open = component.erasable() ?
                ImGui::CollapsingHeader(typeName, &notDelete) :
                ImGui::CollapsingHeader(typeName);

            if (!notDelete)
                component.sceneObject()->removeComponent(typeName);
            else if (open)
            {
                if (auto transform = graph::asTransform(&component))
                    inspectTransform(*transform);
                else if (auto function = inspectFunction(component))
                    function(*this, component);
            }
        }

        void
            MySceneWindow::inspectCamera(MySceneWindow& window, CameraProxy& proxy)
        {
            auto editor = window.editor();
            auto camera = proxy.camera();

            {
                auto isCurrent = camera == CameraProxy::current();

                ImGui::Checkbox("Current", &isCurrent);
                CameraProxy::setCurrent(isCurrent ? camera : nullptr);
            }
            SceneWindowBase::inspectCamera(*camera);
            if (ImGui::Button("Move to Editor View"))
            {
                editor->camera()->set(*camera);
                // Update to continue drawing in the same frame
                editor->update();
            }
            ImGui::SameLine();
            if (ImGui::Button("Set From Editor View"))
            {
                auto c = editor->camera();
                auto t = proxy.transform();

                t->setPosition(c->position());
                t->setRotation(c->rotation());
                camera->setProjection(*c);
            }
        }

        void
            MySceneWindow::inspectLight(MySceneWindow&, LightProxy& proxy)
        {
            auto light = proxy.light();

            SceneWindowBase::inspectLight(*light);
            light->turnOn(light->isTurnedOn() && proxy.sceneObject()->visible());
        }

        void
            MySceneWindow::inspectPrimitive(MySceneWindow&, TriangleMeshProxy& proxy)
        {
            ImGui::inputText("Mesh", proxy.meshName());
            if (ImGui::BeginDragDropTarget())
            {
                if (auto* payload = ImGui::AcceptDragDropPayload("TriangleMesh"))
                {
                    auto mit = *(MeshMapIterator*)payload->Data;
                    proxy.setMesh(*Assets::loadMesh(mit), mit->first);
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::SameLine();
            if (ImGui::Button("...###PrimitiveMesh"))
                ImGui::OpenPopup("PrimitiveMeshPopup");
            if (ImGui::BeginPopup("PrimitiveMeshPopup"))
            {
                if (auto& meshes = Assets::meshes(); !meshes.empty())
                {
                    for (auto mit = meshes.begin(); mit != meshes.end(); ++mit)
                        if (ImGui::Selectable(mit->first.c_str()))
                            proxy.setMesh(*Assets::loadMesh(mit), mit->first);
                }
                ImGui::EndPopup();
            }
            ImGui::Separator();

            auto primitive = proxy.mapper()->primitive();
            auto material = primitive->material();

            ImGui::inputText("Material", material->name());
            if (ImGui::BeginDragDropTarget())
            {
                if (auto* payload = ImGui::AcceptDragDropPayload("Material"))
                {
                    auto mit = *(MaterialMapIterator*)payload->Data;

                    assert(mit->second != nullptr);
                    primitive->setMaterial(material = mit->second);
                }
                ImGui::EndDragDropTarget();
            }
            inspectMaterial(*material);
            proxy.actor()->visible = proxy.sceneObject()->visible();
        }


        Component*
            MySceneWindow::addComponentMenu()
        {
            return nullptr;
        }

        inline void
            MySceneWindow::addComponentButton(SceneObject& object)
        {
            static auto ok = true;

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponentPopup");
            if (ImGui::BeginPopup("AddComponentPopup"))
            {
                auto component = addComponentMenu();

                if (ImGui::MenuItem("Light"))
                    component = LightProxy::New();
                if (ImGui::MenuItem("Camera"))
                    component = CameraProxy::New();
                if (nullptr != component)
                    ok = object.addComponent(component);
                ImGui::EndPopup();
            }
            if (!ok)
                ok = !showErrorMessage("Unable to add this component type.");
        }

        inline void
            MySceneWindow::inspectSceneObject(SceneObject& object)
        {
            addComponentButton(object);
            ImGui::Separator();
            ImGui::objectNameInput(object);
            ImGui::SameLine();

            bool state{ object.visible() };

            ImGui::Checkbox("###visible", &state);
            object.setVisible(state);
            ImGui::Separator();

            auto& components = object.components();

            // Attention: do *NOT* modify this for (the iterator postfix increment
            // is necessary to avoid an exception in case the component is removed
            // by the user during the object inspection)
            for (auto end = components.end(), cit = components.begin(); cit != end;)
                inspectComponent(*((cit++)->get()));
        }

        inline void
            MySceneWindow::inspectScene()
        {
            ImGui::objectNameInput(*_scene);
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Environment"))
            {
                ImGui::colorEdit3("Background", _scene->backgroundColor);
                ImGui::colorEdit3("Ambient Light", _scene->ambientLight);
            }
        }

        inline void
            MySceneWindow::inspectCurrentNode()
        {
            if (_currentNode == nullptr)
                return;
            if (_currentNode.as<Scene>())
                inspectScene();
            else if (auto sceneObject = _currentNode.as<SceneObject>())
                inspectSceneObject(*sceneObject);
        }

        void
            MySceneWindow::inspectorWindow(const char* title)
        {
            if (!_showInspector)
                return;
            assert(title != nullptr);
            ImGui::Begin(title);
            inspectCurrentNode();
            ImGui::End();
        }

        void
            MySceneWindow::assetsWindow()
        {
            if (!_showAssets)
                return;
            ImGui::Begin("Assets");
            if (ImGui::CollapsingHeader("Meshes"))
            {
                auto& meshes = Assets::meshes();

                for (auto mit = meshes.begin(); mit != meshes.end(); ++mit)
                {
                    auto name = mit->first.c_str();
                    auto selected = false;

                    ImGui::Selectable(name, &selected);
                    if (ImGui::BeginDragDropSource())
                    {
                        ImGui::Text(name);
                        ImGui::SetDragDropPayload("TriangleMesh", &mit, sizeof(mit));
                        ImGui::EndDragDropSource();
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Materials"))
            {
                auto& materials = Assets::materials();

                for (auto mit = materials.begin(); mit != materials.end(); ++mit)
                {
                    auto name = mit->first.c_str();
                    auto selected = false;

                    ImGui::Selectable(name, &selected);
                    if (ImGui::BeginDragDropSource())
                    {
                        ImGui::Text(name);
                        ImGui::SetDragDropPayload("Material", &mit, sizeof(mit));
                        ImGui::EndDragDropSource();
                    }
                }
            }
            ImGui::End();
        }

        SceneObject*
            MySceneWindow::pickObject(SceneObject* object, const Ray3f& ray, float& t) const
        {
            if (!object->visible())
                return nullptr;

            SceneObject* nearest{};

            for (auto& component : object->components())
                if (auto proxy = dynamic_cast<PrimitiveProxy*>(&*component))
                {
                    auto p = proxy->mapper()->primitive();

                    if (p->bounds().size().min() == 0)
                        puts("Unable to pick scene object");
                    else
                    {
                        Intersection hit;

                        if (p->intersect(ray, hit) && hit.distance < t)
                        {
                            t = hit.distance;
                            nearest = object;
                        }
                    }
                    break;
                }
            for (auto& child : object->children())
                if (auto temp = pickObject(&child, ray, t))
                    nearest = temp;
            return nearest;
        }

        SceneObject*
            MySceneWindow::pickObject(int x, int y) const
        {
            auto ray = makeRay(x, y);
            auto t = math::Limits<float>::inf();

            return pickObject(_scene->root(), ray, t);
        }

        bool
            MySceneWindow::onPickObject(int x, int y)
        {
            if (auto o = pickObject(x, y))
                if (o->selectable())
                {
                    if (auto p = _currentNode.as<SceneObject>())
                        p->setSelected(false);
                    o->setSelected(true);
                    _currentNode = o;
                }
            return true;
        }

        bool
            MySceneWindow::onPressKey(int key)
        {
            if (_viewMode != ViewMode::Editor || key != GLFW_KEY_F)
                return false;
            if (auto object = _currentNode.as<SceneObject>())
            {
                auto editor = this->editor();
                auto camera = editor->camera();
                auto d = camera->viewPlaneNormal();
                auto p = object->transform()->position();

                // TODO: set view angle/height and distance
                camera->setPosition(p + d * editor->orbitDistance());
            }
            return true;
        }

    } // end namespace graph

} // end namespace cg
