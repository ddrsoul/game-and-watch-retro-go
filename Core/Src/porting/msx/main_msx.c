#include <odroid_system.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>


#include "main.h"
#include "appid.h"

#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "gw_lcd.h"

#include "MSX.h"
#include "Properties.h"
#include "ArchFile.h"
#include "VideoRender.h"
#include "AudioMixer.h"
#include "Casette.h"
#include "PrinterIO.h"
#include "UartIO.h"
#include "MidiIO.h"
#include "Machine.h"
#include "Board.h"
#include "Emulator.h"
#include "FileHistory.h"
#include "Actions.h"
#include "Language.h"
#include "LaunchFile.h"
#include "ArchEvent.h"
#include "ArchSound.h"
#include "ArchNotifications.h"
#include "JoystickPort.h"
#include "InputEvent.h"
#include "R800.h"
#include "save_msx.h"
#include "gw_malloc.h"

extern BoardInfo boardInfo;
static Properties* properties;
static Machine *msxMachine;
static Mixer* mixer;
// Default is MSX2+
static int selected_msx_index = 2;
// Default is 50Hz
static int selected_frequency_index = 1;

static odroid_gamepad_state_t previous_joystick_state;
int msx_button_a_key_index = 5; /* EC_SPACE index */
int msx_button_b_key_index = 7; /* EC_CTRL index */
int msx_button_game_key = EC_RETURN;
int msx_button_time_key = EC_CTRL;
int msx_button_start_key = EC_RETURN;
int msx_button_select_key = EC_CTRL;

static int selected_disk_index = 0;
#define MSX_DISK_EXTENSION "cdk"

static int selected_key_index = 0;

/* strings for options */
static char disk_name[PROP_MAXPATH];
static char msx_name[6];
static char key_name[10];
static char frequency_name[5];
static char a_button_name[10];
static char b_button_name[10];

/* Volume management */
static int8_t currentVolume = -1;
static const uint8_t volume_table[ODROID_AUDIO_VOLUME_MAX + 1] = {
    0,
    6,
    12,
    19,
    25,
    35,
    46,
    60,
    80,
    100,
};

/* Framebuffer management */
static unsigned image_buffer_base_width;
static unsigned image_buffer_current_width;
static unsigned image_buffer_height;
static int double_width;

#define FPS_NTSC  60
#define FPS_PAL   50
static int8_t msx_fps = FPS_PAL;

#define AUDIO_MSX_SAMPLE_RATE 16000

static void createMsxMachine(int msxType);
static void setPropertiesMsx(Machine *machine, int msxType);
static void setupEmulatorRessources(int msxType);
static void createProperties();

static bool msx_system_LoadState(char *pathName)
{
    loadMsxState((UInt8 *)ACTIVE_FILE->save_address);
    return true;
}

static bool msx_system_SaveState(char *pathName)
{
    saveMsxState((UInt8 *)ACTIVE_FILE->save_address,ACTIVE_FILE->save_size);
    return true;
}

void save_gnw_msx_data() {
    SaveState* state;
    state = saveStateOpenForWrite("main_msx");
    saveStateSet(state, "selected_msx_index", selected_msx_index);
    saveStateSet(state, "selected_disk_index", selected_disk_index);
    saveStateSet(state, "msx_button_a_key_index", msx_button_a_key_index);
    saveStateSet(state, "msx_button_b_key_index", msx_button_b_key_index);
    saveStateSet(state, "selected_frequency_index", selected_frequency_index);
    saveStateSet(state, "selected_key_index", selected_key_index);
    saveStateSet(state, "msx_fps", msx_fps);
    saveStateClose(state);
}

void load_gnw_msx_data() {
    SaveState* state;
    if (initLoadMsxState((UInt8 *)ACTIVE_FILE->save_address)) {
        state = saveStateOpenForRead("main_msx");
        selected_msx_index = saveStateGet(state, "selected_msx_index", 0);
        selected_disk_index = saveStateGet(state, "selected_disk_index", 0);
        msx_button_a_key_index = saveStateGet(state, "msx_button_a_key_index", 0);
        msx_button_b_key_index = saveStateGet(state, "msx_button_b_key_index", 0);
        selected_frequency_index = saveStateGet(state, "selected_frequency_index", 0);
        selected_key_index = saveStateGet(state, "selected_key_index", 0);
        msx_fps = saveStateGet(state, "msx_fps", 0);
        saveStateClose(state);
    }
}

