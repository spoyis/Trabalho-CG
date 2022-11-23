#pragma once

#include "SweeperProxy.h"
#include "graphics/SceneWindowBase.h"
#include "graph/SceneObjectBuilder.h"
#include "graphics/Assets.h"
#include <cassert>

namespace cg::graph
{
    class MySceneObjectBuilder : public SceneObjectBuilder
    {
    public:
        SweeperProxy* makeSweeper(const std::string& meshName)
        {
            return SweeperProxy::New(meshName);
        }
        SceneObject* createSweeperObject(const std::string& meshName)
        {
            auto object = SceneObject::New(*_scene);

            object->setName("%s %d", meshName.c_str(), ++_primitiveId);
            object->addComponent(makeSweeper(meshName));
            return object;
        }
    };

	class MySceneWindow : public MySceneObjectBuilder, public SceneWindowBase {
    public:
        template <typename C = SharedObject>
        using InspectFunction = void (*)(MySceneWindow&, C&);

        MySceneWindow(const char* title, int width, int height) : SceneWindowBase{ title, width, height }
        {
            registerInspectFunction<CameraProxy>(inspectCamera);
            registerInspectFunction<LightProxy>(inspectLight);
            registerInspectFunction<TriangleMeshProxy>(inspectPrimitive);
            registerInspectFunction<SweeperProxy>(inspectSweeper);
        }

        static void inspectSweeper(MySceneWindow&, SweeperProxy& proxy);


        template <typename C>
        void registerInspectFunction(InspectFunction<C> function)
        {
            assert(function != nullptr);
            _inspectFunctions[typeid(C).hash_code()] = (InspectFunction<>)function;
        }

        Scene* scene() const
        {
            return (Scene*)editor()->scene();
        }

    protected:
        enum class ViewMode
        {
            Editor = 0,
            Renderer = 1
        };

        ViewMode _viewMode{};
        bool _showHierarchy{ true };
        bool _showInspector{ true };
        bool _showAssets{ true };

        void setScene(Scene&);

        void render() override;
        bool onPickObject(int, int) override;
        bool onPressKey(int) override;

        virtual void renderScene();
        virtual void createObjectMenu();
        virtual Component* addComponentMenu();

        void drawSelectedObject(const SceneObject&);
        void drawComponents(const SceneObject&);

        virtual SceneObject* pickObject(int, int) const;
        SceneObject* pickObject(SceneObject*, const Ray3f&, float&) const;

        void hierarchyWindow(const char* = "Hierarchy");
        void inspectorWindow(const char* = "Inspector");
        void assetsWindow();

        static void inspectTransform(Transform&);
        static void inspectCamera(MySceneWindow&, CameraProxy&);
        static void inspectLight(MySceneWindow&, LightProxy&);
        static void inspectPrimitive(MySceneWindow&, TriangleMeshProxy&);


    private:
        using InspectMap = std::unordered_map<size_t, InspectFunction<>>;

        SceneNode _currentNode{};
        InspectMap _inspectFunctions;

        SceneBase* makeScene() override;

        void createObjectButton();
        bool treeNode(SceneNode, ImGuiTreeNodeFlags);
        bool deleteObjectPopup(SceneObject&);
        bool objectHierarchy(SceneObject&);
        void addComponentButton(SceneObject&);

        auto inspectFunction(Component & component)
        {
            return _inspectFunctions[typeid(component).hash_code()];
        }

        void inspectSceneObject(SceneObject&);
        void inspectScene();
        void inspectCurrentNode();
        void inspectComponent(Component&);
        
	};
}