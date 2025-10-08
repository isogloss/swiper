# Swiper Authentication System - Private Tutorial

**This document is intended only for the repository owner (@isogloss)**

## Overview

This document describes the new server-based authentication system that has been integrated into Swiper. The system uses Hardware ID (HWID) locking to prevent unauthorized use and key sharing.

## Architecture

### Client-Side Components (Already Implemented)

The following components have been added to the client application:

1. **hwid.h** - Hardware ID Generation
   - Located in `src/main/hwid.h`
   - Generates a unique identifier based on the system's volume serial number
   - Uses Windows API `GetVolumeInformationW()` to get the C: drive's serial number
   - Returns an 8-character hexadecimal string (e.g., "A1B2C3D4")

2. **auth.h** - Authentication Client
   - Located in `src/main/auth.h`
   - Handles communication with the authentication server
   - Uses the `cpr` library for HTTP POST requests
   - Sends license key and HWID to the server for validation
   - Shows authentication dialog to the user

3. **Integration Points**
   - Authentication is checked at application startup in both `main.cpp` and `imgui_ui.cpp`
   - If authentication fails, the application exits immediately
   - License key is read from the `SWIPER_LICENSE_KEY` environment variable (for simplicity)

### Server-Side Components (You Need to Implement)

You need to set up a server that handles authentication requests. Here's what you need:

#### 1. Authentication Endpoint

Create a REST API endpoint that accepts POST requests:

**Endpoint:** `POST /api/verify`

**Request Format (application/x-www-form-urlencoded):**
```
key=USER_LICENSE_KEY&hwid=HARDWARE_ID
```

**Response Format (JSON):**
```json
{
  "valid": true,
  "message": "Authentication successful"
}
```

Or for invalid keys:
```json
{
  "valid": false,
  "message": "Invalid license key"
}
```

#### 2. Database Schema

You'll need a database to store license keys and their associated HWIDs:

```sql
CREATE TABLE licenses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    license_key VARCHAR(255) UNIQUE NOT NULL,
    hwid VARCHAR(8),
    activated_at DATETIME,
    expires_at DATETIME,
    is_active BOOLEAN DEFAULT TRUE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

#### 3. Server Implementation Example (Node.js/Express)

Here's a basic pseudo-code implementation using Node.js:

```javascript
const express = require('express');
const bodyParser = require('body-parser');
const sqlite3 = require('sqlite3');

const app = express();
const db = new sqlite3.Database('./licenses.db');

app.use(bodyParser.urlencoded({ extended: true }));

app.post('/api/verify', async (req, res) => {
    const { key, hwid } = req.body;
    
    // Validate input
    if (!key || !hwid) {
        return res.status(400).json({
            valid: false,
            message: "Missing key or hwid"
        });
    }
    
    // Check if the license exists and is active
    db.get('SELECT * FROM licenses WHERE license_key = ? AND is_active = 1', 
           [key], (err, license) => {
        if (err) {
            return res.status(500).json({
                valid: false,
                message: "Server error"
            });
        }
        
        if (!license) {
            return res.status(200).json({
                valid: false,
                message: "Invalid license key"
            });
        }
        
        // Check if license has expired
        if (license.expires_at && new Date(license.expires_at) < new Date()) {
            return res.status(200).json({
                valid: false,
                message: "License expired"
            });
        }
        
        // If no HWID is set, bind this license to the current HWID
        if (!license.hwid) {
            db.run('UPDATE licenses SET hwid = ?, activated_at = CURRENT_TIMESTAMP WHERE id = ?',
                   [hwid, license.id], (err) => {
                if (err) {
                    return res.status(500).json({
                        valid: false,
                        message: "Failed to activate license"
                    });
                }
                
                return res.status(200).json({
                    valid: true,
                    message: "License activated successfully"
                });
            });
        }
        // If HWID is already set, verify it matches
        else if (license.hwid === hwid) {
            return res.status(200).json({
                valid: true,
                message: "Authentication successful"
            });
        }
        // HWID mismatch - license is locked to a different machine
        else {
            return res.status(200).json({
                valid: false,
                message: "License is locked to another machine"
            });
        }
    });
});

