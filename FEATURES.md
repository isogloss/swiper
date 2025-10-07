# Swiper Feature Guide

## License Key Activation

The application now requires a valid license key to unlock core functionality.

### How to Activate:

1. Launch the application
2. Navigate to the **License** tab (first tab in the UI)
3. Enter a valid license key in the text field
4. Click **Validate License**

### Valid License Keys (for testing):
- `DEMO-KEY`
- `TEST-LICENSE`
- Any key starting with `SWIPER-` (e.g., `SWIPER-1234-5678-9ABC`)

### What Gets Unlocked:
- **Start Projection** button
- **Inject into Game** button

Until a valid license is entered, these features remain disabled.

---

## On-Demand Projection Window

The projection window is now created on-demand for better resource management.

### Behavior:

**Before Starting:**
- No projection window is created at application startup
- Only the main control panel window is visible

**When Starting Projection:**
- Click **Start Projection** button (after activating license)
- A fullscreen, borderless projection window is created on the selected monitor
- The window covers the entire selected monitor for an immersive experience

**When Stopping Projection:**
- Click **Stop Projection** button
- The projection window is destroyed
- Resources are freed

**ESC Key:**
- Pressing ESC while in the projection window closes it
- This automatically updates the UI to "Projection Stopped" state

---

## Fullscreen Projection

The projection window is now a true fullscreen experience:

### Features:
- **Borderless**: No window borders or title bars
- **Fullscreen**: Covers the entire selected monitor
- **Always on Top**: Projection window stays on top of other windows
- **Monitor Selection**: Choose which monitor to project to in the Settings tab

### How to Use:
1. Go to **Settings** tab
2. Select your preferred monitor from the dropdown
3. Go to **Projection** tab
4. Click **Start Projection**
5. The projection window will appear fullscreen on the selected monitor

---

## Tab Structure

The application now has three tabs:

1. **License**: Enter and validate your license key
2. **Settings**: Configure FPS and select projection monitor
3. **Projection**: Start/stop projection and inject into games

---

## Implementation Details

### License Validation
- Simple pattern-based validation
- In production, this should be replaced with server-side validation or cryptographic signatures
- Current implementation accepts specific test keys for demonstration

### Window Lifecycle
- Projection window class is registered at startup
- Window is created only when needed (lazy initialization)
- Window is properly destroyed when stopped
- Resources are managed efficiently

### Monitor Support
- Full monitor information is enumerated at startup
- Each monitor's position and size are stored
- Projection window is positioned and sized to match selected monitor exactly
