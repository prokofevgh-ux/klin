#include "task.h"
#include <QJsonObject>
#include <QColor>

int Task::nextId = 1;

Task::Task()
    : id(nextId++), priority(Priority::Medium), status(TaskStatus::Pending),
      createdAt(QDateTime::currentDateTime()) {
}

Task::Task(const QString& title, const QString& description, const QDate& deadline,
           Priority priority, const QString& category)
    : id(nextId++), title(title), description(description), deadline(deadline),
      priority(priority), category(category), status(TaskStatus::Pending),
      createdAt(QDateTime::currentDateTime()) {
}

void Task::setStatus(TaskStatus status) {
    this->status = status;
    if (status == TaskStatus::Completed && completedAt.isNull()) {
        completedAt = QDateTime::currentDateTime();
    } else if (status == TaskStatus::Pending) {
        completedAt = QDateTime();
    }
}

bool Task::isOverdue() const {
    if (status == TaskStatus::Completed) {
        return false;
    }
    return deadline < QDate::currentDate();
}

bool Task::isDueToday() const {
    if (status == TaskStatus::Completed) {
        return false;
    }
    return deadline == QDate::currentDate();
}

bool Task::isDueThisWeek() const {
    if (status == TaskStatus::Completed) {
        return false;
    }
    QDate today = QDate::currentDate();
    QDate weekEnd = today.addDays(7 - today.dayOfWeek());
    return deadline >= today && deadline <= weekEnd;
}

QString Task::priorityToString() const {
    switch (priority) {
        case Priority::High: return "Высокий";
        case Priority::Medium: return "Средний";
        case Priority::Low: return "Низкий";
        default: return "Средний";
    }
}

QString Task::statusToString() const {
    switch (status) {
        case TaskStatus::Pending: return "В работе";
        case TaskStatus::Completed: return "Выполнено";
        default: return "В работе";
    }
}

QColor Task::priorityColor() const {
    switch (priority) {
        case Priority::High: return QColor(220, 53, 69); // Красный
        case Priority::Medium: return QColor(255, 193, 7); // Желтый
        case Priority::Low: return QColor(40, 167, 69); // Зеленый
        default: return QColor(128, 128, 128); // Серый
    }
}

QJsonObject Task::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["title"] = title;
    json["description"] = description;
    json["deadline"] = deadline.toString(Qt::ISODate);
    json["priority"] = static_cast<int>(priority);
    json["category"] = category;
    json["status"] = static_cast<int>(status);
    json["createdAt"] = createdAt.toString(Qt::ISODate);
    if (!completedAt.isNull()) {
        json["completedAt"] = completedAt.toString(Qt::ISODate);
    }
    return json;
}

Task Task::fromJson(const QJsonObject& json) {
    Task task;
    task.id = json["id"].toInt();
    task.title = json["title"].toString();
    task.description = json["description"].toString();
    task.deadline = QDate::fromString(json["deadline"].toString(), Qt::ISODate);
    task.priority = static_cast<Priority>(json["priority"].toInt());
    task.category = json["category"].toString();
    task.status = static_cast<TaskStatus>(json["status"].toInt());
    task.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    if (json.contains("completedAt")) {
        task.completedAt = QDateTime::fromString(json["completedAt"].toString(), Qt::ISODate);
    }

    // Обновляем nextId, чтобы избежать конфликтов
    if (task.id >= Task::nextId) {
        Task::nextId = task.id + 1;
    }

    return task;
}

