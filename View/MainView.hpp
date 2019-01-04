/**
 * @file MainView.hpp
 * @author paul
 * @date 30.12.18
 * @brief MainView @TODO
 */

#ifndef SERIALTOOLBOX_MAINVIEW_HPP
#define SERIALTOOLBOX_MAINVIEW_HPP


#include <gtkmm.h>
#include "../Util/Listener.hpp"

namespace view {
    class MainView {
    public:
        explicit MainView(const std::string &uiFile);

        void setPorts(const std::vector<std::string> &ports, int activeIndex = 0);

        auto getWindow() -> Gtk::Window&;

        const util::Listener<std::string> portComboListener;
        const util::Listener<int> baudSpinListener;
        const util::Listener<int> dataBitsSpinListener;
        const util::Listener<int> stopBitsSpinListener;

        auto getPort() const -> std::string;
        auto getBaud() const -> int;
        auto getDataBits() const -> int;
        auto getStopBits() const -> int;
        auto getAsciiEnabled() const -> bool;
        auto getHexEnabled() const -> bool;
        auto getDecEnabled() const -> bool;
        auto getBinEnabled() const -> bool;

    private:
        Gtk::Window *mainWindow;

        // Top bar
        Gtk::ComboBoxText *portCombo;
        Gtk::SpinButton *baudSpin;
        Gtk::SpinButton *dataBitsSpin;
        Gtk::SpinButton *stopBitsSpin;

        // Sidebar
        Gtk::CheckButton *checkAscii;
        Gtk::CheckButton *checkHex;
        Gtk::CheckButton *checkDec;
        Gtk::CheckButton *checkBin;

        void portComboHandler();
        void baudSpinHandler();
        void dataBitsSpinHandler();
        void stopBitsSpinHandler();
    };
}

#endif
