#include "operatorPage.h"
#include "flowControl.h"
#include <QApplication>
#include <QFont>
#include <QFrame>
#include <QStyle>

OperatorPage::OperatorPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setMachineStateToSetup();    // Default state
}

OperatorPage::~OperatorPage() {}

void OperatorPage::bindFlowControl(FlowControl* fc)
{
    if (!fc)
        return;

    // Connect Backend Signals -> UI Slots
    connect(fc, &FlowControl::requestCameraFrameUpdate, this, &OperatorPage::updateCameraFrame);
    connect(fc, &FlowControl::sendLogMessage, this, &OperatorPage::logMessage);

    // Listen for backend dynamically requesting the UI to allow a fiducial lock
    connect(fc, &FlowControl::requestUserFiducialLock, this, &OperatorPage::promptFiducialAlignment);

    // Connect UI Signals -> Backend interactions
    connect(this, &OperatorPage::requestSetFiducial, fc, [fc](int index) { fc->setUserConfirmsPosition(true); });

    connect(this, &OperatorPage::requestStartJob, fc, &FlowControl::startJob);
    connect(this, &OperatorPage::requestPauseJob, fc, &FlowControl::pauseJob);
    connect(this, &OperatorPage::requestStopJob, fc, &FlowControl::stopJob);
    connect(this, &OperatorPage::requestJog, fc, &FlowControl::jogMachine);
}

void OperatorPage::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    createVisionPanel(mainLayout);
    createControlPanel(mainLayout);
}

void OperatorPage::createVisionPanel(QHBoxLayout* mainLayout)
{
    QVBoxLayout* visionLayout = new QVBoxLayout();

    // Header for the camera
    cameraStatusLabel = new QLabel("Active View: Waiting for camera...", this);
    QFont headerFont  = cameraStatusLabel->font();
    headerFont.setPointSize(14);
    headerFont.setBold(true);
    cameraStatusLabel->setFont(headerFont);
    cameraStatusLabel->setStyleSheet("color: #2c3e50;");

    // The actual video feed display
    cameraFeedLabel = new QLabel("NO SIGNAL", this);
    cameraFeedLabel->setAlignment(Qt::AlignCenter);
    cameraFeedLabel->setStyleSheet(
        "QLabel { background-color: #000000; color: #ff0000; font-weight: bold; font-size: 24px; border: 2px solid #7f8c8d; border-radius: 5px; }");
    cameraFeedLabel->setMinimumSize(640, 480);
    cameraFeedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    visionLayout->addWidget(cameraStatusLabel);
    visionLayout->addWidget(cameraFeedLabel);

    // Assign 60% of the screen width to the vision layout
    mainLayout->addLayout(visionLayout, 6);
}

void OperatorPage::createControlPanel(QHBoxLayout* mainLayout)
{
    QVBoxLayout* controlLayout = new QVBoxLayout();

    modeStackedWidget          = new QStackedWidget(this);
    modeStackedWidget->addWidget(createSetupPage());            // Index 0
    modeStackedWidget->addWidget(createRunPage());              // Index 1
    modeStackedWidget->addWidget(createTapeSwapPage());         // Index 2
    modeStackedWidget->addWidget(createFiducialAlignPage());    // Index 3

    jogGroupBox = createJogPanel();

    controlLayout->addWidget(modeStackedWidget, 1);
    controlLayout->addWidget(jogGroupBox, 0);    // Fixed size at bottom

    // Assign 40% of the screen width to the control layout
    mainLayout->addLayout(controlLayout, 4);
}

