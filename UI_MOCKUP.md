# UI Layout Mockup

## Main Control Panel Window

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌─────────┬─────────┬─────────┐                │
│  │ License │ Settings│Projection│  <-- Tabs    │
│  └─────────┴─────────┴─────────┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  Tab Content Area                         │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

---

## License Tab (Index 0) - Default Tab

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌═════════┬─────────┬─────────┐                │
│  │ License │ Settings│Projection│                │
│  └═════════┴─────────┴─────────┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  Enter License Key:                       │  │
│  │  ┌────────────────────────────────────┐  │  │
│  │  │                                    │  │  │
│  │  └────────────────────────────────────┘  │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Validate License │                    │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  Status: No license entered               │  │
│  │                                           │  │
│  │  Please enter a valid license key to     │  │
│  │  unlock projection and injection features.│  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**State: Not Validated**
- License key input field is empty
- Validate License button is enabled
- Status shows: "No license entered"
- Start Projection button (in Projection tab) is DISABLED
- Inject button (in Projection tab) is DISABLED

---

## License Tab - After Validation (Valid Key)

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌═════════┬─────────┬─────────┐                │
│  │ License │ Settings│Projection│                │
│  └═════════┴─────────┴─────────┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  Enter License Key:                       │  │
│  │  ┌────────────────────────────────────┐  │  │
│  │  │ DEMO-KEY                           │  │  │
│  │  └────────────────────────────────────┘  │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Validate License │                    │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  Status: License Valid!                   │  │
│  │                                           │  │
│  │  All features unlocked.                   │  │
│  │  You can now use projection and injection.│  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**State: Validated**
- License key "DEMO-KEY" entered and validated
- Status shows: "License Valid!"
- Start Projection button is now ENABLED
- Inject button is now ENABLED

---

## Settings Tab (Index 1)

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌─────────┬═════════┬─────────┐                │
│  │ License │ Settings│Projection│                │
│  └─────────┴═════════┴─────────┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  FPS (Frames Per Second):                 │  │
│  │  ┌──────┐ ▲                               │  │
│  │  │  60  │ ▼                               │  │
│  │  └──────┘                                 │  │
│  │                                           │  │
│  │                                           │  │
│  │  Projection Monitor:                      │  │
│  │  ┌────────────────────────────────┐ ▼    │  │
│  │  │ Monitor 1: \\.\DISPLAY1        │      │  │
│  │  └────────────────────────────────┘      │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**Controls:**
- FPS spinner (1-144 range)
- Monitor dropdown (lists all available monitors)

---

## Projection Tab (Index 2) - Before Starting

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌─────────┬─────────┬═════════┐                │
│  │ License │ Settings│Projection│                │
│  └─────────┴─────────┴═════════┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Start Projection │  ✓ ENABLED         │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Stop Projection  │  ✗ DISABLED        │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Inject into Game │  ✓ ENABLED         │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  Status: Ready                            │  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**State: Ready (Licensed, Not Running)**
- Start Projection: ENABLED
- Stop Projection: DISABLED (grayed out)
- Inject into Game: ENABLED
- Status: "Ready"

---

## Projection Tab - While Running

```
┌──────────────────────────────────────────────────┐
│  Swiper Control Panel                        [_][□][×]│
├──────────────────────────────────────────────────┤
│  ┌─────────┬─────────┬═════════┐                │
│  │ License │ Settings│Projection│                │
│  └─────────┴─────────┴═════════┘                │
│  ┌───────────────────────────────────────────┐  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Start Projection │  ✗ DISABLED        │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Stop Projection  │  ✓ ENABLED         │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │                                           │  │
│  │                                           │  │
│  │  ┌──────────────────┐                    │  │
│  │  │ Inject into Game │  ✗ DISABLED        │  │
│  │  └──────────────────┘                    │  │
│  │                                           │  │
│  │  Status: Projection Running               │  │
│  │                                           │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**State: Projection Running**
- Start Projection: DISABLED (grayed out)
- Stop Projection: ENABLED
- Inject into Game: DISABLED (can't inject while projecting)
- Status: "Projection Running"

---

## Projection Window (Fullscreen on Selected Monitor)

### Example: Monitor 2 (1920x1080)

```
╔════════════════════════════════════════════════════════════════╗
║                                                                ║
║                     Swiper Projection Window                   ║
║                                                                ║
║                  Waiting for captured frames...                ║
║                                                                ║
║                       Press ESC to exit                        ║
║                                                                ║
║                                                                ║
║          (This window covers the ENTIRE monitor)               ║
║                                                                ║
║                                                                ║
║                                                                ║
║  Position: (1920, 0)                                          ║
║  Size: 1920x1080                                              ║
║  Style: Borderless (WS_POPUP)                                 ║
║  TopMost: Yes (WS_EX_TOPMOST)                                 ║
║                                                                ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
```

**Characteristics:**
- No title bar
- No borders
- No system buttons (minimize/maximize/close)
- Covers entire monitor (fullscreen)
- Always on top
- Black background
- White text centered
- Can be closed with ESC key

---

## User Workflow

### First Time Setup:
1. Launch application → **License tab** appears first
2. Enter license key (e.g., "DEMO-KEY")
3. Click "Validate License"
4. See "License Valid!" message
5. Go to **Settings tab**
6. Select desired projection monitor
7. Adjust FPS if needed
8. Go to **Projection tab**
9. Click "Start Projection"
10. Fullscreen projection window appears on selected monitor

### Regular Use:
1. Launch application
2. Enter license key (if not remembered)
3. Click "Start Projection"
4. Use the projection
5. Click "Stop Projection" when done

### Quick Exit:
- Press **ESC** in projection window → Closes projection, keeps control panel open
- Close control panel → Exits entire application

---

## Button State Matrix

| License Valid | Projection Running | Start Button | Stop Button | Inject Button |
|--------------|-------------------|--------------|-------------|---------------|
| ❌ No        | -                 | ❌ Disabled  | ❌ Disabled | ❌ Disabled   |
| ✅ Yes       | ❌ No             | ✅ Enabled   | ❌ Disabled | ✅ Enabled    |
| ✅ Yes       | ✅ Yes            | ❌ Disabled  | ✅ Enabled  | ❌ Disabled   |

---

## Color Scheme (Windows Default)

- **Background**: Light gray (standard Windows window color)
- **Text**: Black on light gray
- **Buttons**: 3D raised style with gray background
- **Disabled buttons**: Gray text
- **Input fields**: White background with black text
- **Projection window**: Black background, white text
- **Status labels**: Black text, updates based on state

---

## Window Sizes

### Main Control Panel:
- Width: 420 pixels
- Height: 360 pixels
- Position: Default (centered by Windows)
- Style: Standard overlapped window with title bar and system buttons

### Projection Window:
- Width: **Full monitor width** (e.g., 1920px)
- Height: **Full monitor height** (e.g., 1080px)
- Position: **Monitor top-left corner** (e.g., x=1920, y=0 for second monitor)
- Style: Borderless popup (WS_POPUP | WS_EX_TOPMOST)
