#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

class Database {
public:
    Database(const std::string& db_path);
    ~Database();
    
    bool initialize();
    bool execute(const std::string& sql);
    
    // User operations
    bool create_user(const std::string& username, const std::string& email, const std::string& password_hash);
    nlohmann::json get_user_by_id(int user_id);
    nlohmann::json get_user_by_username(const std::string& username);
    std::vector<nlohmann::json> get_all_users();
    bool update_user(int user_id, const std::string& username, const std::string& email);
    bool delete_user(int user_id);
    
    // Task operations
    bool create_task(const std::string& title, const std::string& description, int user_id);
    nlohmann::json get_task_by_id(int task_id);
    std::vector<nlohmann::json> get_tasks_by_user(int user_id);
    std::vector<nlohmann::json> get_all_tasks();
    bool update_task(int task_id, const std::string& title, const std::string& description, bool completed);
    bool delete_task(int task_id);

private:
    sqlite3* db;
    std::string db_path;
    
    bool create_tables();
    nlohmann::json row_to_json_user(sqlite3_stmt* stmt);
    nlohmann::json row_to_json_task(sqlite3_stmt* stmt);
};
