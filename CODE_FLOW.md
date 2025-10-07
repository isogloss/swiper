# Code Flow Documentation

This document traces the execution flow through the codebase for key operations.

---

## Application Startup

### Entry Point: WinMain (main.cpp)

```cpp
WinMain()
  ↓
  AllocConsole()  // Create debug console
  ↓
  InitializeIPC()  // Create shared memory, events, mutexes
  ↓
  InitializeMainWindow(hInstance)
    ↓
    RegisterClassExW(&wc)  // Register "SwiperMainWindowClass"
    ↓
    CreateWindowExW(...)  // Create control panel window
    ↓
    new UIManager()  // Allocate UI manager
    ↓
    g_pUIManager->Initialize(g_hMainWindow, hInstance)
      ↓
      EnumerateMonitors()  // Populate monitor list
        ↓
        EnumDisplayMonitors(..., MonitorEnumProc, ...)
          ↓
          [For each monitor]
            ↓
            GetMonitorInfoW(hMonitor, &monitorInfo)
            ↓
            Store display string in m_config.availableMonitors
            ↓
            Store MonitorInfo (handle, rect, name) in m_monitors
      ↓
      CreateTabControl(hParent)
        ↓
        CreateWindowExW(WC_TABCONTROLW, ...)
        ↓
        TabCtrl_InsertItem(0, "License")
        ↓
        TabCtrl_InsertItem(1, "Settings")
        ↓
        TabCtrl_InsertItem(2, "Projection")
      ↓
      CreateLicenseTab(hParent, hInstance)
        ↓
        Create license key label
        ↓
        Create license key edit control
        ↓
        Create validate button
        ↓
        Create status label
      ↓
      CreateSettingsTab(hParent, hInstance)
        ↓
        Create FPS label, edit, updown controls
        ↓
        Create monitor label, combobox
        ↓
        Populate combobox with monitor names
      ↓
      CreateProjectionTab(hParent, hInstance)
        ↓
        Create Start Projection button (enabled if licensed)
        ↓
        Create Stop Projection button (disabled)
        ↓
        Create Inject button (enabled if licensed)
        ↓
        Create status label
      ↓
      OnTabChanged(0)  // Show License tab first
        ↓
        ShowWindow(m_hLicenseTab, SW_SHOW)
        ↓
        ShowWindow(m_hSettingsTab, SW_HIDE)
        ↓
        ShowWindow(m_hProjectionTab, SW_HIDE)
      ↓
      UpdateProjectionState(false)  // Disable buttons (not licensed yet)
    ↓
    ShowWindow(g_hMainWindow, SW_SHOW)
  ↓
  RegisterClassExW(&wc)  // Register "SwiperWindowClass" (projection)
  ↓
  g_isWindowClassRegistered = true
  ↓
  [Enter Message Loop]
    ↓
    while (g_isRunning)
      ↓
      PeekMessage(&msg, ...)
      ↓
      TranslateMessage(&msg)
      ↓
      DispatchMessage(&msg)
        ↓
        → MainWindowProc() or WindowProc()
      ↓
      RenderLoop()  // Check for captured frames
      ↓
      Sleep(1)
```

---

## License Validation Flow

### User Action: Enter license key and click "Validate License"

```cpp
User types in m_hLicenseKeyEdit control
  ↓
User clicks m_hValidateButton
  ↓
WM_COMMAND message sent to MainWindowProc
  ↓
MainWindowProc(hwnd, WM_COMMAND, wParam, lParam)
  ↓
  g_pUIManager->OnCommand(wParam, lParam)
    ↓
    switch (LOWORD(wParam))
      ↓
      case ID_VALIDATE_BUTTON:
        ↓
        OnValidateLicense()
          ↓
          GetWindowTextW(m_hLicenseKeyEdit, licenseKey, 256)
          ↓
          ValidateLicenseKey(licenseKey)
            ↓
            Check if key.length() >= 10
            ↓
            Check if key starts with "SWIPER-"
            ↓
            OR key == "DEMO-KEY"
            ↓
            OR key == "TEST-LICENSE"
            ↓
            return true/false
          ↓
          if (valid)
            ↓
            m_isLicenseValid = true
            ↓
            SetWindowTextW(m_hLicenseStatusLabel, "Status: License Valid!")
            ↓
            EnableWindow(m_hStartButton, TRUE)
            ↓
            EnableWindow(m_hInjectButton, TRUE)
            ↓
            Log: "License validated successfully!"
          else
            ↓
            m_isLicenseValid = false
            ↓
            SetWindowTextW(m_hLicenseStatusLabel, "Status: Invalid License Key")
            ↓
            EnableWindow(m_hStartButton, FALSE)
            ↓
            EnableWindow(m_hStopButton, FALSE)
            ↓
            EnableWindow(m_hInjectButton, FALSE)
            ↓
            Log: "License validation failed!"
```

