// This file is a bit spaghetti-style, but should be easy to understand.
// All parts of OSP can be configured through just C++, and understanding what
// this file is doing is a good start to getting familar with the codebase.
// Replace this entire file eventually.

#include <iostream>
//#include <format>
#include <memory>
#include <thread>
#include <stack>
#include <random>

#include "OSPMagnum.h"
#include "DebugObject.h"

#include "osp/Universe.h"
#include "osp/Satellites/SatActiveArea.h"
#include "osp/Satellites/SatVehicle.h"
#include "osp/Resource/SturdyImporter.h"
#include "osp/Resource/Package.h"

#include "osp/Active/ActiveScene.h"
#include "osp/Active/SysVehicle.h"

#include "adera/Machines/UserControl.h"
#include "adera/Machines/Rocket.h"

#include "planet-a/Satellites/SatPlanet.h"
#include "planet-a/Active/SysPlanetA.h"


/**
 * Starts a magnum application, an active area, and links them together
 */
void magnum_application();

/**
 * As the name implies. This should only be called once for the entire lifetime
 * of the program.
 *
 * prefer not to use names like this anywhere else but main.cpp
 */
void load_a_bunch_of_stuff();

/**
 * Creates a BlueprintVehicle and adds a random mess of 10 part_spamcans to it
 *
 * Also creates a
 *
 * Call load_a_bunch_of_stuff before this function to make sure part_spamcan
 * is loaded
 *
 * @param name
 * @return
 */
osp::Satellite& debug_add_random_vehicle(std::string const& name);

/**
 * The spaghetti command line interface that gets inputs from stdin. This
 * function will only return once the user exits.
 * @return An error code maybe
 */
int debug_cli_loop();

// called only from commands to display information
void debug_print_help();
void debug_print_sats();
void debug_print_hier();
void debug_print_update_order();

// Deals with the underlying OSP universe, with the satellites and stuff. A
// Magnum application or OpenGL context is not required for the universe to
// exist. This also stores loaded resources in packages.
osp::OSPApplication g_osp;

// Deals with the window, OpenGL context, and other game engine stuff that
// often have "Active" written all over them
std::unique_ptr<osp::OSPMagnum> g_ospMagnum;
std::thread g_magnumThread;

// lazily save the arguments to pass to Magnum
int g_argc;
char** g_argv;

int main(int argc, char** argv)
{   
    // eventually do more important things here.
    // just lazily save the arguments
    g_argc = argc;
    g_argv = argv;

    // start doing debug cli loop
    return debug_cli_loop();
}

int debug_cli_loop()
{

    debug_print_help();

    std::string command;

    while(true)
    {
        std::cout << "> ";
        std::cin >> command;

        if (command == "help")
        {
            debug_print_help();
        }
        if (command == "list_uni")
        {
            debug_print_sats();
        }
        if (command == "list_ent")
        {
            debug_print_hier();
        }
        if (command == "list_upd")
        {
            debug_print_update_order();
        }
        else if (command == "start")
        {
            if (g_magnumThread.joinable())
            {
                g_magnumThread.join();
            }
            std::thread t(magnum_application);
            g_magnumThread.swap(t);
        }
        else if (command == "exit")
        {
            if (g_ospMagnum)
            {
                // request exit if application exists
                g_ospMagnum->exit();
            }
            break;
        }
        else
        {
            std::cout << "that doesn't do anything ._.\n";
        }

    }

    // wait for magnum thread to exit if it exists
    if (g_magnumThread.joinable())
    {
        g_magnumThread.join();
    }

    // destory the universe
    g_osp.get_universe().get_sats().clear();
    return 0;
}

