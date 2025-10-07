#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <Windows.h>
#include <cpr/cpr.h>
#include "hwid.h"

namespace Auth {

// Server configuration
// NOTE: Repository owner should replace this with their actual authentication server URL
const std::string AUTH_SERVER_URL = "https://your-auth-server.com/api/verify";

/**
 * Structure to hold authentication result
 */
struct AuthResult {
    bool success;
    std::string message;
};

/**
 * Authenticate the user with the server by sending the license key and HWID.
 * 
 * @param licenseKey The license key entered by the user
 * @return AuthResult containing success status and message
 */
inline AuthResult Authenticate(const std::string& licenseKey) {
    AuthResult result = { false, "" };
    
    // Generate the Hardware ID
    std::string hwid = HWID::Generate();
    if (hwid.empty()) {
        result.message = "Failed to generate Hardware ID";
        return result;
    }
    
    try {
        // Send POST request to the authentication server
        auto response = cpr::Post(
            cpr::Url{AUTH_SERVER_URL},
            cpr::Payload{
                {"key", licenseKey},
                {"hwid", hwid}
            },
            cpr::Timeout{5000}  // 5 second timeout
        );
        
        // Check if the request was successful
        if (response.status_code == 200) {
            // Check the response body for authentication status
            // Expected format: {"valid": true/false, "message": "..."}
            std::string responseBody = response.text;
            
            // Simple parsing - check if response contains "valid":true
            if (responseBody.find("\"valid\":true") != std::string::npos ||
                responseBody.find("\"valid\": true") != std::string::npos) {
                result.success = true;
                result.message = "Authentication successful";
            } else {
                result.success = false;
                result.message = "Invalid license key or HWID mismatch";
            }
        } else if (response.status_code == 0) {
            result.message = "Failed to connect to authentication server";
        } else {
            result.message = "Authentication server error (HTTP " + std::to_string(response.status_code) + ")";
        }
    } catch (const std::exception& e) {
        result.message = std::string("Authentication error: ") + e.what();
    }
    
    return result;
}

/**
 * Show a simple authentication dialog to the user and verify their license key.
 * This is the main entry point for authentication that should be called at startup.
 * 
 * @return true if authentication succeeded, false otherwise
 */
inline bool ShowAuthenticationDialog() {
    // Create a simple input dialog for the license key
    char licenseKey[256] = {0};
    
    // Use a Windows message box with input (we'll use a simple approach)
    // In a production system, you might want a custom dialog
    
    // For demonstration, we'll use a simple MessageBox approach
    // The user would need to enter their key via a text input dialog
    // Here we'll simulate with a hardcoded test
    
    // In a real implementation, you would:
    // 1. Show a custom dialog with a text input field
    // 2. Get the license key from the user
    // 3. Call Authenticate() with that key
    
    // For now, we'll show a message box asking for the key
    // This is a placeholder - a real implementation would use a proper input dialog
    
    MessageBoxA(
        nullptr,
        "This application requires a valid license key.\n\n"
        "Please enter your license key in the next dialog.",
        "Authentication Required",
        MB_OK | MB_ICONINFORMATION
    );
    
    // Note: A proper implementation would show a dialog box with a text input
    // For this implementation, we'll create a simple approach using environment variable
    // or a configuration file. Here's a basic version:
    
    // Try to read license key from environment variable (for testing)
    const char* envKey = std::getenv("SWIPER_LICENSE_KEY");
    if (envKey != nullptr && strlen(envKey) > 0) {
        strcpy_s(licenseKey, sizeof(licenseKey), envKey);
    } else {
        // No key provided - authentication fails
        MessageBoxA(
            nullptr,
            "No license key provided.\n\n"
            "Please set the SWIPER_LICENSE_KEY environment variable or contact support.",
            "Authentication Failed",
            MB_OK | MB_ICONERROR
        );
        return false;
    }
    
    // Authenticate with the server
    AuthResult result = Authenticate(licenseKey);
    
    if (result.success) {
        return true;
    } else {
        // Show error message
        std::string errorMsg = "Authentication failed:\n\n" + result.message;
        MessageBoxA(
            nullptr,
            errorMsg.c_str(),
            "Authentication Failed",
            MB_OK | MB_ICONERROR
        );
        return false;
    }
}

} // namespace Auth

#endif // AUTH_H