---

## Start Projection Flow

### User Action: Click "Start Projection" button

```cpp
User clicks m_hStartButton (must be enabled = licensed)
  ↓
WM_COMMAND message sent to MainWindowProc
  ↓
MainWindowProc(hwnd, WM_COMMAND, wParam, lParam)
  ↓
  g_pUIManager->OnCommand(wParam, lParam)
    ↓
    switch (LOWORD(wParam))
      ↓
      case ID_START_BUTTON:
        ↓
        if (m_isLicenseValid)
          ↓
          OnStartProjection()
            ↓
            Log: "Starting projection at X FPS on monitor Y"
            ↓
            CreateProjectionWindow()  // External function in main.cpp
              ↓
              if (g_hWnd != nullptr)
                ↓
                ShowWindow(g_hWnd, SW_SHOW)  // Reuse existing
                ↓
                return true
              ↓
              if (!g_isWindowClassRegistered)
                ↓
                return false
              ↓
              if (!g_pUIManager)
                ↓
                return false
              ↓
              config = g_pUIManager->GetConfig()
              ↓
              monitors = g_pUIManager->GetMonitors()
              ↓
              if (config.selectedMonitor >= monitors.size())
                ↓
                return false
              ↓
              monitorInfo = monitors[config.selectedMonitor]
              ↓
              x = monitorInfo.rect.left
              ↓
              y = monitorInfo.rect.top
              ↓
              width = monitorInfo.rect.right - left
              ↓
              height = monitorInfo.rect.bottom - top
              ↓
              Log: "Creating fullscreen projection window..."
              ↓
              Log: "Position: (x, y), Size: widthxheight"
              ↓
              g_hWnd = CreateWindowExW(
                WS_EX_TOPMOST,           // Always on top
                L"SwiperWindowClass",    // Registered class
                L"Swiper Projection",    // Title (not visible)
                WS_POPUP,                // No borders
                x, y, width, height,     // Fullscreen on monitor
                nullptr, nullptr, g_hInstance, nullptr
              )
              ↓
              if (!g_hWnd)
                ↓
                Log error
                ↓
                return false
              ↓
              ShowWindow(g_hWnd, SW_SHOW)
              ↓
              UpdateWindow(g_hWnd)
              ↓
              Log: "Projection window created successfully"
              ↓
              return true
            ↓
            if (success)
              ↓
              m_isProjectionRunning = true
              ↓
              UpdateProjectionState(true)
                ↓
                EnableWindow(m_hStartButton, FALSE)  // Disable start
                ↓
                EnableWindow(m_hStopButton, TRUE)    // Enable stop
                ↓
                EnableWindow(m_hInjectButton, FALSE) // Disable inject
              ↓
              SetWindowTextW(m_hStatusLabel, "Status: Projection Running")
            else
              ↓
              SetWindowTextW(m_hStatusLabel, "Status: Failed to start projection")
```

---

## Stop Projection Flow

### User Action: Click "Stop Projection" button OR press ESC in projection window

```cpp
[Path 1: Stop Button]
User clicks m_hStopButton
  ↓
WM_COMMAND message sent to MainWindowProc
  ↓
MainWindowProc(hwnd, WM_COMMAND, wParam, lParam)
  ↓
  g_pUIManager->OnCommand(wParam, lParam)
    ↓
    case ID_STOP_BUTTON:
      ↓
      OnStopProjection()

[Path 2: ESC Key]
User presses ESC in projection window
  ↓
WM_KEYDOWN message sent to WindowProc
  ↓
WindowProc(g_hWnd, WM_KEYDOWN, VK_ESCAPE, ...)
  ↓
  DestroyProjectionWindow()
  ↓
  g_pUIManager->OnStopProjection()

[Common Path: OnStopProjection]
OnStopProjection()
  ↓
  Log: "Stopping projection"
  ↓
  DestroyProjectionWindow()  // External function in main.cpp
    ↓
    if (g_hWnd)
      ↓
      Log: "Destroying projection window"
      ↓
      DestroyWindow(g_hWnd)
        ↓
        → WindowProc(hwnd, WM_DESTROY, ...)
          ↓
          g_hWnd = nullptr
          ↓
          return 0  // Don't call PostQuitMessage
      ↓
      g_hWnd = nullptr  // (redundant but safe)
  ↓
  m_isProjectionRunning = false
  ↓
  UpdateProjectionState(false)
    ↓
    EnableWindow(m_hStartButton, TRUE && m_isLicenseValid)  // Re-enable start
    ↓
    EnableWindow(m_hStopButton, FALSE)  // Disable stop
    ↓
    EnableWindow(m_hInjectButton, TRUE && m_isLicenseValid)  // Re-enable inject
  ↓
  SetWindowTextW(m_hStatusLabel, "Status: Projection Stopped")
```

