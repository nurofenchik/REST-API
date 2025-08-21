#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8080/api"

echo -e "${BLUE}🚀 Testing C++ REST API${NC}"
echo "=================================="

# Test health endpoint
echo -e "\n${YELLOW}1. Testing health endpoint...${NC}"
curl -s $BASE_URL/health | jq '.'

# Test user registration
echo -e "\n${YELLOW}2. Testing user registration...${NC}"
REGISTER_RESPONSE=$(curl -s -X POST $BASE_URL/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "test@example.com",
    "password": "testpass123"
  }')
echo $REGISTER_RESPONSE | jq '.'

# Test user login
echo -e "\n${YELLOW}3. Testing user login...${NC}"
LOGIN_RESPONSE=$(curl -s -X POST $BASE_URL/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "testpass123"
  }')
echo $LOGIN_RESPONSE | jq '.'

# Extract token for authenticated requests
TOKEN=$(echo $LOGIN_RESPONSE | jq -r '.data.token // empty')

if [ -z "$TOKEN" ] || [ "$TOKEN" = "null" ]; then
    echo -e "${RED}❌ Failed to get authentication token${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Authentication token obtained${NC}"

# Test getting all users
echo -e "\n${YELLOW}4. Testing get all users...${NC}"
curl -s $BASE_URL/users | jq '.'

# Test creating a task
echo -e "\n${YELLOW}5. Testing task creation...${NC}"
TASK_RESPONSE=$(curl -s -X POST $BASE_URL/tasks \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "title": "Test Task",
    "description": "This is a test task created by the API test script"
  }')
echo $TASK_RESPONSE | jq '.'

# Test getting all tasks
echo -e "\n${YELLOW}6. Testing get all tasks...${NC}"
curl -s $BASE_URL/tasks | jq '.'

# Test creating another task
echo -e "\n${YELLOW}7. Creating another task...${NC}"
curl -s -X POST $BASE_URL/tasks \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "title": "Complete API Documentation",
    "description": "Write comprehensive API documentation"
  }' | jq '.'

# Test updating a task (assuming task ID 1 exists)
echo -e "\n${YELLOW}8. Testing task update...${NC}"
curl -s -X PUT $BASE_URL/tasks/1 \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "title": "Updated Test Task",
    "description": "This task has been updated",
    "completed": true
  }' | jq '.'

# Test getting specific task
echo -e "\n${YELLOW}9. Testing get specific task...${NC}"
curl -s $BASE_URL/tasks/1 | jq '.'

# Test getting user tasks (assuming user ID 1)
echo -e "\n${YELLOW}10. Testing get user tasks...${NC}"
curl -s $BASE_URL/users/1/tasks | jq '.'

# Test unauthorized access
echo -e "\n${YELLOW}11. Testing unauthorized access...${NC}"
curl -s -X POST $BASE_URL/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Unauthorized Task",
    "description": "This should fail"
  }' | jq '.'

echo -e "\n${GREEN}🎉 API testing completed!${NC}"
echo -e "${BLUE}Check the responses above for any errors.${NC}"
