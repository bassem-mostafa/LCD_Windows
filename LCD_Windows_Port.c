// #############################################################################
// #### Copyright ##############################################################
// #############################################################################

/*
 * Copyright 2024 BaSSeM
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// #############################################################################
// #### Description ############################################################
// #############################################################################

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

#include "Platform.h"

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

#ifndef DEBUG
    #define DEBUG
#endif

#ifdef DEBUG
    #undef DEBUG
#endif

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#ifdef LCD_WINDOWS

// #############################################################################
// #### Include(s) #############################################################
// #############################################################################

    #include "../../LCD_Internal.h"
    #include "LCD_Windows_Port.h"

    #include <stdbool.h>
    #include <stdio.h>

    #include <windows.h>

// #############################################################################
// #### Private Macro(s) #######################################################
// #############################################################################

    #define LCD_WINDOW_CLASS_NAME   "Platform LCD"
    #define LCD_WINDOW_WIDTH        0
    #define LCD_WINDOW_HEIGHT       0
    #define LCD_WINDOW_PIXEL_SIZE   5
    #define LCD_WINDOW_PADDING_SIZE ( 2 * LCD_WINDOW_PIXEL_SIZE )

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

typedef struct LCD_Window
{
    char Title[ UTIL_SizeOfWithoutNull( LCD_WINDOW_CLASS_NAME ) + UTIL_SizeOf( "LCD_xx" ) ];
    LCD_Size_t Size;
    LCD_Instance_t * Instance;
    HWND Handle;
    LCD_Screen_t Screen;
} LCD_Window_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

static LRESULT CALLBACK LCD_WindowCallback( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static bool LCD_WindowClassRegistered = false;

static LCD_Window_t LCD_Window[ LCD_NUMBER_OF_INSTANCES ];

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

static LRESULT CALLBACK LCD_WindowCallback( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    do
    {
        LCD_Trace( "%s( hwnd=%p, msg=%d, wParam=%p, lParam=%p )", __FUNCTION__, hwnd, msg, wParam, lParam );

        LCD_Window_t * Window = NULL;
        for ( LCD_t LCD = LCD_1; LCD < LCD_NUMBER_OF_INSTANCES; ++LCD )
        {
            if ( LCD_Window[ LCD ].Handle == hwnd )
            {
                Window = &LCD_Window[ LCD ];
                break;
            }
        }
        switch ( msg )
        {
            case WM_PAINT:
                do
                {
                    if ( Window == NULL
                         || Window->Instance == NULL
                         || Window->Screen == NULL )
                    {
                        break;
                    }
                    LCD_Debug( "LCD_%d WM_PAINT", Window->Instance->LCD );

                    HDC WindowDeviceContext = NULL;
                    HBRUSH hbrush_background = NULL;
                    HBRUSH hbrush_foreground = NULL;
                    PAINTSTRUCT WindowPaintStruct;
                    do
                    {
                        WindowDeviceContext = BeginPaint( Window->Handle, &WindowPaintStruct );
                        if ( WindowDeviceContext == NULL )
                        {
                            LCD_Warning( "BeginPaint: Failed" );
                            break;
                        }
                        hbrush_background = CreateSolidBrush( RGB( 0xDD, 0xEE, 0xFF ) );
                        hbrush_foreground = CreateSolidBrush( RGB( 0x00, 0x11, 0x22 ) );
                        if ( hbrush_background == NULL
                             || hbrush_foreground == NULL )
                        {
                            LCD_Warning( "CreateSolidBrush: Failed" );
                        }
                        for ( int row = 0; row < Window->Size.Height; ++row )
                        {
                            for ( int column = 0; column < Window->Size.Width; ++column )
                            {
                                RECT pixel = {
                                    ( LCD_WINDOW_PADDING_SIZE / 2 ) + ( LCD_WINDOW_PIXEL_SIZE * column ),
                                    ( LCD_WINDOW_PADDING_SIZE / 2 ) + ( LCD_WINDOW_PIXEL_SIZE * row ),
                                    ( LCD_WINDOW_PADDING_SIZE / 2 ) + ( LCD_WINDOW_PIXEL_SIZE * column ) + LCD_WINDOW_PIXEL_SIZE,
                                    ( LCD_WINDOW_PADDING_SIZE / 2 ) + ( LCD_WINDOW_PIXEL_SIZE * row ) + LCD_WINDOW_PIXEL_SIZE
                                };
                                LCD_Pixel_t LCD_Pixel = 0;
                                switch ( Window->Instance->LCD )
                                {
                                    case LCD_1:
                                        LCD_Pixel = ( LCD_Pixel_t ) ( *( LCD_LM6063DCW_A_Screen_t * ) ( Window->Screen ) )[ row ][ column ];
                                        break;
                                    case LCD_2:
                                        LCD_Pixel = ( LCD_Pixel_t ) ( *( LCD_LMB162AFC_Screen_t * ) ( Window->Screen ) )[ row ][ column ];
                                        break;
                                    default:
                                        LCD_Pixel = 0;
                                        break;
                                }
                                if ( LCD_Pixel )
                                {
                                    if ( FillRect( WindowDeviceContext, &pixel, hbrush_foreground ) == 0 )
                                    {
                                        LCD_Warning( "FillRect: Failed" );
                                    }
                                }
                                else
                                {
                                    if ( FillRect( WindowDeviceContext, &pixel, ( HBRUSH ) ( COLOR_WINDOW + 1 ) ) == 0 )
                                    {
                                        LCD_Warning( "FillRect: Failed" );
                                    }
                                }
                                if ( FrameRect( WindowDeviceContext, &pixel, hbrush_background ) == 0 )
                                {
                                    LCD_Warning( "FrameRect: Failed" );
                                }
                            }
                        }
                    }
                    while ( 0 );

                    if ( WindowDeviceContext != NULL && EndPaint( hwnd, &WindowPaintStruct ) == FALSE )
                    {
                        LCD_Warning( "EndPaint: Failed" );
                    }
                    if ( hbrush_background != NULL && DeleteObject( ( HGDIOBJ ) hbrush_background ) == FALSE )
                    {
                        LCD_Warning( "DeleteObject: Failed" );
                    }
                    if ( hbrush_foreground != NULL && DeleteObject( ( HGDIOBJ ) hbrush_foreground ) == FALSE )
                    {
                        LCD_Warning( "DeleteObject: Failed" );
                    }
                }
                while ( 0 );
                break;
            case WM_CLOSE:
                DestroyWindow( hwnd );
                break;
            case WM_DESTROY:
                PostQuitMessage( 0 );
                break;
            default:
                break;
        }
    }
    while ( 0 );
    return DefWindowProc( hwnd, msg, wParam, lParam );
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

LCD_Status_t LCD_IsValid( LCD_t LCD )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( LCD=%d )", __FUNCTION__, LCD );
        switch ( LCD )
        {
            case LCD_1:
            case LCD_2:
                LCD_Status = LCD_Status_Success;
                break;
            default:
                LCD_Status = LCD_Status_ArgumentInvalid;
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_Initialize( LCD_Instance_t * LCD_Instance )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p )", __FUNCTION__, LCD_Instance );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    RAM_Status_t RAM_Status = RAM_Status_Error;
                    if ( ( RAM_Status = RAM_Allocate( RAM_1, ( RAM_Reference_t * ) &LCD_Instance->LM6063DCW_A, UTIL_SizeOf( LCD_LM6063DCW_A_Instance_t ) ) ) != RAM_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    // TODO Update SPI_x, GPIO_x fields
                    LCD_Instance->LM6063DCW_A->SPI = SPI_1;
                    LCD_Instance->LM6063DCW_A->ChipSelect = GPIO_1;
                    LCD_Instance->LM6063DCW_A->RegisterSelect = GPIO_1;
                    LCD_Instance->LM6063DCW_A->Context = NULL;
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Initialize( LCD_Instance->LM6063DCW_A ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    RAM_Status_t RAM_Status = RAM_Status_Error;
                    if ( ( RAM_Status = RAM_Allocate( RAM_1, ( RAM_Reference_t * ) &LCD_Instance->LMB162AFC, UTIL_SizeOf( LCD_LMB162AFC_Instance_t ) ) ) != RAM_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    // TODO Update SPI_x, GPIO_x fields
                    LCD_Instance->LMB162AFC->SPI = SPI_1;
                    LCD_Instance->LMB162AFC->ChipSelect = GPIO_1;
                    LCD_Instance->LMB162AFC->RegisterSelect = GPIO_1;
                    LCD_Instance->LMB162AFC->ReadWrite = GPIO_1;
                    LCD_Instance->LMB162AFC->Context = NULL;
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_Initialize( LCD_Instance->LMB162AFC ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
        switch ( LCD_Status )
        {
            case LCD_Status_Success:
                do
                {
                    if ( LCD_WindowClassRegistered == false )
                    {
                        WNDCLASSEX WindowClass;
                        WindowClass.cbSize = UTIL_SizeOf( WindowClass );
                        WindowClass.style = ( UINT ) 0;
                        WindowClass.lpfnWndProc = ( WNDPROC ) LCD_WindowCallback;
                        WindowClass.cbClsExtra = ( int ) 0;
                        WindowClass.cbWndExtra = ( int ) 0;
                        WindowClass.hInstance = ( HINSTANCE ) NULL;
                        WindowClass.hIcon = ( HICON ) LoadIcon( NULL, IDI_APPLICATION );
                        WindowClass.hCursor = ( HCURSOR ) LoadCursor( NULL, IDC_ARROW );
                        WindowClass.hbrBackground = ( HBRUSH ) ( COLOR_WINDOW + 1 );
                        WindowClass.lpszMenuName = ( LPCSTR ) NULL;
                        WindowClass.lpszClassName = ( LPCSTR ) LCD_WINDOW_CLASS_NAME;
                        WindowClass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
                        if ( RegisterClassEx( &WindowClass ) == 0 )
                        {
                            MessageBox( NULL, "Window Class Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK );
                            LCD_Status = LCD_Status_Error;
                            break;
                        }
                        LCD_WindowClassRegistered = true;
                    }

                    LCD_Window_t * Window = &LCD_Window[ LCD_Instance->LCD ];
                    Window->Instance = LCD_Instance;
                    snprintf( Window->Title, UTIL_SizeOf( Window->Title ), "%s_%d", LCD_WINDOW_CLASS_NAME, Window->Instance->LCD );

                    if ( ( LCD_Status = LCD_GetSize( Window->Instance->LCD, &Window->Size ) ) != LCD_Status_Success )
                    {
                        // use default
                        Window->Size.Height = LCD_WINDOW_HEIGHT;
                        Window->Size.Width = LCD_WINDOW_WIDTH;
                    }
                    if ( ( LCD_Status = LCD_GetScreen( Window->Instance->LCD, &Window->Screen ) ) != LCD_Status_Success )
                    {
                        // FIXME
                    }
                    RECT ScreenSize = { 0, 0, LCD_WINDOW_PADDING_SIZE + ( LCD_WINDOW_PIXEL_SIZE * Window->Size.Width ), LCD_WINDOW_PADDING_SIZE + ( LCD_WINDOW_PIXEL_SIZE * Window->Size.Height ) };
                    if ( AdjustWindowRectEx( ( LPRECT ) &ScreenSize,
                                             ( DWORD ) WS_OVERLAPPEDWINDOW // MUST match @CreateWindowEx argument, overlapped window ex: WS_OVERLAPPEDWINDOW^(WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX)
                                                 ^ ( WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX ),
                                             false,
                                             0 // MUST match @CreateWindowEx argument, extended styles ex: WS_EX_CLIENTEDGE
                                             )
                         == 0 )
                    {
                        MessageBox( NULL, "Window Size Setting Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK );
                        LCD_Status = LCD_Status_Error;
                        break;
                    }

                    Window->Handle = CreateWindowEx(
                        ( DWORD ) 0,                      // extended styles ex: WS_EX_CLIENTEDGE
                        ( LPCSTR ) LCD_WINDOW_CLASS_NAME, // class name
                        ( LPCSTR ) Window->Title,         // window name
                        ( DWORD ) WS_OVERLAPPEDWINDOW     // overlapped window ex: WS_OVERLAPPEDWINDOW^(WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX)
                            ^ ( WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX ),
                        ( int ) CW_USEDEFAULT,                      // horizontal position
                        ( int ) CW_USEDEFAULT,                      // vertical position
                        ( int ) ScreenSize.right - ScreenSize.left, // width
                        ( int ) ScreenSize.bottom - ScreenSize.top, // height
                        ( HWND ) NULL,                              // parent or owner window
                        ( HMENU ) NULL,                             // class menu used
                        ( HINSTANCE ) NULL,                         // instance handle
                        ( LPVOID ) NULL                             // window creation data
                    );
                    if ( Window->Handle == NULL )
                    {
                        MessageBox( NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK );
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                // Nothing to be done
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_Cycle( LCD_Instance_t * LCD_Instance )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p )", __FUNCTION__, LCD_Instance );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Cycle( LCD_Instance->LM6063DCW_A ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_Cycle( LCD_Instance->LMB162AFC ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                LCD_Status = LCD_Status_Success;
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
        switch ( LCD_Status )
        {
            case LCD_Status_Success:
                do
                {
                    LCD_Window_t * Window = &LCD_Window[ LCD_Instance->LCD ];
                    if ( Window->Handle == NULL )
                    {
                        // Nothing to be done
                        LCD_Status = LCD_Status_Success;
                        break;
                    }
                    ShowWindow( Window->Handle, true );
                    UpdateWindow( Window->Handle );
                    MSG Msg;
                    if ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) > 0 )
                    {
                        TranslateMessage( &Msg );
                        DispatchMessage( &Msg );
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                // Nothing to be done
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_DeInitialize( LCD_Instance_t * LCD_Instance )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p )", __FUNCTION__, LCD_Instance );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_DeInitialize( LCD_Instance->LM6063DCW_A ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    RAM_Status_t RAM_Status = RAM_Status_Error;
                    if ( ( RAM_Status = RAM_DeAllocate( RAM_1, ( RAM_Reference_t * ) &LCD_Instance->LM6063DCW_A ) ) != RAM_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_DeInitialize( LCD_Instance->LMB162AFC ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    RAM_Status_t RAM_Status = RAM_Status_Error;
                    if ( ( RAM_Status = RAM_DeAllocate( RAM_1, ( RAM_Reference_t * ) &LCD_Instance->LMB162AFC ) ) != RAM_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
        switch ( LCD_Status )
        {
            case LCD_Status_Success:
                do
                {
                    LCD_Window_t * Window = &LCD_Window[ LCD_Instance->LCD ];
                    if ( Window->Handle != NULL )
                    {
                        DestroyWindow( Window->Handle );
                        Window->Handle = NULL;
                    }
                    if ( LCD_WindowClassRegistered == true )
                    {
                        int WindowCount = 0;
                        for ( LCD_t LCD = LCD_1; LCD < LCD_NUMBER_OF_INSTANCES; ++LCD )
                        {
                            LCD_Window_t * Window = &LCD_Window[ LCD_Instance->LCD ];
                            if ( Window->Handle != NULL )
                            {
                                WindowCount++;
                            }
                        }
                        if ( WindowCount == 0 )
                        {
                            // All windows has been destroyed
                            if ( UnregisterClass( LCD_WINDOW_CLASS_NAME, NULL ) == 0 )
                            {
                                MessageBox( NULL, "Window Class Un-Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK );
                                LCD_Status = LCD_Status_Error;
                                break;
                            }
                            LCD_WindowClassRegistered = false;
                        }
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                // Nothing to be done
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_GetSize( LCD_Instance_t * LCD_Instance, LCD_Size_t * LCD_Size )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p )", __FUNCTION__, LCD_Instance );
        if ( LCD_Size == NULL )
        {
            LCD_Status = LCD_Status_ArgumentInvalid;
            break;
        }
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                LCD_Size->Width = LCD_LM6063DCW_A_WIDTH;
                LCD_Size->Height = LCD_LM6063DCW_A_HEIGHT;
                LCD_Status = LCD_Status_Success;
                break;
            case LCD_2:
                LCD_Size->Width = LCD_LMB162AFC_WIDTH;
                LCD_Size->Height = LCD_LMB162AFC_HEIGHT;
                LCD_Status = LCD_Status_Success;
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_SetCursor( LCD_Instance_t * LCD_Instance, LCD_Coordinate_t LCD_Coordinate )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p, Coordinate={Row=%d, Column=%d} )", __FUNCTION__, LCD_Instance, LCD_Coordinate.Row, LCD_Coordinate.Column );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    LCD_LM6063DCW_A_Coordinate_t LCD_LM6063DCW_A_Coordinate = { LCD_Coordinate.Row, LCD_Coordinate.Column };
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_SetCursor( LCD_Instance->LM6063DCW_A, LCD_LM6063DCW_A_Coordinate ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    LCD_LMB162AFC_Coordinate_t LCD_LMB162AFC_Coordinate = { LCD_Coordinate.Row, LCD_Coordinate.Column };
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_SetCursor( LCD_Instance->LMB162AFC, LCD_LMB162AFC_Coordinate ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_SetPixel( LCD_Instance_t * LCD_Instance, LCD_Coordinate_t LCD_Coordinate, LCD_Pixel_t LCD_Pixel )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p, Coordinate={Row=%d, Column=%d}, Pixel=%02X )", __FUNCTION__, LCD_Instance, LCD_Coordinate.Row, LCD_Coordinate.Column, LCD_Pixel );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    LCD_LM6063DCW_A_Coordinate_t LCD_LM6063DCW_A_Coordinate = { LCD_Coordinate.Row, LCD_Coordinate.Column };
                    LCD_LM6063DCW_A_Pixel_t LCD_LM6063DCW_A_Pixel = LCD_Pixel;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_SetPixel( LCD_Instance->LM6063DCW_A, LCD_LM6063DCW_A_Coordinate, LCD_LM6063DCW_A_Pixel ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    LCD_LMB162AFC_Coordinate_t LCD_LMB162AFC_Coordinate = { LCD_Coordinate.Row, LCD_Coordinate.Column };
                    LCD_LMB162AFC_Pixel_t LCD_LMB162AFC_Pixel = LCD_Pixel;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_SetPixel( LCD_Instance->LMB162AFC, LCD_LMB162AFC_Coordinate, LCD_LMB162AFC_Pixel ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_GetScreen( LCD_Instance_t * LCD_Instance, LCD_Screen_t * LCD_Screen )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p, Screen=%p )", __FUNCTION__, LCD_Instance, LCD_Screen );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_GetScreen( LCD_Instance->LM6063DCW_A, ( LCD_LM6063DCW_A_Screen_t ** ) LCD_Screen ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_GetScreen( LCD_Instance->LMB162AFC, ( LCD_LMB162AFC_Screen_t ** ) LCD_Screen ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        // FIXME
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                LCD_Status = LCD_Status_Error;
                break;
        }
    }
    while ( 0 );
    return LCD_Status;
}

LCD_Status_t LCD_Instance_Write( LCD_Instance_t * LCD_Instance, LCD_Character_t LCD_Character )
{
}

LCD_Status_t LCD_Instance_Flush( LCD_Instance_t * LCD_Instance )
{
    LCD_Status_t LCD_Status = LCD_Status_Error;
    do
    {
        LCD_Trace( "%s( Instance=%p )", __FUNCTION__, LCD_Instance );
        if ( ( LCD_Status = LCD_Instance_IsValid( LCD_Instance ) ) != LCD_Status_Success )
        {
            break;
        }
        switch ( LCD_Instance->LCD )
        {
            case LCD_1:
                do
                {
                    LCD_LM6063DCW_A_Status_t LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Status_Error;
                    if ( ( LCD_LM6063DCW_A_Status = LCD_LM6063DCW_A_Flush( LCD_Instance->LM6063DCW_A ) ) != LCD_LM6063DCW_A_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            case LCD_2:
                do
                {
                    LCD_LMB162AFC_Status_t LCD_LMB162AFC_Status = LCD_LMB162AFC_Status_Error;
                    if ( ( LCD_LMB162AFC_Status = LCD_LMB162AFC_Flush( LCD_Instance->LMB162AFC ) ) != LCD_LMB162AFC_Status_Success )
                    {
                        LCD_Status = LCD_Status_Error;
                        break;
                    }
                    LCD_Status = LCD_Status_Success;
                }
                while ( 0 );
                break;
            default:
                break;
        }
        LCD_Window_t * Window = &LCD_Window[ LCD_Instance->LCD ];
        if ( InvalidateRect( Window->Handle, NULL, FALSE ) == 0 )
        {
            LCD_Status = LCD_Status_Error;
            break;
        }
        LCD_Status = LCD_Status_Success;
    }
    while ( 0 );
    return LCD_Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* LCD_WINDOWS */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