app.listen(3000, () => {
    console.log('Authentication server running on port 3000');
});
```

#### 4. Python/Flask Alternative

If you prefer Python, here's a Flask example:

```python
from flask import Flask, request, jsonify
import sqlite3
from datetime import datetime

app = Flask(__name__)

@app.route('/api/verify', methods=['POST'])
def verify():
    key = request.form.get('key')
    hwid = request.form.get('hwid')
    
    if not key or not hwid:
        return jsonify({'valid': False, 'message': 'Missing parameters'}), 400
    
    conn = sqlite3.connect('licenses.db')
    cursor = conn.cursor()
    
    # Get license
    cursor.execute('SELECT * FROM licenses WHERE license_key = ? AND is_active = 1', (key,))
    license = cursor.fetchone()
    
    if not license:
        conn.close()
        return jsonify({'valid': False, 'message': 'Invalid license key'})
    
    license_id, license_key, stored_hwid, activated_at, expires_at, is_active, created_at = license
    
    # Check expiration
    if expires_at and datetime.fromisoformat(expires_at) < datetime.now():
        conn.close()
        return jsonify({'valid': False, 'message': 'License expired'})
    
    # Bind to HWID if not yet bound
    if not stored_hwid:
        cursor.execute('UPDATE licenses SET hwid = ?, activated_at = ? WHERE id = ?',
                      (hwid, datetime.now().isoformat(), license_id))
        conn.commit()
        conn.close()
        return jsonify({'valid': True, 'message': 'License activated'})
    
    # Verify HWID matches
    if stored_hwid == hwid:
        conn.close()
        return jsonify({'valid': True, 'message': 'Authentication successful'})
    else:
        conn.close()
        return jsonify({'valid': False, 'message': 'HWID mismatch'})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3000)
```

## Deployment

### Option 1: Cloud Hosting (Recommended)

Deploy your authentication server to a cloud provider:
- **Heroku**: Simple deployment with add-on database support
- **AWS Lambda + API Gateway**: Serverless option
- **DigitalOcean**: VPS for full control
- **Google Cloud Run**: Containerized deployment

### Option 2: VPS Hosting

1. Rent a VPS (e.g., from DigitalOcean, Linode, or Vultr)
2. Install Node.js or Python
3. Set up a reverse proxy (nginx) with SSL
4. Install and configure your database (SQLite, PostgreSQL, or MySQL)
5. Deploy your authentication server
6. Use Let's Encrypt for free SSL certificates

### Security Considerations

1. **Use HTTPS**: Always use SSL/TLS for the authentication endpoint
2. **Rate Limiting**: Implement rate limiting to prevent brute force attacks
3. **Logging**: Log all authentication attempts for security auditing
4. **Key Generation**: Use cryptographically secure random generation for license keys
5. **Database Security**: Never expose your database directly to the internet

## Managing License Keys

### Creating New License Keys

Create a simple admin script to generate license keys:

```python
import secrets
import sqlite3
from datetime import datetime, timedelta

def generate_license_key():
    """Generate a secure random license key"""
    return '-'.join([secrets.token_hex(4).upper() for _ in range(4)])