/* Core stubs */
void frameBufferDataDestroy(FrameBufferData* frameData){}
void frameBufferSetActive(FrameBufferData* frameData){}
void frameBufferSetMixMode(FrameBufferMixMode mode, FrameBufferMixMode mask){}
void frameBufferClearDeinterlace(){}
void frameBufferSetInterlace(FrameBuffer* frameBuffer, int val){}
void archTrap(UInt8 value){}
void videoUpdateAll(Video* video, Properties* properties){}

/* framebuffer */

uint16_t* frameBufferGetLine(FrameBuffer* frameBuffer, int y)
{
   return (lcd_get_active_buffer() + sizeof(uint16_t) * (y * image_buffer_current_width + 24));
}

FrameBuffer* frameBufferGetDrawFrame(void)
{
   return (void*)lcd_get_active_buffer();
}

FrameBuffer* frameBufferFlipDrawFrame(void)
{
   return (void*)lcd_get_active_buffer();
}

static int fbScanLine = 0;

void frameBufferSetScanline(int scanline)
{
   fbScanLine = scanline;
}

int frameBufferGetScanline(void)
{
   return fbScanLine;
}

FrameBufferData* frameBufferDataCreate(int maxWidth, int maxHeight, int defaultHorizZoom)
{
   return (void*)lcd_get_active_buffer();
}

FrameBufferData* frameBufferGetActive()
{
    return (void*)lcd_get_active_buffer();
}

void   frameBufferSetLineCount(FrameBuffer* frameBuffer, int val)
{
    image_buffer_height = val;
}

int    frameBufferGetLineCount(FrameBuffer* frameBuffer) {
    return image_buffer_height;
}

int frameBufferGetMaxWidth(FrameBuffer* frameBuffer)
{
    return FB_MAX_LINE_WIDTH;
}

int frameBufferGetDoubleWidth(FrameBuffer* frameBuffer, int y)
{
    return double_width;
}

void frameBufferSetDoubleWidth(FrameBuffer* frameBuffer, int y, int val)
{
    double_width = val;
}

/** GuessROM() ***********************************************/
/** Guess MegaROM mapper of a ROM.                          **/
/*************************************************************/
int GuessROM(const uint8_t *buf,int size)
{
    int i;
    int counters[6] = { 0, 0, 0, 0, 0, 0 };

    int mapper;

    /* No result yet */
    mapper = ROM_UNKNOWN;

    if (size <= 0x10000) {
        if (size == 0x10000) {
            if (buf[0x4000] == 'A' && buf[0x4001] == 'B') mapper = ROM_PLAIN;
            else mapper = ROM_ASCII16;
            return mapper;
        } 
        
        if (size <= 0x4000 && buf[0] == 'A' && buf[1] == 'B') {
            UInt16 text = buf[8] + 256 * buf[9];
            if ((text & 0xc000) == 0x8000) {
                return ROM_BASIC;
            }
        }
        return ROM_PLAIN;
    }

    /* Count occurences of characteristic addresses */
    for (i = 0; i < size - 3; i++) {
        if (buf[i] == 0x32) {
            UInt32 value = buf[i + 1] + ((UInt32)buf[i + 2] << 8);

            switch(value) {
            case 0x4000: 
            case 0x8000: 
            case 0xa000: 
                counters[3]++;
                break;

            case 0x5000: 
            case 0x9000: 
            case 0xb000: 
                counters[2]++;
                break;

            case 0x6000: 
                counters[3]++;
                counters[4]++;
                counters[5]++;
                break;

            case 0x6800: 
            case 0x7800: 
                counters[4]++;
                break;

            case 0x7000: 
                counters[2]++;
                counters[4]++;
                counters[5]++;
                break;

            case 0x77ff: 
                counters[5]++;
                break;
            }
        }
    }

    /* Find which mapper type got more hits */
    mapper = 0;

    counters[4] -= counters[4] ? 1 : 0;

    for (i = 0; i <= 5; i++) {
        if (counters[i] > 0 && counters[i] >= counters[mapper]) {
            mapper = i;
        }
    }

    if (mapper == 5 && counters[0] == counters[5]) {
        mapper = 0;
    }

    switch (mapper) {
        default:
        case 0: mapper = ROM_STANDARD; break;
        case 1: mapper = ROM_MSXDOS2; break;
        case 2: mapper = ROM_KONAMI5; break;
        case 3: mapper = ROM_KONAMI4; break;
        case 4: mapper = ROM_ASCII8; break;
        case 5: mapper = ROM_ASCII16; break;
    }

    /* Return the most likely mapper type */
    return(mapper);
}

