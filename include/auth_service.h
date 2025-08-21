#pragma once
#include <string>
#include <optional>

class AuthService {
public:
    static std::string hash_password(const std::string& password);
    static bool verify_password(const std::string& password, const std::string& hash);
    static std::string generate_jwt_token(int user_id, const std::string& username);
    static std::optional<std::pair<int, std::string>> verify_jwt_token(const std::string& token);
    
private:
    static const std::string JWT_SECRET;
    static const int JWT_EXPIRY_HOURS;
};
