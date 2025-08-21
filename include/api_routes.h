#pragma once
#include <crow.h>
#include <nlohmann/json.hpp>
#include "database.h"

class APIRoutes {
public:
    APIRoutes(std::shared_ptr<Database> db);
    void setup_routes(crow::SimpleApp& app);

private:
    std::shared_ptr<Database> database;
    
    // Utility methods
    nlohmann::json create_error_response(const std::string& message, int code = 400);
    nlohmann::json create_success_response(const std::string& message, const nlohmann::json& data = nlohmann::json::object());
    std::optional<std::pair<int, std::string>> authenticate_request(const crow::request& req);
    
    // Auth routes
    crow::response login(const crow::request& req);
    crow::response register_user(const crow::request& req);
    
    // User routes
    crow::response get_users();
    crow::response get_user(int user_id);
    crow::response update_user(const crow::request& req, int user_id);
    crow::response delete_user(const crow::request& req, int user_id);
    
    // Task routes
    crow::response get_tasks();
    crow::response create_task(const crow::request& req);
    crow::response get_task(int task_id);
    crow::response update_task(const crow::request& req, int task_id);
    crow::response delete_task(const crow::request& req, int task_id);
    crow::response get_user_tasks(int user_id);
};
