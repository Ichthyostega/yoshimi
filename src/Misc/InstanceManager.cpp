/*
    InstanceManager.h - manage lifecycle of Synth-Engine instances

    Copyright 2024,  Ichthyostega

    Based on existing code from main.cpp
    Copyright 2009-2011, Alan Calvert
    Copyright 2014-2021, Will Godfrey & others

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License (version 2
    or later) for more details.

    You should have received a copy of the GNU General Public License
    along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "Misc/InstanceManager.h"
#include "Misc/SynthEngine.h"
#include "MusicIO/MusicClient.h"
#include "Misc/FormatFuncs.h"
#include "Misc/Util.h"
#ifdef GUI_FLTK
    #include "MasterUI.h"
#endif

#include <mutex>
#include <memory>
#include <thread>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <array>
#include <set>

using std::string;
using std::function;
using std::make_unique;
using std::unique_ptr;
using std::for_each;
using std::move;
using std::set;

using func::asString;

using Guard = const std::lock_guard<std::mutex>;



namespace { // implementation details...

    // Maximum number of SynthEngine instances allowed.
    // Historically, this limit was imposed due to using a 32bit field;
    // theoretically this number is unlimited, yet in practice, the system's
    // available resources will likely impose an even stricter limit...
    const uint MAX_INSTANCES = 32;


    /** Combinations to try, in given order, when booting an instance */
    auto drivers_to_probe(Config const& current)
    {
        using Scenario = std::pair<audio_driver,midi_driver>;
        return std::array{Scenario{current.audioEngine, current.midiEngine}
                         ,Scenario{jack_audio, alsa_midi}
                         ,Scenario{jack_audio, jack_midi}
                         ,Scenario{alsa_audio, alsa_midi}
                         ,Scenario{jack_audio, no_midi}
                         ,Scenario{alsa_audio, no_midi}
                         ,Scenario{no_audio, alsa_midi}
                         ,Scenario{no_audio, jack_midi}
                         ,Scenario{no_audio, no_midi}//this one always will do the work :)
                         };
    }

    string display(audio_driver audio)
    {
        switch (audio)
        {
            case   no_audio : return "no_audio";
            case jack_audio : return "jack_audio";
            case alsa_audio : return "alsa_audio";
            default:
                throw std::logic_error("Unknown audio driver ID");
    }   }

    string display(midi_driver midi)
    {
        switch (midi)
        {
            case   no_midi : return "no_midi";
            case jack_midi : return "jack_midi";
            case alsa_midi : return "alsa_midi";
            default:
                throw std::logic_error("Unknown MIDI driver ID");
        }
    }


    /**
     * Instance Lifecycle
     */
    enum LifePhase {
        PENDING = 0,
        BOOTING,
        RUNNING,
        WANING,
        DEFUNCT
    };
}


/**
 * Descriptor: Synth-Engine instance
 */
class InstanceManager::Instance
{

        unique_ptr<SynthEngine> synth;
        unique_ptr<MusicClient> client;

        LifePhase state{PENDING};

    public: // can be moved and swapped, but not copied...
       ~Instance()                           = default;
        Instance(Instance&&)                 = default;
        Instance(Instance const&)            = delete;
        Instance& operator=(Instance&&)      = delete;
        Instance& operator=(Instance const&) = delete;

        Instance(uint id);

        bool startUp();
        void shutDown();
        void buildGuiMaster();
        void enterRunningState();

        auto& getSynth() { return *synth; }
        Config& runtime() { return synth->getRuntime(); }
        LifePhase getState() { return state; }
        uint getID() const { return synth->getUniqueId(); }
    private:
        bool isPrimary()  { return 0 == getID(); }
};



/**
 * A housekeeper and caretaker responsible for clear-out of droppings.
 * - maintains a registry of all engine instances
 * - serves to further the lifecycle
 * - operates a running state duty cycle
 */
class InstanceManager::SynthGroom
{
        std::mutex mtx;

        using Location = void*;
        struct LocationHash
        {
            size_t operator()(Location const& loc) const { return size_t(loc); }
        };

        using Table = std::unordered_map<const Location, Instance, LocationHash>;

        Table registry;
        Instance* primary{nullptr};

    public: // can be moved and swapped, but not copied...
       ~SynthGroom()                             = default;
        SynthGroom(SynthGroom &&)                = default;
        SynthGroom(SynthGroom const&)            = delete;
        SynthGroom& operator=(SynthGroom &&)     = delete;
        SynthGroom& operator=(SynthGroom const&) = delete;

        // can be default created
        SynthGroom() = default;

