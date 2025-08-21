<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# C++ REST API Project Instructions

This is a C++ REST API project using the Crow framework. When working with this codebase:

## Key Technologies:
- **Crow Framework**: Modern C++ web framework for REST APIs
- **SQLite**: Lightweight database for data persistence
- **JWT-CPP**: For authentication and authorization
- **nlohmann/json**: For JSON serialization/deserialization
- **CMake**: Build system

## Code Style Guidelines:
- Use modern C++17 features
- Follow RAII principles
- Use smart pointers when appropriate
- Implement proper error handling with exceptions
- Use const-correctness
- Follow naming conventions: snake_case for variables/functions, PascalCase for classes

## API Design Patterns:
- RESTful endpoints with proper HTTP methods
- JSON request/response format
- Consistent error response structure
- JWT-based authentication
- Input validation and sanitization

## Database Design:
- Use prepared statements to prevent SQL injection
- Implement proper database connection pooling
- Handle database errors gracefully
- Use transactions for data consistency

## Security Considerations:
- Always validate user input
- Use HTTPS in production
- Implement rate limiting
- Hash passwords properly
- Validate JWT tokens on protected routes