static bool update_disk_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    char game_name[PROP_MAXPATH];
    int disk_count = 0;
    int max_index = 0;
    retro_emulator_file_t *disk_file = NULL;
    const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
    disk_count = rom_get_ext_count(msx_system,MSX_DISK_EXTENSION);
    if (disk_count > 0) {
        max_index = disk_count - 1;
    } else {
        max_index = 0;
    }

    if (event == ODROID_DIALOG_PREV) {
        selected_disk_index = selected_disk_index > 0 ? selected_disk_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_disk_index = selected_disk_index < max_index ? selected_disk_index + 1 : 0;
    }

    disk_file = (retro_emulator_file_t *)rom_get_ext_file_at_index(msx_system,MSX_DISK_EXTENSION,selected_disk_index);
    if (disk_count > 0) {
        sprintf(game_name,"%s.%s",disk_file->name,disk_file->ext);
        emulatorSuspend();
        insertDiskette(properties, 0, game_name, NULL, -1);
        emulatorResume();
    }
    strcpy(option->value, disk_file->name);
    return event == ODROID_DIALOG_ENTER;
}

static bool update_frequency_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = 1;

    if (event == ODROID_DIALOG_PREV) {
        selected_frequency_index = selected_frequency_index > 0 ? selected_frequency_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_frequency_index = selected_frequency_index < max_index ? selected_frequency_index + 1 : 0;
    }

    switch (selected_frequency_index) {
        case 0: // Force 60Hz;
            strcpy(option->value, "60Hz");
            break;
        case 1: // Force 50Hz;
            strcpy(option->value, "50Hz");
            break;
    }

    if (event == ODROID_DIALOG_ENTER) {
        int frequency;
        switch (selected_frequency_index) {
            case 0: // Force 60Hz;
                msx_fps = 60;
                common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_NTSC + 0.5f);
                memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
                HAL_SAI_DMAStop(&hsai_BlockA1);
                HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, (2 * AUDIO_MSX_SAMPLE_RATE / msx_fps));
                vdpSetSyncMode(VDP_SYNC_60HZ);
                emulatorSetFrequency(msx_fps , &frequency);
                boardSetFrequency(frequency);
                break;
            case 1: // Force 50Hz;
                msx_fps = 50;
                common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_PAL + 0.5f);
                memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
                HAL_SAI_DMAStop(&hsai_BlockA1);
                HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, (2 * AUDIO_MSX_SAMPLE_RATE / msx_fps));
                vdpSetSyncMode(VDP_SYNC_50HZ);
                emulatorSetFrequency(msx_fps , &frequency);
                boardSetFrequency(frequency);
                break;
        }
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool update_msx_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = 2;

    if (event == ODROID_DIALOG_PREV) {
        selected_msx_index = selected_msx_index > 0 ? selected_msx_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_msx_index = selected_msx_index < max_index ? selected_msx_index + 1 : 0;
    }

    switch (selected_msx_index) {
        case 0: // MSX1;
            strcpy(option->value, "MSX1");
            break;
        case 1: // MSX2;
            strcpy(option->value, "MSX2");
            break;
        case 2: // MSX2+;
            strcpy(option->value, "MSX2+");
            break;
    }

    if (event == ODROID_DIALOG_ENTER) {
        int frequency;
        boardInfo.destroy();
        boardDestroy();
        ahb_init();
        itc_init();
        printf("Changing system %d\n",selected_msx_index);
        setupEmulatorRessources(selected_msx_index);
        printf("emulatorSetFrequency\n");
        emulatorSetFrequency(msx_fps, &frequency);
        boardSetFrequency(frequency);
    }
    return event == ODROID_DIALOG_ENTER;
}

