#include "api_routes.h"
#include "auth_service.h"
#include <iostream>
#include <regex>

APIRoutes::APIRoutes(std::shared_ptr<Database> db) : database(db) {}

void APIRoutes::setup_routes(crow::SimpleApp& app) {
    // Enable CORS
    CROW_ROUTE(app, "/").methods("OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
    });

    // Health check
    CROW_ROUTE(app, "/api/health")
    ([this]() {
        auto response = create_success_response("API is running");
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
    
    // Auth routes
    CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
    ([this](const crow::request& req) {
        return register_user(req);
    });
    
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
    ([this](const crow::request& req) {
        return login(req);
    });
    
    // User routes
    CROW_ROUTE(app, "/api/users").methods("GET"_method)
    ([this]() {
        return get_users();
    });
    
    CROW_ROUTE(app, "/api/users/<int>").methods("GET"_method)
    ([this](int user_id) {
        return get_user(user_id);
    });
    
    CROW_ROUTE(app, "/api/users/<int>").methods("PUT"_method)
    ([this](const crow::request& req, int user_id) {
        return update_user(req, user_id);
    });
    
    CROW_ROUTE(app, "/api/users/<int>").methods("DELETE"_method)
    ([this](const crow::request& req, int user_id) {
        return delete_user(req, user_id);
    });
    
    // Task routes
    CROW_ROUTE(app, "/api/tasks").methods("GET"_method)
    ([this]() {
        return get_tasks();
    });
    
    CROW_ROUTE(app, "/api/tasks").methods("POST"_method)
    ([this](const crow::request& req) {
        return create_task(req);
    });
    
    CROW_ROUTE(app, "/api/tasks/<int>").methods("GET"_method)
    ([this](int task_id) {
        return get_task(task_id);
    });
    
    CROW_ROUTE(app, "/api/tasks/<int>").methods("PUT"_method)
    ([this](const crow::request& req, int task_id) {
        return update_task(req, task_id);
    });
    
    CROW_ROUTE(app, "/api/tasks/<int>").methods("DELETE"_method)
    ([this](const crow::request& req, int task_id) {
        return delete_task(req, task_id);
    });
    
    CROW_ROUTE(app, "/api/users/<int>/tasks").methods("GET"_method)
    ([this](int user_id) {
        return get_user_tasks(user_id);
    });
}

nlohmann::json APIRoutes::create_error_response(const std::string& message, int code) {
    return nlohmann::json{
        {"success", false},
        {"error", {
            {"message", message},
            {"code", code}
        }}
    };
}

nlohmann::json APIRoutes::create_success_response(const std::string& message, const nlohmann::json& data) {
    nlohmann::json response = {
        {"success", true},
        {"message", message}
    };
    
    if (!data.empty()) {
        response["data"] = data;
    }
    
    return response;
}

std::optional<std::pair<int, std::string>> APIRoutes::authenticate_request(const crow::request& req) {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty()) {
        return std::nullopt;
    }
    
    // Check for Bearer token
    std::regex bearer_regex("Bearer\\s+(.+)");
    std::smatch matches;
    
    if (!std::regex_match(auth_header, matches, bearer_regex) || matches.size() != 2) {
        return std::nullopt;
    }
    
    std::string token = matches[1].str();
    return AuthService::verify_jwt_token(token);
}

