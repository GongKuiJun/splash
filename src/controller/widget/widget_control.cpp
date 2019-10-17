#include "./controller/widget/widget_control.h"

#include <imgui.h>

#include "./core/scene.h"
#include "./widget_node_view.h"

using namespace std;

namespace Splash
{

/*************/
void GuiControl::render()
{
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("##nodeView", ImVec2(availableSize.x * 0.6, availableSize.y), true);

    // Node view
    if (!_nodeView)
    {
        auto scene = dynamic_cast<Scene*>(_root);
        auto nodeView = make_shared<GuiNodeView>(scene, "Nodes");
        _nodeView = dynamic_pointer_cast<GuiWidget>(nodeView);
    }
    _nodeView->render();

    // Node configuration
    // Select the object the control
    int index = 0;
    string clickedNode = dynamic_pointer_cast<GuiNodeView>(_nodeView)->getClickedNode(); // Used to set the object
                                                                                         // selected for configuration
    auto objectNames = getObjectList();
    for (const auto& name : objectNames)
    {
        if (name == clickedNode)
            _targetIndex = index;
        index++;
    }

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##objectParameters", ImVec2(0, 0), true);

    // Initialize the target
    if (_targetIndex >= 0)
    {
        if (objectNames.size() <= static_cast<uint32_t>(_targetIndex))
            return;
        _targetObjectName = objectNames[_targetIndex];
    }

    if (_targetObjectName != "")
    {
        if (ImGui::Button("Delete selected object"))
        {
            setWorldAttribute("deleteObject", {_targetObjectName});
            _targetObjectName = "";
            _targetIndex = -1;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        auto attributes = getObjectAttributes(_targetObjectName);
        drawAttributes(_targetObjectName, attributes);
    }

    ImGui::EndChild();
}

/*************/
int GuiControl::updateWindowFlags()
{
    ImGuiWindowFlags flags = 0;
    if (_nodeView)
        flags |= _nodeView->updateWindowFlags();
    return flags;
}

} // namespace Splash