struct msx_key_info {
    int  key_id;
    const char *name;
    bool auto_release;
};

struct msx_key_info msx_keyboard[] = {
    {EC_F1,"F1",true},
    {EC_F2,"F2",true},
    {EC_F3,"F3",true},
    {EC_F4,"F4",true},
    {EC_F5,"F5",true},
    {EC_SPACE,"Space",true},
    {EC_LSHIFT,"Shift",false},
    {EC_CTRL,"Control",false},
    {EC_GRAPH,"Graph",true},
    {EC_BKSPACE,"BS",true},
    {EC_TAB,"Tab",true},
    {EC_CAPS,"CapsLock",true},
    {EC_CODE,"Code",true},
    {EC_SELECT,"Select",true},
    {EC_RETURN,"Return",true},
    {EC_DEL,"Delete",true},
    {EC_INS,"Insert",true},
    {EC_STOP,"Stop",true},
    {EC_ESC,"Esc",true},
    {EC_1,"1/!",true},
    {EC_2,"2/@",true},
    {EC_3,"3/#",true},
    {EC_4,"4/$",true},
    {EC_5,"5/\%",true},
    {EC_6,"6/^",true},
    {EC_7,"7/&",true},
    {EC_8,"8/*",true},
    {EC_9,"9/(",true},
    {EC_0,"0/)",true},
    {EC_NUM0,"0",true},
    {EC_NUM1,"1",true},
    {EC_NUM2,"2",true},
    {EC_NUM3,"3",true},
    {EC_NUM4,"4",true},
    {EC_NUM5,"5",true},
    {EC_NUM6,"6",true},
    {EC_NUM7,"7",true},
    {EC_NUM8,"8",true},
    {EC_NUM9,"9",true},
    {EC_A,"a",true},
    {EC_B,"b",true},
    {EC_C,"c",true},
    {EC_D,"d",true},
    {EC_E,"e",true},
    {EC_F,"f",true},
    {EC_G,"g",true},
    {EC_H,"h",true},
    {EC_I,"i",true},
    {EC_J,"j",true},
    {EC_K,"k",true},
    {EC_L,"l",true},
    {EC_M,"m",true},
    {EC_N,"n",true},
    {EC_O,"o",true},
    {EC_P,"p",true},
    {EC_Q,"q",true},
    {EC_R,"r",true},
    {EC_S,"s",true},
    {EC_T,"t",true},
    {EC_U,"u",true},
    {EC_V,"v",true},
    {EC_W,"w",true},
    {EC_X,"x",true},
    {EC_Y,"y",true},
    {EC_Z,"z",true},
    {EC_COLON,":",true},
};

#define RELEASE_KEY_DELAY 5
static struct msx_key_info *pressed_key = NULL;
static struct msx_key_info *release_key = NULL;
static int release_key_delay = RELEASE_KEY_DELAY;
static bool update_keyboard_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;

    if (event == ODROID_DIALOG_PREV) {
        selected_key_index = selected_key_index > 0 ? selected_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_key_index = selected_key_index < max_index ? selected_key_index + 1 : 0;
    }

    if (eventMap[msx_keyboard[selected_key_index].key_id]) {
        // If key is pressed, add a * in front of key name
        option->value[0] = '*';
        strcpy(option->value+1, msx_keyboard[selected_key_index].name);
    } else {
        strcpy(option->value, msx_keyboard[selected_key_index].name);
    }

    if (event == ODROID_DIALOG_ENTER) {
        pressed_key = &msx_keyboard[selected_key_index];
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool update_a_button_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;

    if (event == ODROID_DIALOG_PREV) {
        msx_button_a_key_index = msx_button_a_key_index > 0 ? msx_button_a_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        msx_button_a_key_index = msx_button_a_key_index < max_index ? msx_button_a_key_index + 1 : 0;
    }

    strcpy(option->value, msx_keyboard[msx_button_a_key_index].name);
    return event == ODROID_DIALOG_ENTER;
}

