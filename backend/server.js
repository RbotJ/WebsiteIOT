//server.js
require('dotenv').config();
const express = require('express');
const cors = require('cors');
const path = require('path');
const { connectDB } = require('./src/config/db');

const app = express();
app.use(cors());
app.use(express.json());

const MAX_RETRIES = 5;
const RETRY_DELAY = 5000; // 5 seconds

// Function to attempt DB connection with retry logic
async function initializeDatabase() {
    let retries = 0;
    while (retries < MAX_RETRIES) {
        try {
            console.log(`🔍 Attempting MySQL connection (Attempt ${retries + 1}/${MAX_RETRIES})...`);
            await connectDB();
            console.log('✅ Database connected successfully');
            return true;
        } catch (error) {
            console.error(`❌ Database connection failed: ${error.message}`);
            retries++;
            if (retries < MAX_RETRIES) {
                console.log(`🔄 Retrying in ${RETRY_DELAY / 1000} seconds...`);
                await new Promise(res => setTimeout(res, RETRY_DELAY));
            } else {
                console.error('❌ Maximum retry attempts reached. Exiting.');
                process.exit(1);
            }
        }
    }
}

// Serve Static Files (If Needed)
const frontendPath = path.join(__dirname, 'public');
app.use(express.static(frontendPath));

// Import Routes
const authRoutes = require('./src/routes/authRoutes');
const deviceRoutes = require('./src/routes/deviceRoutes');
const testRoutes = require('./src/routes/testRoutes');

// Use Routes
app.use('/api/auth', authRoutes);
app.use('/api/devices', deviceRoutes);
app.use('/api/test', testRoutes);

// API Route Example
app.get('/api', (req, res) => {
    console.log('✅ API status check hit');
    res.json({ message: 'API is running...' });
});

// Catch-All for Non-API Routes (Only If Not an API Call)
app.get('*', (req, res, next) => {
    if (req.originalUrl.startsWith('/api')) return next(); // Allow API routes to be handled
    res.sendFile(path.join(frontendPath, 'index.html')); // Serve frontend
});

// Initialize Database & Start Server
initializeDatabase().then(() => {
    const PORT = process.env.PORT || 3000;
    app.listen(PORT, () => console.log(`✅ Server running on port ${PORT}`));
}).catch(err => {
    console.error('❌ Server startup failed:', err);
});
