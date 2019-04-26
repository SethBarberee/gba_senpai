#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <memory>

#include "libretro.h"

/*
 * Tell libretro about this core, it's name, version and which rom file types it supports.
 */
void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "GBA Senpai";
    info->library_version = "0.1.0";
    info->need_fullpath = false;
    info->valid_extensions = "gba"; // file types supported
}

/*
 * Tell libretro about the AV system; the fps, sound sample rate and the
 * resolution of the display.
 */
void retro_get_system_av_info(struct retro_system_av_info *info) {
    memset(info, 0, sizeof(*info));
    // TODO find real settings
    info->timing.fps            = 50.0f;
    info->timing.sample_rate    = 44100;    // 44.1kHz
    info->geometry.base_width   = 330;
    info->geometry.base_height  = 410;
    info->geometry.max_width    = 330;
    info->geometry.max_height   = 410;
    info->geometry.aspect_ratio = 330.0f / 410.0f;
}

void retro_run(void){
}

void retro_init(void){
}
