/**
 * Open Space Program
 * Copyright © 2019-2020 Open Space Program Project
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <memory>

#include <osp/Active/activetypes.h>
#include <osp/Universe.h>
#include <osp/UserInputHandler.h>
#include <osp/types.h>

namespace testapp
{

class IDebugObject
{
public:
    // DebugObject(ActiveScene &scene, ActiveEnt ent);
    virtual ~IDebugObject() = default;


    //virtual void set_entity(ActiveEnt m_ent);
};

template <class Derived>
class DebugObject : public IDebugObject
{
    friend Derived;
public:
    DebugObject(osp::active::ActiveScene &scene,
                osp::active::ActiveEnt ent) noexcept :
            m_scene(scene),
            m_ent(ent) {};
    virtual ~DebugObject() = default;

private:
    osp::active::ActiveScene &m_scene;
    osp::active::ActiveEnt m_ent;
};

struct ACompDebugObject
{
    ACompDebugObject(std::unique_ptr<IDebugObject> ptr) noexcept:
        m_obj(std::move(ptr)) {}
    ACompDebugObject(ACompDebugObject&& move) noexcept = default;
    ~ACompDebugObject() noexcept = default;
    ACompDebugObject& operator=(ACompDebugObject&& move) = default;

    std::unique_ptr<IDebugObject> m_obj;
};


class DebugCameraController : public DebugObject<DebugCameraController>
{

public:
    DebugCameraController(osp::active::ActiveScene &scene,
                          osp::active::ActiveEnt ent,
                          osp::input::UserInputHandler &rInput);
    ~DebugCameraController() = default;
    void update_vehicle_mod_pre();
    void update_physics_pre();
    void update_physics_post();

    bool try_switch_vehicle();
    osp::active::ActiveEnt try_get_vehicle_ent();

private:

    //osp::active::ActiveEnt m_orbiting;
    osp::universe::Satellite m_selected;
    osp::Vector3 m_orbitPos;
    float m_orbitDistance;

    osp::active::UpdateOrderHandle_t m_updateVehicleModPre;
    osp::active::UpdateOrderHandle_t m_updatePhysicsPre;
    osp::active::UpdateOrderHandle_t m_updatePhysicsPost;

    osp::input::ControlSubscriber m_controls;
    // Mouse inputs
    //osp::MouseMovementHandle m_mouseMotion;
    //osp::ScrollInputHandle m_scrollInput;
    osp::input::ButtonControlIndex m_rmb;
    // Keyboard inputs
    osp::input::ButtonControlIndex m_up;
    osp::input::ButtonControlIndex m_dn;
    osp::input::ButtonControlIndex m_lf;
    osp::input::ButtonControlIndex m_rt;
    osp::input::ButtonControlIndex m_switch;

    osp::input::ButtonControlIndex m_selfDestruct;

};

}

