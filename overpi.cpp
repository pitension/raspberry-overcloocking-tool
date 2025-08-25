#include <gtkmm.h>
#include <glibmm.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <array>
#include <memory>
#include <cstdio>
#include <libintl.h>
#include <locale.h>

#define _(string) gettext(string)

// Structure to store profile configuration
struct ProfileConfig {
    std::string arm_freq;
    std::string gpu_freq;
    std::string over_voltage;
    std::string force_turbo;
    std::string temp_limit;
    std::string desc;
    std::string color;
};

class PiOverclockApp : public Gtk::Window {
public:
    PiOverclockApp();
    virtual ~PiOverclockApp();

protected:
    // Interface controls
    Gtk::Box m_mainBox;
    Gtk::Box m_controlBox;
    Gtk::Label m_profileLabel;
    Gtk::ComboBoxText m_profileCombo;
    Gtk::Button m_applyHotBtn;
    Gtk::Button m_applyPermBtn;
    Gtk::Button m_infoBtn;
    
    Gtk::Frame m_metricsFrame;
    Gtk::Box m_metricsBox;
    Gtk::Label m_cpuTempLabel;
    Gtk::Label m_cpuFreqLabel;
    Gtk::Label m_gpuFreqLabel;
    Gtk::Label m_cpuGovLabel;
    Gtk::Label m_throttlingStatusLabel;
    Gtk::Label m_throttlingDetailsLabel;
    Gtk::Label m_statusLabel;
    
    Gtk::Frame m_descFrame;
    Gtk::Box m_descBox;
    Gtk::Label m_descLabel;
    
    Gtk::Frame m_warningFrame;
    Gtk::Label m_warningLabel;
    
    // State variables
    std::map<std::string, ProfileConfig> m_profiles;
    std::string m_currentProfile;
    
    // Thread for system information updates
    std::thread m_updateThread;
    bool m_threadRunning;
    
    // Methods
    void loadProfiles();
    void onProfileChanged();
    void onApplyHotClicked();
    void onApplyPermClicked();
    void onInfoClicked();
    void updateSystemInfo();
    void applyHotProfile(const std::string& profileName, bool showMessage = true);
    void applyPermanentProfile(const std::string& profileName);
    std::string getThrottlingInfo();
    std::string decodeThrottling(const std::string& throttledHex);
    std::string execCommand(const std::string& cmd);
    bool fileExists(const std::string& filename);
    void showMessageDialog(const std::string& title, const std::string& message, Gtk::MessageType type);
    bool showQuestionDialog(const std::string& title, const std::string& message);
};