def create_license(duration_days=365):
    """Create a new license that expires in duration_days"""
    key = generate_license_key()
    expires_at = datetime.now() + timedelta(days=duration_days)
    
    conn = sqlite3.connect('licenses.db')
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO licenses (license_key, expires_at) 
        VALUES (?, ?)
    ''', (key, expires_at.isoformat()))
    conn.commit()
    conn.close()
    
    print(f"Created license: {key}")
    print(f"Expires: {expires_at}")
    return key

# Create a license
if __name__ == '__main__':
    create_license(365)  # 1 year license
```

### Resetting HWID (for users switching computers)

Create an admin script to reset a user's HWID:

```python
import sqlite3

def reset_hwid(license_key):
    """Reset the HWID for a license, allowing it to be activated on a new machine"""
    conn = sqlite3.connect('licenses.db')
    cursor = conn.cursor()
    cursor.execute('UPDATE licenses SET hwid = NULL, activated_at = NULL WHERE license_key = ?',
                  (license_key,))
    conn.commit()
    
    if cursor.rowcount > 0:
        print(f"HWID reset for license: {license_key}")
    else:
        print(f"License not found: {license_key}")
    
    conn.close()

# Usage
if __name__ == '__main__':
    reset_hwid('XXXX-XXXX-XXXX-XXXX')
```

## Configuring the Client

### Update Server URL

In `src/main/auth.h`, update the `AUTH_SERVER_URL` constant with your actual server URL:

```cpp
const std::string AUTH_SERVER_URL = "https://your-actual-domain.com/api/verify";
```

### User Instructions

For end users, they need to:
1. Set the environment variable `SWIPER_LICENSE_KEY` with their license key
2. Run the application

**Windows (PowerShell):**
```powershell
$env:SWIPER_LICENSE_KEY = "XXXX-XXXX-XXXX-XXXX"
.\swiper.exe
```

**Windows (Command Prompt):**
```cmd
set SWIPER_LICENSE_KEY=XXXX-XXXX-XXXX-XXXX
swiper.exe
```

## Installing Dependencies

### Installing cpr Library (Client Dependency)

The client uses the `cpr` library for HTTP requests. Install it using vcpkg:

**Windows:**
```cmd
# Install vcpkg if you haven't already
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install cpr
.\vcpkg install cpr:x64-windows

# Integrate with your build system
.\vcpkg integrate install
```

**When building with CMake:**
```cmd
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

If cpr is not found, the authentication features will be disabled (the build will still succeed, but without authentication).

## Testing the System

### Local Testing

For local development testing:

1. Run your authentication server locally (e.g., `http://localhost:3000/api/verify`)
2. Update `AUTH_SERVER_URL` in `auth.h` to point to `http://localhost:3000/api/verify`
3. Create a test license in your database
4. Set the `SWIPER_LICENSE_KEY` environment variable
5. Run the Swiper application

### Production Testing

1. Deploy your authentication server to a secure HTTPS endpoint
2. Update `AUTH_SERVER_URL` in `auth.h` to your production URL
3. Rebuild the application
4. Test with valid and invalid license keys
5. Test HWID locking by trying to use the same key on different machines

## Advanced Features (Future Enhancements)

Consider implementing these features:

1. **License Management Portal**: Web interface for users to manage their licenses
2. **Multiple Activations**: Allow a license to be used on N machines simultaneously
3. **Subscription Model**: Implement recurring billing with automatic expiration
4. **Trial Keys**: Temporary licenses that expire after X days
5. **Feature Flags**: Enable/disable specific features based on license tier
6. **Offline Grace Period**: Allow the app to work offline for a limited time
7. **Custom Dialog**: Replace the simple MessageBox with a proper license key input dialog
8. **Webhook Notifications**: Get notified when licenses are activated or expire

## Troubleshooting

### Common Issues

1. **"Failed to connect to authentication server"**
   - Check that your server is running and accessible
   - Verify firewall settings
   - Ensure HTTPS certificate is valid

2. **"HWID mismatch"**
   - User is trying to use a license on a different machine
   - Reset the HWID using the admin script

3. **Build fails with cpr not found**
   - Install cpr using vcpkg
   - Set the CMAKE_TOOLCHAIN_FILE properly
   - Or build without authentication (it will still compile)

## Security Notes

**Important**: 
- Keep your database credentials secure
- Never commit your server code with production credentials to a public repository
- Use environment variables for sensitive configuration
- Implement IP-based rate limiting to prevent abuse
- Monitor your authentication logs for suspicious activity
- Consider implementing 2FA for administrative functions

## Conclusion

This authentication system provides a solid foundation for protecting your application. The HWID locking prevents casual key sharing, and the server-based validation ensures you have full control over license management.

For questions or issues, please review this document carefully or consult the code comments in `auth.h` and `hwid.h`.
