// application.cpp

#include "pch.h"
#include "application.h"

namespace speed_guru
{
  //------------------------------------------------- Windows Callback function

  LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
  {
    LRESULT result = 0;

    switch (message)
    {
    case WM_DESTROY:
    {
      Application::GetInstance().m_is_running = false;
      PostQuitMessage(0);
    } break;

    case WM_CLOSE:
    {
      Application::GetInstance().m_is_running = false;
      PostQuitMessage(0);
    } break;

    default:
      return DefWindowProc(hwnd, message, wparam, lparam);
    }
    return result;
  }

  Application::Application()
  {
    // initialize member variables

    m_is_running = false;
    m_app_flags = AF_DEFAULT;

    m_application_instance_handle = NULL;

    m_window_handle = NULL;
    m_is_minimized = false;
    m_is_paused = false;
    m_window_title = L"speed_guru";
    m_window_width = 1280;
    m_window_height = 900;
    m_icon_handle = NULL;

    // process the command line

    process_command_line();

    // setup timing variables

    QueryPerformanceFrequency(&m_time_frequency);
    ZeroMemory(&m_time_last_frame, sizeof(LARGE_INTEGER));
    ZeroMemory(&m_time_this_frame, sizeof(LARGE_INTEGER));
    m_time_delta = 0.0f;

    // setup logger

    m_log_directory = get_log_directory();
    m_log_filename = get_log_filename();

    std::wfstream out_file;

    // check to see if the log is switched off; if so, low key turn it off
    if (m_app_flags & AF_NO_LOG)
    {
      m_is_log_active = false;
    }
    else
    {
      m_is_log_active = true;

      std::wstring os = m_log_directory + L"\\" + m_log_filename;
      out_file.open(os, std::ios_base::app);

      if (out_file.is_open())
      {
        out_file << L"Speed Guru Log File" << L'\n' << L"-------------------\n\n";
        out_file << L"[" << get_date_and_time_string(true).c_str() << L"] Application initialized.\n";
        out_file.close();
      }
      else
      {
        MessageBox(NULL, L"Unable to open .logg file!", L"Logg error", MB_OK);
      }
    }
  }

  Application::~Application()
  {
    post_to_log(L"Application terminating.");
  }

