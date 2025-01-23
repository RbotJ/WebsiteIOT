//server.js
require('dotenv').config();
const express = require('express');
const cors = require('cors');
const path = require('path');
const { connectDB } = require('./src/config/db');

const app = express();
app.use(cors());
app.use(express.json());

// Log server startup
console.log('🚀 Starting server...');

// Check database connection
connectDB().then(() => console.log('✅ Database connected successfully'))
          .catch(err => console.error('❌ Database connection failed:', err.message));

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

// Start Server (Placing at the end ensures routes are loaded first)
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`✅ Server running on port ${PORT}`));
