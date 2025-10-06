#include "ui.h"
#include <iostream>
#include <sstream>

// Global UI manager instance
UIManager* g_pUIManager = nullptr;

// Monitor enumeration callback
static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    std::vector<std::wstring>* monitors = (std::vector<std::wstring>*)dwData;
    
    MONITORINFOEXW monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFOEXW);
    
    if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
        std::wstringstream ss;
        ss << L"Monitor " << (monitors->size() + 1) << L": " << monitorInfo.szDevice;
        monitors->push_back(ss.str());
    }
    
    return TRUE;
}

UIManager::UIManager() 
    : m_hTabControl(nullptr)
    , m_hSettingsTab(nullptr)
    , m_hProjectionTab(nullptr)
    , m_hFPSLabel(nullptr)
    , m_hFPSEdit(nullptr)
    , m_hFPSUpDown(nullptr)
    , m_hMonitorLabel(nullptr)
    , m_hMonitorCombo(nullptr)
    , m_hStartButton(nullptr)
    , m_hStopButton(nullptr)
    , m_hInjectButton(nullptr)
    , m_hStatusLabel(nullptr)
    , m_currentTab(0)
    , m_isProjectionRunning(false) {
}

UIManager::~UIManager() {
    Cleanup();
}

bool UIManager::Initialize(HWND hParent, HINSTANCE hInstance) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
    
    // Enumerate monitors
    EnumerateMonitors();
    
    // Create tab control
    if (!CreateTabControl(hParent)) {
        return false;
    }
    
    // Create tabs
    if (!CreateSettingsTab(hParent, hInstance)) {
        return false;
    }
    
    if (!CreateProjectionTab(hParent, hInstance)) {
        return false;
    }
    
    // Show settings tab by default
    OnTabChanged(0);
    
    return true;
}

void UIManager::Cleanup() {
    // Window handles will be destroyed with parent window
}

void UIManager::EnumerateMonitors() {
    m_config.availableMonitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_config.availableMonitors);
    
    if (m_config.availableMonitors.empty()) {
        m_config.availableMonitors.push_back(L"Primary Monitor");
    }
}

bool UIManager::CreateTabControl(HWND hParent) {
    // Create tab control
    m_hTabControl = CreateWindowExW(
        0,
        WC_TABCONTROLW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        10, 10, 380, 280,
        hParent,
        (HMENU)ID_TAB_CONTROL,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!m_hTabControl) {
        return false;
    }
    
    // Add tabs
    TCITEMW tie;
    tie.mask = TCIF_TEXT;
    
    tie.pszText = (LPWSTR)L"Settings";
    TabCtrl_InsertItem(m_hTabControl, 0, &tie);
    
    tie.pszText = (LPWSTR)L"Projection";
    TabCtrl_InsertItem(m_hTabControl, 1, &tie);
    
    return true;
}

bool UIManager::CreateSettingsTab(HWND hParent, HINSTANCE hInstance) {
    // Create settings tab container
    m_hSettingsTab = CreateWindowExW(
        0,
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        20, 40, 360, 240,
        hParent,
        nullptr,
        hInstance,
        nullptr
    );
    
    if (!m_hSettingsTab) {
        return false;
    }
    
    // FPS Label
    m_hFPSLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"FPS (Frames Per Second):",
        WS_CHILD | WS_VISIBLE,
        10, 20, 200, 20,
        m_hSettingsTab,
        nullptr,
        hInstance,
        nullptr
    );
    
    // FPS Edit Control
    m_hFPSEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"60",
        WS_CHILD | WS_VISIBLE | ES_NUMBER,
        10, 45, 100, 25,
        m_hSettingsTab,
        (HMENU)ID_FPS_EDIT,
        hInstance,
        nullptr
    );
    
    // FPS Up-Down Control
    m_hFPSUpDown = CreateWindowExW(
        0,
        UPDOWN_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
        0, 0, 0, 0,
        m_hSettingsTab,
        (HMENU)ID_FPS_UPDOWN,
        hInstance,
        nullptr
    );
    
    SendMessage(m_hFPSUpDown, UDM_SETBUDDY, (WPARAM)m_hFPSEdit, 0);
    SendMessage(m_hFPSUpDown, UDM_SETRANGE, 0, MAKELPARAM(144, 1));
    SendMessage(m_hFPSUpDown, UDM_SETPOS, 0, MAKELPARAM(60, 0));
    
    // Monitor Label
    m_hMonitorLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"Projection Monitor:",
        WS_CHILD | WS_VISIBLE,
        10, 90, 200, 20,
        m_hSettingsTab,
        nullptr,
        hInstance,
        nullptr
    );
    
    // Monitor Combo Box
    m_hMonitorCombo = CreateWindowExW(
        0,
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        10, 115, 300, 200,
        m_hSettingsTab,
        (HMENU)ID_MONITOR_COMBO,
        hInstance,
        nullptr
    );
    
    // Populate monitor combo box
    for (const auto& monitor : m_config.availableMonitors) {
        SendMessageW(m_hMonitorCombo, CB_ADDSTRING, 0, (LPARAM)monitor.c_str());
    }
    SendMessageW(m_hMonitorCombo, CB_SETCURSEL, 0, 0);
    
    return true;
}

