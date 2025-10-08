#ifndef HWID_H
#define HWID_H

#include <Windows.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace HWID {

/**
 * Generate a Hardware ID (HWID) based on the system's volume serial number.
 * This creates a unique identifier for the machine by querying the C: drive.
 * 
 * @return A hex string representing the HWID (e.g., "A1B2C3D4")
 */
inline std::string Generate() {
    DWORD volumeSerialNumber = 0;
    
    // Get the volume serial number of the C: drive
    if (!GetVolumeInformationW(
        L"C:\\",              // Root path
        nullptr,              // Volume name buffer (not needed)
        0,                    // Volume name buffer size
        &volumeSerialNumber,  // Volume serial number
        nullptr,              // Maximum component length (not needed)
        nullptr,              // File system flags (not needed)
        nullptr,              // File system name buffer (not needed)
        0                     // File system name buffer size
    )) {
        // If we can't get the volume serial number, return an empty string
        return "";
    }
    
    // Convert the serial number to a hex string
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << volumeSerialNumber;
    return ss.str();
}

} // namespace HWID

#endif // HWID_H
