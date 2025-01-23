//monitorBackend.js
const axios = require('axios');

const BACKEND_URL = process.env.BACKEND_URL || "https://websiteit-production.up.railway.app"; // Use env var

const endpoints = [
    { name: "Root API", path: "/", method: "GET" },
    { name: "Authentication - Register", path: "/api/auth/register", method: "POST", body: { email: "test@example.com", password: "password123" } },
    { name: "Authentication - Login", path: "/api/auth/login", method: "POST", body: { email: "test@example.com", password: "password123" } },
    { name: "Get Devices", path: "/api/devices", method: "GET" },
];

async function checkEndpoint(endpoint) {
    try {
        const config = {
            method: endpoint.method,
            url: `${BACKEND_URL}${endpoint.path}`,
            data: endpoint.body || null,
            validateStatus: () => true // Avoid throwing errors on non-200 responses
        };

        const response = await axios(config);
        const timestamp = new Date().toISOString();

        if (response.status >= 200 && response.status < 300) {
            console.log(`✅ [${timestamp}] ${endpoint.name} is working (Status: ${response.status})`);
        } else {
            console.log(`❌ [${timestamp}] ${endpoint.name} failed (Status: ${response.status})`);
        }
    } catch (error) {
        console.log(`⚠️ [${new Date().toISOString()}] Error reaching ${endpoint.name}: ${error.message}`);
    }
}

async function runChecks() {
    console.log("🔍 Running backend health check...");
    for (const endpoint of endpoints) {
        await checkEndpoint(endpoint);
    }
    console.log("✅ Backend monitoring completed.");
}

// Run the script after a short delay to allow Railway to initialize (optional)
setTimeout(runChecks, 5000);
