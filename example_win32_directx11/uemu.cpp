#include "uemu.h"
#include "imgui.h"

bool DinoLabs::init( )
{
    std::cout << "DinoLabs UEMU RX monitor\n";

    hSerial = CreateFileA(
        port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if ( hSerial == INVALID_HANDLE_VALUE ) {
        std::cout << "failed to open COM port\n";
        return false;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof( dcb );

    GetCommState( hSerial, &dcb );

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    SetCommState( hSerial, &dcb );

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    SetCommTimeouts( hSerial, &timeouts );
    DinoLabs::rx_thread = std::thread( DinoLabs::threaded_rx_reader );

    return true;
}

void DinoLabs::threaded_rx_reader( )
{
    while ( true )
    {
        

        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
    }
}

bool DinoLabs::shutdown( )
{
    if ( hSerial != INVALID_HANDLE_VALUE ) {
        CloseHandle( hSerial );
        hSerial = INVALID_HANDLE_VALUE;
    }
    return true;
}

std::vector<uint8_t> DinoLabs::fetch( )
{
    uint8_t buffer[256];
    DWORD bytesRead = 0;

    BOOL ok = ReadFile(
        DinoLabs::hSerial,
        buffer,
        sizeof( buffer ),
        &bytesRead,
        NULL
    );

    if ( !ok || bytesRead == 0 )
        return {};

    return std::vector<uint8_t>( buffer, buffer + bytesRead );
}

bool DinoLabs::send( const std::vector<uint8_t> &data )
{
    if ( DinoLabs::hSerial == INVALID_HANDLE_VALUE )
        return false;

    DWORD written = 0;

    BOOL ok = WriteFile(
        hSerial,
        data.data( ),
        (DWORD)data.size( ),
        &written,
        NULL
    );

    return ok && written == data.size( );
}

void DinoLabs::copy_frame( std::vector<uint8_t> frame )
{
    std::string str = "";
    for ( uint8_t b : frame )
    {
        str += std::format( "0x{:X}, ", b );
    }

    ImGui::SetClipboardText( str.c_str( ) );
}


void DinoLabs::print_bytes( const std::vector<uint8_t> &data )
{
    for ( uint8_t b : data ) {
        printf( "%02X ", b );
    }
    printf( "\n" );
}

uint8_t DinoLabs::calculate_checksum( std::vector<uint8_t> partial_frame )
{
    uint8_t chk = 0;
    for ( uint8_t b : partial_frame ) chk ^= b;
    return chk;
}

bool DinoLabs::frame_is_valid( std::vector<uint8_t> &frame ) {
    if ( frame.empty( ) ) return false;
    uint8_t chk = 0;
    for ( size_t i = 0; i < frame.size( ) - 1; i++ ) chk ^= frame[i];
    return chk == frame.back( );
}

void DinoLabs::present( )
{
    ImGui::SetNextWindowSize( ImVec2( 1600, 500 ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "dinolabs.tech UEMU v1.0.0" );
    ImGui::Text( "initalized: %d fps: %d", DinoLabs::initalized, (int)ImGui::GetIO().Framerate );

    auto _rx_frame = DinoLabs::fetch( );
    DinoLabs::rx_frame.clear( );
    for ( auto &b : _rx_frame )
    {
        DinoLabs::rx_frame += std::format( "{:X}, ", b );
    }

    ImGui::Text( "RX: %s", DinoLabs::rx_frame.c_str( ) );

    if ( DinoLabs::frame_is_valid( _rx_frame ) )
    {
        DinoLabs__ControllerRecv recv( _rx_frame );

        ImGui::Text( "RX -> %d throttle thing, %d amps max, %d kmph max, %d start mode, %d current mode", recv.amps, recv.current_limit_amps, recv.speed_limit_kmph, recv.start_mode, recv.mode );
    }

    if ( !initalized )
    {
        if ( DinoLabs::init( ) )
        {
            initalized = true;
        }
    }

    ImGui::Checkbox( "Send Known Stable Frame", &send_stable_frame );
    ImGui::Checkbox( "Send Target Speed", &send_target_speed );
    ImGui::SliderFloat( "Target Speed", &target_speed, 0.f, 300.f, "%.1f" );
    ImGui::Checkbox( "Send Target Watts", &send_target_watts );
    ImGui::SliderInt( "Target Watts", &target_watts, 0, 9999);

    if ( ImGui::Button( "Normal" ) )
    {
        error_code = DinoLabs__ControllerErrorType__NO_ERROR;
    }

    ImGui::Checkbox( "Braking", &is_braking );

    if ( ImGui::Button( "Motor Phase Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__MOTOR_PHASE_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "Brake Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__BRAKE_CUTOFF_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "BMS Short Circut Protection Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__BMS_SHORT_PROTECTION_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "Controller Short Circut Protection Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__CONTROLLER_SHORT_PROTECTION_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "Throttle Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__THROTTLE_COMMUNICATION_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "UART Communication Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__GENERAL_COMMUNICATION_ERROR;
    }

    ImGui::SameLine( ); if ( ImGui::Button( "Hall Sensor Error" ) )
    {
        error_code = DinoLabs__ControllerErrorType__HALL_SENSOR_ERROR;
    }

    for ( int i = 0; i < 10; i++ )
    {
        char buf[64];
        sprintf( buf, "Byte %d", i );
        ImGui::SliderInt( buf, &DinoLabs::bytes[i], 0, 255, "%02X" );
    }

    auto [b4, b5] = DinoLabs::speed_to_bytes( target_speed );
    auto [b1, b2] = DinoLabs::watts_to_bytes( target_watts );
    std::vector<uint8_t> frame = { 0x02, 0x0E, 0x01,
        (uint8_t)error_code,
        (uint8_t)( DinoLabs::is_braking ? 0xE4 : 0xC4 ),
        /* was byte1 */ (uint8_t)( DinoLabs::send_target_watts ? b1 : DinoLabs::bytes[1] ),
        /* was byte2 */ (uint8_t)( DinoLabs::send_target_watts ? b2 : DinoLabs::bytes[2] ),
        (uint8_t)DinoLabs::bytes[3],
        (uint8_t)( send_target_speed ? b4 : DinoLabs::bytes[4] ) /*MPH1 BYTE*/,
        (uint8_t)( send_target_speed ? b5 : DinoLabs::bytes[5] ) /*MPH1 BYTE*/,
        (uint8_t)DinoLabs::bytes[6],
        (uint8_t)DinoLabs::bytes[7],
        (uint8_t)DinoLabs::bytes[8]
    };

    frame.push_back( DinoLabs::calculate_checksum( frame ) );

    if ( frame.size( ) != 14 )
    {
        printf( "Mismatched Frame Size! Expected: 14, Got: %d", frame.size( ) );
    }

    if ( DinoLabs::send_stable_frame )
    {
        frame = { 0x02, 0x0e, 0x01, 0x00, 0xc4, 0x00, 0x00, 0x00, 0x0b, 0xb8, 0x00, 0x00, 0x00, 0x7a };
    }

    // DinoLabs::print_bytes( DinoLabs::fetch( ) );
    DinoLabs::send( frame );

    if ( ImGui::Button( "Copy Frame" ) )
    {
        DinoLabs::copy_frame( frame );
    }

    ImGui::End( );
}
