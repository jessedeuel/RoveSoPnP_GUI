#include "jobsPage.h"

#include "board.hpp"

jobsPage::jobsPage(std::shared_ptr<Components> board_instance, QWidget* parent) : QWidget(parent)
{
    m_pBoard_instance = board_instance;

    m_pLayout         = new QGridLayout();
    this->setLayout(m_pLayout);

    m_pTable      = new QTableWidget(0, COUNT);

    m_pEmptyLabel = new QLabel("No Job Uploaded", this);
    m_pEmptyLabel->setStyleSheet("color: red;");
    m_pEmptyLabel->setAlignment(Qt::AlignCenter);
    QFont font = m_pEmptyLabel->font();
    font.setPointSize(32);
    font.setBold(true);
    m_pEmptyLabel->setFont(font);

    QStringList horzHeaders;
    horzHeaders << "Qty" << "Value" << "Package" << "Pitch" << "Width" << "Orientation" << "X";
    m_pTable->setHorizontalHeaderLabels(horzHeaders);

    m_pUploadJobButton        = new QPushButton("Upload Job", this);
    m_pUpdateComponentsButton = new QPushButton("Update Components", this);
    m_pCurrentJobTextEdit     = new QTextEdit("", this);

    m_pPlacementMapSize       = new QLabel("Components to Place: ", this);
    m_pLostMapSize            = new QLabel("Unknown Cuttapes: ", this);

    QFontMetrics fm           = QFontMetrics(m_pCurrentJobTextEdit->font());
    int line_height           = fm.lineSpacing();
    int margins               = m_pCurrentJobTextEdit->document()->documentMargin() + m_pCurrentJobTextEdit->frameWidth() * 2;
    m_pCurrentJobTextEdit->setMaximumHeight(line_height + (int) margins * 2);

    QPixmap pix("../images/Cuttape.png");
    m_pCuttapeImage = new QLabel(this);
    m_pCuttapeImage->setPixmap(pix);
    m_pCuttapeImage->setFixedSize(pix.width() / 2.5, pix.height() / 2.5);
    m_pCuttapeImage->setScaledContents(true);

    m_pLayout->addWidget(m_pTable, 0, 0, 3, 2);
    m_pLayout->addWidget(m_pEmptyLabel, 0, 0, 3, 2);
    m_pLayout->addWidget(m_pCurrentJobTextEdit, 3, 0);
    m_pLayout->addWidget(m_pUploadJobButton, 3, 1);
    m_pLayout->addWidget(m_pUpdateComponentsButton, 3, 3);
    m_pLayout->addWidget(m_pCuttapeImage, 0, 3);
    m_pLayout->addWidget(m_pPlacementMapSize, 1, 3);
    m_pLayout->addWidget(m_pLostMapSize, 2, 3);

    m_pTable->setColumnWidth(AMOUNT, 30);
    m_pTable->setColumnWidth(PACKAGE, 325);
    m_pTable->setColumnWidth(REMOVE, 20);

    updatePage();

    connect(m_pUploadJobButton, &QPushButton::clicked, this, &jobsPage::onUploadJobButtonClicked);
    connect(m_pUpdateComponentsButton, &QPushButton::clicked, this, &jobsPage::onUpdateComponentsButtonClicked);

    qDebug() << "jobsPage initialized.";
}

jobsPage::~jobsPage()
{
    qDebug() << "jobsPage destroyed.";
}

