#ifndef MEN_WIN32_H
#define MEN_WIN32_H

#ifdef _WIN32

#include "clog/clog.hh"
#include <dwmapi.h>

#include <locale>
#include <string>
#include <string_view>

#include "../meh_vk/meh_vulkan.hh"
#include "../../meh_IMGUI_widgets.hh"

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// static LRESULT (*custom_procw)(HWND, UINT, WPARAM, LPARAM);

extern "C" struct meh_window {
  // bool logging = false;
  bool render_frame;
  uint32_t width, height;
  HMODULE user32;
  HMODULE dwmapi;
  HMODULE kernel32;
  
  wchar_t *title;
  HINSTANCE h_instance = nullptr;
  HWND hwnd = nullptr;
  MSG msg = {};

  // bool using_custom_proc = false;
  bool using_custom_hit_test = false;
  RECT window;
  POINT mouse_pos;
  
  // for delta time
  LARGE_INTEGER frequency, current_time, previous_time;
  double delta_time, total_time=0.0;
};

typedef bool (*hit_fn_ptr)(uint32_t x, uint32_t y, long top, long left, long right, long bottom);

typedef unsigned int U32;

// kernel32.dll functions
typedef HINSTANCE(WINAPI *WINPFN_GetModuleHandleW)(LPCWSTR);
typedef HMODULE(WINAPI *WINPFN_LoadLibraryW)(LPCWSTR);
typedef BOOL(WINAPI *WINPFN_FreeLibrary)(HMODULE);

// user32.dll functions
typedef ATOM(WINAPI *WINPFN_RegisterClassW)(WNDCLASSW *);
typedef VOID(WINAPI *WINPFN_PostQuitMessage)(int);
typedef LRESULT(CALLBACK *WINPFN_DefWindowProcW)(HWND, UINT, WPARAM, LPARAM);
typedef HWND(WINAPI *WINPFN_CreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD,
                                             int, int, int, int, HWND, HMENU,
                                             HINSTANCE, LPVOID);

typedef BOOL(WINAPI *WINPFN_ShowWindow)(HWND, int);
typedef BOOL(WINAPI *WINPFN_UpdateWindow)(HWND hWnd);
typedef BOOL(WINAPI *WINPFN_TranslateMessage)(MSG *);
typedef LRESULT(WINAPI *WINPFN_DispatchMessageW)(MSG *);
// typedef BOOL(WINAPI *WINPFN_GetMessageW)(LPMSG, HWND, UINT, UINT);
typedef BOOL(WINAPI *WINPFN_PeekMessageW)(LPMSG, HWND, UINT, UINT, UINT);
typedef BOOL(WINAPI *WINPFN_AdjustWindowRectEx)(LPRECT, DWORD, BOOL, DWORD);
typedef BOOL(WINAPI *WINPFN_GetWindowRect)(HWND, LPRECT);
typedef BOOL(WINAPI *WINPFN_SetWindowPos)(HWND, HWND, int, int, int, int, UINT);

// dwmapi.dll functions
typedef HRESULT(__stdcall *WINPFN_DwmEnableBlurBehindWindow)(
    HWND, const DWM_BLURBEHIND *);
typedef HRESULT(__stdcall *WINPFN_DwmIsCompositionEnabled)(BOOL *);
typedef BOOL(__stdcall *WINPFN_DwmDefWindowProc)(HWND, UINT, WPARAM, LPARAM,
                                                 LRESULT *);

namespace meh {

uint32_t RECTWIDTH(RECT rect);
uint32_t RECTHEIGHT(RECT rect);

bool load_dwmpai_functions(HMODULE const &dwmapi_lib);
bool load_user32_functions(HMODULE const &user32_lib);
// bool load_kernel32_functions(HMODULE kernel32_lib);

#define MEH_IMGUI_DWMAPI_FUNCTIONS(name) extern WINPFN_##name name;
#define MEH_IMGUI_USER32_FUNCTIONS(name) extern WINPFN_##name name;
// #define MEH_IMGUI_KERNEL32_FUNCTIONS(name) extern WINPFN_##name name;

//  bool load_renderer_functions(HMODULE renderer_library);
#include "meh_win32_functions.inl"

  bool init_lib(struct meh_window &window, uint32_t window_type);
  //bool init_borderless(meh_window &window);

bool meh_window(struct meh_window &window, std::string_view window_title,
                uint32_t x_position, uint32_t y_position, uint32_t width,
                uint32_t height, uint32_t window_type);
  
inline U32 get_width(HWND hwnd) {
  RECT temp_rect;
  GetWindowRect(hwnd, &temp_rect);
  return RECTWIDTH(temp_rect);
}

void renderer_enabled();

void set_top_hit_fn(hit_fn_ptr hit_fn);
void set_left_hit_fn(hit_fn_ptr hit_fn); 
void set_right_hit_fn(hit_fn_ptr hit_fn); 
void set_bottom_hit_fn(hit_fn_ptr hit_fn);


inline U32 get_height(HWND hwnd) {
  RECT temp_rect;
  GetWindowRect(hwnd, &temp_rect);
  return RECTHEIGHT(temp_rect);
}

//void set_custom_proc(LRESULT (*custom_proc)(HWND, UINT, WPARAM, LPARAM));
  POINT get_mouse_pos();
} // namespace meh

#endif
#endif