  void Application::process_command_line()
  {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; i < argc; ++i)
    {
      std::wstring key = argv[i];
      if (key[0] == '-')
      {
        key.erase(0, 1);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if ((wcscmp(key.c_str(), L"nolog") == 0) || (wcscmp(key.c_str(), L"nologg") == 0))
        {
          m_app_flags |= AF_NO_LOG;
        }
      }
    }
  }

  void Application::start(HINSTANCE instance_handle)
  {
    // grab the instance handle
    if (instance_handle == NULL)
    {
      post_to_log(L"Bad instance handle passed from o/s. Shutting down.");
      m_is_running = false;
      return;
    }
    else
    {
      m_application_instance_handle = instance_handle;
    }

    // create the main window
    if (!create_main_window())
    {
      post_to_log(L"Main window creation failed. Shutting down.");
      m_is_running = false;
      return;
    }
    else
    {
      m_is_running = true;
    }

    // TODO: initialize the renderer

    // TODO: initialize the game

    // start the message loop
    MSG message;
    while (m_is_running)
    {
      if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
      else
      {
        QueryPerformanceCounter(&m_time_this_frame);
        m_time_delta = (float)(m_time_this_frame.QuadPart - m_time_last_frame.QuadPart) / (float)m_time_frequency.QuadPart;
        m_time_last_frame = m_time_this_frame;

        // update and render
        update();
      }
    }

  }

  bool Application::create_main_window()
  {
    // create the window class
    WNDCLASSEX window_class;
    ZeroMemory(&window_class, sizeof(WNDCLASSEX));
    const wchar_t* DEFAULT_WNDCLASSEX_NAME = L"speed_guru_window_class_name";

    // populate the class
    window_class.style = CS_VREDRAW | CS_HREDRAW;
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.lpfnWndProc = WndProc;
    window_class.hInstance = m_application_instance_handle;
    window_class.lpszClassName = DEFAULT_WNDCLASSEX_NAME;
    window_class.lpszMenuName = nullptr;

    // load custom icon or, failing that, the default icon
    /*m_icon_handle = LoadIcon(m_application_instance_handle, MAKEINTRESOURCE(IDI_MAIN_ICON));
    if (m_icon_handle != nullptr)
    {
      window_class.hIcon = m_icon_handle;
      window_class.hIconSm = m_icon_handle;
    }
    else
    {
      window_class.hIcon = LoadIcon(0, IDI_APPLICATION);
      window_class.hIconSm = LoadIcon(0, IDI_APPLICATION);
    }*/

    // can be removed once custom icon implemented
    window_class.hIcon = LoadIcon(0, IDI_APPLICATION);
    window_class.hIconSm = LoadIcon(0, IDI_APPLICATION);
    // can be removed once custom icon implemented

    window_class.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    window_class.hCursor = LoadCursor(0, IDC_ARROW);

    if (!RegisterClassEx(&window_class))
    {
      post_to_log(L"Failed to register window class. Shutting down.");
      return m_is_running = false;
    }

    // correct the xy dimensions for the presence of the titlebar &c.
    DWORD window_style = WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT window_rect;
    ZeroMemory(&window_rect, sizeof(RECT));
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = m_window_width;
    window_rect.bottom = m_window_height;
    AdjustWindowRect(&window_rect, window_style, false);

    // get a window handle
    m_window_handle = CreateWindowEx(
      0,
      DEFAULT_WNDCLASSEX_NAME,
      m_window_title.c_str(),
      window_style,
      0,
      0,
      window_rect.right - window_rect.left,
      window_rect.bottom - window_rect.top,
      0,
      0,
      m_application_instance_handle,
      0);

    // guard against a failure to create the window
    if (m_window_handle == NULL)
    {
      post_to_log(L"Failed to create window handle. Shutting down.");
      return m_is_running = false;
    }

    post_to_log(L"Main window created.");
    ShowWindow(m_window_handle, SW_SHOW);
    return true;
  }

  void Application::update()
  {

  }

  std::wstring Application::get_time_string(bool is_decorated)
  {
    time_t now = time(0);
    tm local_time;
    localtime_s(&local_time, &now);
    std::wstringstream wss;
    wss << std::put_time(&local_time, L"%T");

    std::wstring time_string = wss.str();

    if (!is_decorated)
    {
      std::wstring remove_char = L":";
      for (wchar_t c : remove_char)
      {
        time_string.erase(std::remove(time_string.begin(), time_string.end(), c), time_string.end());
      }
    }

    return time_string;
  }

  std::wstring Application::get_date_string(bool is_decorated)
  {
    time_t now = time(0);
    tm local_time;
    localtime_s(&local_time, &now);
    std::wstringstream wss;
    wss << std::put_time(&local_time, L"%d/%m/%y");

    std::wstring date_string = wss.str();

    if (!is_decorated)
    {
      std::wstring remove_char = L"/";
      for (wchar_t c : remove_char)
      {
        date_string.erase(std::remove(date_string.begin(), date_string.end(), c), date_string.end());
      }
    }

    return date_string;
  }

  std::wstring Application::get_date_and_time_string(bool is_decorated)
  {
    std::wstring date_time_string;

    if (is_decorated)
    {
      date_time_string = get_date_string(true) + L" " + get_time_string(true);
    }
    else
    {
      date_time_string = get_date_string(false) + get_time_string(false);
    }

    return date_time_string;
  }

  std::wstring Application::get_log_directory()
  {
    wchar_t path[1024];
    wchar_t* p_appdatalocal;

    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &p_appdatalocal);

    wcscpy_s(path, p_appdatalocal);
    wcscat_s(path, L"\\");
    wcscat_s(path, L"speed_guru");
    CreateDirectory(path, NULL);

    wcscat_s(path, L"\\logg");
    CreateDirectory(path, NULL);

    return path;
  }

  std::wstring Application::get_log_filename()
  {
    wchar_t filename[1024];
    wcscpy_s(filename, L"speed_guru");
    wcscat_s(filename, get_date_and_time_string(false).c_str());
    wcscat_s(filename, L".logg");
    return filename;
  }

  bool Application::activate_log()
  {
    if (m_is_log_active)
    {
      return false;
    }
    else
    {
      post_to_log(L"Logger activated.");
      return m_is_log_active = true;
    }
  }

  bool Application::deactivate_log()
  {
    if (!m_is_log_active)
    {
      return false;
    }
    else
    {
      post_to_log(L"Logger deactivated.");
      m_is_log_active = false;
      return true;
    }
  }

  void Application::post_to_log(const wchar_t* p_write_string, ...)
  {
    if (!m_is_log_active) return;

    wchar_t buffer[4096];
    va_list args;

    va_start(args, p_write_string);
    vswprintf_s(buffer, p_write_string, args);
    va_end(args);

    std::wfstream out_file;

    std::wstring os = m_log_directory + L"\\" + m_log_filename;
    out_file.open(os, std::ios_base::app);

    if (out_file.is_open())
    {
      std::wstring s = buffer;
      out_file << L"[" << get_date_and_time_string() << L"] " << s << L'\n';
      out_file.close();
    }
    else
    {
      MessageBox(NULL, L"Unable to open .logg file!", L"Logg error", MB_OK);
    }
  }
}