static bool update_b_button_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;

    if (event == ODROID_DIALOG_PREV) {
        msx_button_b_key_index = msx_button_b_key_index > 0 ? msx_button_b_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        msx_button_b_key_index = msx_button_b_key_index < max_index ? msx_button_b_key_index + 1 : 0;
    }

    strcpy(option->value, msx_keyboard[msx_button_b_key_index].name);
    return event == ODROID_DIALOG_ENTER;
}

static void msxInputUpdate(odroid_gamepad_state_t *joystick)
{
    if ((joystick->values[ODROID_INPUT_LEFT]) && !previous_joystick_state.values[ODROID_INPUT_LEFT]) {
        eventMap[EC_LEFT]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_LEFT]) && previous_joystick_state.values[ODROID_INPUT_LEFT]) {
        eventMap[EC_LEFT]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_RIGHT]) && !previous_joystick_state.values[ODROID_INPUT_RIGHT]) {
        eventMap[EC_RIGHT]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_RIGHT]) && previous_joystick_state.values[ODROID_INPUT_RIGHT]) {
        eventMap[EC_RIGHT]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_UP]) && !previous_joystick_state.values[ODROID_INPUT_UP]) {
        eventMap[EC_UP]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_UP]) && previous_joystick_state.values[ODROID_INPUT_UP]) {
        eventMap[EC_UP]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_DOWN]) && !previous_joystick_state.values[ODROID_INPUT_DOWN]) {
        eventMap[EC_DOWN]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_DOWN]) && previous_joystick_state.values[ODROID_INPUT_DOWN]) {
        eventMap[EC_DOWN]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_A]) && !previous_joystick_state.values[ODROID_INPUT_A]) {
        eventMap[msx_keyboard[msx_button_a_key_index].key_id]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_A]) && previous_joystick_state.values[ODROID_INPUT_A]) {
        eventMap[msx_keyboard[msx_button_a_key_index].key_id]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_B]) && !previous_joystick_state.values[ODROID_INPUT_B]) {
        eventMap[msx_keyboard[msx_button_b_key_index].key_id]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_B]) && previous_joystick_state.values[ODROID_INPUT_B]) {
        eventMap[msx_keyboard[msx_button_b_key_index].key_id]  = 0;
    }
    // Game button on G&W
    if ((joystick->values[ODROID_INPUT_START]) && !previous_joystick_state.values[ODROID_INPUT_START]) {
        eventMap[msx_button_game_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_START]) && previous_joystick_state.values[ODROID_INPUT_START]) {
        eventMap[msx_button_game_key]  = 0;
    }
    // Time button on G&W
    if ((joystick->values[ODROID_INPUT_SELECT]) && !previous_joystick_state.values[ODROID_INPUT_SELECT]) {
        eventMap[msx_button_time_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_SELECT]) && previous_joystick_state.values[ODROID_INPUT_SELECT]) {
        eventMap[msx_button_time_key]  = 0;
    }
    // Start button on Zelda G&W
    if ((joystick->values[ODROID_INPUT_X]) && !previous_joystick_state.values[ODROID_INPUT_X]) {
        eventMap[msx_button_start_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_X]) && previous_joystick_state.values[ODROID_INPUT_X]) {
        eventMap[msx_button_start_key]  = 0;
    }
    // Select button on Zelda G&W
    if ((joystick->values[ODROID_INPUT_Y]) && !previous_joystick_state.values[ODROID_INPUT_Y]) {
        eventMap[msx_button_select_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_Y]) && previous_joystick_state.values[ODROID_INPUT_Y]) {
        eventMap[msx_button_select_key]  = 0;
    }

    // Handle keyboard emulation
    if (pressed_key != NULL) {
        eventMap[pressed_key->key_id] = (eventMap[pressed_key->key_id] + 1) % 2;
        if (pressed_key->auto_release) {
            release_key = pressed_key;
        }
        pressed_key = NULL;
    } else if (release_key != NULL) {
        if (release_key_delay == 0) {
            eventMap[release_key->key_id] = 0;
            release_key = NULL;
            release_key_delay = RELEASE_KEY_DELAY;
        } else {
            release_key_delay--;
        }
    }

    memcpy(&previous_joystick_state,joystick,sizeof(odroid_gamepad_state_t));
}

