#include "mainwindow.h"
#include "task.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDate>
#include <QFrame>
#include <QApplication>
#include <QFont>
#include <QScrollBar>
#include <QTabWidget>
#include <QListWidget>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <QScreen>
#include <QGuiApplication>

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#define POL_MOBILE 1
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), taskManager(new TaskManager()) {
    setWindowTitle("Выполнение задач по дням");
#ifdef POL_MOBILE
    setMinimumSize(320, 480);
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        resize(geo.width(), geo.height());
    } else {
        resize(360, 640);
    }
#else
    setMinimumSize(800, 600);
    resize(1000, 700);
#endif

    // Глобальные стили — фиолетовая тема
    QString styleSheet = R"(
        QMainWindow { background-color: #f5f3ff; }
        QWidget { background-color: #f5f3ff; font-family: 'Segoe UI', sans-serif; }

        QFrame#headerFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #7c3aed, stop:1 #6d28d9);
            border-radius: 12px;
            padding: 16px;
        }
        QLabel#dateLabel {
            color: #f5f3ff;
            font-size: 16pt;
            font-weight: bold;
            letter-spacing: 0.5px;
        }
        QDateEdit#dateSelector {
            background-color: rgba(255,255,255,0.95);
            color: #0d0d0d;
            border: none;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 12pt;
            min-width: 120px;
        }
        QDateEdit#dateSelector:hover { background-color: white; }
        QDateEdit#dateSelector::drop-down {
            border: none;
            background: transparent;
        }

        QTableWidget#tasksTable {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #c4b5fd;
            gridline-color: #ddd6fe;
            padding: 4px;
            alternate-background-color: #faf5ff;
        }
        QTableWidget#tasksTable::item {
            padding: 12px 8px;
            font-size: 11pt;
            color: #0d0d0d;
        }
        QTableWidget#tasksTable::item:selected {
            background-color: #ede9fe;
            color: #0d0d0d;
        }
        QHeaderView::section {
            background: #ede9fe;
            color: #0d0d0d;
            padding: 14px 8px;
            border: none;
            border-bottom: 2px solid #c4b5fd;
            border-right: 1px solid #ddd6fe;
            font-weight: bold;
            font-size: 11pt;
        }
        QHeaderView::section:first { border-top-left-radius: 8px; }
        QHeaderView::section:last { border-top-right-radius: 8px; }
        QTableWidget#tasksTable QTableCornerButton::section {
            background: #ede9fe;
            border-top-left-radius: 8px;
        }

        QPushButton#addButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #8b5cf6, stop:1 #7c3aed);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 14px 28px;
            font-size: 13pt;
            font-weight: bold;
        }
        QPushButton#addButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #a78bfa, stop:1 #8b5cf6);
        }
        QPushButton#addButton:pressed {
            background: #6d28d9;
        }

        QScrollBar:vertical {
            background: #ede9fe;
            width: 10px;
            border-radius: 5px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #c4b5fd;
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #a78bfa; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )";
    setStyleSheet(styleSheet);

    // Загружаем задачи из файла
    taskManager->loadFromFile();

    setupUI();
    updateDailyTasks();
}

MainWindow::~MainWindow() {
    // Сохраняем задачи перед закрытием
    taskManager->saveToFile();
    delete taskManager;
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    if (isMobile()) {
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(10, 10, 10, 10);
    } else {
        mainLayout->setSpacing(12);
        mainLayout->setContentsMargins(16, 16, 16, 16);
    }

    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->tabBar()->setExpanding(true);
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #c4b5fd;
            border-radius: 12px;
            background: #ffffff;
            top: -1px;
        }
        QTabBar::tab {
            background: #ede9fe;
            color: #5b21b6;
            padding: 14px 20px;
            margin-right: 2px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            font-weight: bold;
            font-size: 12pt;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background: #ffffff;
            color: #6d28d9;
            border: 1px solid #c4b5fd;
            border-bottom: none;
        }
        QTabBar::tab:hover:!selected {
            background: #ddd6fe;
            color: #5b21b6;
        }
    )");

    setupTasksTab();
    setupEnglishTab();
    setupPrayerTab();

    mainLayout->addWidget(tabWidget);
}