crow::response APIRoutes::register_user(const crow::request& req) {
    try {
        auto json_data = nlohmann::json::parse(req.body);
        
        if (!json_data.contains("username") || !json_data.contains("email") || !json_data.contains("password")) {
            auto error = create_error_response("Missing required fields: username, email, password");
            return crow::response(400, error.dump());
        }
        
        std::string username = json_data["username"];
        std::string email = json_data["email"];
        std::string password = json_data["password"];
        
        // Validate email format
        std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!std::regex_match(email, email_regex)) {
            auto error = create_error_response("Invalid email format");
            return crow::response(400, error.dump());
        }
        
        // Check if user already exists
        auto existing_user = database->get_user_by_username(username);
        if (!existing_user.empty()) {
            auto error = create_error_response("Username already exists");
            return crow::response(409, error.dump());
        }
        
        // Hash password and create user
        std::string password_hash = AuthService::hash_password(password);
        bool success = database->create_user(username, email, password_hash);
        
        if (success) {
            auto response = create_success_response("User registered successfully");
            crow::response res(201, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to create user");
            return crow::response(500, error.dump());
        }
        
    } catch (const nlohmann::json::exception& e) {
        auto error = create_error_response("Invalid JSON format");
        return crow::response(400, error.dump());
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::login(const crow::request& req) {
    try {
        auto json_data = nlohmann::json::parse(req.body);
        
        if (!json_data.contains("username") || !json_data.contains("password")) {
            auto error = create_error_response("Missing username or password");
            return crow::response(400, error.dump());
        }
        
        std::string username = json_data["username"];
        std::string password = json_data["password"];
        
        auto user = database->get_user_by_username(username);
        if (user.empty()) {
            auto error = create_error_response("Invalid credentials");
            return crow::response(401, error.dump());
        }
        
        std::string stored_hash = user["password_hash"];
        if (!AuthService::verify_password(password, stored_hash)) {
            auto error = create_error_response("Invalid credentials");
            return crow::response(401, error.dump());
        }
        
        // Generate JWT token
        int user_id = user["id"];
        std::string token = AuthService::generate_jwt_token(user_id, username);
        
        nlohmann::json user_data = {
            {"id", user["id"]},
            {"username", user["username"]},
            {"email", user["email"]},
            {"token", token}
        };
        
        auto response = create_success_response("Login successful", user_data);
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        return res;
        
    } catch (const nlohmann::json::exception& e) {
        auto error = create_error_response("Invalid JSON format");
        return crow::response(400, error.dump());
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::get_users() {
    try {
        auto users = database->get_all_users();
        auto response = create_success_response("Users retrieved successfully", users);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::get_user(int user_id) {
    try {
        auto user = database->get_user_by_id(user_id);
        if (user.empty()) {
            auto error = create_error_response("User not found");
            return crow::response(404, error.dump());
        }
        
        auto response = create_success_response("User retrieved successfully", user);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::update_user(const crow::request& req, int user_id) {
    // Authentication required
    auto auth_result = authenticate_request(req);
    if (!auth_result.has_value()) {
        auto error = create_error_response("Authentication required");
        return crow::response(401, error.dump());
    }
    
    int authenticated_user_id = auth_result->first;
    if (authenticated_user_id != user_id) {
        auto error = create_error_response("Unauthorized to update this user");
        return crow::response(403, error.dump());
    }
    
    try {
        auto json_data = nlohmann::json::parse(req.body);
        
        if (!json_data.contains("username") || !json_data.contains("email")) {
            auto error = create_error_response("Missing required fields: username, email");
            return crow::response(400, error.dump());
        }
        
        std::string username = json_data["username"];
        std::string email = json_data["email"];
        
        bool success = database->update_user(user_id, username, email);
        if (success) {
            auto response = create_success_response("User updated successfully");
            crow::response res(200, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to update user");
            return crow::response(500, error.dump());
        }
        
    } catch (const nlohmann::json::exception& e) {
        auto error = create_error_response("Invalid JSON format");
        return crow::response(400, error.dump());
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::delete_user(const crow::request& req, int user_id) {
    // Authentication required
    auto auth_result = authenticate_request(req);
    if (!auth_result.has_value()) {
        auto error = create_error_response("Authentication required");
        return crow::response(401, error.dump());
    }
    
    int authenticated_user_id = auth_result->first;
    if (authenticated_user_id != user_id) {
        auto error = create_error_response("Unauthorized to delete this user");
        return crow::response(403, error.dump());
    }
    
    try {
        bool success = database->delete_user(user_id);
        if (success) {
            auto response = create_success_response("User deleted successfully");
            crow::response res(200, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to delete user");
            return crow::response(500, error.dump());
        }
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::get_tasks() {
    try {
        auto tasks = database->get_all_tasks();
        auto response = create_success_response("Tasks retrieved successfully", tasks);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::create_task(const crow::request& req) {
    // Authentication required
    auto auth_result = authenticate_request(req);
    if (!auth_result.has_value()) {
        auto error = create_error_response("Authentication required");
        return crow::response(401, error.dump());
    }
    
    int user_id = auth_result->first;
    
    try {
        auto json_data = nlohmann::json::parse(req.body);
        
        if (!json_data.contains("title")) {
            auto error = create_error_response("Missing required field: title");
            return crow::response(400, error.dump());
        }
        
        std::string title = json_data["title"];
        std::string description = json_data.value("description", "");
        
        bool success = database->create_task(title, description, user_id);
        if (success) {
            auto response = create_success_response("Task created successfully");
            crow::response res(201, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to create task");
            return crow::response(500, error.dump());
        }
        
    } catch (const nlohmann::json::exception& e) {
        auto error = create_error_response("Invalid JSON format");
        return crow::response(400, error.dump());
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::get_task(int task_id) {
    try {
        auto task = database->get_task_by_id(task_id);
        if (task.empty()) {
            auto error = create_error_response("Task not found");
            return crow::response(404, error.dump());
        }
        
        auto response = create_success_response("Task retrieved successfully", task);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::update_task(const crow::request& req, int task_id) {
    // Authentication required
    auto auth_result = authenticate_request(req);
    if (!auth_result.has_value()) {
        auto error = create_error_response("Authentication required");
        return crow::response(401, error.dump());
    }
    
    try {
        // Check if task exists and belongs to user
        auto existing_task = database->get_task_by_id(task_id);
        if (existing_task.empty()) {
            auto error = create_error_response("Task not found");
            return crow::response(404, error.dump());
        }
        
        int task_user_id = existing_task["user_id"];
        int authenticated_user_id = auth_result->first;
        
        if (task_user_id != authenticated_user_id) {
            auto error = create_error_response("Unauthorized to update this task");
            return crow::response(403, error.dump());
        }
        
        auto json_data = nlohmann::json::parse(req.body);
        
        std::string title = json_data.value("title", existing_task["title"]);
        std::string description = json_data.value("description", existing_task["description"]);
        bool completed = json_data.value("completed", existing_task["completed"]);
        
        bool success = database->update_task(task_id, title, description, completed);
        if (success) {
            auto response = create_success_response("Task updated successfully");
            crow::response res(200, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to update task");
            return crow::response(500, error.dump());
        }
        
    } catch (const nlohmann::json::exception& e) {
        auto error = create_error_response("Invalid JSON format");
        return crow::response(400, error.dump());
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::delete_task(const crow::request& req, int task_id) {
    // Authentication required
    auto auth_result = authenticate_request(req);
    if (!auth_result.has_value()) {
        auto error = create_error_response("Authentication required");
        return crow::response(401, error.dump());
    }
    
    try {
        // Check if task exists and belongs to user
        auto existing_task = database->get_task_by_id(task_id);
        if (existing_task.empty()) {
            auto error = create_error_response("Task not found");
            return crow::response(404, error.dump());
        }
        
        int task_user_id = existing_task["user_id"];
        int authenticated_user_id = auth_result->first;
        
        if (task_user_id != authenticated_user_id) {
            auto error = create_error_response("Unauthorized to delete this task");
            return crow::response(403, error.dump());
        }
        
        bool success = database->delete_task(task_id);
        if (success) {
            auto response = create_success_response("Task deleted successfully");
            crow::response res(200, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } else {
            auto error = create_error_response("Failed to delete task");
            return crow::response(500, error.dump());
        }
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}

crow::response APIRoutes::get_user_tasks(int user_id) {
    try {
        auto tasks = database->get_tasks_by_user(user_id);
        auto response = create_success_response("User tasks retrieved successfully", tasks);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
        
    } catch (const std::exception& e) {
        auto error = create_error_response("Internal server error");
        return crow::response(500, error.dump());
    }
}