static void createOptionMenu(odroid_dialog_choice_t *options) {
    int index=0;
    if (strcmp(ROM_EXT,MSX_DISK_EXTENSION) == 0) {
        options[index].id = 100;
        options[index].label = "Change Dsk";
        options[index].value = disk_name;
        options[index].enabled = 1;
        options[index].update_cb = &update_disk_cb;
        index++;
    }
    options[index].id = 100;
    options[index].label = "Select MSX";
    options[index].value = msx_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_msx_cb;
    index++;
    options[index].id = 100;
    options[index].label = "Frequency";
    options[index].value = frequency_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_frequency_cb;
    index++;
    options[index].id = 100;
    options[index].label = "A Button";
    options[index].value = a_button_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_a_button_cb;
    index++;
    options[index].id = 100;
    options[index].label = "B Button";
    options[index].value = b_button_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_b_button_cb;
    index++;
    options[index].id = 100;
    options[index].label = "Press Key";
    options[index].value = key_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_keyboard_cb;
    index++;
    options[index].id = 0x0F0F0F0F;
    options[index].label = "LAST";
    options[index].value = "LAST";
    options[index].enabled = 0xFFFF;
    options[index].update_cb = NULL;
}

static void setPropertiesMsx(Machine *machine, int msxType) {
    int i = 0;

    switch(msxType) {
        case 0: // MSX1
            machine->board.type = BOARD_MSX;
            machine->video.vdpVersion = VDP_TMS9929A;
            machine->video.vramSize = 16 * 1024;
            machine->cmos.enable = 0;

            machine->slot[0].subslotted = 0;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 0;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 8; // 64kB of RAM
            machine->slotInfo[i].romType = RAM_NORMAL;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_TC8566AF;
            strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
            i++;

            machine->slotInfoCount = i;
            break;

        case 1: // MSX2
            machine->board.type = BOARD_MSX_S3527;
            machine->video.vdpVersion = VDP_V9938;
            machine->video.vramSize = 128 * 1024;
            machine->cmos.enable = 1;

            machine->slot[0].subslotted = 0;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 1;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 2;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 16; // 128kB of RAM
            machine->slotInfo[i].romType = RAM_MAPPER;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX2.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_NORMAL;
            strcpy(machine->slotInfo[i].name, "MSX2EXT.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_TC8566AF;
            strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_MSXMUSIC; // FMPAC
            strcpy(machine->slotInfo[i].name, "MSX2PMUS.rom");
            i++;

            machine->slotInfoCount = i;
            break;

        case 2: // MSX2+
            machine->board.type = BOARD_MSX_T9769B;
            machine->video.vdpVersion = VDP_V9958;
            machine->video.vramSize = 128 * 1024;
            machine->cmos.enable = 1;

            machine->slot[0].subslotted = 1;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 1;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 16; // 128kB of RAM
            machine->slotInfo[i].romType = RAM_MAPPER;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 0;
            machine->slotInfo[i].romType = ROM_F4DEVICE; //ROM_F4INVERTED;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX2P.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_NORMAL;
            strcpy(machine->slotInfo[i].name, "MSX2PEXT.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 2;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_TC8566AF;
            strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 2;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_MSXMUSIC; // FMPAC
            strcpy(machine->slotInfo[i].name, "MSX2PMUS.rom");
            i++;

            machine->slotInfoCount = i;
            break;
    }
}

static void createMsxMachine(int msxType) {
    msxMachine = ahb_calloc(1,sizeof(Machine));

    msxMachine->cpu.freqZ80 = 3579545;
    msxMachine->cpu.freqR800 = 7159090;
    msxMachine->fdc.count = 1;
    msxMachine->cmos.batteryBacked = 1;
    msxMachine->audio.psgstereo = 0;
    msxMachine->audio.psgpan[0] = 0;
    msxMachine->audio.psgpan[1] = -1;
    msxMachine->audio.psgpan[2] = 1;

    msxMachine->cpu.hasR800 = 0;
    msxMachine->fdc.enabled = 1;

    setPropertiesMsx(msxMachine,msxType);
}

static void insertGame() {
    char game_name[PROP_MAXPATH];
    sprintf(game_name,"%s.%s",ACTIVE_FILE->name,ACTIVE_FILE->ext);
    if (0 == strcmp(ACTIVE_FILE->ext,MSX_DISK_EXTENSION)) {
        if (selected_disk_index == -1) {
            const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
            selected_disk_index = rom_get_index_for_file_ext(msx_system,ACTIVE_FILE);

            insertDiskette(properties, 0, game_name, NULL, -1);
        } else {
            retro_emulator_file_t *disk_file = NULL;
            const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
            disk_file = (retro_emulator_file_t *)rom_get_ext_file_at_index(msx_system,MSX_DISK_EXTENSION,selected_disk_index);
            sprintf(game_name,"%s.%s",disk_file->name,disk_file->ext);
            insertDiskette(properties, 0, game_name, NULL, -1);
        }
        // We load SCC-I cartridge for disk games requiring it
        insertCartridge(properties, 0, CARTNAME_SNATCHER, NULL, ROM_SNATCHER, -1);
        // If game name contains konami, we setup a Konami key mapping
        if (strcasestr(ACTIVE_FILE->name,"konami")) {
            msx_button_a_key_index = 5; /* EC_SPACE index */
            msx_button_b_key_index = 52; /* n key index */
            msx_button_game_key = EC_F4;
            msx_button_time_key = EC_F3;
            msx_button_start_key = EC_F1;
            msx_button_select_key = EC_F2;
        }
    } else {
        printf("Rom Mapper %d\n",ACTIVE_FILE->mapper);
        uint16_t mapper = ACTIVE_FILE->mapper;
        if (mapper == ROM_UNKNOWN) {
            mapper = GuessROM(ACTIVE_FILE->address,ACTIVE_FILE->size);
        }
        // If game is using konami mapper, we setup a Konami key mapping
        switch (mapper)
        {
            case ROM_KONAMI5:
            case ROM_KONAMI4:
            case ROM_KONAMI4NF:
                msx_button_a_key_index = 5; /* EC_SPACE index */
                msx_button_b_key_index = 52; /* n key index */
                msx_button_game_key = EC_F4;
                msx_button_time_key = EC_F3;
                msx_button_start_key = EC_F1;
                msx_button_select_key = EC_F2;
                break;
        }
        printf("insertCartridge msx mapper %d\n",mapper);
        insertCartridge(properties, 0, game_name, NULL, mapper, -1);
    }
}

static void createProperties() {
    properties = propCreate(1, EMU_LANG_ENGLISH, P_KBD_EUROPEAN, P_EMU_SYNCNONE, "");
    properties->sound.stereo = 0;
    if (msx_fps == FPS_NTSC) {
        properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
    } else {
        properties->emulation.vdpSyncMode = P_VDP_SYNC50HZ;
    }
    properties->emulation.enableFdcTiming = 0;
    properties->emulation.noSpriteLimits = 0;
    properties->sound.masterVolume = 0;

    currentVolume = -1;
    // Default : enable SCC and disable MSX-MUSIC
    // This will be changed dynamically if the game use MSX-MUSIC
    properties->sound.mixerChannel[MIXER_CHANNEL_SCC].enable = 1;
    properties->sound.mixerChannel[MIXER_CHANNEL_MSXMUSIC].enable = 1;
    properties->sound.mixerChannel[MIXER_CHANNEL_PSG].pan = 0;
    properties->sound.mixerChannel[MIXER_CHANNEL_MSXMUSIC].pan = 0;
    properties->sound.mixerChannel[MIXER_CHANNEL_SCC].pan = 0;
}

static void setupEmulatorRessources(int msxType)
{
    int i;
    mixer = mixerCreate();
    createProperties();
    createMsxMachine(msxType);
    emulatorInit(properties, mixer);
    insertGame();
    emulatorRestartSound();

    for (i = 0; i < MIXER_CHANNEL_TYPE_COUNT; i++)
    {
        mixerSetChannelTypeVolume(mixer, i, properties->sound.mixerChannel[i].volume);
        mixerSetChannelTypePan(mixer, i, properties->sound.mixerChannel[i].pan);
        mixerEnableChannelType(mixer, i, properties->sound.mixerChannel[i].enable);
    }

    mixerSetMasterVolume(mixer, properties->sound.masterVolume);
    mixerEnableMaster(mixer, properties->sound.masterEnable);

    boardSetFdcTimingEnable(properties->emulation.enableFdcTiming);
    boardSetY8950Enable(0/*properties->sound.chip.enableY8950*/);
    boardSetYm2413Enable(1/*properties->sound.chip.enableYM2413*/);
    boardSetMoonsoundEnable(0/*properties->sound.chip.enableMoonsound*/);
    boardSetVideoAutodetect(1/*properties->video.detectActiveMonitor*/);

    emulatorStartMachine(NULL, msxMachine);
    // Enable SCC and disable MSX-MUSIC as G&W is not powerfull enough to handle both at same time
    // If a game wants to play MSX-MUSIC sound, the mapper will detect it and it will disable SCC
    // and enbale MSX-MUSIC
    mixerEnableChannelType(boardGetMixer(), MIXER_CHANNEL_SCC, 1);
    mixerEnableChannelType(boardGetMixer(), MIXER_CHANNEL_MSXMUSIC, 0);
}

void app_main_msx(uint8_t load_state, uint8_t start_paused)
{
    int frequency;
    odroid_dialog_choice_t options[10];
    bool drawFrame;
    size_t offset = 0;
    uint8_t volume = 0;
    dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;

    selected_disk_index = -1;

    if (load_state) {
        load_gnw_msx_data();
    }
    createOptionMenu(options);

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
    } else {
        common_emu_state.pause_after_frames = 0;
    }
    common_emu_state.frame_time_10us = (uint16_t)(100000 / msx_fps + 0.5f);

    odroid_system_init(APPID_MSX, AUDIO_MSX_SAMPLE_RATE);
    odroid_system_emu_init(&msx_system_LoadState, &msx_system_SaveState, NULL);

    image_buffer_base_width    =  320;
    image_buffer_current_width =  image_buffer_base_width;
    image_buffer_height        =  240;

    memset(lcd_get_active_buffer(), 0, sizeof(framebuffer1));
    memset(lcd_get_inactive_buffer(), 0, sizeof(framebuffer1));

    setupEmulatorRessources(selected_msx_index);

    if (load_state) {
        loadMsxState((UInt8 *)ACTIVE_FILE->save_address);
    }
    emulatorSetFrequency(msx_fps , &frequency);
    boardSetFrequency(frequency);
    while (1) {
        wdog_refresh();
        drawFrame = common_emu_frame_loop();
        odroid_gamepad_state_t joystick;
        odroid_input_read_gamepad(&joystick);
        common_emu_input_loop(&joystick, options);
        msxInputUpdate(&joystick);
        ((R800*)boardInfo.cpuRef)->terminate = 0;
        boardInfo.run(boardInfo.cpuRef);

        if (drawFrame) {
            common_ingame_overlay();
            lcd_swap();
        }

        offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : (AUDIO_MSX_SAMPLE_RATE/msx_fps);
        mixerSyncAudioBuffer(mixer, &audiobuffer_dma[offset], (AUDIO_MSX_SAMPLE_RATE/msx_fps));
        volume = odroid_audio_volume_get();
        if (volume != currentVolume) {
            if (volume == 0) {
                mixerSetEnable(mixer,0);
            } else {
                mixerSetEnable(mixer,1);
                mixerSetMasterVolume(mixer,volume_table[volume]);
            }
            currentVolume = volume;
        }

        if(!common_emu_state.skip_frames) {
            for(uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
                while (dma_state == last_dma_state) {
                    cpumon_sleep();
                }
                last_dma_state = dma_state;
            }
        }
    }
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels) {
    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, 2 * AUDIO_MSX_SAMPLE_RATE / msx_fps);

    mixerSetStereo(mixer, 0);
//    mixerSetSampleRate(mixer,AUDIO_MSX_SAMPLE_RATE);
}

void archSoundDestroy(void) {}