QWidget* OperatorPage::createSetupPage()
{
    QWidget* page       = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* setupGroup    = new QGroupBox("Job Ready");
    QVBoxLayout* groupLayout = new QVBoxLayout(setupGroup);

    QLabel* instructionLabel = new QLabel("Ensure the board is securely loaded in the tray.\n\nWhen ready, click START JOB. The machine will automatically home, then "
                                          "prompt you to align the camera for each fiducial marker.");
    instructionLabel->setWordWrap(true);

    QFont font = instructionLabel->font();
    font.setPointSize(11);
    instructionLabel->setFont(font);

    QPushButton* btnStartJob = new QPushButton("START JOB");
    btnStartJob->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; font-weight: bold; font-size: 16px; padding: 20px; border-radius: 5px; } QPushButton:hover { "
        "background-color: #2ecc71; }");

    connect(btnStartJob, &QPushButton::clicked, this, &OperatorPage::onStartClicked);

    groupLayout->addWidget(instructionLabel);
    groupLayout->addStretch();
    groupLayout->addWidget(btnStartJob);

    layout->addWidget(setupGroup);
    return page;
}

QWidget* OperatorPage::createRunPage()
{
    QWidget* page       = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* runGroup      = new QGroupBox("Running Job Status");
    QVBoxLayout* groupLayout = new QVBoxLayout(runGroup);

    currentTaskLabel         = new QLabel("Status: Idle");
    QFont font               = currentTaskLabel->font();
    font.setPointSize(12);
    font.setBold(true);
    currentTaskLabel->setFont(font);
    currentTaskLabel->setWordWrap(true);

    jobProgressBar = new QProgressBar();
    jobProgressBar->setRange(0, 100);
    jobProgressBar->setValue(0);
    jobProgressBar->setTextVisible(true);

    runLogList = new QListWidget();
    runLogList->setStyleSheet("background-color: #ecf0f1; border: 1px solid #bdc3c7;");

    QHBoxLayout* btnLayout = new QHBoxLayout();
    pauseBtn               = new QPushButton("PAUSE");
    QPushButton* stopBtn   = new QPushButton("STOP / ABORT");

    pauseBtn->setStyleSheet("background-color: #f39c12; color: white; font-weight: bold; padding: 10px;");
    stopBtn->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 10px;");

    connect(pauseBtn, &QPushButton::clicked, this, &OperatorPage::onPauseClicked);
    connect(stopBtn, &QPushButton::clicked, this, &OperatorPage::onStopClicked);

    btnLayout->addWidget(pauseBtn);
    btnLayout->addWidget(stopBtn);

    groupLayout->addWidget(currentTaskLabel);
    groupLayout->addWidget(jobProgressBar);
    groupLayout->addWidget(new QLabel("Action Log:"));
    groupLayout->addWidget(runLogList);
    groupLayout->addLayout(btnLayout);

    layout->addWidget(runGroup);
    return page;
}

QWidget* OperatorPage::createFiducialAlignPage()
{
    QWidget* page       = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* alignGroup = new QGroupBox("User Action Required: Vision Alignment");
    alignGroup->setStyleSheet("QGroupBox { border: 2px solid #8e44ad; border-radius: 5px; margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; left: 10px; "
                              "color: #8e44ad; font-weight: bold; }");
    QVBoxLayout* groupLayout = new QVBoxLayout(alignGroup);

    fiducialPromptLabel      = new QLabel("Please jog the gantry to center the camera over the target Fiducial.");
    fiducialPromptLabel->setWordWrap(true);

    QFont font = fiducialPromptLabel->font();
    font.setPointSize(12);
    font.setBold(true);
    fiducialPromptLabel->setFont(font);

    btnLockDynamicFiducial = new QPushButton("Lock Target");
    btnLockDynamicFiducial->setStyleSheet(
        "QPushButton { background-color: #8e44ad; color: white; font-weight: bold; font-size: 16px; padding: 15px; border-radius: 5px; } "
        "QPushButton:hover { background-color: #9b59b6; }");

    connect(btnLockDynamicFiducial, &QPushButton::clicked, this, &OperatorPage::onDynamicFiducialLocked);

    groupLayout->addWidget(fiducialPromptLabel);
    groupLayout->addStretch();
    groupLayout->addWidget(btnLockDynamicFiducial);

    layout->addWidget(alignGroup);
    return page;
}

