#include <crow.h>
#include <iostream>
#include <memory>
#include "database.h"
#include "api_routes.h"

int main() {
    // Initialize database
    auto database = std::make_shared<Database>("rest_api.db");
    if (!database->initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1;
    }
    
    std::cout << "Database initialized successfully!" << std::endl;
    
    // Create Crow application
    crow::SimpleApp app;
    
    // Enable logging
    app.loglevel(crow::LogLevel::Info);
    
    // Setup API routes
    APIRoutes api_routes(database);
    api_routes.setup_routes(app);
    
    // Add global CORS middleware
    CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)
    ([](const crow::request& req, const std::string& path) {
        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
    });
    
    // Welcome route
    CROW_ROUTE(app, "/")
    ([]() {
        nlohmann::json welcome = {
            {"message", "Welcome to C++ REST API"},
            {"version", "1.0.0"},
            {"author", "Your Name"},
            {"endpoints", {
                {"POST /api/auth/register", "Register a new user"},
                {"POST /api/auth/login", "Login user"},
                {"GET /api/users", "Get all users"},
                {"GET /api/users/:id", "Get user by ID"},
                {"PUT /api/users/:id", "Update user (authenticated)"},
                {"DELETE /api/users/:id", "Delete user (authenticated)"},
                {"GET /api/tasks", "Get all tasks"},
                {"POST /api/tasks", "Create task (authenticated)"},
                {"GET /api/tasks/:id", "Get task by ID"},
                {"PUT /api/tasks/:id", "Update task (authenticated)"},
                {"DELETE /api/tasks/:id", "Delete task (authenticated)"},
                {"GET /api/users/:id/tasks", "Get tasks by user ID"},
                {"GET /api/health", "Health check"}
            }}
        };
        
        crow::response res(200, welcome.dump(2));
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
    
    // Set port from environment or default to 8080
    int port = 8080;
    if (const char* env_port = std::getenv("PORT")) {
        port = std::atoi(env_port);
    }
    
    std::cout << "Starting REST API server on port " << port << "..." << std::endl;
    std::cout << "API Documentation available at: http://localhost:" << port << std::endl;
    
    // Run the app
    app.port(port).multithreaded().run();
    
    return 0;
}
