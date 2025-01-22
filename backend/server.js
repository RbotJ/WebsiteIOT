require('dotenv').config();
const express = require('express');
const cors = require('cors');
const connectDB = require('./src/config/db');

const app = express();
app.use(cors());
app.use(express.json());

// Import Routes
const authRoutes = require('./src/routes/authRoutes');
const deviceRoutes = require('./src/routes/deviceRoutes');

// Use Routes
app.use('/api/auth', authRoutes);
app.use('/api/devices', deviceRoutes);

// Database Connection
connectDB();

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