QWidget* OperatorPage::createTapeSwapPage()
{
    QWidget* page       = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* swapGroup = new QGroupBox("ATTENTION: Tape Swap Required");
    swapGroup->setStyleSheet("QGroupBox { border: 2px solid #e74c3c; border-radius: 5px; margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; left: 10px; "
                             "color: #e74c3c; font-weight: bold; }");
    QVBoxLayout* groupLayout  = new QVBoxLayout(swapGroup);

    tapeSwapInstructionsLabel = new QLabel("Machine Paused.\n\nPlease load component into the feeder.");
    tapeSwapInstructionsLabel->setWordWrap(true);
    QFont font = tapeSwapInstructionsLabel->font();
    font.setPointSize(12);
    tapeSwapInstructionsLabel->setFont(font);

    QLabel* tipLabel = new QLabel("<i>Tip: Use the jog controls below to fine-tune the tape position so the first component is aligned with the gantry camera.</i>");
    tipLabel->setWordWrap(true);

    QPushButton* btnConfirmSwap = new QPushButton("Tape Loaded & Aligned - Resume Job");
    btnConfirmSwap->setStyleSheet("QPushButton { background-color: #2980b9; color: white; font-weight: bold; padding: 15px; border-radius: 5px; } QPushButton:hover { "
                                  "background-color: #3498db; }");

    connect(btnConfirmSwap, &QPushButton::clicked, this, &OperatorPage::onTapeSwapConfirmed);

    groupLayout->addWidget(tapeSwapInstructionsLabel);
    groupLayout->addSpacing(15);
    groupLayout->addWidget(tipLabel);
    groupLayout->addStretch();
    groupLayout->addWidget(btnConfirmSwap);

    layout->addWidget(swapGroup);
    return page;
}

QGroupBox* OperatorPage::createJogPanel()
{
    QGroupBox* group        = new QGroupBox("Manual Jog Controls");
    QVBoxLayout* mainLayout = new QVBoxLayout(group);

    QHBoxLayout* stepLayout = new QHBoxLayout();
    stepLayout->addWidget(new QLabel("Step Size (mm):"));
    jogStepSpinBox = new QDoubleSpinBox();
    jogStepSpinBox->setRange(0.01, 100.0);
    jogStepSpinBox->setValue(1.0);
    jogStepSpinBox->setSingleStep(0.1);
    stepLayout->addWidget(jogStepSpinBox);
    stepLayout->addStretch();
    mainLayout->addLayout(stepLayout);

    QGridLayout* grid = new QGridLayout();

    // Helper lambda to create directional buttons
    auto createBtn = [this](const QString& text, const QString& axis, double dirMultiplier)
    {
        QPushButton* btn = new QPushButton(text);
        btn->setMinimumSize(50, 50);
        connect(btn, &QPushButton::clicked, this, [this, axis, dirMultiplier]() { emit requestJog(axis, jogStepSpinBox->value() * dirMultiplier); });
        return btn;
    };

    // Y Axis
    grid->addWidget(createBtn("Y+", "Y", 1.0), 0, 1);
    grid->addWidget(createBtn("Y-", "Y", -1.0), 2, 1);
    // X Axis
    grid->addWidget(createBtn("X-", "X", -1.0), 1, 0);
    grid->addWidget(createBtn("X+", "X", 1.0), 1, 2);
    // Z Axis (Vertical Layout to the side)
    grid->addWidget(createBtn("Z+ (Up)", "Z", 1.0), 0, 4);
    grid->addWidget(createBtn("Z- (Dn)", "Z", -1.0), 2, 4);
    // Rotation Axis
    grid->addWidget(createBtn("Rot CCW", "A", -1.0), 0, 5);
    grid->addWidget(createBtn("Rot CW", "A", 1.0), 2, 5);

    // Spacer between XY pad and Z/Rot pad
    grid->setColumnMinimumWidth(3, 20);

    mainLayout->addLayout(grid);
    return group;
}

