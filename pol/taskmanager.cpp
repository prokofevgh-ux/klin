#include "taskmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

TaskManager::TaskManager() {
    // Используем папку AppData для хранения данных
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    dataFile = appDataPath + "/tasks.json";

    loadFromFile();
}

TaskManager::~TaskManager() {
    saveToFile();
}

void TaskManager::addTask(const Task& task) {
    tasks.append(task);
    saveToFile();
}

void TaskManager::updateTask(const Task& task) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getId() == task.getId()) {
            tasks[i] = task;
            saveToFile();
            return;
        }
    }
}

void TaskManager::deleteTask(int taskId) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getId() == taskId) {
            tasks.removeAt(i);
            saveToFile();
            return;
        }
    }
}

Task* TaskManager::getTask(int taskId) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getId() == taskId) {
            return &tasks[i];
        }
    }
    return nullptr;
}

QList<Task> TaskManager::getTasksByStatus(TaskStatus status) const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.getStatus() == status) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getTasksByPriority(Priority priority) const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.getPriority() == priority) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getTasksByCategory(const QString& category) const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.getCategory() == category) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getOverdueTasks() const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.isOverdue()) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getTodayTasks() const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.isDueToday()) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getWeekTasks() const {
    QList<Task> result;
    for (const Task& task : tasks) {
        if (task.isDueThisWeek() && !task.isDueToday()) {
            result.append(task);
        }
    }
    return result;
}

QStringList TaskManager::getCategories() const {
    QStringList categories;
    for (const Task& task : tasks) {
        if (!task.getCategory().isEmpty() && !categories.contains(task.getCategory())) {
            categories.append(task.getCategory());
        }
    }
    categories.sort();
    return categories;
}

bool TaskManager::saveToFile(const QString& filename) {
    QJsonArray jsonArray;
    for (const Task& task : tasks) {
        jsonArray.append(task.toJson());
    }

    QJsonDocument doc(jsonArray);
    QFile file(filename.isEmpty() ? dataFile : filename);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Не удалось открыть файл для записи:" << file.fileName();
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

bool TaskManager::loadFromFile(const QString& filename) {
    QFile file(filename.isEmpty() ? dataFile : filename);

    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения:" << file.fileName();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        return false;
    }

    tasks.clear();
    QJsonArray jsonArray = doc.array();
    for (const QJsonValue& value : jsonArray) {
        if (value.isObject()) {
            tasks.append(Task::fromJson(value.toObject()));
        }
    }

    // Если задач нет — добавляем задачи по умолчанию: английский и молитва
    if (tasks.isEmpty()) {
        QDate today = QDate::currentDate();
        Task englishTask(
            QStringLiteral("Английский"),
            QStringLiteral("Практика английского: слова, грамматика или чтение"),
            today,
            Priority::Medium,
            QStringLiteral("Английский")
        );
        Task prayerTask(
            QStringLiteral("Молитва"),
            QStringLiteral("Утренняя или вечерняя молитва"),
            today,
            Priority::High,
            QStringLiteral("Молитва")
        );
        tasks.append(englishTask);
        tasks.append(prayerTask);
        saveToFile();
    }

    return true;
}

int TaskManager::getCompletedTodayCount() const {
    QDate today = QDate::currentDate();
    int count = 0;
    for (const Task& task : tasks) {
        if (task.getStatus() == TaskStatus::Completed &&
            task.getCompletedAt().date() == today) {
            count++;
        }
    }
    return count;
}

int TaskManager::getCompletedThisWeekCount() const {
    QDate today = QDate::currentDate();
    QDate weekStart = today.addDays(-today.dayOfWeek() + 1);
    int count = 0;
    for (const Task& task : tasks) {
        if (task.getStatus() == TaskStatus::Completed &&
            !task.getCompletedAt().isNull() &&
            task.getCompletedAt().date() >= weekStart) {
            count++;
        }
    }
    return count;
}

QMap<QDate, int> TaskManager::getDailyCompletionStats(int days) const {
    QMap<QDate, int> stats;
    QDate today = QDate::currentDate();

    // Инициализируем все даты нулями
    for (int i = 0; i < days; ++i) {
        stats[today.addDays(-i)] = 0;
    }

    // Подсчитываем выполненные задачи
    for (const Task& task : tasks) {
        if (task.getStatus() == TaskStatus::Completed && !task.getCompletedAt().isNull()) {
            QDate completedDate = task.getCompletedAt().date();
            if (stats.contains(completedDate)) {
                stats[completedDate]++;
            }
        }
    }

    return stats;
}

QMap<QString, int> TaskManager::getCategoryStats() const {
    QMap<QString, int> stats;
    for (const Task& task : tasks) {
        QString category = task.getCategory().isEmpty() ? "Без категории" : task.getCategory();
        stats[category]++;
    }
    return stats;
}

QMap<int, int> TaskManager::getPriorityStats() const {
    QMap<int, int> stats; // день недели (1-7) -> количество выполненных
    for (const Task& task : tasks) {
        if (task.getStatus() == TaskStatus::Completed && !task.getCompletedAt().isNull()) {
            int dayOfWeek = task.getCompletedAt().date().dayOfWeek();
            stats[dayOfWeek]++;
        }
    }
    return stats;
}

void TaskManager::ensureDataFile() {
    QFileInfo fileInfo(dataFile);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}