PiOverclockApp::PiOverclockApp() 
: m_mainBox(Gtk::ORIENTATION_VERTICAL, 5),
  m_controlBox(Gtk::ORIENTATION_HORIZONTAL, 5),
  m_profileLabel(_("Profile:")),
  m_metricsBox(Gtk::ORIENTATION_VERTICAL, 5),
  m_descBox(Gtk::ORIENTATION_VERTICAL, 5),
  m_threadRunning(true) {
    
    // Configure main window
    set_title(_("RPi 400 Overclock Control - Hot Mode"));
    set_default_size(900, 600);
    set_border_width(10);
    
    // Load profiles
    loadProfiles();
    
    // Configure controls
    m_profileCombo.append(_("Minimum"));
    m_profileCombo.append(_("Normal"));
    m_profileCombo.append(_("Moderate"));
    m_profileCombo.append(_("High"));
    m_profileCombo.append(_("Extreme"));
    m_profileCombo.set_active_text(_("Normal"));
    m_currentProfile = "Normal";
    
    m_applyHotBtn.set_label(_("Apply Hot"));
    m_applyPermBtn.set_label(_("Apply Permanently"));
    m_infoBtn.set_label(_("System Info"));
    
    m_metricsFrame.set_label(_("System Status"));
    m_descFrame.set_label(_("Profile Description"));
    
    m_warningLabel.set_label(_("⚠️ WARNING: EXTREME profiles can damage your hardware. Require active cooling."));
    m_warningLabel.set_halign(Gtk::ALIGN_CENTER);
    
    // Pack controls
    m_controlBox.pack_start(m_profileLabel, Gtk::PACK_SHRINK);
    m_controlBox.pack_start(m_profileCombo, Gtk::PACK_SHRINK);
    m_controlBox.pack_start(m_applyHotBtn, Gtk::PACK_SHRINK);
    m_controlBox.pack_start(m_applyPermBtn, Gtk::PACK_SHRINK);
    m_controlBox.pack_start(m_infoBtn, Gtk::PACK_SHRINK);
    
    m_metricsBox.pack_start(m_cpuTempLabel);
    m_metricsBox.pack_start(m_cpuFreqLabel);
    m_metricsBox.pack_start(m_gpuFreqLabel);
    m_metricsBox.pack_start(m_cpuGovLabel);
    m_metricsBox.pack_start(m_throttlingStatusLabel);
    m_metricsBox.pack_start(m_throttlingDetailsLabel);
    m_metricsBox.pack_start(m_statusLabel);
    m_metricsFrame.add(m_metricsBox);
    
    m_descBox.pack_start(m_descLabel);
    m_descFrame.add(m_descBox);
    
    m_warningFrame.add(m_warningLabel);
    
    m_mainBox.pack_start(m_controlBox, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_metricsFrame);
    m_mainBox.pack_start(m_descFrame);
    m_mainBox.pack_start(m_warningFrame, Gtk::PACK_SHRINK);
    
    add(m_mainBox);
    
    // Connect signals
    m_profileCombo.signal_changed().connect(sigc::mem_fun(*this, &PiOverclockApp::onProfileChanged));
    m_applyHotBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onApplyHotClicked));
    m_applyPermBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onApplyPermClicked));
    m_infoBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onInfoClicked));
    
    // Start update thread
    m_updateThread = std::thread([this]() {
        while (m_threadRunning) {
            Glib::signal_idle().connect_once(sigc::mem_fun(*this, &PiOverclockApp::updateSystemInfo));
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
    
    // Update initial interface
    onProfileChanged();
    updateSystemInfo();
    
    show_all_children();
}

PiOverclockApp::~PiOverclockApp() {
    m_threadRunning = false;
    if (m_updateThread.joinable()) {
        m_updateThread.join();
    }
}

void PiOverclockApp::loadProfiles() {
    // Minimum Profile
    ProfileConfig minimum;
    minimum.arm_freq = "600";
    minimum.gpu_freq = "500";
    minimum.over_voltage = "0";
    minimum.force_turbo = "0";
    minimum.temp_limit = "80";
    minimum.desc = _("Minimum operation configuration\n- CPU: 600 MHz\n- GPU: 500 MHz\n- Overvoltage: 0\n- Force Turbo: 0");
    minimum.color = "#27ae60";
    m_profiles[_("Minimum")] = minimum;
    
    // Normal Profile
    ProfileConfig normal;
    normal.arm_freq = "1800";
    normal.gpu_freq = "500";
    normal.over_voltage = "0";
    normal.force_turbo = "0";
    normal.temp_limit = "80";
    normal.desc = _("Factory default configuration\n- CPU: 1800 MHz\n- GPU: 500 MHz\n- Overvoltage: 0\n- Force Turbo: 0");
    normal.color = "#27ae60";
    m_profiles[_("Normal")] = normal;
    
    // Moderate Profile
    ProfileConfig moderate;
    moderate.arm_freq = "1900";
    moderate.gpu_freq = "550";
    moderate.over_voltage = "2";
    moderate.force_turbo = "0";
    moderate.temp_limit = "75";
    moderate.desc = _("Slight performance increase\n- CPU: 1900 MHz\n- GPU: 550 MHz\n- Overvoltage: 2\n- Force Turbo: 0");
    moderate.color = "#3498db";
    m_profiles[_("Moderate")] = moderate;
    
    // High Profile
    ProfileConfig high;
    high.arm_freq = "2000";
    high.gpu_freq = "600";
    high.over_voltage = "4";
    high.force_turbo = "0";
    high.temp_limit = "70";
    high.desc = _("Improved performance\n- CPU: 2000 MHz\n- GPU: 600 MHz\n- Overvoltage: 4\n- Force Turbo: 0");
    high.color = "#f39c12";
    m_profiles[_("High")] = high;
    
    // Extreme Profile
    ProfileConfig extreme;
    extreme.arm_freq = "2200";
    extreme.gpu_freq = "750";
    extreme.over_voltage = "8";
    extreme.force_turbo = "0";
    extreme.temp_limit = "65";
    extreme.desc = _("Maximum performance (dangerous)\n- CPU: 2200 MHz\n- GPU: 750 MHz\n- Overvoltage: 8\n- Force Turbo: 0\n\n⚠️ REQUIRES GOOD COOLING");
    extreme.color = "#e74c3c";
    m_profiles[_("Extreme")] = extreme;
}

void PiOverclockApp::onProfileChanged() {
    m_currentProfile = m_profileCombo.get_active_text();
    if (m_profiles.find(m_currentProfile) != m_profiles.end()) {
        m_descLabel.set_label(m_profiles[m_currentProfile].desc);
        m_statusLabel.set_label(_("Current profile: ") + m_currentProfile);
    }
}

void PiOverclockApp::onApplyHotClicked() {
    std::string selectedProfile = m_profileCombo.get_active_text();
    
    if (selectedProfile == _("Extreme")) {
        if (!showQuestionDialog(_("⚠️ DANGER - EXTREME OVERCLOCK"), 
            _("EXTREME PROFILE IS DANGEROUS:\n\n"
            "• Can permanently damage your Raspberry Pi\n"
            "• Generates very high temperatures\n"
            "• Requires excellent active cooling\n"
            "• Void warranty permanently\n\n"
            "Are you absolutely sure to continue?"))) {
            m_profileCombo.set_active_text(_("Normal"));
            onProfileChanged();
            return;
        }
    }
    
    if (showQuestionDialog(_("Apply Hot"), 
        std::string(_("Profile '%s' will be applied hot.\n\n"
        "⚠️ LIMITATIONS:\n"
        "- Only CPU frequency will be adjusted\n"
        "- Overvoltage and GPU require permanent application\n"
        "- Changes will be lost on reboot\n\n"
        "Do you want to continue?")).replace(std::string(_("Profile '%s' will be applied hot.")).find("%s"), 2, selectedProfile))) {
        applyHotProfile(selectedProfile);
    }
}

void PiOverclockApp::onApplyPermClicked() {
    std::string selectedProfile = m_profileCombo.get_active_text();
    
    ProfileConfig settings = m_profiles[selectedProfile];
    std::string message = std::string(_("Are you sure to apply profile '%s' permanently?\n\n"
        "This will modify the system configuration file and include:\n"
        "- CPU: %s MHz\n"
        "- GPU: %s MHz\n"
        "- Overvoltage: %s\n"
        "- Force Turbo: %s\n\n"
        "⚠️ WILL REQUIRE REBOOT to apply all changes."));
    
    // Replace placeholders
    size_t pos = message.find("%s");
    if (pos != std::string::npos) message.replace(pos, 2, selectedProfile);
    
    pos = message.find("%s");
    if (pos != std::string::npos) message.replace(pos, 2, settings.arm_freq);
    
    pos = message.find("%s");
    if (pos != std::string::npos) message.replace(pos, 2, settings.gpu_freq);
    
    pos = message.find("%s");
    if (pos != std::string::npos) message.replace(pos, 2, settings.over_voltage);
    
    pos = message.find("%s");
    if (pos != std::string::npos) message.replace(pos, 2, settings.force_turbo);
    
    if (showQuestionDialog(_("Confirm Permanent Change"), message)) {
        applyPermanentProfile(selectedProfile);
    }
}

void PiOverclockApp::onInfoClicked() {
    std::string info = _("=== SYSTEM INFORMATION ===\n\n");
    
    // Get CPU information
    try {
        std::ifstream minFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
        std::ifstream maxFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
        std::ifstream curFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
        
        std::string minFreq, maxFreq, curFreq;
        if (minFile) std::getline(minFile, minFreq);
        if (maxFile) std::getline(maxFile, maxFreq);
        if (curFile) std::getline(curFile, curFreq);
        
        if (!minFreq.empty() && !maxFreq.empty() && !curFreq.empty()) {
            int min = std::stoi(minFreq) / 1000;
            int max = std::stoi(maxFreq) / 1000;
            int cur = std::stoi(curFreq) / 1000;
            info += std::string(_("CPU: ")) + std::to_string(cur) + _(" MHz (Min: ") + std::to_string(min) + _(" MHz, Max: ") + std::to_string(max) + _(" MHz)\n");
        }
    } catch (...) {
        info += _("CPU: N/A\n");
    }
    
    // Get governor
    try {
        std::ifstream govFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        std::string governor;
        if (govFile) std::getline(govFile, governor);
        info += std::string(_("Governor: ")) + governor + "\n";
    } catch (...) {
        info += _("Governor: N/A\n");
    }
    
    // Get throttling
    info += std::string(_("Throttling: ")) + getThrottlingInfo() + "\n";
    
    // Get temperature
    try {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        std::string tempStr;
        if (tempFile) std::getline(tempFile, tempStr);
        if (!tempStr.empty()) {
            float temp = std::stof(tempStr) / 1000.0;
            info += std::string(_("Temperature: ")) + std::to_string(temp) + _(" °C\n");
        }
    } catch (...) {
        info += _("Temperature: N/A\n");
    }
    
    // Show dialog with information
    Gtk::MessageDialog dialog(*this, info, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    dialog.set_title(_("System Information"));
    dialog.run();
}

void PiOverclockApp::updateSystemInfo() {
    // Update temperature
    try {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        std::string tempStr;
        if (tempFile) std::getline(tempFile, tempStr);
        if (!tempStr.empty()) {
            float temp = std::stof(tempStr) / 1000.0;
            m_cpuTempLabel.set_label(std::string(_("CPU Temperature: ")) + std::to_string(temp) + _(" °C"));
        }
    } catch (...) {
        m_cpuTempLabel.set_label(_("CPU Temperature: N/A"));
    }
    
    // Update CPU frequency
    try {
        std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
        std::string freqStr;
        if (freqFile) std::getline(freqFile, freqStr);
        if (!freqStr.empty()) {
            int freq = std::stoi(freqStr) / 1000;
            m_cpuFreqLabel.set_label(std::string(_("CPU Frequency: ")) + std::to_string(freq) + _(" MHz"));
        }
    } catch (...) {
        m_cpuFreqLabel.set_label(_("CPU Frequency: N/A"));
    }
    
    // Update GPU frequency
    std::string gpuFreq = execCommand("vcgencmd measure_clock v3d");
    if (!gpuFreq.empty() && gpuFreq.find("=") != std::string::npos) {
        try {
            std::string freqStr = gpuFreq.substr(gpuFreq.find("=") + 1);
            int freq = std::stoi(freqStr) / 1000000;
            m_gpuFreqLabel.set_label(std::string(_("GPU Frequency: ")) + std::to_string(freq) + _(" MHz"));
        } catch (...) {
            m_gpuFreqLabel.set_label(_("GPU Frequency: N/A"));
        }
    } else {
        m_gpuFreqLabel.set_label(_("GPU Frequency: N/A"));
    }
    
    // Update governor
    try {
        std::ifstream govFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        std::string governor;
        if (govFile) std::getline(govFile, governor);
        m_cpuGovLabel.set_label(std::string(_("CPU Governor: ")) + governor);
    } catch (...) {
        m_cpuGovLabel.set_label(_("CPU Governor: N/A"));
    }
    
    // Update throttling information
    std::string throttlingInfo = getThrottlingInfo();
    m_throttlingStatusLabel.set_label(std::string(_("Status: ")) + throttlingInfo);
    m_throttlingDetailsLabel.set_label(std::string(_("Details: ")) + decodeThrottling(execCommand("vcgencmd get_throttled")));
}

void PiOverclockApp::applyHotProfile(const std::string& profileName, bool showMessage) {
    if (m_profiles.find(profileName) == m_profiles.end()) return;

    ProfileConfig settings = m_profiles[profileName];

    try {
        // Change governor to performance
        std::string cmdGov = "cpufreq-set -g performance";
        int retGov = system(cmdGov.c_str());
        if (retGov != 0) {
            showMessageDialog(_("Error"), _("Could not change governor to performance.\nMake sure 'cpufrequtils' is installed and you have root permissions."), Gtk::MESSAGE_ERROR);
            return;
        }

        // Change minimum and maximum frequencies
        std::string cmdFreq = "cpufreq-set -d " + settings.arm_freq + "MHz -u " + settings.arm_freq + "MHz";
        int retFreq = system(cmdFreq.c_str());
        if (retFreq != 0) {
            showMessageDialog(_("Error"), _("Could not adjust CPU frequency.\nMake sure 'cpufrequtils' is installed and you have root permissions."), Gtk::MESSAGE_ERROR);
            return;
        }

        // Update interface status
        m_currentProfile = profileName;
        m_statusLabel.set_label(std::string(_("Current profile: ")) + profileName + _(" (Hot applied)"));

        if (showMessage) {
            std::string message = std::string(_("Profile %s partially applied hot.\n\n"
                "✅ CPU frequency adjusted correctly\n"
                "⚠️ Overvoltage and GPU not applied (require reboot)\n"
                "⚠️ Changes will be lost on reboot")).replace(
                std::string(_("Profile %s partially applied hot.")).find("%s"), 2, profileName);
            
            showMessageDialog(_("Partial Success"), message, Gtk::MESSAGE_INFO);
        }

        // Update metrics
        updateSystemInfo();

    } catch (const std::exception& e) {
        showMessageDialog(_("Error"), std::string(_("An error occurred while applying the profile: ")) + e.what(), Gtk::MESSAGE_ERROR);
    }
}

void PiOverclockApp::applyPermanentProfile(const std::string& profileName) {
    if (m_profiles.find(profileName) == m_profiles.end()) return;
    
    ProfileConfig settings = m_profiles[profileName];
    std::string configFile = "/boot/firmware/config.txt";
    std::string backupFile = "/boot/firmware/config.txt.bak";
    
    try {
        // Read current configuration
        std::vector<std::string> lines;
        if (fileExists(configFile)) {
            std::ifstream inFile(configFile);
            std::string line;
            while (std::getline(inFile, line)) {
                lines.push_back(line);
            }
            inFile.close();
        }
        
        // Create backup if it doesn't exist
        if (!fileExists(backupFile)) {
            std::ofstream outFile(backupFile);
            for (const auto& line : lines) {
                outFile << line << std::endl;
            }
            outFile.close();
        }
        
        // Remove existing configurations
        std::vector<std::string> newLines;
        std::vector<std::string> settingsToRemove = {"arm_freq", "gpu_freq", "over_voltage", "force_turbo"};
        
        for (const auto& line : lines) {
            bool keepLine = true;
            for (const auto& setting : settingsToRemove) {
                if (line.find(setting + "=") == 0) {
                    keepLine = false;
                    break;
                }
            }
            if (keepLine) {
                newLines.push_back(line);
            }
        }
        
        // Add new configurations
        newLines.push_back("arm_freq=" + settings.arm_freq);
        newLines.push_back("gpu_freq=" + settings.gpu_freq);
        newLines.push_back("over_voltage=" + settings.over_voltage);
        newLines.push_back("force_turbo=" + settings.force_turbo);
        
        // Save temporary changes
        std::ofstream tmpFile("/tmp/config.txt.tmp");
        for (const auto& line : newLines) {
            tmpFile << line << std::endl;
        }
        tmpFile.close();
        
        // Replace configuration file
        system("sudo mv /tmp/config.txt.tmp /boot/firmware/config.txt");
        system("sudo chown root:root /boot/firmware/config.txt");
        system("sudo chmod 644 /boot/firmware/config.txt");
        
        std::string message = std::string(_("Profile %s applied permanently.\n\n"
            "All settings (CPU, GPU, Overvoltage) have been saved.\n"
            "Backup saved at: %s\n\n"
            "Do you want to reboot now to apply all changes?"));
        
        // Replace placeholders
        size_t pos = message.find("%s");
        if (pos != std::string::npos) message.replace(pos, 2, profileName);
        
        pos = message.find("%s");
        if (pos != std::string::npos) message.replace(pos, 2, backupFile);
        
        if (showQuestionDialog(_("Success"), message)) {
            system("sudo reboot");
        }
        
    } catch (const std::exception& e) {
        showMessageDialog(_("Error"), std::string(_("Could not modify config.txt: ")) + e.what(), Gtk::MESSAGE_ERROR);
    }
}

std::string PiOverclockApp::getThrottlingInfo() {
    std::string result = execCommand("vcgencmd get_throttled");
    if (result.empty() || result.find("=") == std::string::npos) {
        return _("Error getting throttling");
    }
    
    std::string throttledHex = result.substr(result.find("=") + 1);
    return decodeThrottling(throttledHex);
}

std::string PiOverclockApp::decodeThrottling(const std::string& throttledHex) {
    try {
        unsigned long throttledCode = std::stoul(throttledHex, nullptr, 16);
        std::vector<std::string> messages;
        
        // Current bits
        if (throttledCode & 0x1) messages.push_back(_("Under-voltage now"));
        if (throttledCode & 0x2) messages.push_back(_("Frequency capped now"));
        if (throttledCode & 0x4) messages.push_back(_("Throttling now"));
        if (throttledCode & 0x8) messages.push_back(_("Temperature limit now"));
        
        // Historical bits
        if (throttledCode & 0x10000) messages.push_back(_("Under-voltage occurred"));
        if (throttledCode & 0x20000) messages.push_back(_("Frequency capped occurred"));
        if (throttledCode & 0x40000) messages.push_back(_("Throttling occurred"));
        if (throttledCode & 0x80000) messages.push_back(_("Temperature limit occurred"));
        
        if (messages.empty()) {
            return _("No throttling");
        }
        
        std::string result;
        for (size_t i = 0; i < messages.size(); i++) {
            if (i > 0) result += ", ";
            result += messages[i];
        }
        return result;
    } catch (...) {
        return std::string(_("Hexadecimal value: ")) + throttledHex;
    }
}

std::string PiOverclockApp::execCommand(const std::string& cmd) {
    char buffer[128];
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

bool PiOverclockApp::fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void PiOverclockApp::showMessageDialog(const std::string& title, const std::string& message, Gtk::MessageType type) {
    Gtk::MessageDialog dialog(*this, message, false, type, Gtk::BUTTONS_OK, true);
    dialog.set_title(title);
    dialog.run();
}

bool PiOverclockApp::showQuestionDialog(const std::string& title, const std::string& message) {
    Gtk::MessageDialog dialog(*this, message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
    dialog.set_title(title);
    int result = dialog.run();
    return (result == Gtk::RESPONSE_YES);
}

int main(int argc, char *argv[]) {
    // Set up internationalization
    setlocale(LC_ALL, "");
    bindtextdomain("overpi", "/usr/share/locale");
    textdomain("overpi");
    
    // Check if we are on Raspberry Pi
    if (access("/sys/class/thermal/thermal_zone0/temp", F_OK) == -1) {
        std::cerr << _("Error: This application only works on Raspberry Pi") << std::endl;
        return 1;
    }
    
    // Check superuser permissions
    if (geteuid() != 0) {
        std::cerr << _("Error: Run with: sudo ") << argv[0] << std::endl;
        return 1;
    }
    
    auto app = Gtk::Application::create(argc, argv, "org.rpi.overclock");
    PiOverclockApp window;
    return app->run(window);
}