void MainWindow::setupTasksTab() {
    QWidget* tasksPage = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tasksPage);
    int pad = isMobile() ? 10 : 16;
    layout->setSpacing(pad);
    layout->setContentsMargins(pad, pad, pad, pad);

    // Блок геймификации — синий
    QFrame* gameFrame = new QFrame(this);
    gameFrame->setObjectName("headerFrame");
    gameFrame->setStyleSheet("QFrame#headerFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #2563eb, stop:1 #1d4ed8); border-radius: 12px; padding: 12px; }");
    QHBoxLayout* gameLayout = new QHBoxLayout(gameFrame);
    levelLabel = new QLabel(this);
    levelLabel->setStyleSheet("color: #0d0d0d; font-size: 14pt; font-weight: bold;");
    xpBar = new QProgressBar(this);
    xpBar->setTextVisible(true);
    xpBar->setFormat("%v / %m XP");
    xpBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #1e40af;
            border-radius: 8px;
            background: rgba(255,255,255,0.85);
            text-align: center;
            color: #0d0d0d;
            font-weight: bold;
            font-size: 11pt;
        }
        QProgressBar::chunk { background: #93c5fd; border-radius: 6px; }
    )");
    xpBar->setMinimum(0);
    streakLabel = new QLabel(this);
    streakLabel->setStyleSheet("color: #0d0d0d; font-size: 12pt; font-weight: bold;");
    gameLayout->addWidget(levelLabel);
    gameLayout->addWidget(xpBar, 1);
    gameLayout->addWidget(streakLabel);
    layout->addWidget(gameFrame);

    // Заголовок и выбор даты
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setSpacing(16);

    dateLabel = new QLabel("Задачи на:", this);
    dateLabel->setObjectName("dateLabel");

    dateSelector = new QDateEdit(this);
    dateSelector->setObjectName("dateSelector");
    dateSelector->setDate(QDate::currentDate());
    dateSelector->setCalendarPopup(true);
    dateSelector->setDisplayFormat("dd.MM.yyyy");
    connect(dateSelector, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);

    headerLayout->addWidget(dateLabel);
    headerLayout->addWidget(dateSelector);
    headerLayout->addStretch();

    layout->addWidget(headerFrame);

    tasksTable = new QTableWidget(this);
    tasksTable->setObjectName("tasksTable");
    tasksTable->setColumnCount(5);
    tasksTable->setHorizontalHeaderLabels(QStringList()
        << "✓"
        << "Название"
        << "Описание"
        << "Приоритет"
        << "Категория");
    tasksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tasksTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tasksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tasksTable->setAlternatingRowColors(true);
    tasksTable->setShowGrid(true);
    tasksTable->verticalHeader()->setVisible(false);
    tasksTable->horizontalHeader()->setStretchLastSection(true);
    tasksTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tasksTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tasksTable->horizontalHeader()->setMinimumSectionSize(80);
    connect(tasksTable, &QTableWidget::cellChanged,
            this, &MainWindow::onTaskStatusChanged);

    layout->addWidget(tasksTable, 1);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("➕ Добавить задачу", this);
    addButton->setObjectName("addButton");
    addButton->setCursor(Qt::PointingHandCursor);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddTask);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    tabWidget->addTab(tasksPage, "📋 Задачи");
    refreshGameWidget();
}