        Instance& createInstance(uint instanceID =0)
        {
            Guard lock(mtx);
            instanceID = allocateID(instanceID);
            Instance newEntry{instanceID};
            auto& instance = registry.emplace(&newEntry.getSynth(), move(newEntry))
                                     .first->second;
            if (!primary)
                primary = & instance;
            return instance;
        }

        Instance& getPrimary()
        {
            assert(primary);
            return *primary;
        }

        uint instanceCnt()  const
        {
            return registry.size();
        }

        void dutyCycle(function<void(SynthEngine&)>& handleEvents);

    private:
        uint allocateID(uint);
        void handleStartRequest();
        void clearZombies();
};


InstanceManager::InstanceManager()
    : groom{make_unique<SynthGroom>()}
    , cmdOptions{}
    { }

InstanceManager::~InstanceManager() { }


/** Create Synth-Engine and Connector for a given ID,
 *  possibly loading an existing config for that ID.
 * @remark Engines are created but not yet activated
 */
InstanceManager::Instance::Instance(uint id)
    : synth{make_unique<SynthEngine>(id)}
    , client{make_unique<MusicClient>(*synth)}
    { }



/** boot up this engine instance into working state.
 * - probe a working IO / client setup
 * - init the SynthEngine
 * - start the IO backend
 * @return `true` on success
 * @note after a successful boot, `state == BOOTING`,
 *       which enables some post-boot-hooks to run,
 *       and notably prompts the GUI to become visible;
 *       after that, the state will transition to `RUNNING`.
 *       However, if boot-up fails, `state == EXPIRING` and
 *       further transitioning to `DEFUNCT` after shutdown.
 */
bool InstanceManager::Instance::startUp()
{
    state = BOOTING;
    assert (not runtime().runSynth);
    for (auto [tryAudio,tryMidi] : drivers_to_probe(runtime()))
    {
        if (client->open(tryAudio, tryMidi))
        {
            if (tryAudio == runtime().audioEngine and
                tryMidi == runtime().midiEngine)
                runtime().configChanged = true;
            runtime().audioEngine = tryAudio;
            runtime().midiEngine = tryMidi;
            runtime().runSynth = true;  // mark as active and enable background threads
            runtime().Log("Using "+display(tryAudio)+" for audio and "+display(tryMidi)+" for midi", _SYS_::LogError);
            break;
        }
    }
    if (not runtime().runSynth)
        runtime().Log("Failed to instantiate MusicClient",_SYS_::LogError);
    else
    {
        if (not synth->Init(client->getSamplerate(), client->getBuffersize()))
            runtime().Log("SynthEngine init failed",_SYS_::LogError);
        else
        {
            if (not client->start())
                runtime().Log("Failed to start MusicIO",_SYS_::LogError);
            else
            {// engine started successfully....
#ifdef GUI_FLTK
                if (runtime().showGui)
                    synth->setWindowTitle(client->midiClientName());
                else
                    runtime().toConsole = false;
#else
                runtime().toConsole = false;
#endif
                runtime().StartupReport(client->midiClientName());

                if (isPrimary())
                    std::cout << "\nYay! We're up and running :-)\n";
                else
                    std::cout << "\nStarted "<< synth->getUniqueId() << "\n";

                state = BOOTING;
                return true;
    }   }   }

    state = WANING;
    runtime().Log("Bail: Yoshimi stages a strategic retreat :-(",_SYS_::LogError);
    shutDown();
    return false;
}



/** */
void InstanceManager::Instance::shutDown()
{
    runtime().runSynth = false;
    client->close();
    runtime().flushLog();
    state = DEFUNCT;
}


/** install and start-up the primary SynthEngine and runtime */
bool InstanceManager::bootPrimary()
{
    assert (0 == groom->instanceCnt());
    return groom->createInstance(0).startUp();
    //////////////////////////////////////////OOO TODO ensure that the command options are parsed and special handling for the primary is done!!
    //////////////////////////////////////////OOO do we somehow need to /wait/ for the primary instance to become live?
}

/**
 * Request to allocate a new SynthEngine instance.
 * @return ID of the new instance or zero, if no further instance can be created
 * @remark the new instance will start up asynchronously
 * @warning this function can block for an extended time (>33ms),
 *          since it contends with the event handling duty cycle.
 */
uint InstanceManager::requestNewInstance()
{
    if (groom->instanceCnt() < MAX_INSTANCES)
        return groom->createInstance().getID();

    groom->getPrimary().runtime().LogError("Maximum number("+asString(MAX_INSTANCES)
                                          +") of Synth-Engine instances exceeded.");
    return 0;
}

/**
 * Initiate restoring of specific instances, as persisted in the base config.
 * This function must be called after the »primary« SynthEngine was started, but prior
 * to launching any further instances; the new allotted engines will start asynchronously
 */
