
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <memory>

#include "libretro/libretro.h"

constexpr int CYCLES_PER_FRAME = 280896;
// TODO maybe move this elsewhere?
const int FRAME_WIDTH = 240;
const int FRAME_HEIGHT = 160;
unsigned long cycles_per_frame = CYCLES_PER_FRAME;

// Generously taken from VBA-M
static const struct retro_controller_description port_gba[] = {
    { "GBA Joypad", RETRO_DEVICE_JOYPAD },
    { NULL, 0 },
};

static const struct retro_controller_info ports_gba[] = {
    { port_gba, 1 },
    { NULL, 0 },
};

static struct retro_input_descriptor input_gba[] = {
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,  "B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,  "A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,  "L" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,  "R" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,  "Turbo B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,  "Turbo A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "Solar Sensor (Darker)" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "Solar Sensor (Lighter)" },
    { 0, 0, 0, 0, NULL },
};


// Callbacks
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

unsigned retro_api_version(void) { return RETRO_API_VERSION; }


// Cheats
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

// Load a cartridge
bool retro_load_game(const struct retro_game_info *info)
{
    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports_gba);
    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_gba);
    return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }

// Unload the cartridge
void retro_unload_game(void) { }

unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }

// libretro unused api functions
void retro_set_controller_port_device(unsigned port, unsigned device) {}


void *retro_get_memory_data(unsigned id) { return nullptr; }
size_t retro_get_memory_size(unsigned id){ return 0; }

// Serialisation methods
size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void *data, size_t size) { return false; }
bool retro_unserialize(const void *data, size_t size) { return false; }

// End of retrolib
void retro_deinit(void) { }

// libretro global setters
void retro_set_environment(retro_environment_t cb) {
  environ_cb = cb;

  // Allow to run without a rom... should probably change in the future
  bool no_rom = true;
  cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {}
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_init(void)
{
    /* set up some logging */
    struct retro_log_callback log;
    unsigned level = 0;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;
    else
        log_cb = nullptr;

    // the performance level is guide to frontend to give an idea of how intensive this core is to run
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);

    log_cb(RETRO_LOG_DEBUG, "[gba_senpai]: Initializing core....\n");
}


/*
 * Tell libretro about this core, it's name, version and which rom files it supports.
 */
void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(retro_system_info));
    info->library_name = "GBA Senpai";
    info->library_version = "0.1";
    info->need_fullpath = false;
    info->valid_extensions = "bin|gba";
}

/*
 * Tell libretro about the AV system; the fps, sound sample rate and the
 * resolution of the display.
 */
void retro_get_system_av_info(struct retro_system_av_info *info) {

    int pixel_format = RETRO_PIXEL_FORMAT_RGB565;

    memset(info, 0, sizeof(retro_system_av_info));
    info->timing.fps            = 59.73; // TODO need to verify
    info->timing.sample_rate    = 32768; // confirmed with manual
    info->geometry.base_width   = FRAME_WIDTH;
    info->geometry.base_height  = FRAME_HEIGHT;
    info->geometry.max_width    = FRAME_WIDTH;
    info->geometry.max_height   = FRAME_HEIGHT;
    info->geometry.aspect_ratio = (3.0f / 2.0f);

    // the performance level is guide to frontend to give an idea of how intensive this core is to run
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

// Reset the Vectrex
void retro_reset(void)
{
    log_cb(RETRO_LOG_DEBUG, "[gba_senpai]: Resetting core....\n");
}



// Run a single frames with out Vectrex emulation.
void retro_run(void)
{
    // 548 audio samples per fram (32.768kHz @ 59.73 fps)
    for(int i = 0; i < 548; i++){
        audio_cb(1, 1);
    }
    // API requires that input_poll callback be called once
    input_poll_cb();
}