bool UIManager::CreateProjectionTab(HWND hParent, HINSTANCE hInstance) {
    // Create projection tab container
    m_hProjectionTab = CreateWindowExW(
        0,
        L"STATIC",
        L"",
        WS_CHILD,
        20, 40, 360, 240,
        hParent,
        nullptr,
        hInstance,
        nullptr
    );
    
    if (!m_hProjectionTab) {
        return false;
    }
    
    // Start Button
    m_hStartButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Start Projection",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 20, 150, 35,
        m_hProjectionTab,
        (HMENU)ID_START_BUTTON,
        hInstance,
        nullptr
    );
    
    // Stop Button
    m_hStopButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Stop Projection",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        10, 65, 150, 35,
        m_hProjectionTab,
        (HMENU)ID_STOP_BUTTON,
        hInstance,
        nullptr
    );
    
    // Inject Button
    m_hInjectButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Inject into Game",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 120, 150, 35,
        m_hProjectionTab,
        (HMENU)ID_INJECT_BUTTON,
        hInstance,
        nullptr
    );
    
    // Status Label
    m_hStatusLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"Status: Ready",
        WS_CHILD | WS_VISIBLE,
        10, 170, 340, 40,
        m_hProjectionTab,
        nullptr,
        hInstance,
        nullptr
    );
    
    return true;
}

void UIManager::OnTabChanged(int newTab) {
    m_currentTab = newTab;
    
    // Hide all tabs
    ShowWindow(m_hSettingsTab, SW_HIDE);
    ShowWindow(m_hProjectionTab, SW_HIDE);
    
    // Show selected tab
    if (newTab == 0) {
        ShowWindow(m_hSettingsTab, SW_SHOW);
    } else if (newTab == 1) {
        ShowWindow(m_hProjectionTab, SW_SHOW);
    }
}

void UIManager::OnCommand(WPARAM wParam, LPARAM lParam) {
    int controlId = LOWORD(wParam);
    int notifyCode = HIWORD(wParam);
    
    switch (controlId) {
        case ID_START_BUTTON:
            OnStartProjection();
            break;
            
        case ID_STOP_BUTTON:
            OnStopProjection();
            break;
            
        case ID_INJECT_BUTTON:
            OnInject();
            break;
            
        case ID_FPS_EDIT:
            if (notifyCode == EN_CHANGE) {
                wchar_t buffer[32];
                GetWindowTextW(m_hFPSEdit, buffer, 32);
                m_config.fps = _wtoi(buffer);
                if (m_config.fps < 1) m_config.fps = 1;
                if (m_config.fps > 144) m_config.fps = 144;
            }
            break;
            
        case ID_MONITOR_COMBO:
            if (notifyCode == CBN_SELCHANGE) {
                m_config.selectedMonitor = (int)SendMessageW(m_hMonitorCombo, CB_GETCURSEL, 0, 0);
            }
            break;
    }
}

void UIManager::OnStartProjection() {
    std::wcout << L"Starting projection at " << m_config.fps << L" FPS on monitor " 
              << m_config.selectedMonitor << std::endl;
    
    m_isProjectionRunning = true;
    UpdateProjectionState(true);
    SetWindowTextW(m_hStatusLabel, L"Status: Projection Running");
}

void UIManager::OnStopProjection() {
    std::wcout << L"Stopping projection" << std::endl;
    
    m_isProjectionRunning = false;
    UpdateProjectionState(false);
    SetWindowTextW(m_hStatusLabel, L"Status: Projection Stopped");
}

void UIManager::OnInject() {
    std::wcout << L"Inject button clicked - Please select a process from console" << std::endl;
    SetWindowTextW(m_hStatusLabel, L"Status: Select process in console to inject");
    
    // Trigger process selection in console
    extern void ProcessSelection();
    ProcessSelection();
}

void UIManager::UpdateProjectionState(bool isRunning) {
    m_isProjectionRunning = isRunning;
    EnableWindow(m_hStartButton, !isRunning);
    EnableWindow(m_hStopButton, isRunning);
}