void InstanceManager::triggerRestoreInstances()
{
    assert (1 == groom->instanceCnt());
    for (uint id=1; id<MAX_INSTANCES; ++id)
        if (groom->getPrimary().runtime().activeInstances.test(id))
            groom->createInstance(id);
}


void InstanceManager::performWhileActive(function<void(SynthEngine&)> handleEvents)
{
    while (groom->getPrimary().runtime().runSynth)
    {
        groom->dutyCycle(handleEvents);
        std::this_thread::yield();
    }     // tiny break allowing other threads to acquire the mutex
}

void InstanceManager::SynthGroom::dutyCycle(function<void(SynthEngine&)>& handleEvents)
{
    Guard lock(mtx); // warning: concurrent modifications could corrupt instance lifecycle

    for (auto& [_,instance] : registry)
    {
        switch (instance.getState())
        {
            case BOOTING:
                 // successfully booted, make ready for use
                if (primary->runtime().showGui)
                    instance.buildGuiMaster();
                instance.enterRunningState();
            break;
            case RUNNING:
                 // perform GUI and command returns for this instance
                handleEvents(instance.getSynth());
            break;
            default:
                /* do nothing */
            break;
        }
    }
    clearZombies();
    handleStartRequest();
}


/**
 * respond to the request to start a new engine instance, if any
 * @note deliberately handling only a single request, as start-up is
 *       time consuming and risks tailback in other instances' GUI queues.
 */
void InstanceManager::SynthGroom::handleStartRequest()
{
    for (auto& [_,instance] : registry)
        if (PENDING == instance.getState())
        {
            instance.startUp();
            return;  // only one per duty cycle
        }
}

void InstanceManager::SynthGroom::clearZombies()
{
    for (auto it = registry.begin(); it != registry.end();)
    {
        if (it->second.getState() == DEFUNCT)
            it = registry.erase(it);
        else
            ++it;
    }
}

/**
 * Allocate an unique Synth-ID not yet in use.
 * @param desiredID explicitly given desired ID;
 *                  set to zero to request allocation of next free ID
 * @throws std::logic_error if a desired ID is given which is already in use
 * @return new ID which is not currently in use.
 * @remark when called for the first time, ID = 0 will be returned, which
 *         also marks the associated instance as »primary instance«, responsible
 *         for coordinates some application global aspects.
 */
uint InstanceManager::SynthGroom::allocateID(uint desiredID)
{
    set<uint> allIDs;
    for_each(registry.begin(),registry.end()
            ,[&](auto& entry){ allIDs.insert(entry.second.getID()); });

    if (desiredID > 0
        and allIDs.find(desiredID) != allIDs.end())
        throw std::logic_error("Instance Lifecycle broken: "
                               "attempt to allocate a duplicate Synth-ID.");

    if (not desiredID)
    {
        for (uint id : allIDs)
            if (desiredID < id)
                break;
            else
                ++desiredID;
    }

    assert(desiredID < MAX_INSTANCES);
    assert((!primary and 0==desiredID)
          or(primary and 0 < desiredID));

    return desiredID;
}


void InstanceManager::Instance::buildGuiMaster()
{
#ifdef GUI_FLTK
    MasterUI& guiMaster = synth->interchange.createGuiMaster();
    guiMaster.Init();
#endif
}

void InstanceManager::Instance::enterRunningState()
{
    // trigger post-boot-Hook to run in the Synth-thread...
    CommandBlock triggerMsg;

    triggerMsg.data.type    = TOPLEVEL::type::Integer | TOPLEVEL::type::Write;
    triggerMsg.data.control = TOPLEVEL::control::dataExchange;
    triggerMsg.data.part    = TOPLEVEL::section::main;
    triggerMsg.data.source  = TOPLEVEL::action::noAction;
    //                               // Important: not(action::lowPrio) since we want direct execution in Synth
    triggerMsg.data.offset    = UNUSED;
    triggerMsg.data.kit       = UNUSED;
    triggerMsg.data.engine    = UNUSED;
    triggerMsg.data.insert    = UNUSED;
    triggerMsg.data.parameter = UNUSED;
    triggerMsg.data.miscmsg   = UNUSED;
    triggerMsg.data.spare0    = UNUSED;
    triggerMsg.data.spare1    = UNUSED;
    triggerMsg.data.value     = 0;

    // MIDI ringbuffer is the only one always active
    synth->interchange.fromMIDI.write(triggerMsg.bytes);

    // this instance is now in fully operational state...
    runtime().activeInstances.set(this->getID());
    state = RUNNING;
}
