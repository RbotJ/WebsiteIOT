require('dotenv').config();
const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
app.use(cors());
app.use(express.json());

// Serve Static Files (If Needed)
const frontendPath = path.join(__dirname, 'public');  // Ensure index.html is inside 'public'
app.use(express.static(frontendPath));

// API Route Example
app.get('/api', (req, res) => {
    res.json({ message: 'API is running...' });
});

// Catch-All Route (Avoids Overriding API Routes)
app.get('*', (req, res) => {
    res.sendFile(path.join(frontendPath, 'index.html'));
});

// Start Server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));

//testRoutes
const testRoutes = require('./src/routes/testRoutes');
app.use('/api/test', testRoutes);

