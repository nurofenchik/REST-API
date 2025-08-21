#include "auth_service.h"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <iostream>
#include <cstring>
#include <nlohmann/json.hpp>
#include <random>

const std::string AuthService::JWT_SECRET = "your-super-secret-jwt-key-change-this-in-production";
const int AuthService::JWT_EXPIRY_HOURS = 24;

std::string AuthService::hash_password(const std::string& password) {
    // Simplified hash function for now (use proper hashing in production)
    std::hash<std::string> hasher;
    size_t hash = hasher(password + "salt123");
    
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

bool AuthService::verify_password(const std::string& password, const std::string& stored_hash) {
    return hash_password(password) == stored_hash;
}

std::string AuthService::generate_jwt_token(int user_id, const std::string& username) {
    // Simplified token generation (use proper JWT in production)
    nlohmann::json payload;
    payload["user_id"] = user_id;
    payload["username"] = username;
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    payload["exp"] = timestamp + (JWT_EXPIRY_HOURS * 3600);
    
    std::string token = payload.dump();
    
    // Simple base64-like encoding
    std::string encoded;
    for (char c : token) {
        encoded += std::to_string(static_cast<int>(c)) + ".";
    }
    
    return encoded;
}

std::optional<std::pair<int, std::string>> AuthService::verify_jwt_token(const std::string& token) {
    try {
        // Simple token verification (use proper JWT in production)
        std::string decoded;
        std::stringstream ss(token);
        std::string segment;
        
        while (std::getline(ss, segment, '.')) {
            if (!segment.empty()) {
                decoded += static_cast<char>(std::stoi(segment));
            }
        }
        
        if (decoded.empty()) {
            return std::nullopt;
        }
        
        auto payload = nlohmann::json::parse(decoded);
        
        // Check expiration
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        if (payload["exp"].get<long>() < timestamp) {
            return std::nullopt;
        }
        
        int user_id = payload["user_id"];
        std::string username = payload["username"];
        
        return std::make_pair(user_id, username);
        
    } catch (const std::exception& e) {
        std::cerr << "Token verification failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}