void jobsPage::onUploadJobButtonClicked()
{
    qDebug("Upload Job Button clicked");
    const QStringList filters({"PnP Files (*.csv *.xlsx *.ods)", "All Files (*)"});
    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setDirectory("./");
    connect(&dialog,
            &QFileDialog::fileSelected,
            this,
            [&](const QString& filePath) mutable
            {
                qDebug() << "Selected file: " << filePath;
                m_pCurrentJobTextEdit->setPlainText(filePath);
            });

    if (dialog.exec() != QDialog::Accepted)
    {
        qDebug() << "Job file selection canceled by user.";
        return;
    }

    m_sJobFilePath = m_pCurrentJobTextEdit->toPlainText().toStdString();

    if (m_sJobFilePath.empty())
    {
        qDebug() << "Job File Path is empty!";
        return;
    }

    qDebug() << "Job File Path: |" << m_sJobFilePath.c_str() << "|";

    m_pBoard_instance->parseCSV(m_sJobFilePath.c_str());
    updatePage();
}

void jobsPage::updatePage()
{
    m_pTable->clearContents();
    Components::placement_map_t component_map = m_pBoard_instance->getPlacementMap();
    Components::lost_cuttape_map_t lost_map   = m_pBoard_instance->getLostCuttapes();

    int placement_map_size                    = 0;
    int row                                   = 0;
    m_pTable->setRowCount(component_map.size() + lost_map.size());
    for (Components::placement_map_t::iterator cuttape_it = component_map.begin(); cuttape_it != component_map.end(); cuttape_it++)
    {
        QTableWidgetItem* number_of_components = new QTableWidgetItem(QString::fromStdString(std::to_string(cuttape_it->second.size())));
        m_pTable->setItem(row, AMOUNT, number_of_components);

        QTableWidgetItem* value = new QTableWidgetItem(QString::fromStdString(std::get<0>(cuttape_it->first)));
        m_pTable->setItem(row, VALUE, value);

        QTableWidgetItem* package = new QTableWidgetItem(QString::fromStdString(std::get<1>(cuttape_it->first)));
        m_pTable->setItem(row, PACKAGE, package);

        QLineEdit* pitchLine = new QLineEdit();
        pitchLine->setText(QString::number(std::get<2>(cuttape_it->first).pitch, 'f', 3));
        m_pTable->setCellWidget(row, PITCH, pitchLine);

        QLineEdit* widthLine = new QLineEdit();
        widthLine->setText(QString::number(std::get<2>(cuttape_it->first).width, 'f', 3));
        m_pTable->setCellWidget(row, WIDTH, widthLine);

        QStringList orientations;
        orientations << "None" << "C1" << "C2" << "C3" << "C4" << "M1";
        QComboBox* orientationCombo = new QComboBox();
        orientationCombo->addItems(orientations);
        orientationCombo->setCurrentIndex(std::get<2>(cuttape_it->first).orient);
        m_pTable->setCellWidget(row, ORIENT, orientationCombo);

        QTableWidgetItem* removeCheck = new QTableWidgetItem();
        removeCheck->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        removeCheck->setCheckState(Qt::Unchecked);
        m_pTable->setItem(row, REMOVE, removeCheck);

        placement_map_size += cuttape_it->second.size();
        row++;
    }

    // Lost Cuttapes
    for (Components::lost_cuttape_map_t::iterator cuttape_it = lost_map.begin(); cuttape_it != lost_map.end(); cuttape_it++)
    {
        QTableWidgetItem* number_of_components = new QTableWidgetItem(QString::fromStdString(std::to_string(cuttape_it->second.size())));
        m_pTable->setItem(row, AMOUNT, number_of_components);

        QTableWidgetItem* value = new QTableWidgetItem(QString::fromStdString(std::get<0>(cuttape_it->first)));
        m_pTable->setItem(row, VALUE, value);

        QTableWidgetItem* package = new QTableWidgetItem(QString::fromStdString(std::get<1>(cuttape_it->first)));
        m_pTable->setItem(row, PACKAGE, package);

        QLineEdit* pitchLine = new QLineEdit();
        pitchLine->setPlaceholderText("Pitch (mm)");
        pitchLine->setStyleSheet("background-color: yellow;");
        m_pTable->setCellWidget(row, PITCH, pitchLine);

        QLineEdit* widthLine = new QLineEdit();
        widthLine->setPlaceholderText("Width (mm)");
        widthLine->setStyleSheet("background-color: yellow;");
        m_pTable->setCellWidget(row, WIDTH, widthLine);

        QStringList orientations;
        orientations << "None" << "C1" << "C2" << "C3" << "C4" << "M1";
        QComboBox* orientationCombo = new QComboBox();
        orientationCombo->addItems(orientations);
        orientationCombo->setStyleSheet("background-color: yellow;");
        m_pTable->setCellWidget(row, ORIENT, orientationCombo);

        row++;
    }

    m_pPlacementMapSize->setText(QString::fromStdString("Components to Place: " + std::to_string(placement_map_size)));
    m_pLostMapSize->setText(QString::fromStdString("Unknown Cuttapes: " + std::to_string((lost_map.size()))));

    if (row == 0)
    {
        m_pEmptyLabel->setVisible(true);
        m_pTable->setVisible(false);
    }
    else
    {
        m_pEmptyLabel->setVisible(false);
        m_pTable->setVisible(true);
    }
}