void MainWindow::setupEnglishTab() {
    QWidget* englishPage = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(englishPage);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    QLabel* levelTitle = new QLabel("Уровень", this);
    levelTitle->setStyleSheet("font-weight: bold; font-size: 12pt; color: #0d0d0d;");
    englishLevelCombo = new QComboBox(this);
    for (int i = 0; i < EnglishData::LEVEL_COUNT; i++) {
        englishLevelCombo->addItem(EnglishData::levelName(i));
    }
    englishLevelCombo->setStyleSheet("QComboBox { background: white; color: #0d0d0d; border-radius: 8px; border: 1px solid #c4b5fd; padding: 8px; }");
    connect(englishLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEnglishLevelChanged);

    QLabel* lessonTitle = new QLabel("Уроки (1 — 50)", this);
    lessonTitle->setStyleSheet("font-weight: bold; font-size: 12pt; color: #0d0d0d;");
    englishLessonList = new QListWidget(this);
    englishLessonList->setStyleSheet("QListWidget { background: white; color: #0d0d0d; border-radius: 8px; border: 1px solid #c4b5fd; padding: 4px; }");
    for (int i = 1; i <= EnglishData::LESSONS_PER_LEVEL; i++) {
        englishLessonList->addItem(QString::number(i));
    }
    englishLessonList->setCurrentRow(0);
    connect(englishLessonList, &QListWidget::currentRowChanged, this, &MainWindow::onEnglishLessonSelected);
    leftLayout->addWidget(levelTitle);
    leftLayout->addWidget(englishLevelCombo);
    leftLayout->addWidget(lessonTitle);
    leftLayout->addWidget(englishLessonList);
    splitter->addWidget(leftPanel);

    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    QLabel* wordsTitle = new QLabel("Слова урока", this);
    wordsTitle->setStyleSheet("font-weight: bold; font-size: 12pt; color: #0d0d0d;");
    englishWordsTable = new QTableWidget(this);
    englishWordsTable->setColumnCount(2);
    englishWordsTable->setHorizontalHeaderLabels(QStringList() << "Слово" << "Перевод");
    englishWordsTable->horizontalHeader()->setStretchLastSection(true);
    englishWordsTable->setStyleSheet("QTableWidget { background: white; color: #0d0d0d; border-radius: 8px; border: 1px solid #e2e8f0; }");
    rightLayout->addWidget(wordsTitle);

    QHBoxLayout* addRowLayout = new QHBoxLayout();
    englishWordEdit = new QLineEdit(this);
    englishWordEdit->setPlaceholderText("Слово");
    englishTranslationEdit = new QLineEdit(this);
    englishTranslationEdit->setPlaceholderText("Перевод");
    englishAddWordBtn = new QPushButton("➕ Добавить", this);
    englishAddWordBtn->setStyleSheet("QPushButton { background: #4c6ef5; color: white; border-radius: 8px; padding: 8px 16px; font-weight: bold; }");
    englishRemoveWordBtn = new QPushButton("Удалить", this);
    englishRemoveWordBtn->setStyleSheet("QPushButton { background: #e2e8f0; color: #0d0d0d; border-radius: 8px; padding: 8px 16px; }");
    addRowLayout->addWidget(englishWordEdit);
    addRowLayout->addWidget(englishTranslationEdit);
    addRowLayout->addWidget(englishAddWordBtn);
    addRowLayout->addWidget(englishRemoveWordBtn);
    rightLayout->addLayout(addRowLayout);
    rightLayout->addWidget(englishWordsTable, 1);
    connect(englishAddWordBtn, &QPushButton::clicked, this, &MainWindow::onEnglishAddWord);
    connect(englishRemoveWordBtn, &QPushButton::clicked, this, &MainWindow::onEnglishRemoveWord);

    splitter->addWidget(rightPanel);
    splitter->setSizes(QList<int>() << 140 << 400);
    mainLayout->addWidget(splitter);
    tabWidget->addTab(englishPage, "🇬🇧 Английский");
    onEnglishLessonSelected(0);
}

void MainWindow::onEnglishLevelChanged(int) {
    englishLessonList->setCurrentRow(0);
    onEnglishLessonSelected(0);
}

