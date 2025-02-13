// main.cpp

#include "pch.h"
#include "application.h"

int WINAPI wWinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPWSTR cmd_line, int show_cmd)
{
  speed_guru::Application::GetInstance().Run(this_inst);

  return 0;
}