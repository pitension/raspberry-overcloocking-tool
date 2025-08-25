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

// ... (el resto del código permanece similar, pero usando _() para los textos)

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
    
    // ... (similar para los otros perfiles)
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
        _("Profile '%s' will be applied hot.\n\n"
        "⚠️ LIMITATIONS:\n"
        "- Only CPU frequency will be adjusted\n"
        "- Overvoltage and GPU require permanent application\n"
        "- Changes will be lost on reboot\n\n"
        "Do you want to continue?"), selectedProfile.c_str())) {
        applyHotProfile(selectedProfile);
    }
}

// ... (continuar con el resto del código)

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
        std::cerr << _("Error: Run with: sudo %s") << argv[0] << std::endl;
        return 1;
    }
    
    auto app = Gtk::Application::create(argc, argv, "org.rpi.overclock");
    PiOverclockApp window;
    return app->run(window);
}
