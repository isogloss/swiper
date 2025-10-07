#ifndef UI_H
#define UI_H

#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <vector>

// UI Configuration
struct UIConfig {
    int fps = 60;
    int selectedMonitor = 0;
    std::vector<std::wstring> availableMonitors;
};

// Monitor information structure
struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rect;
    std::wstring name;
};

// UI Manager class
class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialize UI components
    bool Initialize(HWND hParent, HINSTANCE hInstance);
    
    // Cleanup UI resources
    void Cleanup();
    
    // Get current configuration
    const UIConfig& GetConfig() const { return m_config; }
    
    // UI event handlers
    void OnStartProjection();
    void OnStopProjection();
    void OnInject();
    
    // Update UI state
    void UpdateProjectionState(bool isRunning);
    
    // Get window handles
    HWND GetTabControl() const { return m_hTabControl; }
    HWND GetSettingsTab() const { return m_hSettingsTab; }
    HWND GetProjectionTab() const { return m_hProjectionTab; }
    HWND GetLicenseTab() const { return m_hLicenseTab; }
    
    // License validation
    bool IsLicenseValid() const { return m_isLicenseValid; }
    
    // Get monitor information
    const std::vector<MonitorInfo>& GetMonitors() const { return m_monitors; }
    
    // Handle tab changes
    void OnTabChanged(int newTab);
    
    // Handle control notifications
    void OnCommand(WPARAM wParam, LPARAM lParam);
    
private:
    // Create UI elements
    bool CreateTabControl(HWND hParent);
    bool CreateSettingsTab(HWND hParent, HINSTANCE hInstance);
    bool CreateProjectionTab(HWND hParent, HINSTANCE hInstance);
    bool CreateLicenseTab(HWND hParent, HINSTANCE hInstance);
    
    // Enumerate monitors
    void EnumerateMonitors();
    
    // License validation
    void OnValidateLicense();
    bool ValidateLicenseKey(const std::wstring& key);
    
    // Window handles
    HWND m_hTabControl;
    HWND m_hSettingsTab;
    HWND m_hProjectionTab;
    HWND m_hLicenseTab;
    
    // Settings tab controls
    HWND m_hFPSLabel;
    HWND m_hFPSEdit;
    HWND m_hFPSUpDown;
    HWND m_hMonitorLabel;
    HWND m_hMonitorCombo;
    
    // Projection tab controls
    HWND m_hStartButton;
    HWND m_hStopButton;
    HWND m_hInjectButton;
    HWND m_hStatusLabel;
    
    // License tab controls
    HWND m_hLicenseKeyLabel;
    HWND m_hLicenseKeyEdit;
    HWND m_hValidateButton;
    HWND m_hLicenseStatusLabel;
    
    // Configuration
    UIConfig m_config;
    int m_currentTab;
    bool m_isProjectionRunning;
    bool m_isLicenseValid;
    
    // Monitor information
    std::vector<MonitorInfo> m_monitors;
    
    // Control IDs
    static const int ID_TAB_CONTROL = 1001;
    static const int ID_FPS_EDIT = 1002;
    static const int ID_FPS_UPDOWN = 1003;
    static const int ID_MONITOR_COMBO = 1004;
    static const int ID_START_BUTTON = 1005;
    static const int ID_STOP_BUTTON = 1006;
    static const int ID_INJECT_BUTTON = 1007;
    static const int ID_LICENSE_KEY_EDIT = 1008;
    static const int ID_VALIDATE_BUTTON = 1009;
    
    friend class UIManagerHelper;
};

// Global UI manager instance
extern UIManager* g_pUIManager;

#endif // UI_H
