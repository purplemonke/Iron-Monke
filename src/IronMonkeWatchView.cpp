
#include "IronMonkeWatchView.hpp"
#include "config.hpp"
#include "monkecomputer/shared/ViewLib/MonkeWatch.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "UnityEngine/Vector3.hpp"

DEFINE_TYPE(IronMonke::IronMonkeWatchView);

using namespace GorillaUI;
using namespace UnityEngine;

extern bool isRoom;
extern void powerCheck();

namespace IronMonke
{
    void IronMonkeWatchView::Awake()
    {
        toggleHandler = new UIToggleInputHandler(EKeyboardKey::Enter, EKeyboardKey::Enter, true);
        settingSelector = new UISelectionHandler(EKeyboardKey::Up, EKeyboardKey::Down, EKeyboardKey::Enter, true, false);
        powerSelector = new UISelectionHandler(EKeyboardKey::Left, EKeyboardKey::Right, EKeyboardKey::Enter, false, true);

        settingSelector->max = 2;
        powerSelector->max = 5;

        powerSelector->currentSelectionIndex = config.power;
    }

    void IronMonkeWatchView::DidActivate(bool firstActivation)
    {
        std::function<void(bool)> fun = std::bind(&IronMonkeWatchView::OnEnter, this, std::placeholders::_1);
        settingSelector->selectionCallback = fun;
        Redraw();
    }

    void IronMonkeWatchView::OnEnter(int index)
    {
        if(index == 0) config.enabled ^= 1;
    }

    void IronMonkeWatchView::Redraw()
    {
        text = "";

        DrawHeader();
        DrawBody();

        MonkeWatch::Redraw();
    }

    void IronMonkeWatchView::DrawHeader()
    {
        text += "<color=#000000><<</color> <color=#FF0000>Ir</color><color=#FF8700>o</color><color=#FFFB00>n M</color><color=#0FFF00>o</color><color=#0036FF>nk</color><color=#B600FF>e</color> <color=#000000>>></color>\n";
    }

    void IronMonkeWatchView::DrawBody()
    {
        text += settingSelector->currentSelectionIndex == 0 ? "<color=#fd0000>></color>" : " ";
        text += config.enabled ? "<color=#00ff00>Enabled</color>" : "<color=#ff0000>Disabled</color>";

        text += "\n\n";
        text += "<b><i>Thrust:</i></b>\n";
        text += settingSelector->currentSelectionIndex == 1 ? "<color=#ff0000><></color> " : " ";
        text += "<color=#AADDAA><></color> ";
        switch (powerSelector->currentSelectionIndex) {
            case 0:
                text += "Default";
                break;
            case 1:
                text += "2";
                break;
            case 2:
                text += "3";
                break;
            case 3:
                text += "4";
                break;
            case 4:
                text += "Maximum";
                break;
            default:
                break;
        }

        if (config.enabled && !isRoom)
        {
            text += "\n\nBut is disabled\ndue to not being in\na private room\n";
        }
        text += "\nMade by Buzz Bzzz bzz BZZZ The 18th#0431";
    }

    void IronMonkeWatchView::OnKeyPressed(int value)
    {
        EKeyboardKey key = (EKeyboardKey)value;
        if (!settingSelector->HandleKey(key)) // if it was not up/down/enter
        {
            switch (settingSelector->currentSelectionIndex)
            {
                case 0:
                    break;
                case 1:
                    powerSelector->HandleKey(key);
                    break;
                default:
                    break;
            }

            config.power = powerSelector->currentSelectionIndex;
            powerCheck();
            SaveConfig();
        }
        Redraw();
    }
}