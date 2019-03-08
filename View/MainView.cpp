/**
 * @file MainView.cpp
 * @author paul
 * @date 30.12.18
 * @brief Wrapper for the MainView (view)
 */

#include "MainView.hpp"

#include <cassert>

#include <QFile>
#include <QUiLoader>
#include <QMessageBox>

#define FIND_WIDGET(x) \
x=std::unique_ptr<decltype(x)::element_type>(mainWindow->findChild<decltype(x)::element_type*>(#x)); \
assert(x != nullptr)

namespace view {
    MainView::MainView(const std::string &uiFile) : receivePosition{0,0}, sendPosition{0,0} {
        QFile file{uiFile.c_str()};
        if(!file.open(QFile::ReadOnly)) {
            throw std::runtime_error("Could not find ui file");
        }
        QUiLoader loader;

        mainWindow = std::shared_ptr<QWidget>(loader.load(&file));
        file.close();
        mainWindow->showMaximized();

        FIND_WIDGET(portCombo);
        FIND_WIDGET(parityCombo);
        FIND_WIDGET(baudSpin);
        FIND_WIDGET(dataBitsSpin);
        FIND_WIDGET(stopBitsSpin);
        FIND_WIDGET(baudLabel);
        FIND_WIDGET(parityLabel);
        FIND_WIDGET(stopBitsLabel);
        FIND_WIDGET(dataBitsLabel);
        FIND_WIDGET(checkBin);
        FIND_WIDGET(checkDec);
        FIND_WIDGET(checkHex);
        FIND_WIDGET(checkAscii);
        FIND_WIDGET(encodingSendCombo);
        FIND_WIDGET(toSendEntry);
        FIND_WIDGET(repetitionsSpin);
        FIND_WIDGET(periodSpin);
        FIND_WIDGET(sendButton);
        FIND_WIDGET(sendGrid);
        FIND_WIDGET(receiveGrid);
        FIND_WIDGET(comboLinebreak);
        FIND_WIDGET(labelRxCount);
        FIND_WIDGET(labelTxCount);
        FIND_WIDGET(buttonResetRx);
        FIND_WIDGET(buttonResetTx);
        FIND_WIDGET(buttonClearReceived);
        FIND_WIDGET(buttonClearSent);

        mainWindow->connect(portCombo.get(),  QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
                this, [this](const QString &port){
            portComboListener(port.toLocal8Bit().data());
        });
        mainWindow->connect(baudSpin.get(), QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int baud){
            baudSpinListener(baud);
        });
        mainWindow->connect(dataBitsSpin.get(), QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int dataBits){
            dataBitsSpinListener(dataBits);
        });
        mainWindow->connect(stopBitsSpin.get(), QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int stopBits){
            stopBitsSpinListener(stopBits);
        });
        mainWindow->connect(parityCombo.get(),  QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int sel){
            parityListener(sel);
        });

        mainWindow->connect(checkAscii.get(),  QOverload<int>::of(&QCheckBox::stateChanged),
                this, [this](int state){
            asciiEnabledListener(state == Qt::Checked);
        });
        mainWindow->connect(checkBin.get(),  QOverload<int>::of(&QCheckBox::stateChanged),
                this, [this](int state){
            binEnabledListener(state == Qt::Checked);
        });
        mainWindow->connect(checkDec.get(),  QOverload<int>::of(&QCheckBox::stateChanged),
                this, [this](int state){
            decEnabledListener(state == Qt::Checked);
        });
        mainWindow->connect(checkHex.get(),  QOverload<int>::of(&QCheckBox::stateChanged),
                this, [this](int state){
            hexEnabledListener(state == Qt::Checked);
        });

        mainWindow->connect(comboLinebreak.get(),  QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int index){
            linebreakListener(index);
        });

        mainWindow->connect(sendButton.get(),  QOverload<bool>::of(&QPushButton::clicked), this, [this](bool){
            sendHandler();
        });

        mainWindow->connect(buttonResetRx.get(),  QOverload<bool>::of(&QPushButton::clicked), this, [this](bool){
            resetRxListener();
        });

        mainWindow->connect(buttonResetTx.get(),  QOverload<bool>::of(&QPushButton::clicked), this, [this](bool){
            resetTxListener();
        });

        mainWindow->connect(buttonClearSent.get(),  QOverload<bool>::of(&QPushButton::clicked), this, [this](bool){
            clearTxListener();
        });

        mainWindow->connect(buttonClearReceived.get(),  QOverload<bool>::of(&QPushButton::clicked), this, [this](bool){
            clearRxListener();
        });

        this->representationIds.insert({"ASCII", 0});
        this->representationIds.insert({"HEX", 1});
        this->representationIds.insert({"DEC", 2});
        this->representationIds.insert({"BIN", 3});
    }

    void MainView::setPorts(const std::vector<std::string> &ports, int activeIndex) {
        this->portCombo->clear();
        QStringList stringList;
        for (const auto &port : ports) {
            stringList << port.c_str();
        }
        this->portCombo->addItems(stringList);
        this->portCombo->setCurrentIndex(activeIndex);
    }

    void MainView::sendHandler() {
        std::string repr = this->encodingSendCombo->currentText().toLocal8Bit().data();
        std::string data = this->toSendEntry->text().toLocal8Bit().data();
        int repe = this->repetitionsSpin->value();
        int inte = this->periodSpin->value();
        auto reprId = representationIds.find(repr);
        if (reprId == representationIds.end()) {
            throw std::runtime_error{"Invalid key!"};
        }

        sendClickListener(reprId->second, data, repe, inte);
    }

    auto MainView::getPort() const -> std::string {
        return this->portCombo->currentText().toLocal8Bit().data();
    }

    auto MainView::getBaud() const -> int {
        return this->baudSpin->value();
    }

    auto MainView::getDataBits() const -> int {
        return this->dataBitsSpin->value();
    }

    auto MainView::getStopBits() const -> int {
        return this->stopBitsSpin->value();
    }

    auto MainView::getLinebreak() const -> int {
        return this->comboLinebreak->currentIndex();
    }

    auto MainView::getAsciiEnabled() const -> bool {
        return this->checkAscii->isChecked();
    }

    auto MainView::getHexEnabled() const -> bool {
        return this->checkHex->isChecked();
    }

    auto MainView::getDecEnabled() const -> bool {
        return this->checkDec->isChecked();
    }

    auto MainView::getBinEnabled() const -> bool {
        return this->checkBin->isChecked();
    }

    void MainView::setRxCount(int count) {
        listLock.lock();
        toCall.emplace_back(std::bind(&MainView::setRxCountImpl, this, count));
        listLock.unlock();
        QMetaObject::invokeMethod(this, "mainThreadHandler", Qt::QueuedConnection);
    }

    void MainView::setTxCount(int count) {
        listLock.lock();
        toCall.emplace_back(std::bind(&MainView::setTxCountImpl, this, count));
        listLock.unlock();
        QMetaObject::invokeMethod(this, "mainThreadHandler", Qt::QueuedConnection);
    }

    void MainView::setRxCountImpl(int count) {
        this->labelRxCount->setText(QStringLiteral("RX: %1").arg(count));
    }

    void MainView::setTxCountImpl(int count) {
        this->labelTxCount->setText(QStringLiteral("TX: %1").arg(count));
    }

    void MainView::setSerialOptionsVisibility(bool visible) {
        this->baudSpin->setVisible(visible);
        this->stopBitsSpin->setVisible(visible);
        this->dataBitsSpin->setVisible(visible);
        this->parityCombo->setVisible(visible);
        this->stopBitsLabel->setVisible(visible);
        this->dataBitsLabel->setVisible(visible);
    }

    void MainView::addSendImpl(std::string ascii, std::string dec, std::string hex, std::string bin, bool addNewLine) {
        sendWidgets.emplace_back(std::make_unique<ByteRepresentationWidget>(mainWindow, ascii, dec, bin, hex));
        sendWidgets.back()->setVisibilityBin(this->getBinEnabled());
        sendWidgets.back()->setVisibilityDec(this->getDecEnabled());
        sendWidgets.back()->setVisibilityHex(this->getHexEnabled());
        sendWidgets.back()->setVisibilityAscii(this->getAsciiEnabled());
        this->sendGrid->addLayout(sendWidgets.back().get(), sendPosition.second, sendPosition.first);

        sendPosition.first++;
        if (addNewLine) {
            sendPosition.first = 0;
            sendPosition.second++;
        } else if (sendPosition.first >= flowWidth) {
            sendPosition.first -= flowWidth;
            sendPosition.second++;
        }
    }

    void MainView::addReceivedImpl(std::string ascii, std::string dec, std::string hex, std::string bin, bool addNewLine) {
        receiveWidgets.emplace_back(std::make_unique<ByteRepresentationWidget>(mainWindow, ascii, dec, bin, hex));
        receiveWidgets.back()->setVisibilityBin(this->getBinEnabled());
        receiveWidgets.back()->setVisibilityDec(this->getDecEnabled());
        receiveWidgets.back()->setVisibilityHex(this->getHexEnabled());
        receiveWidgets.back()->setVisibilityAscii(this->getAsciiEnabled());
        this->receiveGrid->addLayout(receiveWidgets.back().get(), receivePosition.second, receivePosition.first);

        receivePosition.first++;
        if (addNewLine) {
            receivePosition.first = 0;
            receivePosition.second++;
        } else if (receivePosition.first >= flowWidth) {
            receivePosition.first -= flowWidth;
            receivePosition.second++;
        }
    }

    void MainView::clearReceived() {
        for (const auto &widget : this->receiveWidgets) {
            this->receiveGrid->removeItem(widget.get());
        }
        this->receiveWidgets.clear();
        this->receivePosition = std::make_pair(0,0);
    }

    void MainView::clearSent() {
        for (const auto &widget : this->sendWidgets) {
            this->sendGrid->removeItem(widget.get());
        }
        this->sendWidgets.clear();
        this->sendPosition = std::make_pair(0,0);
    }

    void MainView::showErrorImpl(std::string title, std::string message) {
        QMessageBox::critical(mainWindow.get(), title.c_str(), message.c_str(), QMessageBox::Ok);
    }

    void MainView::setVisibility(bool ascii, bool dec, bool hex, bool bin) {
        for (auto &widget : this->sendWidgets) {
            widget->setVisibilityAscii(ascii);
            widget->setVisibilityBin(bin);
            widget->setVisibilityDec(dec);
            widget->setVisibilityHex(hex);
        }

        for (auto &widget: this->receiveWidgets) {
            widget->setVisibilityAscii(ascii);
            widget->setVisibilityBin(bin);
            widget->setVisibilityDec(dec);
            widget->setVisibilityHex(hex);
        }
    }

    void MainView::addReceived(std::string ascii, std::string dec, std::string hex, std::string bin, bool addNewLine) {
        listLock.lock();
        toCall.emplace_back(std::bind(&MainView::addReceivedImpl, this, ascii, dec, hex, bin, addNewLine));
        listLock.unlock();
        QMetaObject::invokeMethod(this, "mainThreadHandler", Qt::QueuedConnection);
    }

    void MainView::addSend(std::string ascii, std::string dec, std::string hex, std::string bin, bool addNewLine) {
        listLock.lock();
        toCall.emplace_back(std::bind(&MainView::addSendImpl, this, ascii, dec, hex, bin, addNewLine));
        listLock.unlock();
        QMetaObject::invokeMethod(this, "mainThreadHandler", Qt::QueuedConnection);
    }

    void MainView::showError(std::string title, std::string message) {
        listLock.lock();
        toCall.emplace_back(std::bind(&MainView::showErrorImpl, this, title, message));
        listLock.unlock();
        QMetaObject::invokeMethod(this, "mainThreadHandler", Qt::QueuedConnection);
    }

    void MainView::mainThreadHandler() {
        if(this->listLock.try_lock()) {
            for (const auto &f : this->toCall) {
                f();
            }
            this->toCall.clear();
            listLock.unlock();
        }
    }
}