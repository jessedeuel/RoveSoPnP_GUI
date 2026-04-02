#include "debugPnPTestPage.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QVBoxLayout>

debugPnPTestPage::debugPnPTestPage(std::shared_ptr<GRBL> grbl, QWidget* parent) : QWidget(parent), m_grbl(grbl)
{
    QGridLayout* mainLayout = new QGridLayout(this);

    // ==========================================
    // 1. Gantry Group
    // ==========================================
    QGroupBox* gantryGroup    = new QGroupBox("Gantry (X, Y, Z)", this);
    QGridLayout* gantryLayout = new QGridLayout(gantryGroup);

    m_pXSpin                  = new QDoubleSpinBox(this);
    m_pXSpin->setRange(-2000, 2000);
    m_pYSpin = new QDoubleSpinBox(this);
    m_pYSpin->setRange(-2000, 2000);
    m_pMoveXYBtn = new QPushButton("Move XY", this);

    m_pZSpin     = new QDoubleSpinBox(this);
    m_pZSpin->setRange(-2000, 2000);
    m_pMoveZBtn      = new QPushButton("Set Head Height (Z)", this);
    m_pHomeGantryBtn = new QPushButton("Home Gantry", this);

    gantryLayout->addWidget(new QLabel("X:"), 0, 0);
    gantryLayout->addWidget(m_pXSpin, 0, 1);
    gantryLayout->addWidget(new QLabel("Y:"), 0, 2);
    gantryLayout->addWidget(m_pYSpin, 0, 3);
    gantryLayout->addWidget(m_pMoveXYBtn, 0, 4);

    gantryLayout->addWidget(new QLabel("Z:"), 1, 0);
    gantryLayout->addWidget(m_pZSpin, 1, 1, 1, 3);
    gantryLayout->addWidget(m_pMoveZBtn, 1, 4);

    gantryLayout->addWidget(m_pHomeGantryBtn, 2, 0, 1, 5);
    mainLayout->addWidget(gantryGroup, 0, 0);

    // ==========================================
    // 2. Head Group
    // ==========================================
    QGroupBox* headGroup    = new QGroupBox("Head Controls", this);
    QGridLayout* headLayout = new QGridLayout(headGroup);

    m_pHeadAngleSpin        = new QSpinBox(this);
    m_pHeadAngleSpin->setRange(-720, 720);
    m_pRotateHeadBtn = new QPushButton("Increment Angle", this);
    m_pVacuumOnBtn   = new QPushButton("Vacuum ON", this);
    m_pVacuumOffBtn  = new QPushButton("Vacuum OFF", this);

    headLayout->addWidget(new QLabel("Angle:"), 0, 0);
    headLayout->addWidget(m_pHeadAngleSpin, 0, 1);
    headLayout->addWidget(m_pRotateHeadBtn, 0, 2);
    headLayout->addWidget(m_pVacuumOnBtn, 1, 0, 1, 2);
    headLayout->addWidget(m_pVacuumOffBtn, 1, 2);
    mainLayout->addWidget(headGroup, 0, 1);

    // ==========================================
    // 3. Feeder Group
    // ==========================================
    QGroupBox* feederGroup    = new QGroupBox("Feeder", this);
    QHBoxLayout* feederLayout = new QHBoxLayout(feederGroup);
    m_pFeederLengthSpin       = new QDoubleSpinBox(this);
    m_pFeederLengthSpin->setRange(0, 500);
    m_pFeedBtn = new QPushButton("Feed Length", this);
    feederLayout->addWidget(new QLabel("Length:"));
    feederLayout->addWidget(m_pFeederLengthSpin);
    feederLayout->addWidget(m_pFeedBtn);
    mainLayout->addWidget(feederGroup, 1, 0);

    // ==========================================
    // 4. LED Group
    // ==========================================
    QGroupBox* ledGroup    = new QGroupBox("LED Testing", this);
    QGridLayout* ledLayout = new QGridLayout(ledGroup);

    m_pLedColorCombo       = new QComboBox(this);
    m_pLedColorCombo->addItems({"RED", "ORANGE", "YELLOW", "GREEN", "BLUE", "PURPLE", "WHITE"});
    m_pLed1OnBtn  = new QPushButton("LED 1 ON", this);
    m_pLed1OffBtn = new QPushButton("LED 1 OFF", this);
    m_pLed2OnBtn  = new QPushButton("LED 2 ON", this);
    m_pLed2OffBtn = new QPushButton("LED 2 OFF", this);

    ledLayout->addWidget(new QLabel("Color:"), 0, 0);
    ledLayout->addWidget(m_pLedColorCombo, 0, 1, 1, 3);
    ledLayout->addWidget(m_pLed1OnBtn, 1, 0, 1, 2);
    ledLayout->addWidget(m_pLed1OffBtn, 1, 2, 1, 2);
    ledLayout->addWidget(m_pLed2OnBtn, 2, 0, 1, 2);
    ledLayout->addWidget(m_pLed2OffBtn, 2, 2, 1, 2);
    mainLayout->addWidget(ledGroup, 1, 1);

    // ==========================================
    // 5. FlowControl State Machine
    // ==========================================
    QGroupBox* stateGroup    = new QGroupBox("Flow Control State Machine", this);
    QHBoxLayout* stateLayout = new QHBoxLayout(stateGroup);
    m_pAdvanceCompBtn        = new QPushButton("Set Next Comp State", this);
    m_pTickStateBtn          = new QPushButton("Tick State Machine", this);
    stateLayout->addWidget(m_pAdvanceCompBtn);
    stateLayout->addWidget(m_pTickStateBtn);
    mainLayout->addWidget(stateGroup, 2, 0);    // Placed in the bottom left

    // ==========================================
    // 6. Live Values Group
    // ==========================================
    QGroupBox* valuesGroup    = new QGroupBox("Live Hardware Values", this);
    QVBoxLayout* valuesLayout = new QVBoxLayout(valuesGroup);

    m_pGantryPosLabel         = new QLabel("Gantry: Disconnected", this);

    // Add future labels (head, feeder) here when ready
    valuesLayout->addWidget(m_pGantryPosLabel);
    valuesLayout->addStretch();                  // Pushes the text to the top
    mainLayout->addWidget(valuesGroup, 2, 1);    // Placed in the bottom right

    // ==========================================
    // 7. Terminal Output Group
    // ==========================================
    QGroupBox* terminalGroup    = new QGroupBox("Serial Terminal", this);
    QVBoxLayout* terminalLayout = new QVBoxLayout(terminalGroup);

    m_pTerminalOutput           = new QPlainTextEdit(this);
    m_pTerminalOutput->setReadOnly(true);
    m_pTerminalOutput->setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: monospace; font-size: 14px;");
    terminalLayout->addWidget(m_pTerminalOutput);

    // Spans row 3, columns 0 through 1
    mainLayout->addWidget(terminalGroup, 3, 0, 1, 2);

    // Setup polling timer
    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, &debugPnPTestPage::onUpdateValues);

    // Wire the signal and slot together using a QueuedConnection for thread safety
    connect(this, &debugPnPTestPage::appendTerminalSignal, this, &debugPnPTestPage::onAppendTerminal, Qt::QueuedConnection);

    // Initialize hardware interfaces if GRBL is provided
    if (m_grbl)
    {
        m_gantry       = std::make_unique<Gantry>(m_grbl);
        m_head         = std::make_unique<Head>(m_grbl);
        m_feeder       = std::make_unique<Feeder>(m_grbl);
        m_led1         = std::make_unique<LED>(m_grbl);
        m_led2         = std::make_unique<LED>(m_grbl);

        m_pFlowControl = std::make_unique<FlowControl>(m_grbl, nullptr, nullptr);

        m_pUpdateTimer->start(200);

        // Attach a Lambda function to the GRBL comm instance to capture TX/RX
        m_grbl->setLogCallback(
            [this](const std::string& dir, const std::string& msg)
            {
                QString formattedMsg = QString("[%1] %2").arg(QString::fromStdString(dir), QString::fromStdString(msg));
                emit this->appendTerminalSignal(formattedMsg);
            });
    }

    // ==========================================
    // Signal/Slot Connections
    // ==========================================
    connect(m_pMoveXYBtn, &QPushButton::clicked, this, &debugPnPTestPage::onMoveXYClicked);
    connect(m_pMoveZBtn, &QPushButton::clicked, this, &debugPnPTestPage::onMoveZClicked);
    connect(m_pHomeGantryBtn, &QPushButton::clicked, this, &debugPnPTestPage::onHomeGantryClicked);
    connect(m_pRotateHeadBtn, &QPushButton::clicked, this, &debugPnPTestPage::onRotateHeadClicked);
    connect(m_pVacuumOnBtn, &QPushButton::clicked, this, &debugPnPTestPage::onVacuumOnClicked);
    connect(m_pVacuumOffBtn, &QPushButton::clicked, this, &debugPnPTestPage::onVacuumOffClicked);
    connect(m_pFeedBtn, &QPushButton::clicked, this, &debugPnPTestPage::onFeedClicked);
    connect(m_pLed1OnBtn, &QPushButton::clicked, this, &debugPnPTestPage::onLed1OnClicked);
    connect(m_pLed1OffBtn, &QPushButton::clicked, this, &debugPnPTestPage::onLed1OffClicked);
    connect(m_pLed2OnBtn, &QPushButton::clicked, this, &debugPnPTestPage::onLed2OnClicked);
    connect(m_pLed2OffBtn, &QPushButton::clicked, this, &debugPnPTestPage::onLed2OffClicked);
    connect(m_pAdvanceCompBtn, &QPushButton::clicked, this, &debugPnPTestPage::onAdvanceCompClicked);
    connect(m_pTickStateBtn, &QPushButton::clicked, this, &debugPnPTestPage::onTickStateClicked);
}