// --- Public Slots for Backend Integration ---

void OperatorPage::updateCameraFrame(const QImage& image, const QString& activeCameraName)
{
    if (!image.isNull())
    {
        cameraFeedLabel->setPixmap(QPixmap::fromImage(image).scaled(cameraFeedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    cameraStatusLabel->setText(QString("Active View: %1").arg(activeCameraName));
}

void OperatorPage::setMachineStateToSetup()
{
    modeStackedWidget->setCurrentIndex(SetupMode);
    jogGroupBox->setEnabled(true);
}

void OperatorPage::setMachineStateToRun()
{
    modeStackedWidget->setCurrentIndex(RunMode);
    jogGroupBox->setEnabled(false);    // Disable jogging during automated run
    logMessage("Job Started.");
}

void OperatorPage::updateRunStatus(const QString& currentTask, int progressPercentage)
{
    currentTaskLabel->setText(QString("Status: %1").arg(currentTask));
    jobProgressBar->setValue(progressPercentage);
    logMessage(currentTask);
}

void OperatorPage::promptFiducialAlignment(int fiducialIndex)
{
    // The state machine needs help. Unlock the UI and show the prompt.
    modeStackedWidget->setCurrentIndex(FiducialAlignMode);
    jogGroupBox->setEnabled(true);

    fiducialPromptLabel->setText(
        QString("Please jog the gantry to center the camera directly over Fiducial #%1.\n\nWhen you are satisfied with the alignment, press Lock.").arg(fiducialIndex));
    logMessage(QString("Waiting for operator to align Fiducial #%1...").arg(fiducialIndex));
}

void OperatorPage::triggerTapeSwapRequired(const QString& componentName, const QString& packageType)
{
    // Switch UI to Tape Swap Mode
    modeStackedWidget->setCurrentIndex(TapeSwapMode);

    // Enable jogging so the operator can align the new tape visually
    jogGroupBox->setEnabled(true);

    QString msg = QString("Machine Paused.\n\nPlease remove empty tape and load:\n\nComponent: %1\nPackage: %2").arg(componentName).arg(packageType);
    tapeSwapInstructionsLabel->setText(msg);
    logMessage(QString("Tape swap required for %1").arg(componentName));
}

void OperatorPage::logMessage(const QString& msg)
{
    runLogList->addItem(msg);
    runLogList->scrollToBottom();
}

// --- Internal Handlers ---

void OperatorPage::onStartClicked()
{
    setMachineStateToRun();
    emit requestStartJob();
}

void OperatorPage::onPauseClicked()
{
    if (pauseBtn->text() == "PAUSE")
    {
        pauseBtn->setText("RESUME");
        jogGroupBox->setEnabled(true);    // Allow jog while paused
        logMessage("Job Paused by operator.");
        emit requestPauseJob();
    }
    else
    {
        pauseBtn->setText("PAUSE");
        jogGroupBox->setEnabled(false);
        logMessage("Job Resumed.");
        emit requestStartJob();    // or requestResumeJob() depending on your flow control
    }
}

void OperatorPage::onStopClicked()
{
    setMachineStateToSetup();
    logMessage("Job Stopped/Aborted.");
    emit requestStopJob();
}

void OperatorPage::onDynamicFiducialLocked()
{
    // Temporarily go back to standard "Run Mode" visual while the machine moves or detects
    modeStackedWidget->setCurrentIndex(RunMode);
    jogGroupBox->setEnabled(false);

    // Pass the confirmation to the backend
    emit requestSetFiducial(0);
}

void OperatorPage::onTapeSwapConfirmed()
{
    setMachineStateToRun();
    emit requestTapeSwapConfirmation();
    logMessage("Tape swap confirmed. Resuming job...");
}