void magnum_application()
{
    // Create the application
    g_ospMagnum = std::make_unique<osp::OSPMagnum>(
                        osp::OSPMagnum::Arguments{g_argc, g_argv});

    // Configure Controls

    using Key = osp::OSPMagnum::KeyEvent::Key;
    using VarOp = osp::ButtonVarConfig::VarOperator;
    using VarTrig = osp::ButtonVarConfig::VarTrigger;
    osp::UserInputHandler& userInput = g_ospMagnum->get_input_handler();

    // vehicle control, used by MachineUserControl

    // would help to get an axis for yaw, pitch, and roll, but use individual
    // axis buttons for now
    userInput.config_register_control("vehicle_pitch_up", true,
            {{0, (int) Key::S, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("vehicle_pitch_dn", true,
            {{0, (int) Key::W, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("vehicle_yaw_lf", true,
            {{0, (int) Key::A, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("vehicle_yaw_rt", true,
            {{0, (int) Key::D, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("vehicle_roll_lf", true,
            {{0, (int) Key::Q, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("vehicle_roll_rt", true,
            {{0, (int) Key::E, VarTrig::PRESSED, false, VarOp::AND}});

    // Set throttle max to Z
    userInput.config_register_control("vehicle_thr_max", false,
            {{0, (int) Key::Z, VarTrig::PRESSED, false, VarOp::OR}});
    // Set throttle min to X
    userInput.config_register_control("vehicle_thr_min", false,
            {{0, (int) Key::X, VarTrig::PRESSED, false, VarOp::OR}});
    // Set self destruct to LeftCtrl+C or LeftShift+A. this just prints
    // a silly message.
    userInput.config_register_control("vehicle_self_destruct", false,
            {{0, (int) Key::LeftCtrl, VarTrig::HOLD, false, VarOp::AND},
             {0, (int) Key::C, VarTrig::PRESSED, false, VarOp::OR},
             {0, (int) Key::LeftShift, VarTrig::HOLD, false, VarOp::AND},
             {0, (int) Key::A, VarTrig::PRESSED, false, VarOp::OR}});

    // Camera and Game controls, handled in DebugCameraController

    // Switch to next vehicle
    userInput.config_register_control("game_switch", false,
            {{0, (int) Key::V, VarTrig::PRESSED, false, VarOp::OR}});

    // Set UI Up/down/left/right to arrow keys. this is used to rotate the view
    // for now
    userInput.config_register_control("ui_up", true,
            {{0, (int) Key::Up, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("ui_dn", true,
            {{0, (int) Key::Down, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("ui_lf", true,
            {{0, (int) Key::Left, VarTrig::PRESSED, false, VarOp::AND}});
    userInput.config_register_control("ui_rt", true,
            {{0, (int) Key::Right, VarTrig::PRESSED, false, VarOp::AND}});

    // only call load once, since some stuff might already be loaded
    if (!g_osp.debug_get_packges().size())
    {
        load_a_bunch_of_stuff();
    }

    // create a satellite with an ActiveArea
    osp::Satellite& sat = g_osp.get_universe().sat_create();
    osp::SatActiveArea& area = sat.create_object<osp::SatActiveArea>(
                                    g_ospMagnum->get_input_handler());
    sat.set_position({Vector3s(0, 0, 0), 10});

    // Activate it
    area.activate(g_osp);

    // Register dynamic systems
    area.get_scene()->dynamic_system_add<osp::SysPlanetA>("Planet");

    // Register machines
    area.get_scene()->system_machine_add<osp::SysMachineUserControl>
            ("UserControl", g_ospMagnum->get_input_handler());
    area.get_scene()->system_machine_add<osp::SysMachineRocket>
            ("Rocket");

    // Make active areas load vehicles and planets
    area.activate_func_add(&(osp::SatVehicle::get_id_static()),
                           osp::SysVehicle::area_activate_vehicle);
    area.activate_func_add(&(osp::SatPlanet::get_id_static()),
                           osp::SysPlanetA::area_activate_planet);

    // Add the debug camera controller
    std::unique_ptr<osp::DebugCameraController> camObj
            = std::make_unique<osp::DebugCameraController>(
                    *(area.get_scene()), area.get_camera());
    area.get_scene()->reg_emplace<osp::CompDebugObject>(area.get_camera(),
                                                        std::move(camObj));

    // make the application switch to that area
    g_ospMagnum->set_active_area(area);

    // Note: sat becomes invalid btw, since it refers to a vector elem.
    //       if new satellites are added, this can cause problems

    // Starts the game loop. This function will return when the window is
    // closed. See OSPMagnum::drawEvent
    g_ospMagnum->exec();

    // Close button has been pressed

    std::cout << "Magnum Application closed\n";

    // Kill the active area
    g_osp.get_universe().sat_remove(area.get_satellite());

    // workaround: wipe mesh resources because they're specific to the
    // opengl context
    g_osp.debug_get_packges()[0].clear<Magnum::GL::Mesh>();

    // destruct the application, this closes the window
    g_ospMagnum.reset();
}

void load_a_bunch_of_stuff()
{
    // Create a new package
    osp::Package lazyDebugPack("lzdb", "lazy-debug");

    // Create a sturdy
    osp::SturdyImporter importer;
    importer.open_filepath("OSPData/adera/spamcan.sturdy.gltf");

    // load the sturdy into the package
    importer.load_config(lazyDebugPack);

    // Add package to the univere
    g_osp.debug_get_packges().push_back(std::move(lazyDebugPack));

    // Add 50 vehicles so there's something to load
    g_osp.get_universe().get_sats().reserve(64);

    for (int i = 0; i < 50; i ++)
    {
        // Creates a random mess of spamcans
        osp::Satellite& sat = debug_add_random_vehicle(std::to_string(i));

        // Put them in a long chaotic line
        Vector3sp randomvec(Magnum::Math::Vector3<SpaceInt>(
                i * 1024 * 5,
                (i & 5) * 128,
                (i & 3) * 1024), 10);

        sat.set_position(randomvec);
    }

    // Add a planet too
    osp::Satellite& planet = g_osp.get_universe().sat_create();
    planet.create_object<osp::SatPlanet>();

    //s_partsLoaded = true;
}

osp::Satellite& debug_add_random_vehicle(std::string const& name)
{

    // Start making the blueprint

    osp::BlueprintVehicle blueprint;

    // Part to add, very likely a spamcan
    osp::DependRes<osp::PrototypePart> victim =
            g_osp.debug_get_packges()[0]
            .get<osp::PrototypePart>("part_spamcan");

    // Add 10 parts
    for (int i = 0; i < 10; i ++)
    {
        // Generate random vector
        Vector3 randomvec(std::rand() % 64 - 32,
                          std::rand() % 64 - 32,
                          std::rand() % 64 - 32);

        randomvec /= 32.0f;

        // Add a new [victim] part
        blueprint.add_part(victim, randomvec,
                           Quaternion(), Vector3(1, 1, 1));
        //std::cout << "random: " <<  << "\n";
    }

    // Wire throttle control
    // from (output): a MachineUserControl m_woThrottle
    // to    (input): a MachineRocket m_wiThrottle
    blueprint.add_wire(0, 0, 1,
                       0, 1, 2);

    // Wire attitude control to gimbal
    // from (output): a MachineUserControl m_woAttitude
    // to    (input): a MachineRocket m_wiGimbal
    blueprint.add_wire(0, 0, 0,
                       0, 1, 0);

    // put blueprint in package
    osp::DependRes<osp::BlueprintVehicle> depend = g_osp.debug_get_packges()[0]
            .add<osp::BlueprintVehicle>(name, std::move(blueprint));

    // Create the Satellite containing a SatVehicle

    osp::Satellite &sat = g_osp.get_universe().sat_create();
    osp::SatVehicle &vehicle = sat.create_object<osp::SatVehicle>();

    // set the SatVehicle's blueprint to the one just made
    vehicle.get_blueprint_depend() = std::move(depend);

    return sat;

}

void debug_print_help()
{
    std::cout
        << "OSP-Magnum Temporary Debug CLI\n"
        << "Things to type:\n"
        << "* start     - Create an ActiveArea and start Magnum\n"
        << "* list_uni  - List Satellites in the universe\n"
        << "* list_ent  - List Entities in active scene\n"
        << "* list_upd  - List Update order from active scene\n"
        << "* help      - Show this again\n"
        << "* exit      - Deallocate everything and return memory to OS\n";
}

void debug_print_update_order()
{
    if (!g_ospMagnum)
    {
        std::cout << "Can't do that yet, start the magnum application first!\n";
        return;
    }

    osp::UpdateOrder& order = g_ospMagnum->get_active_area()
                                ->get_scene()->get_update_order();

    std::cout << "Update order:\n";
    for (auto call : order.get_call_list())
    {
        std::cout << "* " << call.m_name << "\n";
    }
}

void debug_print_hier()
{
    if (!g_ospMagnum)
    {
        std::cout << "Can't do that yet, start the magnum application first!\n";
        return;
    }

    std::cout << "Entity Hierarchy:\n";

    std::vector<osp::ActiveEnt> parentNextSibling;
    osp::ActiveScene &scene = *(g_ospMagnum->get_active_area()->get_scene());
    osp::ActiveEnt currentEnt = scene.hier_get_root();

    parentNextSibling.reserve(16);

    while (true)
    {
        // print some info about the entity
        osp::CompHierarchy &hier = scene.reg_get<osp::CompHierarchy>(currentEnt);
        for (unsigned i = 0; i < hier.m_level; i ++)
        {
            // print arrows to indicate level
            std::cout << "  ->";
        }
        std::cout << "[" << int(currentEnt) << "]: " << hier.m_name << "\n";

        if (hier.m_childCount)
        {
            // entity has some children
            currentEnt = hier.m_childFirst;


            // save next sibling for later if it exists
            if (hier.m_siblingNext != entt::null)
            {
                parentNextSibling.push_back(hier.m_siblingNext);
            }
        }
        else if (hier.m_siblingNext != entt::null)
        {
            // no children, move to next sibling
            currentEnt = hier.m_siblingNext;
        }
        else if (parentNextSibling.size())
        {
            // last sibling, and not done yet
            // is last sibling, move to parent's (or ancestor's) next sibling
            currentEnt = parentNextSibling.back();
            parentNextSibling.pop_back();
        }
        else
        {
            break;
        }
    }
}

void debug_print_sats()
{
    std::vector<osp::Satellite>& sats = g_osp.get_universe().get_sats();

    // Loop through g_universe's satellites and print them.
    std::cout << "Universe:\n";
    for (osp::Satellite& sat : sats)
    {
        Vector3sp pos = sat.get_position();
        std::cout << "* " << sat.get_name() << "["
                  << pos.x() << ", " << pos.y() << ", " << pos.z() << "] ("
                  << sat.get_object()->get_id().m_name << ")\n";
    }

}