---

## Projection Window Message Handling

### Window Procedure: WindowProc (main.cpp)

```cpp
WindowProc(hwnd, uMsg, wParam, lParam)
  ↓
  switch (uMsg)
    ↓
    case WM_DESTROY:
      ↓
      g_hWnd = nullptr  // Clear global handle
      ↓
      return 0  // Don't quit app, just close this window
    
    case WM_KEYDOWN:
      ↓
      if (wParam == VK_ESCAPE)
        ↓
        DestroyProjectionWindow()
          ↓
          if (g_hWnd)
            ↓
            DestroyWindow(g_hWnd)
            ↓
            g_hWnd = nullptr
        ↓
        if (g_pUIManager)
          ↓
          g_pUIManager->OnStopProjection()
            ↓
            [See Stop Projection Flow above]
      ↓
      return 0
    
    case WM_PAINT:
      ↓
      BeginPaint(hwnd, &ps)
      ↓
      GetClientRect(hwnd, &rect)
      ↓
      FillRect(hdc, &rect, BLACK_BRUSH)
      ↓
      SetTextColor(hdc, RGB(255, 255, 255))
      ↓
      SetBkMode(hdc, TRANSPARENT)
      ↓
      DrawTextW(hdc, 
        "Swiper Projection Window\n\n"
        "Waiting for captured frames...\n\n"
        "Press ESC to exit",
        ..., DT_CENTER | DT_VCENTER | DT_SINGLELINE)
      ↓
      EndPaint(hwnd, &ps)
      ↓
      return 0
    
    default:
      ↓
      return DefWindowProc(hwnd, uMsg, wParam, lParam)
```

---

## Main Window Message Handling

### Window Procedure: MainWindowProc (main.cpp)

```cpp
MainWindowProc(hwnd, uMsg, wParam, lParam)
  ↓
  switch (uMsg)
    ↓
    case WM_DESTROY:
      ↓
      g_isRunning = false  // Signal app to exit
      ↓
      PostQuitMessage(0)   // Post WM_QUIT to message queue
      ↓
      return 0
    
    case WM_COMMAND:
      ↓
      if (g_pUIManager)
        ↓
        g_pUIManager->OnCommand(wParam, lParam)
          ↓
          [See License Validation / Start/Stop Projection flows]
      ↓
      return 0
    
    case WM_NOTIFY:
      ↓
      pNmhdr = (NMHDR*)lParam
      ↓
      if (pNmhdr->code == TCN_SELCHANGE && g_pUIManager)
        ↓
        newTab = TabCtrl_GetCurSel(pNmhdr->hwndFrom)
        ↓
        g_pUIManager->OnTabChanged(newTab)
          ↓
          m_currentTab = newTab
          ↓
          ShowWindow(m_hLicenseTab, SW_HIDE)
          ↓
          ShowWindow(m_hSettingsTab, SW_HIDE)
          ↓
          ShowWindow(m_hProjectionTab, SW_HIDE)
          ↓
          if (newTab == 0)
            ShowWindow(m_hLicenseTab, SW_SHOW)
          ↓
          else if (newTab == 1)
            ShowWindow(m_hSettingsTab, SW_SHOW)
          ↓
          else if (newTab == 2)
            ShowWindow(m_hProjectionTab, SW_SHOW)
      ↓
      return 0
    
    case WM_PAINT:
      ↓
      BeginPaint(hwnd, &ps)
      ↓
      EndPaint(hwnd, &ps)
      ↓
      return 0
    
    default:
      ↓
      return DefWindowProc(hwnd, uMsg, wParam, lParam)
```

---

## Application Shutdown

