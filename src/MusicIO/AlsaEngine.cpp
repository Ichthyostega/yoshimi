/*
    AlsaEngine.cpp

    Copyright 2009-2010, Alan Calvert

    This file is part of yoshimi, which is free software: you can
    redistribute it and/or modify it under the terms of the GNU General
    Public License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Misc/Util.h"
#include "Misc/Config.h"
#include "Misc/Master.h"
#include "MusicIO/AlsaEngine.h"

AlsaEngine::AlsaEngine() :
    MusicIO()
{
    audio.handle = NULL;
    audio.period_time = 0;
    audio.samplerate = 0;
    audio.buffer_size = 0;
    audio.period_size = 0;
    audio.buffer_size = 0;
    audio.alsaId = -1;
    audio.pThread = 0;
    
    midi.handle = NULL;
    midi.alsaId = -1;
    midi.pThread = 0;

    threadStop = true;
}


bool AlsaEngine::openAudio(WavRecord *recorder)
{
    audio.device = Runtime.audioDevice;
    audio.samplerate = Runtime.Samplerate;
    audio.period_size = Runtime.Buffersize;
    audio.period_time =  audio.period_size * 1000000.0f / audio.samplerate;
    if (alsaBad(snd_pcm_open(&audio.handle, audio.device.c_str(),
                             SND_PCM_STREAM_PLAYBACK, SND_PCM_NO_AUTO_CHANNELS),
            "failed to open alsa audio device:" + audio.device))
        goto bail_out;
        if (!alsaBad(snd_pcm_nonblock(audio.handle, 0), "set blocking failed"))
            if (prepHwparams())
                if (prepSwparams())
                    if (prepBuffers(true))
                        return true;

bail_out:
    Close();
    return false;
}


bool AlsaEngine::openMidi(WavRecord *recorder)
{
    midi.device = Runtime.midiDevice;
    if (midi.device.empty())
        midi.device = "default";
    const char* port_name = "input";
    if (snd_seq_open(&midi.handle, midi.device.c_str(), SND_SEQ_OPEN_INPUT, 0))
    {
        Runtime.Log("Error, failed to open alsa midi device: " + midi.device);
        goto bail_out;
    }
    snd_seq_client_info_t *seq_info;
    snd_seq_client_info_alloca(&seq_info);
    snd_seq_get_client_info(midi.handle, seq_info);
    midi.alsaId = snd_seq_client_info_get_client(seq_info);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_NOTEON);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_NOTEOFF);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_CONTROLLER);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_PGMCHANGE);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_PITCHBEND);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_CONTROL14);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_NONREGPARAM);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_REGPARAM);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_RESET);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_PORT_SUBSCRIBED);
    snd_seq_client_info_event_filter_add(seq_info, SND_SEQ_EVENT_PORT_UNSUBSCRIBED);
    if (0 > snd_seq_set_client_info(midi.handle, seq_info))
        Runtime.Log("Failed to set midi event filtering");
    snd_seq_set_client_name(midi.handle, midiClientName().c_str());

    if (0 > snd_seq_create_simple_port(midi.handle, port_name,
                                       SND_SEQ_PORT_CAP_WRITE
                                       | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                       SND_SEQ_PORT_TYPE_SYNTH))
    {
        Runtime.Log("Error, failed to acquire alsa midi port");
        goto bail_out;
    }
    wavRecorder = recorder;
    midiLatency = getBuffersize();
    return true;

bail_out:
    Close();
    return false;
}


void AlsaEngine::Close(void)
{
    zynMaster->actionLock(lockmute);
    wavRecorder->Close();
    zynMaster->actionLock(unlock);
    threadStop = true;
    if (NULL != audio.handle && audio.pThread)
        if (pthread_cancel(audio.pThread))
            Runtime.Log("Error, failed to cancel Alsa audio thread");
    if (NULL != midi.handle && midi.pThread)
        if (pthread_cancel(midi.pThread))
            Runtime.Log("Error, failed to cancel Alsa midi thread");
    if (audio.handle != NULL)
        alsaBad(snd_pcm_close(audio.handle), "close pcm failed");
    audio.handle = NULL;
    if (NULL != midi.handle)
        if (snd_seq_close(midi.handle) < 0)
            Runtime.Log("Error closing Alsa midi connection");
    midi.handle = NULL;
}


string AlsaEngine::audioClientName(void)
{
    string name = "yoshimi";
    if (!Runtime.nameTag.empty())
        name += ("-" + Runtime.nameTag);
    return name;
}

string AlsaEngine::midiClientName(void)
{
    string name = "yoshimi";
    if (!Runtime.nameTag.empty())
        name += ("-" + Runtime.nameTag);
    return name;
}


bool AlsaEngine::prepHwparams(void)
{
    unsigned int buffer_time = audio.period_time * 4;
    unsigned int ask_samplerate = audio.samplerate;
    unsigned int ask_buffersize = audio.period_size;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16; // Alsa appends _LE/_BE? hmmm
    snd_pcm_access_t axs = SND_PCM_ACCESS_MMAP_INTERLEAVED;
    snd_pcm_hw_params_t  *hwparams;
    snd_pcm_hw_params_alloca(&hwparams);
    if (alsaBad(snd_pcm_hw_params_any(audio.handle, hwparams),
                "alsa audio no playback configurations available"))
        goto bail_out;
    if (alsaBad(snd_pcm_hw_params_set_periods_integer(audio.handle, hwparams),
                "alsa audio cannot restrict period size to integral value"))
        goto bail_out;
    if (!alsaBad(snd_pcm_hw_params_set_access(audio.handle, hwparams, axs),
                 "alsa audio mmap not possible"))
        pcmWrite = &snd_pcm_mmap_writei;
    else
    {
        axs = SND_PCM_ACCESS_RW_INTERLEAVED;
        if (alsaBad(snd_pcm_hw_params_set_access(audio.handle, hwparams, axs),
                     "alsa audio failed to set access, both mmap and rw failed"))
            goto bail_out;
        pcmWrite = &snd_pcm_writei;
    }
    if (alsaBad(snd_pcm_hw_params_set_format(audio.handle, hwparams, format),
                "alsa audio failed to set sample format"))
        goto bail_out;
    alsaBad(snd_pcm_hw_params_set_rate_resample(audio.handle, hwparams, 1),
            "alsa audio failed to set allow resample");
    if (alsaBad(snd_pcm_hw_params_set_rate_near(audio.handle, hwparams,
                                                &audio.samplerate, NULL),
                "alsa audio failed to set sample rate (asked for "
                + asString(ask_samplerate) + ")"))
        goto bail_out;
    if (alsaBad(snd_pcm_hw_params_set_channels(audio.handle, hwparams, 2),
                "alsa audio failed to set channels to 2"))
        goto bail_out;
    if (!alsaBad(snd_pcm_hw_params_set_buffer_time_near(audio.handle, hwparams,
                 &buffer_time, NULL), "initial buffer time setting failed"))
    {
        if (alsaBad(snd_pcm_hw_params_get_buffer_size(hwparams, &audio.buffer_size),
                    "alsa audio failed to get buffer size"))
            goto bail_out;
        if (alsaBad(snd_pcm_hw_params_set_period_time_near(audio.handle, hwparams,
                    &audio.period_time, NULL), "failed to set period time"))
            goto bail_out;
        if (alsaBad(snd_pcm_hw_params_get_period_size(hwparams, &audio.period_size,
                    NULL), "alsa audio failed to get period size"))
            goto bail_out;
    }
    else
    {
        if (alsaBad(snd_pcm_hw_params_set_period_time_near(audio.handle, hwparams,
                    &audio.period_time, NULL), "failed to set period time"))
            goto bail_out;
        audio.buffer_size = audio.period_size * 4;
        if (alsaBad(snd_pcm_hw_params_set_buffer_size_near(audio.handle, hwparams,
                    &audio.buffer_size), "failed to set buffer size"))
            goto bail_out;
    }
    if (alsaBad(snd_pcm_hw_params (audio.handle, hwparams),
                "alsa audio failed to set hardware parameters"))
		goto bail_out;
    if (alsaBad(snd_pcm_hw_params_get_buffer_size(hwparams, &audio.buffer_size),
                "alsa audio failed to get buffer size"))
        goto bail_out;
    if (alsaBad(snd_pcm_hw_params_get_period_size(hwparams, &audio.period_size,
                NULL), "failed to get period size"))
        goto bail_out;
    if (ask_buffersize != audio.period_size)
        Runtime.Log("Asked for buffersize " + asString(ask_buffersize)
                    + ", Alsa dictates " + asString((unsigned int)audio.period_size));
    audioLatency = getBuffersize();
    return true;

bail_out:
    if (audio.handle != NULL)
        Close();
    return false;
}


bool AlsaEngine::prepSwparams(void)
{
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
	snd_pcm_uframes_t boundary;
    if (alsaBad(snd_pcm_sw_params_current(audio.handle, swparams),
                 "alsa audio failed to get swparams"))
        goto bail_out;
    if (alsaBad(snd_pcm_sw_params_get_boundary(swparams, &boundary),
                "alsa audio failed to get boundary"))
        goto bail_out;
    if (alsaBad(snd_pcm_sw_params_set_start_threshold(audio.handle, swparams,
                                                      boundary + 1),
                "failed to set start threshold")) // explicit start, not auto start
        goto bail_out;
    if (alsaBad(snd_pcm_sw_params_set_stop_threshold(audio.handle, swparams,
                                                    boundary),
               "alsa audio failed to set stop threshold"))
        goto bail_out;
    if (alsaBad(snd_pcm_sw_params(audio.handle, swparams),
                "alsa audio failed to set software parameters"))
        goto bail_out;
    return true;

bail_out:
    return false;
}


void *AlsaEngine::_AudioThread(void *arg)
{
    return static_cast<AlsaEngine*>(arg)->AudioThread();
}


void *AlsaEngine::AudioThread(void)
{  
    if (NULL == audio.handle)
    {
        Runtime.Log("Null pcm handle into AlsaEngine::AudioThread");
        return NULL;
    }
    alsaBad(snd_pcm_start(audio.handle), "alsa audio pcm start failed");
    pthread_cleanup_push(_audioCleanup, this);
    while (!threadStop)
    {
        pthread_testcancel();
        audio.pcm_state = snd_pcm_state(audio.handle);
        if (audio.pcm_state != SND_PCM_STATE_RUNNING)
        {
            switch (audio.pcm_state)
            {
                case SND_PCM_STATE_XRUN:
                case SND_PCM_STATE_SUSPENDED:
                    if (!xrunRecover())
                        break;
                    // else fall through to ...
                case SND_PCM_STATE_SETUP:
                    if (alsaBad(snd_pcm_prepare(audio.handle),
                                "alsa audio pcm prepare failed"))
                        break;
                case SND_PCM_STATE_PREPARED:
                    alsaBad(snd_pcm_start(audio.handle), "pcm start failed");
                    break;
                default:
                    Runtime.Log("AlsaEngine::AudioThread, weird SND_PCM_STATE: "
                                + asString(audio.pcm_state));
                    break;
            }
            audio.pcm_state = snd_pcm_state(audio.handle);
        }
        if (audio.pcm_state == SND_PCM_STATE_RUNNING)
        {
            pthread_testcancel();
            getAudio();
            InterleaveShorts();
            Write();
        }
        else
            Runtime.Log("Audio pcm still not RUNNING");
    }
    pthread_cleanup_pop(1);
    return NULL;
}


void AlsaEngine::Write(void)
{
    snd_pcm_uframes_t towrite = getBuffersize();
    snd_pcm_sframes_t wrote = 0;
    short int *data = interleavedShorts;
    while (towrite > 0)
    {
        wrote = pcmWrite(audio.handle, data, towrite);
        if (wrote >= 0)
        {
            if ((snd_pcm_uframes_t)wrote < towrite || wrote == -EAGAIN)
                snd_pcm_wait(audio.handle, 666);
            if (wrote > 0)
            {
                towrite -= wrote;
                data += wrote * 2;
            }
        }
        else // (wrote < 0)
        {
            switch (wrote)
            {
                case -EBADFD:
                    alsaBad(-EBADFD, "alsa audio unfit for writing");
                    break;
                case -EPIPE:
                    xrunRecover();
                    break;
                case -ESTRPIPE:
                    Recover(wrote);
                    break;
                default:
                    alsaBad(wrote, "alsa audio, snd_pcm_writei ==> weird state");
                    break;
            }
            wrote = 0;
        }
    }
}


bool AlsaEngine::Recover(int err)
{
    if (err > 0)
        err = -err;
    bool isgood = false;
    switch (err)
    {
        case -EINTR:
            isgood = true; // nuthin to see here
            break;
        case -ESTRPIPE:
            if (!alsaBad(snd_pcm_prepare(audio.handle),
                         "Error, AlsaEngine failed to recover from suspend"))
                isgood = true;
            break;
        case -EPIPE:
            if (!alsaBad(snd_pcm_prepare(audio.handle),
                         "Error, AlsaEngine failed to recover from underrun"))
                isgood = true;
            break;
        default:
            break;
    }
    return isgood;
}


bool AlsaEngine::xrunRecover(void)
{
    bool isgood = false;
    if (audio.handle != NULL)
    {
        if (!alsaBad(snd_pcm_drop(audio.handle), "pcm drop failed"))
            if (!alsaBad(snd_pcm_prepare(audio.handle), "pcm prepare failed"))
                isgood = true;
        Runtime.Log("Alsa xrun recovery "
                    + ((isgood) ? string("good") : string("not good")));
    }
    return isgood;
}


bool AlsaEngine::Start(void)
{
    int chk;
    pthread_attr_t attr;
    threadStop = false;
    if (NULL != audio.handle)
    {
        chk = 999;
        if (setThreadAttributes(&attr, true))
        {
            if ((chk = pthread_create(&audio.pThread, &attr, _AudioThread, this)))
                Runtime.Log("Error, failed to start alsa audio thread (sched_fifo): "
                            + asString(chk));
        }
        if (chk)
        {
            if (!setThreadAttributes(&attr, false))
                goto bail_out;
            if ((chk = pthread_create(&audio.pThread, &attr, _AudioThread, this)))
            {
                Runtime.Log("Failed to start alsa audio thread (other): "
                            + asString(chk));
                goto bail_out;
            }
        }
    }

    if (NULL != midi.handle)
    {
        chk = 999;
        if (setThreadAttributes(&attr, true, true))
        {
            if ((chk = pthread_create(&midi.pThread, &attr, _MidiThread, this)))
                Runtime.Log("Failed to start Alsa midi thread (schedfifo): "
                            + asString(chk));
        }
        if (chk)
        {
            if (!setThreadAttributes(&attr, false))
                goto bail_out;
            if ((chk = pthread_create(&midi.pThread, &attr, _MidiThread, this)))
            {
                Runtime.Log("Failed to start Alsa midi thread (other): "
                            + asString(chk));
                goto bail_out;
            }
        }
    }
    return true;

bail_out:
    Runtime.Log("Error - bail out of AlsaEngine::Start()");
    Close();
    threadStop = true;
    return false;
}


void *AlsaEngine::_MidiThread(void *arg)
{
    return static_cast<AlsaEngine*>(arg)->MidiThread();
}


void *AlsaEngine::MidiThread(void)
{
    snd_seq_event_t *event;
    unsigned char channel;
    unsigned char note;
    unsigned char velocity;
    int ctrltype;
    int par;
    int chk;
    pthread_cleanup_push(_midiCleanup, this);
    while (!threadStop)
    {
        pthread_testcancel();
        while ((chk = snd_seq_event_input(midi.handle, &event)) > 0)
        {
            pthread_testcancel();
            if (!event)
                continue;
            par = event->data.control.param;
            switch (event->type)
            {
                case SND_SEQ_EVENT_NOTEON:
                    if (event->data.note.note)
                    {
                        channel = event->data.note.channel;
                        note = event->data.note.note;
                        velocity = event->data.note.velocity;
                        setMidiNote(channel, note, velocity);
                    }
                    break;

                case SND_SEQ_EVENT_NOTEOFF:
                    channel = event->data.note.channel;
                    note = event->data.note.note;
                    setMidiNote(channel, note);
                    break;

                case SND_SEQ_EVENT_PITCHBEND:
                    channel = event->data.control.channel;
                    ctrltype = C_pitchwheel;
                    par = event->data.control.value;
                    setMidiController(channel, ctrltype, par);
                    break;

                case SND_SEQ_EVENT_CONTROLLER:
                    channel = event->data.control.channel;
                    ctrltype = event->data.control.param;
                    par = event->data.control.value;
                    setMidiController(channel, ctrltype, par);
                    break;

                case SND_SEQ_EVENT_RESET: // reset to power-on state
                    channel = event->data.control.channel;
                    ctrltype = C_resetallcontrollers;
                    setMidiController(channel, ctrltype, 0);
                    break;

                case SND_SEQ_EVENT_PORT_SUBSCRIBED: // ports connected
                    Runtime.Log("Alsa midi port connected");
                    break;

                case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: // ports disconnected
                    Runtime.Log("Alsa midi port disconnected");
                    break;

                default:
                    Runtime.Log("Other non-handled midi event, type: "
                                + asString((int)event->type));
                    break;
            }
            snd_seq_free_event(event);
        }
        if (chk < 0)
            Runtime.Log("ALSA midi input read failed: " + asString(chk));
    }
    pthread_cleanup_pop(1);
    return NULL;
}


bool AlsaEngine::alsaBad(int op_result, string err_msg)
{
    bool isbad = (op_result < 0); // (op_result < 0) -> is bad -> return true
    if (isbad)
        Runtime.Log("Error, alsa audio: " +err_msg + ": "
                     + string(snd_strerror(op_result)));
    return isbad;
}
