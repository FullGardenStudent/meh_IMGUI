#ifndef MEH_IMGUI_USER32_FUNCTIONS
#define MEH_IMGUI_USER32_FUNCTIONS( function )
#endif

MEH_IMGUI_USER32_FUNCTIONS(RegisterClassW)
MEH_IMGUI_USER32_FUNCTIONS(PostQuitMessage)
MEH_IMGUI_USER32_FUNCTIONS(DefWindowProcW)
MEH_IMGUI_USER32_FUNCTIONS(CreateWindowExW)
MEH_IMGUI_USER32_FUNCTIONS(ShowWindow)
MEH_IMGUI_USER32_FUNCTIONS(UpdateWindow)
MEH_IMGUI_USER32_FUNCTIONS(TranslateMessage)
MEH_IMGUI_USER32_FUNCTIONS(DispatchMessageW)
MEH_IMGUI_USER32_FUNCTIONS(PeekMessageW)
MEH_IMGUI_USER32_FUNCTIONS(AdjustWindowRectEx)
MEH_IMGUI_USER32_FUNCTIONS(GetWindowRect)
MEH_IMGUI_USER32_FUNCTIONS(SetWindowPos)

#undef MEH_IMGUI_USER32_FUNCTIONS

//

#ifndef MEH_IMGUI_DWMAPI_FUNCTIONS
#define MEH_IMGUI_DWMAPI_FUNCTIONS(function)
#endif

MEH_IMGUI_DWMAPI_FUNCTIONS(DwmEnableBlurBehindWindow)
MEH_IMGUI_DWMAPI_FUNCTIONS(DwmIsCompositionEnabled)
MEH_IMGUI_DWMAPI_FUNCTIONS(DwmDefWindowProc)

#undef MEH_IMGUI_DWMAPI_FUNCTIONS

//

#ifndef MEH_IMGUI_KERNEL32_FUNCTIONS
#define MEH_IMGUI_KERNEL32_FUNCTIONS( function )
#endif

//MEH_IMGUI_KERNEL32_FUNCTIONS(GetModuleHandleW)
//MEH_IMGUI_KERNEL32_FUNCTIONS(LoadLibraryW)
//MEH_IMGUI_KERNEL32_FUNCTIONS(FreeLibrary)

#undef MEH_IMGUI_KERNEL32_FUNCTIONS 

//