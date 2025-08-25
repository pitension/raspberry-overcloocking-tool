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

// Estructura para almacenar la configuración de perfil
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
    // Controles de la interfaz
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
    
    // Variables de estado
    std::map<std::string, ProfileConfig> m_profiles;
    std::string m_currentProfile;
    
    // Hilo para actualizar información del sistema
    std::thread m_updateThread;
    bool m_threadRunning;
    
    // Métodos
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
  m_profileLabel("Perfil:"),
  m_metricsBox(Gtk::ORIENTATION_VERTICAL, 5),
  m_descBox(Gtk::ORIENTATION_VERTICAL, 5),
  m_threadRunning(true) {
    
    // Configurar ventana principal
    set_title("RPi 400 Overclock Control - Modo en Caliente");
    set_default_size(900, 600);
    set_border_width(10);
    
    // Cargar perfiles
    loadProfiles();
    
    // Configurar controles
    m_profileCombo.append("Minimo");
    m_profileCombo.append("Normal");
    m_profileCombo.append("Moderado");
    m_profileCombo.append("Alto");
    m_profileCombo.append("Extremo");
    m_profileCombo.set_active_text("Normal");
    m_currentProfile = "Normal";
    
    m_applyHotBtn.set_label("Aplicar en Caliente");
    m_applyPermBtn.set_label("Aplicar Permanentemente");
    m_infoBtn.set_label("Info del Sistema");
    
    m_metricsFrame.set_label("Estado del Sistema");
    m_descFrame.set_label("Descripción del Perfil");
    
    m_warningLabel.set_label("⚠️ ADVERTENCIA: Los perfiles EXTREMO pueden dañar tu hardware. Requieren refrigeración activa.");
    m_warningLabel.set_halign(Gtk::ALIGN_CENTER);
    
    // Empaquetar controles
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
    
    // Conectar señales
    m_profileCombo.signal_changed().connect(sigc::mem_fun(*this, &PiOverclockApp::onProfileChanged));
    m_applyHotBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onApplyHotClicked));
    m_applyPermBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onApplyPermClicked));
    m_infoBtn.signal_clicked().connect(sigc::mem_fun(*this, &PiOverclockApp::onInfoClicked));
    
    // Iniciar hilo de actualización
    m_updateThread = std::thread([this]() {
        while (m_threadRunning) {
            Glib::signal_idle().connect_once(sigc::mem_fun(*this, &PiOverclockApp::updateSystemInfo));
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
    
    // Actualizar interfaz inicial
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
    // Perfil Normal
    ProfileConfig minimo;
    minimo.arm_freq = "600";
    minimo.gpu_freq = "500";
    minimo.over_voltage = "0";
    minimo.force_turbo = "0";
    minimo.temp_limit = "80";
    minimo.desc = "Configuración minime de funcionamiento\n- CPU: 600 MHz\n- GPU: 500 MHz\n- Overvoltage: 0\n- Force Turbo: 0";
    minimo.color = "#27ae60";
    m_profiles["Minimo"] = minimo;
    
    // Perfil Normal
    ProfileConfig normal;
    normal.arm_freq = "1800";
    normal.gpu_freq = "500";
    normal.over_voltage = "0";
    normal.force_turbo = "0";
    normal.temp_limit = "80";
    normal.desc = "Configuración estándar de fábrica\n- CPU: 1800 MHz\n- GPU: 500 MHz\n- Overvoltage: 0\n- Force Turbo: 0";
    normal.color = "#27ae60";
    m_profiles["Normal"] = normal;
    
    // Perfil Moderado
    ProfileConfig moderado;
    moderado.arm_freq = "1900";
    moderado.gpu_freq = "550";
    moderado.over_voltage = "2";
    moderado.force_turbo = "0";
    moderado.temp_limit = "75";
    moderado.desc = "Ligero aumento de rendimiento\n- CPU: 1900 MHz\n- GPU: 550 MHz\n- Overvoltage: 2\n- Force Turbo: 0";
    moderado.color = "#3498db";
    m_profiles["Moderado"] = moderado;
    
    // Perfil Alto
    ProfileConfig alto;
    alto.arm_freq = "2000";
    alto.gpu_freq = "600";
    alto.over_voltage = "4";
    alto.force_turbo = "0";
    alto.temp_limit = "70";
    alto.desc = "Rendimiento mejorado\n- CPU: 2000 MHz\n- GPU: 600 MHz\n- Overvoltage: 4\n- Force Turbo: 0";
    alto.color = "#f39c12";
    m_profiles["Alto"] = alto;
    
    // Perfil Extremo
    ProfileConfig extremo;
    extremo.arm_freq = "2200";
    extremo.gpu_freq = "750";
    extremo.over_voltage = "8";
    extremo.force_turbo = "0";
    extremo.temp_limit = "65";
    extremo.desc = "Máximo rendimiento (peligroso)\n- CPU: 2200 MHz\n- GPU: 750 MHz\n- Overvoltage: 8\n- Force Turbo: 0\n\n⚠️ REQUIERE BUENA REFRIGERACIÓN";
    extremo.color = "#e74c3c";
    m_profiles["Extremo"] = extremo;
}

void PiOverclockApp::onProfileChanged() {
    m_currentProfile = m_profileCombo.get_active_text();
    if (m_profiles.find(m_currentProfile) != m_profiles.end()) {
        m_descLabel.set_label(m_profiles[m_currentProfile].desc);
        m_statusLabel.set_label("Perfil actual: " + m_currentProfile);
    }
}

void PiOverclockApp::onApplyHotClicked() {
    std::string selectedProfile = m_profileCombo.get_active_text();
    
    if (selectedProfile == "Extremo") {
        if (!showQuestionDialog("⚠️ PELIGRO - OVERCLOCK EXTREMO", 
            "EL PERFIL EXTREMO ES PELIGROSO:\n\n"
            "• Puede dañar permanentemente tu Raspberry Pi\n"
            "• Genera temperaturas muy altas\n"
            "• Requiere refrigeración activa excelente\n"
            "• Anula la garantía definitivamente\n\n"
            "¿Estás absolutamente seguro de continuar?")) {
            m_profileCombo.set_active_text("Normal");
            onProfileChanged();
            return;
        }
    }
    
    if (showQuestionDialog("Aplicar en Caliente", 
        "Se aplicará el perfil '" + selectedProfile + "' en caliente.\n\n"
        "⚠️ LIMITACIONES:\n"
        "- Solo se ajustará la frecuencia CPU\n"
        "- Overvoltage y GPU requieren aplicación permanente\n"
        "- Los cambios se perderán al reiniciar\n\n"
        "¿Deseas continuar?")) {
        applyHotProfile(selectedProfile);
    }
}

void PiOverclockApp::onApplyPermClicked() {
    std::string selectedProfile = m_profileCombo.get_active_text();
    
    if (showQuestionDialog("Confirmar Cambio Permanente",
        "¿Estás seguro de aplicar el perfil '" + selectedProfile + "' permanentemente?\n\n"
        "Esto modificará el archivo de configuración del sistema e incluirá:\n"
        "- CPU: " + m_profiles[selectedProfile].arm_freq + " MHz\n"
        "- GPU: " + m_profiles[selectedProfile].gpu_freq + " MHz\n"
        "- Overvoltage: " + m_profiles[selectedProfile].over_voltage + "\n"
        "- Force Turbo: " + m_profiles[selectedProfile].force_turbo + "\n\n"
        "⚠️ REQUERIRÁ REINICIAR para aplicar todos los cambios.")) {
        applyPermanentProfile(selectedProfile);
    }
}

void PiOverclockApp::onInfoClicked() {
    std::string info = "=== INFORMACIÓN DEL SISTEMA ===\n\n";
    
    // Obtener información de CPU
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
            info += "CPU: " + std::to_string(cur) + " MHz (Min: " + std::to_string(min) + " MHz, Max: " + std::to_string(max) + " MHz)\n";
        }
    } catch (...) {
        info += "CPU: N/A\n";
    }
    
    // Obtener governor
    try {
        std::ifstream govFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        std::string governor;
        if (govFile) std::getline(govFile, governor);
        info += "Governor: " + governor + "\n";
    } catch (...) {
        info += "Governor: N/A\n";
    }
    
    // Obtener throttling
    info += "Throttling: " + getThrottlingInfo() + "\n";
    
    // Obtener temperatura
    try {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        std::string tempStr;
        if (tempFile) std::getline(tempFile, tempStr);
        if (!tempStr.empty()) {
            float temp = std::stof(tempStr) / 1000.0;
            info += "Temperatura: " + std::to_string(temp) + " °C\n";
        }
    } catch (...) {
        info += "Temperatura: N/A\n";
    }
    
    // Mostrar diálogo con la información
    Gtk::MessageDialog dialog(*this, info, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    dialog.set_title("Información del Sistema");
    dialog.run();
}

void PiOverclockApp::updateSystemInfo() {
    // Actualizar temperatura
    try {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        std::string tempStr;
        if (tempFile) std::getline(tempFile, tempStr);
        if (!tempStr.empty()) {
            float temp = std::stof(tempStr) / 1000.0;
            m_cpuTempLabel.set_label("Temperatura CPU: " + std::to_string(temp) + " °C");
        }
    } catch (...) {
        m_cpuTempLabel.set_label("Temperatura CPU: N/A");
    }
    
    // Actualizar frecuencia CPU
    try {
        std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
        std::string freqStr;
        if (freqFile) std::getline(freqFile, freqStr);
        if (!freqStr.empty()) {
            int freq = std::stoi(freqStr) / 1000;
            m_cpuFreqLabel.set_label("Frecuencia CPU: " + std::to_string(freq) + " MHz");
        }
    } catch (...) {
        m_cpuFreqLabel.set_label("Frecuencia CPU: N/A");
    }
    
    // Actualizar frecuencia GPU
    std::string gpuFreq = execCommand("vcgencmd measure_clock v3d");
    if (!gpuFreq.empty() && gpuFreq.find("=") != std::string::npos) {
        try {
            std::string freqStr = gpuFreq.substr(gpuFreq.find("=") + 1);
            int freq = std::stoi(freqStr) / 1000000;
            m_gpuFreqLabel.set_label("Frecuencia GPU: " + std::to_string(freq) + " MHz");
        } catch (...) {
            m_gpuFreqLabel.set_label("Frecuencia GPU: N/A");
        }
    } else {
        m_gpuFreqLabel.set_label("Frecuencia GPU: N/A");
    }
    
    // Actualizar governor
    try {
        std::ifstream govFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        std::string governor;
        if (govFile) std::getline(govFile, governor);
        m_cpuGovLabel.set_label("Governor CPU: " + governor);
    } catch (...) {
        m_cpuGovLabel.set_label("Governor CPU: N/A");
    }
    
    // Actualizar información de throttling
    std::string throttlingInfo = getThrottlingInfo();
    m_throttlingStatusLabel.set_label("Estado: " + throttlingInfo);
    m_throttlingDetailsLabel.set_label("Detalles: " + decodeThrottling(execCommand("vcgencmd get_throttled")));
}

void PiOverclockApp::applyHotProfile(const std::string& profileName, bool showMessage) {
    if (m_profiles.find(profileName) == m_profiles.end()) return;

    ProfileConfig settings = m_profiles[profileName];

    try {
        // Cambiar governor a performance
        std::string cmdGov = "cpufreq-set -g performance";
        int retGov = system(cmdGov.c_str());
        if (retGov != 0) {
            showMessageDialog("Error", "No se pudo cambiar el governor a performance.\nAsegúrate de tener instalado 'cpufrequtils' y permisos de root.", Gtk::MESSAGE_ERROR);
            return;
        }

        // Cambiar frecuencias mínima y máxima
        std::string cmdFreq = "cpufreq-set -d " + settings.arm_freq + "MHz -u " + settings.arm_freq + "MHz";
        int retFreq = system(cmdFreq.c_str());
        if (retFreq != 0) {
            showMessageDialog("Error", "No se pudo ajustar la frecuencia CPU.\nAsegúrate de tener instalado 'cpufrequtils' y permisos de root.", Gtk::MESSAGE_ERROR);
            return;
        }

        // Actualizar estado en la interfaz
        m_currentProfile = profileName;
        m_statusLabel.set_label("Perfil actual: " + profileName + " (En caliente)");

        if (showMessage) {
            showMessageDialog("Éxito Parcial",
                "Perfil " + profileName + " aplicado parcialmente en caliente.\n\n"
                "✅ Frecuencia CPU ajustada correctamente\n"
                "⚠️ Overvoltage y GPU no se aplicaron (requieren reinicio)\n"
                "⚠️ Los cambios se perderán al reiniciar",
                Gtk::MESSAGE_INFO);
        }

        // Actualizar métricas
        updateSystemInfo();

    } catch (const std::exception& e) {
        showMessageDialog("Error", "Ocurrió un error al aplicar el perfil: " + std::string(e.what()), Gtk::MESSAGE_ERROR);
    }
}


void PiOverclockApp::applyPermanentProfile(const std::string& profileName) {
    if (m_profiles.find(profileName) == m_profiles.end()) return;
    
    ProfileConfig settings = m_profiles[profileName];
    std::string configFile = "/boot/firmware/config.txt";
    std::string backupFile = "/boot/firmware/config.txt.bak";
    
    try {
        // Leer configuración actual
        std::vector<std::string> lines;
        if (fileExists(configFile)) {
            std::ifstream inFile(configFile);
            std::string line;
            while (std::getline(inFile, line)) {
                lines.push_back(line);
            }
            inFile.close();
        }
        
        // Crear backup si no existe
        if (!fileExists(backupFile)) {
            std::ofstream outFile(backupFile);
            for (const auto& line : lines) {
                outFile << line << std::endl;
            }
            outFile.close();
        }
        
        // Eliminar configuraciones existentes
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
        
        // Añadir nuevas configuraciones
        newLines.push_back("arm_freq=" + settings.arm_freq);
        newLines.push_back("gpu_freq=" + settings.gpu_freq);
        newLines.push_back("over_voltage=" + settings.over_voltage);
        newLines.push_back("force_turbo=" + settings.force_turbo);
        
        // Guardar cambios temporales
        std::ofstream tmpFile("/tmp/config.txt.tmp");
        for (const auto& line : newLines) {
            tmpFile << line << std::endl;
        }
        tmpFile.close();
        
        // Reemplazar archivo de configuración
        system("sudo mv /tmp/config.txt.tmp /boot/firmware/config.txt");
        system("sudo chown root:root /boot/firmware/config.txt");
        system("sudo chmod 644 /boot/firmware/config.txt");
        
        if (showQuestionDialog("Éxito", 
            "Perfil " + profileName + " aplicado permanentemente.\n\n"
            "Todos los ajustes (CPU, GPU, Overvoltage) se han guardado.\n"
            "Backup guardado en: " + backupFile + "\n\n"
            "¿Quieres reiniciar ahora para aplicar los cambios completos?")) {
            system("sudo reboot");
        }
        
    } catch (const std::exception& e) {
        showMessageDialog("Error", "No se pudo modificar config.txt: " + std::string(e.what()), Gtk::MESSAGE_ERROR);
    }
}

std::string PiOverclockApp::getThrottlingInfo() {
    std::string result = execCommand("vcgencmd get_throttled");
    if (result.empty() || result.find("=") == std::string::npos) {
        return "Error al obtener throttling";
    }
    
    std::string throttledHex = result.substr(result.find("=") + 1);
    return decodeThrottling(throttledHex);
}

std::string PiOverclockApp::decodeThrottling(const std::string& throttledHex) {
    try {
        unsigned long throttledCode = std::stoul(throttledHex, nullptr, 16);
        std::vector<std::string> messages;
        
        // Bits actuales
        if (throttledCode & 0x1) messages.push_back("Under-voltage now");
        if (throttledCode & 0x2) messages.push_back("Frecuencia capped now");
        if (throttledCode & 0x4) messages.push_back("Throttling now");
        if (throttledCode & 0x8) messages.push_back("Temperatura limit now");
        
        // Bits históricos
        if (throttledCode & 0x10000) messages.push_back("Under-voltage occurred");
        if (throttledCode & 0x20000) messages.push_back("Frecuencia capped occurred");
        if (throttledCode & 0x40000) messages.push_back("Throttling occurred");
        if (throttledCode & 0x80000) messages.push_back("Temperatura limit occurred");
        
        if (messages.empty()) {
            return "Sin throttling";
        }
        
        std::string result;
        for (size_t i = 0; i < messages.size(); i++) {
            if (i > 0) result += ", ";
            result += messages[i];
        }
        return result;
    } catch (...) {
        return "Valor hexadecimal: " + throttledHex;
    }
}

std::string PiOverclockApp::execCommand(const std::string& cmd) {
    char buffer[128];   // buffer clásico en C
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
    // Verificar si estamos en Raspberry Pi
    if (access("/sys/class/thermal/thermal_zone0/temp", F_OK) == -1) {
        std::cerr << "Error: Esta aplicación solo funciona en Raspberry Pi" << std::endl;
        return 1;
    }
    
    // Verificar permisos de superusuario
    if (geteuid() != 0) {
        std::cerr << "Error: Ejecuta con: sudo " << argv[0] << std::endl;
        return 1;
    }
    
    auto app = Gtk::Application::create(argc, argv, "org.rpi.overclock");
    PiOverclockApp window;
    return app->run(window);
}
