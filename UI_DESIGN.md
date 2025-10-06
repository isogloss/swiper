# Swiper UI Design Specification

## Window Layout

### Main Control Panel Window
- **Title**: "Swiper Control Panel"
- **Size**: 420x360 pixels
- **Style**: Standard Windows window with title bar, minimize, maximize, and close buttons
- **Background**: Default Windows window background color

### Tab Control
- **Position**: 10px from left, 10px from top
- **Size**: 380x280 pixels
- **Tabs**: Two tabs labeled "Settings" and "Projection"

---

## Settings Tab Layout

```
┌─────────────────────────────────────────────────────────────┐
│ [Settings] [Projection]                                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  FPS (Frames Per Second):                                    │
│  ┌──────────┬──┐                                            │
│  │    60    │▲▼│                                            │
│  └──────────┴──┘                                            │
│                                                               │
│                                                               │
│  Projection Monitor:                                         │
│  ┌─────────────────────────────────────────────┐            │
│  │ Monitor 1: \\.\DISPLAY1                    ▼│            │
│  └─────────────────────────────────────────────┘            │
│                                                               │
│                                                               │
│                                                               │
│                                                               │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Settings Tab Controls:
1. **FPS Label**: Static text "FPS (Frames Per Second):"
2. **FPS Edit Box**: Numeric input showing current FPS (default: 60)
3. **FPS Up/Down Control**: Spinner control attached to edit box (range: 1-144)
4. **Monitor Label**: Static text "Projection Monitor:"
5. **Monitor Combo Box**: Dropdown list showing available monitors

---

## Projection Tab Layout

```
┌─────────────────────────────────────────────────────────────┐
│ [Settings] [Projection]                                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────┐                                       │
│  │ Start Projection │                                       │
│  └──────────────────┘                                       │
│                                                               │
│  ┌──────────────────┐                                       │
│  │ Stop Projection  │ (disabled when not running)           │
│  └──────────────────┘                                       │
│                                                               │
│                                                               │
│  ┌──────────────────┐                                       │
│  │ Inject into Game │                                       │
│  └──────────────────┘                                       │
│                                                               │
│                                                               │
│  Status: Ready                                               │
│                                                               │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Projection Tab Controls:
1. **Start Button**: 150x35px button labeled "Start Projection"
   - Enabled when projection is not running
   - Starts the projection with configured FPS and monitor
2. **Stop Button**: 150x35px button labeled "Stop Projection"
   - Disabled when projection is not running
   - Enabled when projection is active
   - Stops the projection
3. **Inject Button**: 150x35px button labeled "Inject into Game"
   - Always enabled
   - Opens process selection in console
4. **Status Label**: Static text showing current status
   - "Status: Ready" - Initial state
   - "Status: Projection Running" - When projection is active
   - "Status: Projection Stopped" - When projection stops
   - "Status: Select process in console to inject" - After inject button clicked

---

## Projection Window

### Design:
- **Title**: "Swiper Projection"
- **Size**: 800x600 pixels (initial size, can be resized)
- **Style**: Borderless, topmost window (WS_POPUP | WS_EX_TOPMOST)
- **Position**: 100px from left, 100px from top (initial position)
- **Background**: Black

### Content:
When no frames are being captured, displays centered text:
```
Swiper Projection Window

Waiting for captured frames...

Press ESC to exit
```

When frames are being captured, the projection window displays the captured game frames.

---

## User Interaction Flow

### Starting a Capture Session:
1. User opens Swiper Control Panel
2. Goes to Settings tab
3. Adjusts FPS (if needed)
4. Selects target monitor (if needed)
5. Goes to Projection tab
6. Clicks "Inject into Game"
7. In console, user sees list of processes (or enters process ID)
8. After injection succeeds, clicks "Start Projection"
9. Projection window shows captured frames

### Stopping a Session:
1. User clicks "Stop Projection" in Projection tab
2. Projection stops
3. User can close windows or start another session

---

## Color Scheme

Using standard Windows colors:
- **Control Panel**: Default Windows window background (typically light gray)
- **Tab Control**: Default tab control styling
- **Buttons**: Standard Windows button styling with hover effects
- **Text**: Black text on white/gray backgrounds
- **Projection Window**: Black background (#000000)
- **Projection Window Text**: White text (#FFFFFF)

---

## Accessibility

- All controls have proper tab order for keyboard navigation
- Buttons can be activated with Enter key when focused
- Tab control can be navigated with arrow keys
- FPS spinner can be adjusted with arrow keys when focused
- ESC key closes the projection window

---

## Technical Notes

- All dimensions are in pixels
- Window positions are relative to screen coordinates
- Controls use standard Windows Common Controls library
- Font: Default system font (typically Segoe UI on Windows 10/11)
- DPI-aware: Will scale appropriately on high-DPI displays
