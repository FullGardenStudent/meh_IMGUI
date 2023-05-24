#ifdef _WIN32
#include "meh_win32.hh"

namespace meh {

POINT mouse_pos;

uint32_t RECTWIDTH(RECT rect) { return rect.right - rect.left; }
uint32_t RECTHEIGHT(RECT rect) { return rect.bottom - rect.top; }

// hit function pointers


hit_fn_ptr top_hit;
hit_fn_ptr left_hit;
hit_fn_ptr right_hit;
hit_fn_ptr bottom_hit;

push_constants pc;

//  static bool resize = false;
static bool l_mouse = false;
bool is_borderless_window = false;

// #define MEH_IMGUI_KERNEL32_FUNCTIONS(name) WINPFN_##name name;
#define MEH_IMGUI_DWMAPI_FUNCTIONS(name) WINPFN_##name name;
//
#define MEH_IMGUI_USER32_FUNCTIONS(name) WINPFN_##name name;
#include "meh_win32_functions.inl"
#undef MEH_IMGUI_USER32_FUNCTIONS
#undef MEH_IMGUI_DWMAPI_FUNCTIONS

bool load_dwmpai_functions(HMODULE const &dwmapi_lib) {
#define MEH_IMGUI_DWMAPI_FUNCTIONS(name)                                       \
  name = (WINPFN_##name)GetProcAddress(dwmapi_lib, #name);                     \
  if (name == nullptr) {                                                       \
    CERROR("Count not load windows dwmapi function named : " #name);           \
    return false;                                                              \
  } else {                                                                     \
    CINFO("Loaded windows dwmapi function " #name);                            \
  }

#include "meh_win32_functions.inl"
  return true;
}
#undef MEH_IMGUI_DWMAPI_FUNCTIONS

bool load_user32_functions(HMODULE const &user32_lib) {
#define MEH_IMGUI_USER32_FUNCTIONS(name)                                       \
  name = (WINPFN_##name)GetProcAddress(user32_lib, #name);                     \
  if (name == nullptr) {                                                       \
    CERROR("Count not load windows user32 function named : " #name);           \
    return false;                                                              \
  } else {                                                                     \
    CINFO("successfully loaded windows user32 function " #name);               \
  }

#include "meh_win32_functions.inl"
  return true;
}

bool load_kernel32_functions(HMODULE kernel32_lib) {
#define MEH_IMGUI_KERNEL32_FUNCTIONS(name)                                     \
  name = (WINPFN_##name)GetProcAddress(kernel32_lib, #name);                   \
  if (name == nullptr) {                                                       \
    CERROR("Count not load windows user32 function named : " #name);           \
    return false;							\
  } else {                                                                     \
    CINFO("successfully loaded windows user32 function " #name);               \
  }
#include "meh_win32_functions.inl"
  return true;
}

bool init_lib(struct meh_window &window, U32 window_type) {
  // kernel32.dll
  window.kernel32 = LoadLibraryW(L"Kernel32.dll");
  CLOG_RF(window.kernel32, "Locating kernel32.dll");
  load_kernel32_functions(window.kernel32);
  // usre32.dll
  window.user32 = LoadLibraryW(L"user32.dll");
  CLOG_RF(window.user32, "Locating user32.dll");
  load_user32_functions(window.user32);

  if (window_type == 1) {
    // load dwmapi.dll
    window.dwmapi = LoadLibraryW(L"dwmapi.dll");
    CLOG_RF(window.dwmapi, "Locating dwmapi.dll");
    load_dwmpai_functions(window.dwmapi);
  }

  // FreeLibrary(n);
  return true;
}

bool default_top_hit(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
    return (mousey >= (uint32_t)top && mousey < (uint32_t)top + 22);
}

bool default_left_hit(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
    return (mousex >= (uint32_t)left && mousex < (uint32_t)left + 5);
}

bool default_right_hit(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
    return (mousex < (uint32_t)right && mousex >= (uint32_t)right - 5);
}

bool default_bottom_hit(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
    return (mousey < (uint32_t)bottom && mousey >(uint32_t)bottom-5);
}

void set_top_hit_fn(hit_fn_ptr hit_fn) {
    top_hit = hit_fn;
}
void set_left_hit_fn(hit_fn_ptr hit_fn) {
    left_hit = hit_fn;
}
void set_right_hit_fn(hit_fn_ptr hit_fn) {
    right_hit = hit_fn;
}
void set_bottom_hit_fn(hit_fn_ptr hit_fn) {
    bottom_hit = hit_fn;
}

void renderer_enabled() {
    is_borderless_window = true;
}

bool l_button() { return l_mouse; }

LRESULT CALLBACK meh_window_proc(HWND hwnd, UINT msg, WPARAM wparam,
                                 LPARAM lparam) {
  switch (msg) {
  case WM_SIZING: {
      if (is_borderless_window) {
          pc = { get_width(hwnd), get_height(hwnd), (U32)mouse_pos.x,
                               (U32)mouse_pos.y, 0 };
          update_ui(pc.screen_width, pc.screen_height, pc.x_pos, pc.y_pos, pc.quad_id, 0);
          update_quads();
          resize(pc);
      }
    break;
  }
  case WM_LBUTTONDOWN: {
    l_mouse = true;
    break;
  }
  case WM_LBUTTONUP: {
    l_mouse = false;
    break;
  }
  case WM_ENTERSIZEMOVE: {
    CINFO("entered size move!");
    return 0;
  }
  case WM_EXITSIZEMOVE: {
    CINFO("exiting size move!");
    return 0;
  }
  case WM_DESTROY: {
    if(is_borderless_window)
        destroy_swapchain();
    PostQuitMessage(0);
  } break;
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

POINT get_mouse_pos() { return mouse_pos; }

LRESULT meh_hit_test(HWND hWnd, LPARAM lParam) {

  // Get the point coordinates for the hit test.
  mouse_pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

  // Get the window rectangle.
  RECT t_window;
  GetWindowRect(hWnd, &t_window);
  // std::cout << "top : " << window.window.top << "\tleft : " <<
  // window.window.left << std::endl;
  //  Get the frame rectangle, adjusted for the style without a caption.
  RECT rcFrame = {};
  AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

  // Determine if the hit test is for resizing. Default middle (1,1).
  USHORT uRow = 1;
  USHORT uCol = 1;
  bool fOnResizeBorder = false;

  // Determine if the point is at the top or bottom of the window.
  if(top_hit((uint32_t)mouse_pos.x,(uint32_t)mouse_pos.y, t_window.top, t_window.left, t_window.right, t_window.bottom)){
    fOnResizeBorder = (mouse_pos.y < (t_window.top - rcFrame.top));
    uRow = 0;
  } else if (bottom_hit((uint32_t)mouse_pos.x, (uint32_t)mouse_pos.y, t_window.top, t_window.left, t_window.right, t_window.bottom)) {
    uRow = 2;
  }

  // Determine if the point is at the left or right of the window.
  if (left_hit((uint32_t)mouse_pos.x, (uint32_t)mouse_pos.y, t_window.top, t_window.left, t_window.right, t_window.bottom)) {
    uCol = 0; // left side
  } else if (right_hit((uint32_t)mouse_pos.x, (uint32_t)mouse_pos.y, t_window.top, t_window.left, t_window.right, t_window.bottom)) {
    uCol = 2; // right side
  }

  // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
  LRESULT hitTests[3][3] = {
      {HTTOPLEFT, fOnResizeBorder ? HTTOP : HTCAPTION, HTTOPRIGHT},
      {HTLEFT, HTNOWHERE, HTRIGHT},
      {HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT},
  };

  return hitTests[uRow][uCol];
}

LRESULT meh_caption_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
                         bool *pfCallDWP) {
  LRESULT lRet = 0;
  HRESULT hr = S_OK;
  bool fCallDWP = true; // Pass on to DefWindowProc?
  RECT temp_rect;

  GetWindowRect(hWnd, &temp_rect);
  mouse_pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

  fCallDWP = !DwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

  // Handle window creation.
  if (message == WM_CREATE) {
    RECT rcClient;
    GetWindowRect(hWnd, &rcClient);

    // Inform application of the frame change.
    SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, RECTWIDTH(rcClient),
                 RECTHEIGHT(rcClient), SWP_FRAMECHANGED);

    fCallDWP = true;
    lRet = 0;
  }

  // Handle the non-client size message.
  if ((message == WM_NCCALCSIZE) && (wParam == TRUE)) {
    // Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
    NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam);

    pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
    pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
    pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
    pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

    lRet = 0;

    // No need to pass the message on to the DefWindowProc.
    fCallDWP = false;
  }

  // Handle hit testing in the NCA if not handled by DwmDefWindowProc.
  if ((message == WM_NCHITTEST) && (lRet == 0)) {
    lRet = meh_hit_test(hWnd, lParam);

    if (lRet != HTNOWHERE) {
      fCallDWP = false;
    }
  }

  *pfCallDWP = fCallDWP;

  return lRet;
}

LRESULT CALLBACK meh_borderless_proc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
  bool fCallDWP = true;
  BOOL fDwmEnabled = FALSE;
  LRESULT lRet = 0;
  HRESULT hr = S_OK;

  // HWND__ a;
  //  Winproc worker for custom frame issues.
  hr = DwmIsCompositionEnabled(&fDwmEnabled);
  if (SUCCEEDED(hr)) {
    lRet = meh_caption_proc(hWnd, message, wParam, lParam, &fCallDWP);
  }

  // Winproc worker for the rest of the application.
  if (fCallDWP) {
    lRet = meh_window_proc(hWnd, message, wParam, lParam);
  }
  return lRet;
}

bool meh_window(struct meh_window &window, std::string_view window_title,
                uint32_t x_position, uint32_t y_position, uint32_t width,
                uint32_t height, uint32_t window_type) {
    std::wstring title(window_title.begin(), window_title.end());
    window.title = (wchar_t*)title.data();// window_title.data();

  /*if (window_type) {
      is_borderless_window = true;
  }
  else {
      is_borderless_window = false;
  }*/

  window.h_instance = GetModuleHandleW(0);

  // if (!window.using_custom_proc) {
  // custom_procw = meh_window_proc;
  // }

  WNDCLASSW wc = {};
  std::memset(&wc, 0, sizeof(wc));
  if (window_type == 0) {
    wc.lpfnWndProc = meh_window_proc;
  } else if (window_type == 1) {
    wc.lpfnWndProc = meh_borderless_proc;
  }
  wc.hInstance = window.h_instance;
  wc.lpszClassName = window.title;
  // wc.hbrBackground = (HBRUSH)(BLACK_BRUSH);
  DWORD style = WS_OVERLAPPEDWINDOW;
  RECT rect = {(LONG)x_position, (LONG)y_position, (LONG)width, (LONG)height};
  AdjustWindowRectEx(&rect, style, 0, NULL);

  // CLOG(RegisterClassW(&wc), "Regsitering window class!");

  RegisterClassW(&wc);

  window.hwnd = CreateWindowExW(0,                   // Optional window styles.
                                window.title,        // Window class
                                window.title,        // Window text
                                WS_OVERLAPPEDWINDOW, // Window style
                                // Size and position
                                rect.left, rect.top, rect.right - rect.left,
                                rect.bottom - rect.top,
                                NULL,              // Parent window
                                NULL,              // Menu
                                window.h_instance, // Instance handle
                                NULL // Additional application data
  );

  if (window.hwnd == NULL) {
    // CFATAL("Failed to create window handle!")
    std::cout << "Failed to create window handle!" << std::endl;
    return 0;
  }

  ShowWindow(window.hwnd, SW_SHOW);
  UpdateWindow(window.hwnd);

   top_hit = default_top_hit;   
   left_hit = default_left_hit;
   right_hit = default_right_hit;
   bottom_hit = default_bottom_hit;

  CINFO("Borderless window created!");
  return true;
}

} // namespace meh

#endif
