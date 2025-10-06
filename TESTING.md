# Testing Plan for Swiper UI Implementation

## Overview
This document outlines the testing procedures for the new tabbed UI implementation in the Swiper application.

## Environment Requirements

### Build Environment:
- Windows 10 or Windows 11
- Visual Studio 2019 or 2022 with C++ Desktop Development workload
- CMake 3.15 or higher
- Windows SDK (included with Visual Studio)

### Test Environment:
- Windows 10 or Windows 11 (x64)
- At least one additional monitor for multi-monitor testing (optional)
- A DirectX application for injection testing (optional)

## Build Verification

### 1. CMake Configuration
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

**Expected Result:**
- CMake generates Visual Studio solution successfully
- No configuration errors
- All required libraries are found (kernel32, user32, gdi32, comctl32)

### 2. Compilation
```bash
cmake --build . --config Release
```

**Expected Result:**
- Both swiper.exe and capture.dll compile without errors
- No warnings for the new UI code
- Output files in build/bin/Release/ and build/lib/Release/

## UI Functional Testing

### 3. Application Launch
**Test:** Launch swiper.exe

**Expected Results:**
- Two windows appear:
  1. "Swiper Control Panel" (420x360px)
  2. "Swiper Projection" (800x600px borderless)
- Console window opens with debug output
- No crash or error messages

### 4. Settings Tab - Initial State
**Test:** Verify Settings tab defaults

**Expected Results:**
- Settings tab is selected by default
- FPS field shows "60"
- FPS spinner controls are functional
- Monitor dropdown is populated with available monitors
- First monitor is selected by default

### 5. Settings Tab - FPS Configuration
**Test Cases:**

a) **Manual Input:**
- Type "30" in FPS field
- Expected: Value changes to 30

b) **Spinner Up:**
- Click up arrow on spinner multiple times
- Expected: Value increases (max 144)

c) **Spinner Down:**
- Click down arrow on spinner multiple times  
- Expected: Value decreases (min 1)

d) **Boundary Testing:**
- Try to set FPS to 0 or negative
- Expected: Value clamps to 1
- Try to set FPS to 200
- Expected: Value clamps to 144

e) **Invalid Input:**
- Type non-numeric characters
- Expected: Only numbers are accepted

### 6. Settings Tab - Monitor Selection
**Test Cases:**

a) **Single Monitor:**
- Check dropdown shows at least "Monitor 1"
- Expected: Shows available monitor(s)

b) **Multiple Monitors (if available):**
- Select different monitors from dropdown
- Expected: Selection changes, no crashes

c) **Monitor Enumeration:**
- Console should show monitor information
- Expected: Correct monitor count and details

### 7. Projection Tab - Navigation
**Test:** Switch to Projection tab

**Expected Results:**
- Projection tab content appears
- Settings tab content is hidden
- Three buttons are visible: Start, Stop, Inject
- Status label shows "Status: Ready"
- Stop button is disabled (grayed out)

### 8. Projection Tab - Button States
**Test Cases:**

a) **Initial State:**
- Start button: Enabled
- Stop button: Disabled
- Inject button: Enabled
- Status: "Status: Ready"

b) **After Start Projection:**
- Click Start button
- Expected:
  - Start button: Disabled
  - Stop button: Enabled
  - Status: "Status: Projection Running"
  - Console shows: "Starting projection at X FPS on monitor Y"

c) **After Stop Projection:**
- Click Stop button
- Expected:
  - Start button: Enabled
  - Stop button: Disabled
  - Status: "Status: Projection Stopped"
  - Console shows: "Stopping projection"

### 9. Injection Workflow
**Test:** Click Inject button

**Expected Results:**
- Console shows "Inject button clicked - Please select a process from console"
- Status changes to "Status: Select process in console to inject"
- Process list appears in console (up to 20 processes)
- Total process count displayed

### 10. Tab Switching
**Test:** Switch between tabs multiple times

**Expected Results:**
- Settings content shows/hides correctly
- Projection content shows/hides correctly
- No visual glitches or overlapping controls
- Switching tabs doesn't affect button states
- Configuration values persist between switches

## Window Management Testing

### 11. Main Window
**Test Cases:**

a) **Resize:**
- Try to resize the Control Panel window
- Expected: Window can be resized normally

b) **Minimize/Maximize:**
- Minimize and restore window
- Expected: Window state changes correctly, UI remains functional

c) **Close:**
- Close Control Panel window
- Expected: Application exits gracefully, both windows close

### 12. Projection Window
**Test Cases:**