### User Action: Close main control panel window

```cpp
User clicks [X] button on control panel
  ↓
WM_CLOSE sent to MainWindowProc
  ↓
DefWindowProc handles WM_CLOSE → sends WM_DESTROY
  ↓
MainWindowProc(hwnd, WM_DESTROY, ...)
  ↓
  g_isRunning = false
  ↓
  PostQuitMessage(0)
  ↓
  return 0

[Back in message loop]
while (g_isRunning)  // Now false
  ↓
  [Exit loop]

CleanupIPC()
  ↓
  g_pSharedData = nullptr
  ↓
  g_pSharedMemoryView.reset()
  ↓
  g_hSharedMemory.reset()
  ↓
  g_hFrameReadyEvent.reset()
  ↓
  g_hMemoryMutex.reset()

if (g_pUIManager)
  ↓
  delete g_pUIManager
  ↓
  g_pUIManager = nullptr

if (g_hWnd)  // If projection window still exists
  ↓
  DestroyWindow(g_hWnd)

if (g_hMainWindow)
  ↓
  DestroyWindow(g_hMainWindow)

Log: "Swiper shutting down..."
  ↓
FreeConsole()
  ↓
return 0
```

---

## Key State Transitions

### License Validation State Machine

```
[Unlicensed State]
  - m_isLicenseValid = false
  - Start button: DISABLED
  - Stop button: DISABLED
  - Inject button: DISABLED
  - Status: "No license entered"
    ↓
    [User enters valid license]
    ↓
[Licensed State]
  - m_isLicenseValid = true
  - Start button: ENABLED
  - Stop button: DISABLED
  - Inject button: ENABLED
  - Status: "License Valid!"
```

### Projection State Machine

```
[Licensed + Not Running]
  - m_isProjectionRunning = false
  - m_isLicenseValid = true
  - Start button: ENABLED
  - Stop button: DISABLED
  - Inject button: ENABLED
  - g_hWnd = nullptr
    ↓
    [User clicks Start Projection]
    ↓
[Licensed + Running]
  - m_isProjectionRunning = true
  - m_isLicenseValid = true
  - Start button: DISABLED
  - Stop button: ENABLED
  - Inject button: DISABLED
  - g_hWnd = valid window handle
    ↓
    [User clicks Stop or presses ESC]
    ↓
[Licensed + Not Running]
  (back to initial licensed state)
```

---

## Critical Global Variables

```cpp
// In main.cpp
HWND g_hWnd = nullptr;              // Projection window handle
HWND g_hMainWindow = nullptr;       // Control panel window handle
HINSTANCE g_hInstance = nullptr;    // Application instance
bool g_isRunning = true;            // Main loop flag
bool g_isWindowClassRegistered = false;  // Projection class registered

// In ui.cpp
UIManager* g_pUIManager = nullptr;  // Global UI manager

// UIManager members
bool m_isLicenseValid;              // License validation state
bool m_isProjectionRunning;         // Projection active state
std::vector<MonitorInfo> m_monitors; // Available monitors
UIConfig m_config;                  // FPS, selected monitor
```

---

## Thread Safety Notes

**Current Implementation:**
- Single-threaded UI (all operations on main thread)
- IPC uses mutex for shared memory access
- No race conditions in current design

**Future Considerations:**
- If rendering is moved to separate thread, protect g_hWnd access
- If license validation becomes async (server API), use callbacks
- Monitor enumeration is synchronous, safe as-is

---

## Error Handling

### Window Creation Failure

```cpp
CreateProjectionWindow()
  ↓
  g_hWnd = CreateWindowExW(...)
  ↓
  if (!g_hWnd)
    ↓
    Log: "Failed to create projection window: " + GetLastError()
    ↓
    return false
  ↓
  [Back in OnStartProjection]
  if (!success)
    ↓
    SetWindowTextW(m_hStatusLabel, "Status: Failed to start projection")
    ↓
    m_isProjectionRunning = false  // Stay in stopped state
```

### Invalid Monitor Selection

```cpp
CreateProjectionWindow()
  ↓
  if (config.selectedMonitor >= monitors.size())
    ↓
    Log: "Invalid monitor selection"
    ↓
    return false
```

### License Validation Failure

```cpp
OnValidateLicense()
  ↓
  if (!ValidateLicenseKey(key))
    ↓
    m_isLicenseValid = false
    ↓
    Keep buttons disabled
    ↓
    Show error message
```
