# C++ REST API with Crow Framework

A modern, feature-rich REST API built with C++ and the Crow framework. This project demonstrates professional-grade API development with authentication, database integration, and comprehensive CRUD operations.

## 🚀 Features

- **Modern C++17** codebase with clean architecture
- **JWT Authentication** for secure user sessions
- **SQLite Database** with prepared statements for security
- **RESTful API Design** with proper HTTP status codes
- **CORS Support** for cross-origin requests
- **Password Hashing** with PBKDF2 and salt
- **JSON API** with structured responses
- **Error Handling** with detailed error messages
- **User Management** (register, login, CRUD operations)
- **Task Management** (create, read, update, delete tasks)
- **Docker Support** for easy deployment

## 🏗️ Architecture

```
├── include/           # Header files
│   ├── api_routes.h   # Route handlers and middleware
│   ├── auth_service.h # Authentication and JWT handling
│   └── database.h     # Database operations
├── src/               # Source files
│   ├── main.cpp       # Application entry point
│   ├── api_routes.cpp # Route implementations
│   ├── auth_service.cpp # Auth service implementation
│   └── database.cpp   # Database implementation
└── CMakeLists.txt     # Build configuration
```

## 🛠️ Dependencies

- **Crow Framework** - Modern C++ web framework
- **SQLite3** - Lightweight database
- **jwt-cpp** - JWT token handling
- **nlohmann/json** - JSON parsing and serialization
- **OpenSSL** - Cryptographic functions
- **CMake** - Build system

## 📋 Prerequisites

### macOS (using Homebrew)
```bash
brew install crow sqlite3 openssl cmake pkg-config
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libcrow-dev libsqlite3-dev libssl-dev cmake pkg-config
```

### Build from Source
If Crow is not available in your package manager, you can build it from source:
```bash
git clone https://github.com/CrowCpp/Crow.git
cd Crow
mkdir build
cd build
cmake .. -DCROW_BUILD_EXAMPLES=OFF
make -j4
sudo make install
```

## 🚀 Getting Started

### 1. Clone the repository
```bash
git clone <your-repo-url>
cd rest-api
```

### 2. Build the project
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 3. Run the application
```bash
./RestAPI
```

The server will start on `http://localhost:8080`

## 📡 API Endpoints

### Authentication
- `POST /api/auth/register` - Register a new user
- `POST /api/auth/login` - Login user

### Users
- `GET /api/users` - Get all users
- `GET /api/users/:id` - Get user by ID
- `PUT /api/users/:id` - Update user (requires authentication)
- `DELETE /api/users/:id` - Delete user (requires authentication)

### Tasks
- `GET /api/tasks` - Get all tasks
- `POST /api/tasks` - Create task (requires authentication)
- `GET /api/tasks/:id` - Get task by ID
- `PUT /api/tasks/:id` - Update task (requires authentication)
- `DELETE /api/tasks/:id` - Delete task (requires authentication)
- `GET /api/users/:id/tasks` - Get tasks by user ID

### Utility
- `GET /api/health` - Health check
- `GET /` - API documentation and welcome message

## 📝 API Usage Examples

### Register a new user
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "johndoe",
    "email": "john@example.com",
    "password": "securepassword123"
  }'
```

### Login
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "johndoe",
    "password": "securepassword123"
  }'
```

### Create a task (with authentication)
```bash
curl -X POST http://localhost:8080/api/tasks \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  -d '{
    "title": "Complete REST API project",
    "description": "Finish implementing all CRUD operations"
  }'
```

### Get all tasks
```bash
curl http://localhost:8080/api/tasks
```

## 🐳 Docker Support

### Build Docker image
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libcrow-dev \
    libsqlite3-dev \
    libssl-dev \
    pkg-config

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

EXPOSE 8080

CMD ["./build/RestAPI"]
```

```bash
docker build -t cpp-rest-api .
docker run -p 8080:8080 cpp-rest-api
```

## 🔒 Security Features

- **Password Hashing**: PBKDF2 with SHA-256 and random salt
- **JWT Tokens**: Secure authentication with expiration
- **SQL Injection Protection**: Prepared statements
- **Input Validation**: Email format and required field validation
- **CORS Support**: Configurable cross-origin resource sharing

## 🧪 Testing

You can test the API using tools like:
- **Postman** - GUI testing tool
- **curl** - Command line testing
- **HTTPie** - Human-friendly HTTP client

Example test script:
```bash
#!/bin/bash
BASE_URL="http://localhost:8080/api"

# Test registration
echo "Testing user registration..."
curl -X POST $BASE_URL/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","email":"test@example.com","password":"testpass123"}'

# Test login
echo "Testing user login..."
TOKEN=$(curl -X POST $BASE_URL/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"testpass123"}' | \
  jq -r '.data.token')

# Test creating a task
echo "Testing task creation..."
curl -X POST $BASE_URL/tasks \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Test Task","description":"This is a test task"}'
```

## 📈 Performance

- **Multi-threaded**: Uses Crow's multi-threading capabilities
- **Connection Pooling**: SQLite with proper connection management
- **Efficient JSON**: Fast JSON parsing with nlohmann/json
- **Memory Management**: RAII and smart pointers

## 🤝 Contributing

1. Fork the project
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Your Name**
- GitHub:
-[@Mark]([https://github.com/yourusername](https://github.com/RePlay-h))
-[@Nurislam](https://github.com/nurofenchik)

## 🙏 Acknowledgments

- [Crow Framework](https://crowcpp.org/) for the excellent C++ web framework
- [nlohmann/json](https://github.com/nlohmann/json) for JSON handling
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp) for JWT implementation

---

⭐ Star this project if you found it helpful!
