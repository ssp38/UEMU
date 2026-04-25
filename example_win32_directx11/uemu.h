/* DinoLabs UKC1 Emulator 1.0.0 */

#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

 /* 0 Index Controller -> Display (0xE bytes)
    Byte 1 packet lengt
    Byte 2 unknown but must be 0x1 to update
    Byte 3 error code
    Byte 4 braking byte, or some status. 0xE4 braking 0xC4 not braking
    Bytes 5 & 6 watts -> watts /= 1.023999; watts++; uint8_t byteA = 0xC0 + ( watts >> 8 ); uint8_t byteB = watts & 0xFF;
    Bytes 8 & 9 speed -> uint16_t ticks = static_cast<uint16_t>( speed_constant_scalar / mph ); return { ticks >> 8, ticks & 0xFF }; inline float speed_constant_scalar = 4648.0f;
    Byte 13 checksum
 */

  /* 0 Index Display -> Controller (0x14 / 20 bytes)
    Byte 1 packet length
    Byte 4 Mode (0x0-0xF)
    Byte 5 amps? throttle input? not sure
    Byte 6 Motor Poles
    Byte 10 Start Mode
    Byte 12 Speed Limit KM/H
    Byte 13 Current Limit (amps)
    Byte 17 CORRESPONDS to throrlte PWM% but not a clean value
    Byte 18 Assist Poles
    Byte 19 checksum
  */

struct DinoLabs__ControllerRecv
{
    uint8_t mode; // [0x0-0x15]
    uint8_t amps; // offset by 0x88
    uint8_t motor_poles;
    uint8_t start_mode;
    uint8_t speed_limit_kmph;
    uint8_t current_limit_amps;
    uint8_t throttle_pwm;
    uint8_t assist_poles;

    DinoLabs__ControllerRecv( std::vector<uint8_t> bytes );
};

// Assumes bytes is a valid rx frame.
inline DinoLabs__ControllerRecv::DinoLabs__ControllerRecv( std::vector<uint8_t> bytes )
{
    if ( bytes.size( ) == 0x14 )
    {
        mode = bytes[4];
        amps = abs( bytes[5]); // could also be throttle input?
        motor_poles = bytes[6];
        start_mode = bytes[10];
        speed_limit_kmph = bytes[12];
        current_limit_amps = abs( bytes[13] );
        throttle_pwm = bytes[17];
        assist_poles = bytes[18];
    }
}

enum DinoLabs__ControllerErrorType
{
    DinoLabs__ControllerErrorType__NO_ERROR = 0x00,
    DinoLabs__ControllerErrorType__MOTOR_PHASE_ERROR = 0x01,
    DinoLabs__ControllerErrorType__BRAKE_CUTOFF_ERROR = 0x02,
    DinoLabs__ControllerErrorType__BMS_SHORT_PROTECTION_ERROR = 0x08,
    DinoLabs__ControllerErrorType__CONTROLLER_SHORT_PROTECTION_ERROR = 0x10,
    DinoLabs__ControllerErrorType__THROTTLE_COMMUNICATION_ERROR = 0x20,
    DinoLabs__ControllerErrorType__HALL_SENSOR_ERROR = 0x40,

    DinoLabs__ControllerErrorType__GENERAL_COMMUNICATION_ERROR = 0x30
};

namespace DinoLabs
{
    inline HANDLE hSerial;
    inline const char *port = "\\\\.\\COM3";
    inline bool initalized = false;


    inline DinoLabs__ControllerErrorType error_code;
    inline bool is_braking = false;
    inline bool send_stable_frame = false;
    inline bool send_target_speed = false;
    inline bool send_target_watts = false;
    inline float target_speed = 75.5;
    inline int target_watts = 1337;

    inline float speed_constant_scalar = 4648.0f;
    inline std::pair<uint8_t, uint8_t> speed_to_bytes( float mph )
    {
        uint16_t ticks = static_cast<uint16_t>( speed_constant_scalar / mph );
        return { ticks >> 8, ticks & 0xFF };
    }

    inline std::pair<uint8_t, uint8_t> watts_to_bytes( int watts )
    {
        if (watts != 0)
        {
            watts /= 1.023999; watts++;
        }
        uint8_t byteA = 0xC0 + ( watts >> 8 );
        uint8_t byteB = watts & 0xFF;
        return { byteA, byteB };
        // watts = ((byteA - 0xC0) << 8) | byteB ?
    }

    inline int bytes[10] = {0x01, 0x0, 0x0, 0x00, 0xb, 0xb8, 0x0, 0x0, 0x0 };
    inline std::string rx_frame = "";
    inline std::thread rx_thread;

    void threaded_rx_reader( );
    std::vector<uint8_t> fetch( );
    std::vector<uint8_t> build_frame( );
    uint8_t calculate_checksum( std::vector<uint8_t> partial_frame );
    void copy_frame( std::vector<uint8_t> frame );
    bool frame_is_valid( std::vector<uint8_t> &frame );
    bool send( const std::vector<uint8_t> &data );
    void present( );
    bool init( );
    bool shutdown( );
    void print_bytes( const std::vector<uint8_t> &data );
}
