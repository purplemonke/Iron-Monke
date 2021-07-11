#include <thread>
#include "modloader/shared/modloader.hpp"
#include "GorillaLocomotion/Player.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils-functions.h"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/ForceMode.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/CapsuleCollider.hpp"
#include "UnityEngine/SphereCollider.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RaycastHit.hpp"
#include "UnityEngine/Physics.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/XR/InputDevice.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "custom-types/shared/register.hpp"
#include "config.hpp"
#include "IronMonkeWatchView.hpp"

ModInfo modInfo;

#define INFO(value...) getLogger().info(value)
#define ERROR(value...) getLogger().error(value)

using namespace UnityEngine;
using namespace UnityEngine::XR;
using namespace GorillaLocomotion;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool isRoom = false;
float thrust = 0;

void powerCheck() {
    if(config.power == 0) {
        thrust = 100;
    }
    if(config.power == 1) {
        thrust = 200;
    }
    if(config.power == 2) {
        thrust = 300;
    }
    if(config.power == 3) {
        thrust = 350;
    }
    if(config.power == 4) {
        thrust = 400;
    }
}

MAKE_HOOK_OFFSETLESS(PhotonNetworkController_OnJoinedRoom, void, Il2CppObject* self) {
    
    PhotonNetworkController_OnJoinedRoom(self);

    Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

    if (currentRoom)
    {
        isRoom = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
    }
    else isRoom = true;

}

bool rightInput = false;
bool leftInput = false;

void updateButton() {
    using namespace GlobalNamespace;

    bool BInput = false;
    bool YInput = false;

    BInput = OVRInput::Get(OVRInput::Button::Two, OVRInput::Controller::RTouch);
    YInput = OVRInput::Get(OVRInput::Button::Two, OVRInput::Controller::LTouch);

    if(isRoom && config.enabled) {
        if(BInput) {
            INFO("BUZZ Pressing B");
            rightInput = true;
        } else rightInput = false;
        if(YInput){
            INFO("BUZZ Pressing Y");
            leftInput = true;
        } else leftInput = false;
    }
}

#include "GlobalNamespace/GorillaTagManager.hpp"

MAKE_HOOK_OFFSETLESS(GorillaTagManager_Update, void, GlobalNamespace::GorillaTagManager* self) {

    using namespace GlobalNamespace;
    using namespace GorillaLocomotion;
    GorillaTagManager_Update(self);
    powerCheck();
    INFO("Running GTManager hook BUZZ");

    Player* playerInstance = Player::get_Instance();
    if(playerInstance == nullptr) return;
    GameObject* go = playerInstance->get_gameObject();
    auto* player = go->GetComponent<GorillaLocomotion::Player*>();

    Rigidbody* playerPhysics = playerInstance->playerRigidBody;
    if(playerPhysics == nullptr) return;

    GameObject* playerGameObject = playerPhysics->get_gameObject();
    if(playerGameObject == nullptr) return;

    if(isRoom && config.enabled) {
        if(rightInput) {
            Transform* rightHandT = player->rightHandTransform;
            playerPhysics->set_useGravity(false);
            playerPhysics->AddForce(rightHandT->get_right() * thrust);
            INFO("BUZZ Right input");
        }
        if(leftInput) {
            Transform* leftHandT = player->leftHandTransform;
            playerPhysics->set_useGravity(false);
            playerPhysics->AddForce(leftHandT->get_right() * -thrust);
            INFO("BUZZ Left input");
        }
        bool wasInput = leftInput | rightInput;
        if(!wasInput) playerPhysics->set_useGravity(true);
        INFO("In private room BUZZ");
    }
    if(!isRoom) {
        playerPhysics->set_useGravity(true);
    }
}

MAKE_HOOK_OFFSETLESS(Player_Update, void, Il2CppObject* self)
{
    using namespace UnityEngine;
    using namespace GlobalNamespace;
    INFO("player update was called");
    Player_Update(self);
    updateButton();
}

extern "C" void setup(ModInfo& info)
{
    info.id = ID;
    info.version = VERSION;

    modInfo = info;
}

extern "C" void load()
{

    GorillaUI::Init();

    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "OnJoinedRoom", 0));
	INSTALL_HOOK_OFFSETLESS(getLogger(), Player_Update, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaTagManager_Update, il2cpp_utils::FindMethodUnsafe("", "GorillaTagManager", "Update", 0));

    custom_types::Register::RegisterType<IronMonke::IronMonkeWatchView>(); 
    GorillaUI::Register::RegisterWatchView<IronMonke::IronMonkeWatchView*>("<b><i><color=#FF0000>Ir</color><color=#FF8700>o</color><color=#FFFB00>n M</color><color=#0FFF00>o</color><color=#0036FF>nk</color><color=#B600FF>e</color></i></b>", VERSION);

    LoadConfig();
}