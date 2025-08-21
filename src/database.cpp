#include "database.h"
#include <iostream>
#include <cstring>

Database::Database(const std::string& db_path) : db(nullptr), db_path(db_path) {}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::initialize() {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    return create_tables();
}

bool Database::create_tables() {
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    const char* create_tasks_table = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            completed BOOLEAN DEFAULT 0,
            user_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
        );
    )";
    
    return execute(create_users_table) && execute(create_tasks_table);
}

bool Database::execute(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

bool Database::create_user(const std::string& username, const std::string& email, const std::string& password_hash) {
    const char* sql = "INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password_hash.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

nlohmann::json Database::get_user_by_id(int user_id) {
    const char* sql = "SELECT id, username, email, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nlohmann::json();
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json user = row_to_json_user(stmt);
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nlohmann::json();
}

nlohmann::json Database::get_user_by_username(const std::string& username) {
    const char* sql = "SELECT id, username, email, password_hash, created_at FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nlohmann::json();
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json user;
        user["id"] = sqlite3_column_int(stmt, 0);
        user["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user["email"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user["password_hash"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nlohmann::json();
}

std::vector<nlohmann::json> Database::get_all_users() {
    const char* sql = "SELECT id, username, email, created_at FROM users;";
    sqlite3_stmt* stmt;
    std::vector<nlohmann::json> users;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return users;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        users.push_back(row_to_json_user(stmt));
    }
    
    sqlite3_finalize(stmt);
    return users;
}

bool Database::update_user(int user_id, const std::string& username, const std::string& email) {
    const char* sql = "UPDATE users SET username = ?, email = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, user_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::delete_user(int user_id) {
    const char* sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::create_task(const std::string& title, const std::string& description, int user_id) {
    const char* sql = "INSERT INTO tasks (title, description, user_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, user_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

nlohmann::json Database::get_task_by_id(int task_id) {
    const char* sql = "SELECT id, title, description, completed, user_id, created_at, updated_at FROM tasks WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nlohmann::json();
    }
    
    sqlite3_bind_int(stmt, 1, task_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json task = row_to_json_task(stmt);
        sqlite3_finalize(stmt);
        return task;
    }
    
    sqlite3_finalize(stmt);
    return nlohmann::json();
}

std::vector<nlohmann::json> Database::get_tasks_by_user(int user_id) {
    const char* sql = "SELECT id, title, description, completed, user_id, created_at, updated_at FROM tasks WHERE user_id = ?;";
    sqlite3_stmt* stmt;
    std::vector<nlohmann::json> tasks;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return tasks;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tasks.push_back(row_to_json_task(stmt));
    }
    
    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<nlohmann::json> Database::get_all_tasks() {
    const char* sql = "SELECT id, title, description, completed, user_id, created_at, updated_at FROM tasks;";
    sqlite3_stmt* stmt;
    std::vector<nlohmann::json> tasks;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tasks.push_back(row_to_json_task(stmt));
    }
    
    sqlite3_finalize(stmt);
    return tasks;
}

bool Database::update_task(int task_id, const std::string& title, const std::string& description, bool completed) {
    const char* sql = "UPDATE tasks SET title = ?, description = ?, completed = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, completed ? 1 : 0);
    sqlite3_bind_int(stmt, 4, task_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::delete_task(int task_id) {
    const char* sql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, task_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

nlohmann::json Database::row_to_json_user(sqlite3_stmt* stmt) {
    nlohmann::json user;
    user["id"] = sqlite3_column_int(stmt, 0);
    user["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user["email"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    return user;
}

nlohmann::json Database::row_to_json_task(sqlite3_stmt* stmt) {
    nlohmann::json task;
    task["id"] = sqlite3_column_int(stmt, 0);
    task["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    task["description"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    task["completed"] = sqlite3_column_int(stmt, 3) == 1;
    task["user_id"] = sqlite3_column_int(stmt, 4);
    task["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    task["updated_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    return task;
}