void MainWindow::setupPrayerTab() {
    QWidget* prayerPage = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(prayerPage);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    QHBoxLayout* imageRow = new QHBoxLayout();
    prayerImageLabel = new QLabel(this);
    prayerImageLabel->setMinimumSize(200, 150);
    prayerImageLabel->setMaximumSize(500, 220);
    prayerImageLabel->setAlignment(Qt::AlignCenter);
    prayerImageLabel->setStyleSheet("QLabel { background: #f8fafc; border: 2px dashed #cbd5e1; border-radius: 12px; color: #0d0d0d; font-size: 11pt; }");
    prayerImageLabel->setText("Изображение для молитвы\n(добавьте фото)");
    prayerImageLabel->setScaledContents(false);
    prayerAddImageBtn = new QPushButton("🖼 Добавить изображение", this);
    prayerAddImageBtn->setStyleSheet("QPushButton { background: #475569; color: #f8fafc; border-radius: 8px; padding: 10px 20px; font-weight: bold; } QPushButton:hover { background: #334155; }");
    connect(prayerAddImageBtn, &QPushButton::clicked, this, &MainWindow::onPrayerAddImage);
    imageRow->addWidget(prayerImageLabel, 1);
    imageRow->addWidget(prayerAddImageBtn);
    mainLayout->addLayout(imageRow);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    QLabel* gospelLabel = new QLabel("Евангелие", this);
    gospelLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #0d0d0d;");
    prayerGospelCombo = new QComboBox(this);
    prayerGospelCombo->addItem("Евангелие от Марка", 16);
    prayerGospelCombo->addItem("Евангелие от Матфея", 28);
    prayerGospelCombo->addItem("Евангелие от Луки", 24);
    prayerGospelCombo->addItem("Евангелие от Иоанна", 21);
    prayerGospelCombo->setStyleSheet("QComboBox { background: white; color: #0d0d0d; border-radius: 8px; border: 1px solid #c4b5fd; padding: 8px; }");
    connect(prayerGospelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPrayerGospelChanged);

    QLabel* chLabel = new QLabel("Главы", this);
    chLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #1e293b;");
    prayerChapterList = new QListWidget(this);
    prayerChapterList->setStyleSheet("QListWidget { background: white; color: #1e293b; border-radius: 8px; border: 1px solid #cbd5e1; padding: 4px; }");
    connect(prayerChapterList, &QListWidget::currentRowChanged, this, &MainWindow::onPrayerChapterSelected);

    leftLayout->addWidget(gospelLabel);
    leftLayout->addWidget(prayerGospelCombo);
    leftLayout->addWidget(chLabel);
    leftLayout->addWidget(prayerChapterList);
    splitter->addWidget(leftPanel);

    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    QLabel* textLabel = new QLabel("Текст главы", this);
    textLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #0d0d0d;");
    prayerChapterText = new QTextEdit(this);
    prayerChapterText->setReadOnly(true);
    prayerChapterText->setStyleSheet("QTextEdit { background: #faf5ff; color: #0d0d0d; border-radius: 8px; border: 1px solid #c4b5fd; padding: 12px; font-size: 11pt; }");
    rightLayout->addWidget(textLabel);
    rightLayout->addWidget(prayerChapterText, 1);
    splitter->addWidget(rightPanel);
    splitter->setSizes(QList<int>() << 180 << 400);
    mainLayout->addWidget(splitter, 1);

    tabWidget->addTab(prayerPage, "✝ Молитва");
    onPrayerGospelChanged(0);
    loadPrayerImageForChapter();
}

void MainWindow::refreshGameWidget() {
    int xp = gameStats.getXP();
    int need = gameStats.getXPForNextLevel();
    int currentLevelXP = 0;
    for (int i = 1; i < gameStats.getLevel(); i++) currentLevelXP += i * 10;
    int xpInLevel = xp - currentLevelXP;
    int needInLevel = gameStats.getLevel() * 10;

    levelLabel->setText(QString("⭐ Уровень %1").arg(gameStats.getLevel()));
    xpBar->setMaximum(needInLevel);
    xpBar->setValue(xpInLevel);
    streakLabel->setText(QString("🔥 Серия: %1 дн.").arg(gameStats.getStreak()));
}

void MainWindow::onEnglishLessonSelected(int index) {
    if (index < 0) return;
    QList<EnglishWord> words = englishData.getWords(index);
    englishWordsTable->setRowCount(words.size());
    for (int i = 0; i < words.size(); i++) {
        englishWordsTable->setItem(i, 0, new QTableWidgetItem(words[i].word));
        englishWordsTable->setItem(i, 1, new QTableWidgetItem(words[i].translation));
    }
}

void MainWindow::onEnglishAddWord() {
    int lessonRow = englishLessonList->currentRow();
    if (lessonRow < 0) return;
    int lessonIndex = englishLevelCombo->currentIndex() * EnglishData::LESSONS_PER_LEVEL + lessonRow;
    QString word = englishWordEdit->text().trimmed();
    QString trans = englishTranslationEdit->text().trimmed();
    if (word.isEmpty()) return;
    englishData.addWord(lessonIndex, word, trans);
    englishWordEdit->clear();
    englishTranslationEdit->clear();
    onEnglishLessonSelected(lessonRow);
}