void jobsPage::onUpdateComponentsButtonClicked()
{
    Components::placement_map_t component_map = m_pBoard_instance->getPlacementMap();
    Components::lost_cuttape_map_t lost_map   = m_pBoard_instance->getLostCuttapes();

    int row                                   = 0;

    for (Components::placement_map_t::iterator cuttape_it = component_map.begin(); cuttape_it != component_map.end(); cuttape_it++)
    {
        float user_pitch     = (qobject_cast<QLineEdit*>(m_pTable->cellWidget(row, PITCH))->text()).toFloat();
        float user_width     = (qobject_cast<QLineEdit*>(m_pTable->cellWidget(row, WIDTH))->text()).toFloat();
        int user_orientation = (qobject_cast<QComboBox*>(m_pTable->cellWidget(row, ORIENT)))->currentIndex();
        bool isRemoved       = m_pTable->item(row, REMOVE)->checkState();

        row++;
        if (isRemoved)
        {
            m_pBoard_instance->removeFromPlacementMapToLostMap(get<Components::VAL>(cuttape_it->first),
                                                               get<Components::PKG>(cuttape_it->first),
                                                               get<Components::CTP>(cuttape_it->first));
        }
        else
        {
            if ((user_pitch != get<Components::CTP>(cuttape_it->first).pitch) || (user_width != get<Components::CTP>(cuttape_it->first).width) ||
                (user_orientation != get<Components::CTP>(cuttape_it->first).orient))
            {
                m_pBoard_instance->updateCuttapeInPlacementMap(get<Components::VAL>(cuttape_it->first),
                                                               get<Components::PKG>(cuttape_it->first),
                                                               get<Components::CTP>(cuttape_it->first),
                                                               {0, user_pitch, user_width, (orientation_t) user_orientation});
            }
        }
    }

    for (Components::lost_cuttape_map_t::iterator cuttape_it = lost_map.begin(); cuttape_it != lost_map.end(); cuttape_it++)
    {
        QString user_pitch      = qobject_cast<QLineEdit*>(m_pTable->cellWidget(row, PITCH))->text();
        QString user_width      = qobject_cast<QLineEdit*>(m_pTable->cellWidget(row, WIDTH))->text();
        QWidget* comboBoxWidget = m_pTable->cellWidget(row, ORIENT);
        int user_orientation    = (qobject_cast<QComboBox*>(comboBoxWidget))->currentIndex();

        row++;
        if (!user_pitch.isEmpty() && !user_pitch.isEmpty())
        {
            m_pBoard_instance->addLostCuttapeToPlacementMap(get<Components::VAL>(cuttape_it->first),
                                                            get<Components::PKG>(cuttape_it->first),
                                                            {0, user_pitch.toFloat(), user_width.toFloat(), (orientation_t) user_orientation});
        }
    }

    updatePage();
}