debugPnPTestPage::~debugPnPTestPage() {}

bool debugPnPTestPage::checkConnection()
{
    if (!m_grbl)
    {
        QMessageBox::warning(this, "Not Connected", "Hardware was not initialized properly.");
        return false;
    }
    return true;
}

void debugPnPTestPage::onMoveXYClicked()
{
    if (!checkConnection())
        return;
    m_gantry->setGlobalPosition(m_pXSpin->value(), m_pYSpin->value());
}

void debugPnPTestPage::onMoveZClicked()
{
    if (!checkConnection())
        return;
    m_gantry->setHeadHeight(m_pZSpin->value());
}

void debugPnPTestPage::onHomeGantryClicked()
{
    if (!checkConnection())
        return;
    m_gantry->home();
}

void debugPnPTestPage::onRotateHeadClicked()
{
    if (!checkConnection())
        return;
    m_head->increment(m_pHeadAngleSpin->value());
}

void debugPnPTestPage::onVacuumOnClicked()
{
    if (!checkConnection())
        return;
    m_head->vacuumOn();
}

void debugPnPTestPage::onVacuumOffClicked()
{
    if (!checkConnection())
        return;
    m_head->vacuumOff();
}

void debugPnPTestPage::onFeedClicked()
{
    if (!checkConnection())
        return;
    m_feeder->increment(m_pFeederLengthSpin->value());
}

