# WebsiteIOT
A website for IOT services


**IoT Platform Architecture Blueprint**

## **Overview**

The IoT platform is a centralized system for managing and monitoring ESP32 IoT devices, hosted on Railway with a **NodeJS backend** and an **Angular frontend**. The platform supports **OTA updates**, real-time monitoring, and device provisioning.

---

## **1. System Architecture**

### **Technology Stack**

- **Frontend:** Angular
- **Backend:** NodeJS (ExpressJS)
- **Database:** MySQL (Railway Managed), Redis (Caching & Session Management)
- **Message Broker:** MQTT (Mosquitto) for real-time device communication
- **Firmware Storage:** Railway Storage
- **Authentication:** JWT-based user authentication
- **Device Communication:** WebSockets + REST API
- **Logging & Monitoring:** Prometheus/Grafana, Logflare (Railway Integration)

---

## **2. Development Roadmap (Step-by-Step)**

This roadmap breaks down the development into **small, manageable tasks** suitable for an intern working 1-2 hours per day.

### **Week 1: Environment Setup & Basic API**

#### **Week 1 Breakdown (Daily Tasks)**

**Day 1: Setting Up *****Development***** Environment (Completed Tasks & Next Steps)**

-

**Day 2: Initialize Backend (NodeJS + ExpressJS) (Completed Tasks)**

- [x] Created a new **NodeJS project** inside Codespaces.
- [x] Installed essential dependencies:
  ```sh
  npm install express dotenv cors pg jsonwebtoken bcryptjs
  ```
- [x] Created `server.js` in the root directory and set up a basic Express server.
- [x] Configured `.env` file for managing environment variables.
- [x] Committed and pushed the backend setup to GitHub for Railway deployment.

-

**Day 3: Database Setup (MySQL on Railway) (Completed Tasks)**

- [x] Created a **MySQL database** on Railway.
- [x] Connected to the database using **pgAdmin** or **DBeaver**.
- [x] Implemented a database connection file (`src/config/db.js`).
- [x] Created initial database tables for `users` and `devices`.
- [x] Added environment variable `DATABASE_URL` in Railway settings.
- [x] Pushed the updates to GitHub and deployed the backend on Railway.

-

**Day 4: Implement Basic API Endpoints (Authentication)**

-

**Day 5: Testing API Locally & Code Documentation**

-

By completing these steps, the backend API will be functional with user authentication, database integration, and JWT-based security, forming the foundation for the rest of the IoT platform.

-

#### **2. Initialize Backend (NodeJS + ExpressJS)**

-

#### **3. Database Setup (MySQL on Railway)**

-

#### **4. Implement Basic API Endpoints (Authentication)**

-

#### **5. Testing API Locally**

-

#### **6. Code Repository & Documentation**

-

By completing these steps, the backend API will be functional with user authentication, database integration, and JWT-based security, forming the foundation for the rest of the IoT platform.

-

### **Week 2: Device Management API**

-

### **Week 3: Frontend UI (Angular)**

-

### **Week 4: MQTT & Real-time Communication**

-

### **Week 5: OTA Update System**

-

### **Week 6: Security & Deployment**

-

---

## **3. API Routes (NodeJS Backend)**

### **User Authentication**

| Endpoint         | Method | Description                       |
| ---------------- | ------ | --------------------------------- |
| `/auth/register` | POST   | Register a new user               |
| `/auth/login`    | POST   | Authenticate and return JWT token |
| `/auth/logout`   | POST   | Logout user                       |
| `/auth/refresh`  | POST   | Refresh JWT token                 |

### **Device Management**

| Endpoint              | Method | Description                   |
| --------------------- | ------ | ----------------------------- |
| `/devices/register`   | POST   | Register a new ESP32 device   |
| `/devices/:id`        | GET    | Get device details            |
| `/devices/:id/status` | GET    | Fetch latest device status    |
| `/devices/:id/update` | POST   | Initiate OTA firmware update  |
| `/devices/:id/delete` | DELETE | Remove device from the system |

### **Firmware Management**

| Endpoint               | Method | Description                          |
| ---------------------- | ------ | ------------------------------------ |
| `/firmware/upload`     | POST   | Upload new firmware version          |
| `/firmware/list`       | GET    | Retrieve available firmware versions |
| `/firmware/:id/delete` | DELETE | Delete firmware version              |

### **Real-time Communication (MQTT Topics)**

| Topic                       | Description                   |
| --------------------------- | ----------------------------- |
| `device/{device_id}/status` | Sends device heartbeat/status |
| `device/{device_id}/ota`    | Triggers OTA firmware update  |
| `device/{device_id}/logs`   | Streams logs from device      |

---

## **4. Database Schema (MySQL)**

### **Users Table**

```sql
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    password TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### **Devices Table**

```sql
REATE TABLE devices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    device_code VARCHAR(50) UNIQUE NOT NULL,
    status TEXT DEFAULT 'offline',
    firmware_version VARCHAR(50),
    last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);
```

### **Firmware Table**

```sql
CREATE TABLE firmware (
    id INT AUTO_INCREMENT PRIMARY KEY,
    version VARCHAR(50) UNIQUE NOT NULL,
    file_url TEXT NOT NULL,
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## **5. Security Considerations**

- **JWT Authentication:** Secure endpoints for users & devices.
- **MQTT ACLs:** Restrict device communication to authorized topics.
- **Firmware Signing:** Ensure firmware authenticity before OTA update.
- **SSL Encryption:** Secure all communications between devices and backend.

---

## **6. Deployment & Scaling**

- **Hosting:** Railway for backend & database, Netlify/Vercel for Angular frontend.
- **Load Balancing:** Use Railway’s auto-scaling features.
- **Monitoring:** Integrate Logflare, Prometheus/Grafana for logging and analytics.

---

## **7. Next Steps**

- Follow the roadmap step by step to build a functional prototype.
- Review progress weekly and refine tasks based on requirements.
- Ensure documentation is updated throughout the development process.

---

This structured plan helps break the project into small steps, making it manageable for an intern. Let me know if you need modifications!