void MainWindow::onEnglishRemoveWord() {
    int lessonRow = englishLessonList->currentRow();
    int wordRow = englishWordsTable->currentRow();
    if (lessonRow < 0 || wordRow < 0) return;
    int lessonIndex = englishLevelCombo->currentIndex() * EnglishData::LESSONS_PER_LEVEL + lessonRow;
    englishData.removeWord(lessonIndex, wordRow);
    onEnglishLessonSelected(lessonRow);
}

void MainWindow::onPrayerGospelChanged(int index) {
    int chapters = prayerGospelCombo->itemData(index).toInt();
    prayerChapterList->clear();
    for (int i = 1; i <= chapters; i++) {
        prayerChapterList->addItem(QString("Глава %1").arg(i));
    }
    prayerChapterList->setCurrentRow(0);
    onPrayerChapterSelected(0);
}

void MainWindow::onPrayerChapterSelected(int index) {
    if (index < 0) {
        prayerChapterText->clear();
        prayerImageLabel->setPixmap(QPixmap());
        prayerImageLabel->setText("Изображение главы\n(выберите главу и добавьте фото)");
        return;
    }
    QString gospel = prayerGospelCombo->currentText();
    int ch = index + 1;
    QString placeholder = QString("%1\nГлава %2\n\nЗдесь можно разместить текст главы или читать по книге.").arg(gospel).arg(ch);
    prayerChapterText->setText(placeholder);
    loadPrayerImageForChapter();
}

QString MainWindow::prayerImagePath(int gospelIndex, int chapterNum) const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        QString("/prayer_%1_%2.png").arg(gospelIndex).arg(chapterNum);
}