void debugPnPTestPage::onLed1OnClicked()
{
    if (!checkConnection())
        return;
    m_led1->setColor(static_cast<LED_COLOR>(m_pLedColorCombo->currentIndex()));
    m_led1->setOn();
}

void debugPnPTestPage::onLed1OffClicked()
{
    if (!checkConnection())
        return;
    m_led1->setOff();
}

void debugPnPTestPage::onLed2OnClicked()
{
    if (!checkConnection())
        return;
    m_led2->setColor(static_cast<LED_COLOR>(m_pLedColorCombo->currentIndex()));
    m_led2->setOn();
}

void debugPnPTestPage::onLed2OffClicked()
{
    if (!checkConnection())
        return;
    m_led2->setOff();
}

void debugPnPTestPage::onAdvanceCompClicked()
{
    if (!checkConnection() || !m_pFlowControl)
        return;

    // Set the state directly using the new FlowState enum
    m_pFlowControl->setState(FlowState::GET_NEXT_COMPONENT);
    qDebug() << "Advanced Component. State machine set to GET_NEXT_COMPONENT";
}

void debugPnPTestPage::onTickStateClicked()
{
    if (!checkConnection() || !m_pFlowControl)
        return;

    m_pFlowControl->tickStateMachine();
    qDebug() << "Tick completed. Current State:" << static_cast<int>(m_pFlowControl->getState());
}

void debugPnPTestPage::onUpdateValues()
{
    // Polled periodically by m_pUpdateTimer
    if (m_gantry)
    {
        // Assuming getGlobalPosition returns a struct with .x and .y
        // Change pos.x/pos.y to pos.first/pos.second if returning std::pair
        auto pos = m_gantry->getGlobalPosition();
        m_pGantryPosLabel->setText(QString("Gantry X: %1  |  Y: %2").arg(pos.x, 0, 'f', 2).arg(pos.y, 0, 'f', 2));
    }
}

void debugPnPTestPage::onAppendTerminal(const QString& text)
{
    // Append the text to the UI
    m_pTerminalOutput->appendPlainText(text);

    // Auto-scroll to the bottom
    QScrollBar* bar = m_pTerminalOutput->verticalScrollBar();
    bar->setValue(bar->maximum());
}