a) **Position:**
- Drag projection window to different positions
- Expected: Window can be moved freely

b) **ESC Key:**
- Press ESC while projection window is focused
- Expected: Application exits

c) **Display:**
- Verify black background with white text
- Text should read: "Swiper Projection Window / Waiting for captured frames... / Press ESC to exit"

## Integration Testing

### 13. Settings → Projection Flow
**Test Sequence:**
1. Set FPS to 30
2. Select a specific monitor
3. Switch to Projection tab
4. Click Start Projection
5. Check console output

**Expected:**
- Console shows: "Starting projection at 30 FPS on monitor [selected]"
- Configuration is correctly applied

### 14. Console Interaction
**Test:** Type commands in console while UI is open

**Expected Results:**
- Console input still works
- Key commands ('1', '2', 'ESC') still functional
- No interference between console and UI

### 15. Concurrent Operation
**Test:** Open multiple windows and interact

**Expected Results:**
- Both windows respond to input
- Tab changes don't affect projection window
- Projection continues while changing settings (after started)

## Error Handling Testing

### 16. Invalid Operations
**Test Cases:**

a) **Start Without Injection:**
- Click Start without injecting into a process
- Expected: Projection starts (waiting for frames)

b) **Multiple Start Clicks:**
- Click Start multiple times rapidly
- Expected: Button disables, only one start occurs

c) **Multiple Inject Clicks:**
- Click Inject multiple times
- Expected: Each click triggers process list

## Performance Testing

### 17. UI Responsiveness
**Test:** Interact with UI rapidly

**Expected Results:**
- Controls respond immediately to clicks
- Tab switching is instantaneous
- No lag or freezing
- FPS spinner updates smoothly

### 18. Memory Usage
**Test:** Run application for extended period

**Expected Results:**
- No memory leaks
- Stable memory usage
- UI remains responsive

## Accessibility Testing

### 19. Keyboard Navigation
**Test Cases:**

a) **Tab Key:**
- Use Tab to navigate between controls
- Expected: Focus moves through all interactive elements

b) **Enter Key:**
- Focus on buttons and press Enter
- Expected: Button activates

c) **Arrow Keys:**
- Navigate tabs with arrow keys (if focused on tab control)
- Expected: Tabs switch

d) **Arrow Keys on Spinner:**
- Focus FPS edit, use Up/Down arrows
- Expected: Value changes

### 20. Screen Reader Compatibility
**Test:** Use Windows Narrator

**Expected Results:**
- Controls are properly labeled
- Button states are announced
- Tab names are readable

## GitHub Actions Workflow Testing

### 21. Release Build Workflow
**Test Procedure:**
1. Create a git tag (e.g., v1.0.0-test)
2. Create a GitHub release from the tag
3. Monitor workflow execution

**Expected Results:**
- Workflow triggers automatically
- Windows runner starts
- CMake configuration succeeds
- Build completes without errors
- swiper.exe and capture.dll are uploaded as release assets

## Regression Testing

### 22. Existing Functionality
**Verify these features still work:**

- [ ] IPC initialization
- [ ] Shared memory creation
- [ ] Process enumeration (console command '1')
- [ ] DLL injection (console command '2')
- [ ] Projection window rendering
- [ ] Frame capture (when DLL is injected)

## Known Limitations

1. **Platform:** Windows-only, cannot test on Linux/macOS
2. **UI Testing:** No automated UI tests, manual testing required
3. **Frame Capture:** Requires actual DirectX application for full end-to-end test
4. **Multi-Monitor:** Full testing requires multiple physical or virtual monitors

## Bug Reporting Template

When reporting issues, include:
- Windows version
- Visual Studio version
- Steps to reproduce
- Expected vs actual behavior
- Console output (if applicable)
- Screenshots of UI state

## Test Sign-Off

| Test Category | Status | Tester | Date | Notes |
|---------------|--------|--------|------|-------|
| Build Verification | ⬜ Pending | | | |
| UI Functional | ⬜ Pending | | | |
| Window Management | ⬜ Pending | | | |
| Integration | ⬜ Pending | | | |
| Error Handling | ⬜ Pending | | | |
| Performance | ⬜ Pending | | | |
| Accessibility | ⬜ Pending | | | |
| GitHub Actions | ⬜ Pending | | | |
| Regression | ⬜ Pending | | | |

---

**Note:** This test plan should be executed by a tester with access to a Windows development environment. Since the code was developed on Linux, it has not been executed but follows Win32 API best practices and standard patterns.