void MainWindow::loadPrayerImageForChapter() {
    int gospelIndex = prayerGospelCombo->currentIndex();
    int chapterRow = prayerChapterList->currentRow();
    if (chapterRow < 0) {
        prayerImageLabel->setPixmap(QPixmap());
        prayerImageLabel->setText("Изображение главы\n(выберите главу и добавьте фото)");
        return;
    }
    int chapterNum = chapterRow + 1;
    QString path = prayerImagePath(gospelIndex, chapterNum);
    if (!QFile::exists(path)) {
        prayerImageLabel->setPixmap(QPixmap());
        prayerImageLabel->setText("Изображение главы " + QString::number(chapterNum) + "\n(добавьте фото)");
        return;
    }
    QPixmap pix(path);
    if (pix.isNull()) return;
    pix = pix.scaled(500, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    prayerImageLabel->setPixmap(pix);
    prayerImageLabel->setText("");
}

void MainWindow::onPrayerAddImage() {
    int gospelIndex = prayerGospelCombo->currentIndex();
    int chapterRow = prayerChapterList->currentRow();
    if (chapterRow < 0) {
        QMessageBox::information(this, "Молитва", "Сначала выберите главу.");
        return;
    }
    int chapterNum = chapterRow + 1;
    QString path = QFileDialog::getOpenFileName(this, "Выберите изображение для главы " + QString::number(chapterNum),
        QString(), "Изображения (*.png *.jpg *.jpeg *.bmp)");
    if (path.isEmpty()) return;
    QPixmap pix(path);
    if (pix.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }
    QString savePath = prayerImagePath(gospelIndex, chapterNum);
    QDir().mkpath(QFileInfo(savePath).absolutePath());
    if (!pix.save(savePath, "PNG")) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изображение.");
        return;
    }
    pix = pix.scaled(500, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    prayerImageLabel->setPixmap(pix);
    prayerImageLabel->setText("");
}

void MainWindow::updateDailyTasks() {
    tasksTable->setRowCount(0);

    QDate selectedDate = dateSelector->date();
    QList<Task> allTasks = taskManager->getAllTasks();

    // Фильтруем задачи по выбранной дате (задачи со сроком на эту дату)
    int row = 0;
    for (const Task& task : allTasks) {
        if (task.getDeadline() == selectedDate) {
            tasksTable->insertRow(row);
            tasksTable->setRowHeight(row, isMobile() ? 56 : 52);

            // Чекбокс выполнения
            QTableWidgetItem* statusItem = new QTableWidgetItem();
            statusItem->setFlags(statusItem->flags() | Qt::ItemIsUserCheckable);
            statusItem->setCheckState(task.getStatus() == TaskStatus::Completed ? Qt::Checked : Qt::Unchecked);
            statusItem->setData(Qt::UserRole, task.getId());
            statusItem->setTextAlignment(Qt::AlignCenter);
            tasksTable->setItem(row, 0, statusItem);

            // Название
            QTableWidgetItem* titleItem = new QTableWidgetItem(task.getTitle());
            titleItem->setFont(QFont("Segoe UI", 11, QFont::Medium));
            titleItem->setForeground(QColor(13, 13, 13));
            if (task.getStatus() == TaskStatus::Completed) {
                titleItem->setForeground(QColor(45, 45, 45));
                titleItem->setFont(QFont("Segoe UI", 11, QFont::Normal));
            }
            tasksTable->setItem(row, 1, titleItem);

            // Описание
            QTableWidgetItem* descItem = new QTableWidgetItem(task.getDescription());
            descItem->setForeground(QColor(13, 13, 13));
            if (task.getStatus() == TaskStatus::Completed) {
                descItem->setForeground(QColor(60, 60, 60));
            }
            tasksTable->setItem(row, 2, descItem);

            // Приоритет
            QTableWidgetItem* priorityItem = new QTableWidgetItem(task.priorityToString());
            priorityItem->setBackground(getPriorityColor(task.getPriority()));
            priorityItem->setForeground(getPriorityTextColor(task.getPriority()));
            priorityItem->setTextAlignment(Qt::AlignCenter);
            priorityItem->setFont(QFont("Segoe UI", 10, QFont::Medium));
            tasksTable->setItem(row, 3, priorityItem);

            // Категория
            QTableWidgetItem* catItem = new QTableWidgetItem(task.getCategory().isEmpty() ? "—" : task.getCategory());
            catItem->setForeground(QColor(13, 13, 13));
            tasksTable->setItem(row, 4, catItem);

            row++;
        }
    }

    tasksTable->resizeColumnsToContents();

    // Обновляем заголовок
    QString dateStr = selectedDate.toString("dd.MM.yyyy");
    if (selectedDate == QDate::currentDate()) {
        dateStr = "Сегодня (" + dateStr + ")";
    } else if (selectedDate == QDate::currentDate().addDays(1)) {
        dateStr = "Завтра (" + dateStr + ")";
    } else if (selectedDate == QDate::currentDate().addDays(-1)) {
        dateStr = "Вчера (" + dateStr + ")";
    }
    dateLabel->setText("Задачи на: " + dateStr + " (" + QString::number(row) + " задач)");
}

void MainWindow::showTaskDialog(const Task* task) {
    QDialog dialog(this);
    dialog.setWindowTitle(task ? "Редактировать задачу" : "Новая задача");
    dialog.setMinimumWidth(520);
    dialog.setStyleSheet(R"(
        QDialog { background-color: #f8fafc; }
        QLabel { color: #0d0d0d; font-size: 11pt; font-weight: bold; }
        QLineEdit, QTextEdit, QDateEdit, QComboBox {
            background-color: white;
            color: #0d0d0d;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            padding: 10px 12px;
            font-size: 11pt;
            selection-background-color: #e8f0fe;
        }
        QLineEdit:focus, QTextEdit:focus, QDateEdit:focus, QComboBox:focus {
            border-color: #5c7cfa;
        }
        QComboBox::drop-down { border: none; padding-right: 8px; }
        QComboBox QAbstractItemView { background: white; border-radius: 8px; }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5c7cfa, stop:1 #4c6ef5);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: bold;
            font-size: 11pt;
        }
        QPushButton:hover { background: #4c6ef5; }
    )");

    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(14);
    form->setContentsMargins(24, 24, 24, 24);

    QLineEdit* titleEdit = new QLineEdit(&dialog);
    titleEdit->setPlaceholderText("Введите название задачи");
    QTextEdit* descriptionEdit = new QTextEdit(&dialog);
    descriptionEdit->setPlaceholderText("Описание (необязательно)");
    descriptionEdit->setMaximumHeight(100);
    QDateEdit* deadlineEdit = new QDateEdit(&dialog);
    deadlineEdit->setDate(dateSelector->date());
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat("dd.MM.yyyy");
    QComboBox* priorityCombo = new QComboBox(&dialog);
    priorityCombo->addItem("Низкий", static_cast<int>(Priority::Low));
    priorityCombo->addItem("Средний", static_cast<int>(Priority::Medium));
    priorityCombo->addItem("Высокий", static_cast<int>(Priority::High));
    QLineEdit* categoryEdit = new QLineEdit(&dialog);
    categoryEdit->setPlaceholderText("Например: Английский, Молитва");

    if (task) {
        titleEdit->setText(task->getTitle());
        descriptionEdit->setPlainText(task->getDescription());
        deadlineEdit->setDate(task->getDeadline());
        priorityCombo->setCurrentIndex(static_cast<int>(task->getPriority()));
        categoryEdit->setText(task->getCategory());
    }

    form->addRow("Название:", titleEdit);
    form->addRow("Описание:", descriptionEdit);
    form->addRow("Срок:", deadlineEdit);
    form->addRow("Приоритет:", priorityCombo);
    form->addRow("Категория:", categoryEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    if (QPushButton* cancelBtn = buttons->button(QDialogButtonBox::Cancel)) {
        cancelBtn->setObjectName("cancelButton");
    }
    form->addRow(buttons);

    dialog.setStyleSheet(dialog.styleSheet() + R"(
        QPushButton#cancelButton {
            background: #e2e8f0;
            color: #0d0d0d;
        }
        QPushButton#cancelButton:hover { background: #cbd5e1; }
    )");

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        Task newTask;
        if (task) {
            newTask = *task;
        }
        newTask.setTitle(titleEdit->text());
        newTask.setDescription(descriptionEdit->toPlainText());
        newTask.setDeadline(deadlineEdit->date());
        newTask.setPriority(static_cast<Priority>(priorityCombo->currentData().toInt()));
        newTask.setCategory(categoryEdit->text());

        if (task) {
            taskManager->updateTask(newTask);
        } else {
            taskManager->addTask(newTask);
        }

        updateDailyTasks();
    }
}

void MainWindow::onAddTask() {
    showTaskDialog();
}

void MainWindow::onTaskStatusChanged(int row, int column) {
    if (column != 0) return;

    QTableWidgetItem* item = tasksTable->item(row, 0);
    if (!item) return;

    // Блокируем сигналы, чтобы избежать рекурсии
    tasksTable->blockSignals(true);

    int taskId = item->data(Qt::UserRole).toInt();
    Task* task = taskManager->getTask(taskId);
    if (task) {
        TaskStatus newStatus = item->checkState() == Qt::Checked ? TaskStatus::Completed : TaskStatus::Pending;
        task->setStatus(newStatus);
        taskManager->updateTask(*task);
        if (newStatus == TaskStatus::Completed) {
            gameStats.addTaskCompleted();
            refreshGameWidget();
        }
        updateDailyTasks();
    }

    tasksTable->blockSignals(false);
}

void MainWindow::onDateChanged() {
    updateDailyTasks();
}

QColor MainWindow::getPriorityColor(Priority priority) const {
    switch (priority) {
        case Priority::High:   return QColor(254, 226, 226);  // мягкий красный
        case Priority::Medium: return QColor(254, 249, 195);   // мягкий жёлтый
        case Priority::Low:    return QColor(220, 252, 231);   // мягкий зелёный
        default: return QColor(248, 250, 252);
    }
}

QColor MainWindow::getPriorityTextColor(Priority priority) const {
    switch (priority) {
        case Priority::High:   return QColor(185, 28, 28);
        case Priority::Medium: return QColor(161, 98, 7);
        case Priority::Low:    return QColor(22, 101, 52);
        default: return QColor(71, 85, 105);
    }
}

QString MainWindow::formatDate(const QDate& date) const {
    if (date == QDate::currentDate()) {
        return "Сегодня";
    } else if (date == QDate::currentDate().addDays(1)) {
        return "Завтра";
    } else {
        return date.toString("dd.MM.yyyy");
    }
}

bool MainWindow::isMobile() const {
#ifdef POL_MOBILE
    return true;
#else
    return false;
#endif
}
