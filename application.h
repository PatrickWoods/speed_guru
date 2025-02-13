#pragma once
/******************************************************************************
*
*     application.h
*
*     - this is a singleton class that controls the application
*
******************************************************************************/

#include "pch.h"

namespace speed_guru
{
  //------------------------------------------- Application flags and constants
  
  const uint32_t AF_DEFAULT = 0;
  const uint32_t AF_NO_LOG = 1;
  const uint32_t AF_DEV_MODE = 2;

  //--------------------------------------------------------- class Application

  class Application
  {
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  public:

    inline static Application& GetInstance() { static Application a; return a; }

    inline static void Run(HINSTANCE inst_handle) { GetInstance().start(inst_handle); }

  protected:
  private:
    Application();
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void process_command_line();

    void start(HINSTANCE instance_handle);
    bool create_main_window();
    void update();

    bool                    m_is_running;
    uint32_t                m_app_flags;

    HINSTANCE               m_application_instance_handle;

    HWND                    m_window_handle;
    bool                    m_is_minimized;
    bool                    m_is_paused;
    std::wstring            m_window_title;
    int                     m_window_width;
    int                     m_window_height;
    HICON                   m_icon_handle;

    //------------------------------------------------- timing member variables

    LARGE_INTEGER           m_time_frequency;
    LARGE_INTEGER           m_time_last_frame;
    LARGE_INTEGER           m_time_this_frame;
    float                   m_time_delta;

    //----------------------------------- logger functions and member variables

    /*  decorated -->       tt:tt:tt    dd/dd/dd
    *   undecorated -->     tttttt      dddddd
    */
    std::wstring get_time_string(bool is_decorated = true);
    std::wstring get_date_string(bool is_decorated = true);
    std::wstring get_date_and_time_string(bool is_decorated = true);

    std::wstring get_log_directory();
    std::wstring get_log_filename();

    bool activate_log();
    bool deactivate_log();

    void post_to_log(const wchar_t* p_write_string, ...);

    std::wstring            m_log_directory;
    std::wstring            m_log_filename;
    bool                    m_is_log_active;

  };
}